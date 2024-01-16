/**
 * libf2fs_zoned.c
 *
 * Copyright (c) 2016 Western Digital Corporation.
 * Written by: Damien Le Moal <damien.lemoal@wdc.com>
 *
 * Dual licensed under the GPL or LGPL version 2 licenses.
 */
#define _LARGEFILE64_SOURCE

#include <f2fs_fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_SYSMACROS_H
#include <sys/sysmacros.h>
#endif
#ifdef HAVE_LINUX_LIMITS_H
#include <linux/limits.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <libgen.h>

#ifdef HAVE_LINUX_BLKZONED_H

/**
 * 根据设备信息和属性名，
 * 获取设备在sysfs文件系统中的路径，并将其存储在缓冲区中。
 * 可能会在挂载设备的时候用到，也可能会在其他需要访问设备属性的时候用到。
 * sysfs文件系统是一种虚拟的文件系统，它可以显示和修改内核中的设备和驱动信息
*/
int get_sysfs_path(struct device_info *dev, const char *attr,
		   char *buf, size_t buflen)
{
	struct stat statbuf;
	char str[PATH_MAX];
	char sysfs_path[PATH_MAX];
	ssize_t len;
	char *delim;
	int ret;

	if (stat(dev->path, &statbuf) < 0)
		return -1;

	snprintf(str, sizeof(str), "/sys/dev/block/%d:%d",
		 major(statbuf.st_rdev), minor(statbuf.st_rdev));
	len = readlink(str, buf, buflen - 1);
	if (len < 0)
		return -1;
	buf[len] = '\0';

	ret = snprintf(sysfs_path, sizeof(sysfs_path),
		       "/sys/dev/block/%s", buf);
	if (ret >= sizeof(sysfs_path))
		return -1;

	/* Test if the device is a partition */
	ret = snprintf(str, sizeof(str), "%s/partition", sysfs_path);
	if (ret >= sizeof(str))
		return -1;
	ret = stat(str, &statbuf);
	if (ret) {
		if (errno == ENOENT) {
			/* Not a partition */
			goto out;
		}
		return -1;
	}

	/*
	 * The device is a partition: remove the device name from the
	 * attribute file path to obtain the sysfs path of the holder device.
	 *   e.g.:  /sys/dev/block/.../sda/sda1 -> /sys/dev/block/.../sda
	 */
	delim = strrchr(sysfs_path, '/');
	if (!delim)
		return -1;
	*delim = '\0';

out:
	ret = snprintf(buf, buflen, "%s/%s", sysfs_path, attr);
	if (ret >= buflen)
		return -1;

	return 0;
}

/**
 * 检查zone 的分区模式
*/
int f2fs_get_zoned_model(int i)
{
	struct device_info *dev = c.devices + i;
	char str[PATH_MAX];
	FILE *file;
	int res;

	/* Check that this is a zoned block device */
	res = get_sysfs_path(dev, "queue/zoned", str, sizeof(str));
	if (res != 0) {
		MSG(0, "\tInfo: can't find /sys, assuming normal block device\n");
		dev->zoned_model = F2FS_ZONED_NONE;
		return 0;
	}

	file = fopen(str, "r");
	if (!file) {
		/*
		 * The kernel does not support zoned block devices, but we have
		 * a block device file. This means that if the zoned file is
		 * not found, then the device is not zoned or is zoned but can
		 * be randomly written (i.e. host-aware zoned model).
		 * Treat the device as a regular block device. Otherwise, signal
		 * the failure to verify the disk zone model.
		 */
		if (errno == ENOENT) {
			dev->zoned_model = F2FS_ZONED_NONE;
			return 0;
		}
		MSG(0, "\tError: Failed to check the device zoned model\n");
		return -1;
	}

	memset(str, 0, sizeof(str));
	res = fscanf(file, "%s", str);
	fclose(file);

	if (res != 1) {
		MSG(0, "\tError: Failed to parse the device zoned model\n");
		return -1;
	}

	if (strcmp(str, "none") == 0) {
		/* Regular block device */
		dev->zoned_model = F2FS_ZONED_NONE;
	} else if (strcmp(str, "host-aware") == 0) {
		/* Host-aware zoned block device: can be randomly written */
		dev->zoned_model = F2FS_ZONED_HA;
	} else if (strcmp(str, "host-managed") == 0) {
		/* Host-managed zoned block device: sequential writes needed */
		dev->zoned_model = F2FS_ZONED_HM;
	} else {
		MSG(0, "\tError: Unsupported device zoned model\n");
		return -1;
	}

	return 0;
}

//可以得知一个zone是由多少个扇区构成的
uint32_t f2fs_get_zone_chunk_sectors(struct device_info *dev)
{
	uint32_t sectors;
	char str[PATH_MAX];
	FILE *file;
	int res;

	res = get_sysfs_path(dev, "queue/chunk_sectors", str, sizeof(str));
	if (res != 0) {
		MSG(0, "\tError: Failed to get device sysfs attribute path\n");
		return 0;
	}

	file = fopen(str, "r");
	if (!file)
		return 0;

	memset(str, 0, sizeof(str));
	res = fscanf(file, "%s", str);
	fclose(file);

	if (res != 1)
		return 0;

	sectors = atoi(str);

	return sectors;
}


int f2fs_get_zone_blocks(int i)
{
	struct device_info *dev = c.devices + i;
	uint64_t sectors;

	/* Get zone size */
	dev->zone_blocks = 0;

	sectors = f2fs_get_zone_chunk_sectors(dev);
	if (!sectors)
		return -1;

	dev->zone_size = sectors << SECTOR_SHIFT; //单位B
	dev->zone_blocks = sectors >> (F2FS_BLKSIZE_BITS - SECTOR_SHIFT);//单位是4KB
	sectors = dev->zone_size / c.sector_size;

	/*
	 * Total number of zones: there may
	 * be a last smaller runt zone.
	 */
	dev->nr_zones = dev->total_sectors / sectors;
	if (dev->total_sectors % sectors)
		dev->nr_zones++;

	return 0;
}
/**
 * 报告一个zone的信息
*/
int f2fs_report_zone(int i, uint64_t sector, struct blk_zone *blkzone)
{
	struct one_zone_report {
		struct blk_zone_report	rep;
		struct blk_zone		zone;
	} *rep;
	int ret = -1;

	static_assert(sizeof(*rep) == sizeof(rep->rep) + sizeof(rep->zone), "");

	rep = calloc(1, sizeof(*rep));
	if (!rep) {
		ERR_MSG("No memory for report zones\n");
		return -ENOMEM;
	}

	rep->rep = (struct blk_zone_report){
		.sector = sector,
		.nr_zones = 1,
	};
	ret = ioctl(c.devices[i].fd, BLKREPORTZONE, rep);
	if (ret != 0) {
		ret = -errno;
		ERR_MSG("ioctl BLKREPORTZONE failed: errno=%d\n", errno);
		goto out;
	}

	*blkzone = rep->zone;
out:
	free(rep);
	return ret;
}


#define F2FS_REPORT_ZONES_BUFSZ	524288   //分区报告的缓冲区大小  单位字节

int f2fs_report_zones(int j, report_zones_cb_t *report_zones_cb, void *opaque)
{
	struct device_info *dev = c.devices + j;
	struct blk_zone_report *rep;
	struct blk_zone *blkz;
	unsigned int i, n = 0;
	uint64_t total_sectors = (dev->total_sectors * c.sector_size)
		>> SECTOR_SHIFT;
	uint64_t sector = 0;
	int ret = -1;

	/**
	 * 定义一个名为rep的指针变量，指向一个blk_zone_report类型的结构体，
	 * 用于存储分区报告的参数，包括扇区号和分区数。
	 * 然后，使用malloc函数，分配一块大小为F2FS_REPORT_ZONES_BUFSZ的内存空间，
	 * 并将其赋值给rep指针。
	*/
	rep = malloc(F2FS_REPORT_ZONES_BUFSZ);
	if (!rep) {
		ERR_MSG("No memory for report zones\n");
		return -ENOMEM;
	}

	//从设备的第一个扇区开始，遍历设备的所有扇区
	while (sector < total_sectors) {

		/* Get zone info */
		//给rep指针指向的结构体的sector成员变量赋值，将其设为当前的扇区号
		rep->sector = sector;
		//根据缓冲区大小计算一次可以获取的分区数
		rep->nr_zones = (F2FS_REPORT_ZONES_BUFSZ - sizeof(struct blk_zone_report))
			/ sizeof(struct blk_zone);

		/**
		 * 使用ioctl函数，向设备文件描述符c.devices[j].fd发送BLKREPORTZONE命令，
		 * 传递rep指针作为参数，用于获取设备的分区信息。
		*/
		ret = ioctl(dev->fd, BLKREPORTZONE, rep);
		if (ret != 0) {
			ret = -errno;
			ERR_MSG("ioctl BLKREPORTZONE failed: errno=%d\n",
				errno);
			goto out;
		}

		if (!rep->nr_zones) {
			ret = -EIO;
			ERR_MSG("Unexpected ioctl BLKREPORTZONE result\n");
			goto out;
		}
		/**
		 * 如果获取到了分区信息，
		 * 那么定义一个名为blkz的指针变量，指向一个blk_zone类型的结构体，用于存储分区的详细信息，
		 * 包括类型，状态，容量，写入指针等。
		 * 然后，将rep指针加一，得到一个指向分区信息数组的指针，并将其赋值给blkz指针。
		*/
		blkz = (struct blk_zone *)(rep + 1);
		for (i = 0; i < rep->nr_zones; i++) {
			/**
			 * 调用分区报告回调函数report_zones_cb，传递分区序号n，分区信息结构体blkz，
			*/
			ret = report_zones_cb(n, blkz, opaque);
			if (ret)
				goto out;
			sector = blk_zone_sector(blkz) + blk_zone_length(blkz);
			n++;
			blkz++;
		}
	}
out:
	free(rep);
	return ret;
}

/**
 * 检查设备的分区状态，并统计分区的容量和类型
*/
int f2fs_check_zones(int j)
{
	struct device_info *dev = c.devices + j;
	struct blk_zone_report *rep;
	struct blk_zone *blkz;
	unsigned int i, n = 0;
	uint64_t total_sectors;
	uint64_t sector;
	int last_is_conv = 1;
	int ret = -1;

	rep = malloc(F2FS_REPORT_ZONES_BUFSZ);
	if (!rep) {
		ERR_MSG("No memory for report zones\n");
		return -ENOMEM;
	}
	//zone包含的block数  是个数组 说明每个zone可以包含不同数量的blk
	dev->zone_cap_blocks = malloc(dev->nr_zones * sizeof(size_t));
	if (!dev->zone_cap_blocks) {
		ERR_MSG("No memory for zone capacity list.\n");
		return -ENOMEM;
	}
	memset(dev->zone_cap_blocks, 0, (dev->nr_zones * sizeof(size_t)));

	dev->nr_rnd_zones = 0;
	sector = 0;
	total_sectors = (dev->total_sectors * c.sector_size) >> 9;

	while (sector < total_sectors) {

		/* Get zone info */
		memset(rep, 0, F2FS_REPORT_ZONES_BUFSZ);
		rep->sector = sector;
		rep->nr_zones = (F2FS_REPORT_ZONES_BUFSZ - sizeof(struct blk_zone_report))
			/ sizeof(struct blk_zone);

		ret = ioctl(dev->fd, BLKREPORTZONE, rep);
		if (ret != 0) {
			ret = -errno;
			ERR_MSG("ioctl BLKREPORTZONE failed\n");
			goto out;
		}

		if (!rep->nr_zones)
			break;

		blkz = (struct blk_zone *)(rep + 1);
		for (i = 0; i < rep->nr_zones && sector < total_sectors; i++) {

			if (blk_zone_cond(blkz) == BLK_ZONE_COND_READONLY ||
			    blk_zone_cond(blkz) == BLK_ZONE_COND_OFFLINE)
				last_is_conv = 0;
			if (blk_zone_conv(blkz) ||
			    blk_zone_seq_pref(blkz)) {
				if (last_is_conv)
					dev->nr_rnd_zones++;
			} else {
				last_is_conv = 0;
			}

			if (blk_zone_conv(blkz)) {
				DBG(2,
				    "Zone %05u: Conventional, cond 0x%x (%s), sector %llu, %llu sectors\n",
				    n,
				    blk_zone_cond(blkz),
				    blk_zone_cond_str(blkz),
				    blk_zone_sector(blkz),
				    blk_zone_length(blkz));
				dev->zone_cap_blocks[n] =
					blk_zone_length(blkz) >>
					(F2FS_BLKSIZE_BITS - SECTOR_SHIFT);
			} else {
				DBG(2,
				    "Zone %05u: type 0x%x (%s), cond 0x%x (%s),"
				    " need_reset %d, non_seq %d, sector %llu,"
				    " %llu sectors, capacity %llu,"
				    " wp sector %llu\n",
				    n,
				    blk_zone_type(blkz),
				    blk_zone_type_str(blkz),
				    blk_zone_cond(blkz),
				    blk_zone_cond_str(blkz),
				    blk_zone_need_reset(blkz),
				    blk_zone_non_seq(blkz),
				    blk_zone_sector(blkz),
				    blk_zone_length(blkz),
				    blk_zone_capacity(blkz, rep->flags),
				    blk_zone_wp_sector(blkz));
				dev->zone_cap_blocks[n] =
					blk_zone_capacity(blkz, rep->flags) >>
					(F2FS_BLKSIZE_BITS - SECTOR_SHIFT);
			}

			sector = blk_zone_sector(blkz) + blk_zone_length(blkz);
			n++;
			blkz++;
		}
	}
	// //flexzns fixed
	// blkz = (struct blk_zone *)(rep + 1);
	// if (dev->zone_size != blk_zone_length(blkz) << SECTOR_SHIFT) {
	// 		dev->zone_size = blk_zone_length(blkz) << SECTOR_SHIFT;
	// 		dev->zone_blocks = dev->zone_size >> F2FS_BLKSIZE_BITS;
	// 		dev->nr_zones = n;
	// }


	if (sector != total_sectors) {
		ERR_MSG("Invalid zones: last sector reported is %llu, expected %llu\n",
			(unsigned long long)(sector << 9) / c.sector_size,
			(unsigned long long)dev->total_sectors);
		ret = -1;
		goto out;
	}

	if (n != dev->nr_zones) {
		ERR_MSG("Inconsistent number of zones: expected %u zones, got %u\n",
			dev->nr_zones, n);
		ret = -1;
		goto out;
	}

	/*
	 * For a multi-device volume, fixed position metadata blocks are
	 * stored * only on the first device of the volume. Checking for the
	 * presence of * conventional zones (randomly writeabl zones) for
	 * storing these blocks * on a host-managed device is thus needed only
	 * for the device index 0.
	 */
	if (j == 0 && dev->zoned_model == F2FS_ZONED_HM &&
			!dev->nr_rnd_zones) {
		ERR_MSG("No conventional zone for super block\n");
		ret = -1;
	}
out:
	free(rep);
	return ret;
}

int f2fs_reset_zone(int i, void *blkzone)
{
	struct blk_zone *blkz = (struct blk_zone *)blkzone;
	struct device_info *dev = c.devices + i;
	struct blk_zone_range range;
	int ret;

	if (!blk_zone_seq(blkz) || blk_zone_empty(blkz))
		return 0;

	/* Non empty sequential zone: reset */
	range.sector = blk_zone_sector(blkz);
	range.nr_sectors = blk_zone_length(blkz);
	//就是通过ioctl 来管理zone
	ret = ioctl(dev->fd, BLKRESETZONE, &range);
	if (ret != 0) {
		ret = -errno;
		ERR_MSG("ioctl BLKRESETZONE failed: errno=%d\n", errno);
	}

	return ret;
}

int f2fs_reset_zones(int j)
{
	struct device_info *dev = c.devices + j;
	struct blk_zone_report *rep;
	struct blk_zone *blkz;
	struct blk_zone_range range;
	uint64_t total_sectors;
	uint64_t sector;
	unsigned int i;
	int ret = -1;

	rep = malloc(F2FS_REPORT_ZONES_BUFSZ);
	if (!rep) {
		ERR_MSG("No memory for report zones\n");
		return -1;
	}

	sector = 0;
	total_sectors = (dev->total_sectors * c.sector_size) >> 9;
	while (sector < total_sectors) {

		/* Get zone info */
		memset(rep, 0, F2FS_REPORT_ZONES_BUFSZ);
		rep->sector = sector;
		rep->nr_zones = (F2FS_REPORT_ZONES_BUFSZ - sizeof(struct blk_zone_report))
			/ sizeof(struct blk_zone);

		ret = ioctl(dev->fd, BLKREPORTZONE, rep);
		if (ret != 0) {
			ret = -errno;
			ERR_MSG("ioctl BLKREPORTZONES failed\n");
			goto out;
		}

		if (!rep->nr_zones)
			break;

		blkz = (struct blk_zone *)(rep + 1);
		for (i = 0; i < rep->nr_zones && sector < total_sectors; i++) {
			if (blk_zone_seq(blkz) &&
			    !blk_zone_empty(blkz)) {
				/* Non empty sequential zone: reset */
				range.sector = blk_zone_sector(blkz);
				range.nr_sectors = blk_zone_length(blkz);
				ret = ioctl(dev->fd, BLKRESETZONE, &range);
				if (ret != 0) {
					ret = -errno;
					ERR_MSG("ioctl BLKRESETZONE failed\n");
					goto out;
				}
			}
			sector = blk_zone_sector(blkz) + blk_zone_length(blkz);
			blkz++;
		}
	}
out:
	free(rep);
	if (!ret)
		MSG(0, "Info: Discarded %"PRIu64" MB\n", (sector << 9) >> 20);
	return ret;
}

uint32_t f2fs_get_usable_segments(struct f2fs_super_block *sb)
{
#ifdef HAVE_BLK_ZONE_REP_V2
	int i, j;
	uint32_t usable_segs = 0, zone_segs;

	if (c.func == RESIZE)
		return get_sb(segment_count_main);

	for (i = 0; i < c.ndevs; i++) {
		if (c.devices[i].zoned_model != F2FS_ZONED_HM) {
			usable_segs += c.devices[i].total_segments;
			continue;
		}
		for (j = 0; j < c.devices[i].nr_zones; j++) {
			zone_segs = c.devices[i].zone_cap_blocks[j] >>
					get_sb(log_blocks_per_seg);
			if (c.devices[i].zone_cap_blocks[j] %
						DEFAULT_BLOCKS_PER_SEGMENT)
				usable_segs += zone_segs + 1;
			else
				usable_segs += zone_segs;
		}
	}
	usable_segs -= (get_sb(main_blkaddr) - get_sb(segment0_blkaddr)) >>
						get_sb(log_blocks_per_seg);
	return usable_segs;
#endif
	return get_sb(segment_count_main);
}

#else

int f2fs_report_zone(int i, uint64_t UNUSED(sector),
		     struct blk_zone *UNUSED(blkzone))
{
	ERR_MSG("%d: Unsupported zoned block device\n", i);
	return -1;
}

int f2fs_report_zones(int i, report_zones_cb_t *UNUSED(report_zones_cb),
					void *UNUSED(opaque))
{
	ERR_MSG("%d: Unsupported zoned block device\n", i);
	return -1;
}

int f2fs_get_zoned_model(int i)
{
	struct device_info *dev = c.devices + i;

	c.zoned_mode = 0;
	dev->zoned_model = F2FS_ZONED_NONE;
	return 0;
}

int f2fs_get_zone_blocks(int i)
{
	struct device_info *dev = c.devices + i;

	c.zoned_mode = 0;
	dev->nr_zones = 0;
	dev->zone_blocks = 0;
	dev->zoned_model = F2FS_ZONED_NONE;

	return 0;
}

int f2fs_check_zones(int i)
{
	ERR_MSG("%d: Unsupported zoned block device\n", i);
	return -1;
}

int f2fs_reset_zone(int i, void *UNUSED(blkzone))
{
	ERR_MSG("%d: Unsupported zoned block device\n", i);
	return -1;
}

int f2fs_reset_zones(int i)
{
	ERR_MSG("%d: Unsupported zoned block device\n", i);
	return -1;
}

uint32_t f2fs_get_usable_segments(struct f2fs_super_block *sb)
{
	return get_sb(segment_count_main);
}
#endif

