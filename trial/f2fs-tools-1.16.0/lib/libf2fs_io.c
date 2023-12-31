/**
 * libf2fs.c
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 * Copyright (c) 2019 Google Inc.
 *             http://www.google.com/
 * Copyright (c) 2020 Google Inc.
 *   Robin Hsu <robinhsu@google.com>
 *  : add quick-buffer for sload compression support
 *
 * Dual licensed under the GPL or LGPL version 2 licenses.
 */
#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif
#include <time.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_LINUX_HDREG_H
#include <linux/hdreg.h>
#endif

#include <stdbool.h>
#include <assert.h>
#include <inttypes.h>
#include "f2fs_fs.h"

struct f2fs_configuration c;

#ifdef HAVE_SPARSE_SPARSE_H
#include <sparse/sparse.h>
struct sparse_file *f2fs_sparse_file;//sparse_file类型的结构体，sparse_file是sparse库中定义的一个结构体，用于表示一个稀疏文件的信息，如文件名，文件大小，文件描述符，块大小，块数等3。f2fs_sparse_file可能是用于存储F2FS文件系统的稀疏文件的信息，F2FS是一种专门为闪存设备设计的文件系统
static char **blocks;     //指向一个字符指针的数组，用于存储稀疏文件的块的数据。每个字符指针指向一个块的数据
uint64_t blocks_count;    //无符号整数的声明，用于存储稀疏文件的块的数量
static char *zeroed_block;//指向一个全零的块的数据，用于在读取稀疏文件时，将没有数据的块填充为零
#endif

//根据数据偏移量找到对应的文件描述符 而且会将offset转变为该设备上的相对偏移量
static int __get_device_fd(__u64 *offset)
{
	__u64 blk_addr = *offset >> F2FS_BLKSIZE_BITS;
	int i;

	for (i = 0; i < c.ndevs; i++) {
		if (c.devices[i].start_blkaddr <= blk_addr &&
				c.devices[i].end_blkaddr >= blk_addr) {
			*offset -=
				c.devices[i].start_blkaddr << F2FS_BLKSIZE_BITS;
			return c.devices[i].fd;
		}
	}
	return -1;
}

#ifndef HAVE_LSEEK64
typedef off_t	off64_t;

static inline off64_t lseek64(int fd, __u64 offset, int set)
{
	return lseek(fd, offset, set);
}
#endif

/* ---------- dev_cache, Least Used First (LUF) policy  ------------------- */
/*
 * Least used block will be the first victim to be replaced when max hash
 * collision exceeds
 */
/**
 * 用于实现一个简单的数据块缓存机制，以提高f2fs的读取性能
*/
static bool *dcache_valid;        /* is the cached block valid? 用于标记缓存中的每个数据块是否有效*/
static off64_t  *dcache_blk;      /* which block it cached 用于记录缓存中的每个数据块对应的原始块号*/
static uint64_t *dcache_lastused; /* last used ticks for cache entries 用于记录缓存中的每个数据块最后一次被使用的时刻*/
static char *dcache_buf;          /* cached block data 用于存储缓存中的数据块内容*/
static uint64_t dcache_usetick;   /* current use tick 用于记录当前的使用时刻，每次访问缓存时递增*/

static uint64_t dcache_raccess; //一个64位整数，用于统计缓存的总读取次数
static uint64_t dcache_rhit;    //一个64位整数，用于统计缓存的命中次数
static uint64_t dcache_rmiss;   //一个64位整数，用于统计缓存的未命中次数
static uint64_t dcache_rreplace;//一个64位整数，用于统计缓存的替换次数

/**
 * 用于在程序退出时释放缓存的内存和统计信息的。
 * 如果这个变量为false，表示缓存还没有注册退出函数，需要调用atexit函数来注册。
 * 如果这个变量为true，表示缓存已经注册了退出函数，无需再次注册。
*/
static bool dcache_exit_registered = false;

/*
 *  Shadow config:
 *
 *  Active set of the configurations.
 *  Global configuration 'dcache_config' will be transferred here when
 *  when dcache_init() is called
 *  用于实现一个基于设备缓存（dcache）的数据块重定位机制，以提高f2fs的写入性能
 */
static dev_cache_config_t dcache_config = {0, 16, 1};//用于存储设备缓存的配置参数，包括缓存大小、哈希表大小和重定位次数
static bool dcache_initialized = false;//用于标记设备缓存是否已经初始化，初始值为false

#define MIN_NUM_CACHE_ENTRY  1024L  //用于定义设备缓存的最小条目数，为1024
#define MAX_MAX_HASH_COLLISION  16  //用于定义设备缓存的最大哈希冲突数，为16

//一个长整型的静态数组，用于存储设备缓存的重定位偏移量，共有16个元素，每个元素的值为20的正负倍数
static long dcache_relocate_offset0[] = {
	20, -20, 40, -40, 80, -80, 160, -160,
	320, -320, 640, -640, 1280, -1280, 2560, -2560,
};

//一个整型的静态数组，用于存储设备缓存的重定位偏移量的索引，共有16个元素，每个元素的值为0到15的随机排列
static int dcache_relocate_offset[16];

//表示打印设备缓存的统计信息
static void dcache_print_statistics(void)
{
	long i;
	long useCnt;       //已使用的缓存条目数

	/* Number of used cache entries */
	useCnt = 0;
	for (i = 0; i < dcache_config.num_cache_entry; i++)
		if (dcache_valid[i])
			++useCnt;

	/*
	 *  c: number of cache entries
	 *  u: used entries
	 *  RA: number of read access blocks
	 *  CH: cache hit
	 *  CM: cache miss
	 *  Repl: read cache replaced
	 */
	printf ("\nc, u, RA, CH, CM, Repl=\n");
	printf ("%ld %ld %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
			dcache_config.num_cache_entry,
			useCnt,
			dcache_raccess,
			dcache_rhit,
			dcache_rmiss,
			dcache_rreplace);
}

//表示释放设备缓存
void dcache_release(void)
{
	/**
	 * 用于标记设备缓存是否已经初始化。
	 * 如果该变量为false，表示设备缓存未初始化，直接返回，无需释放。
	*/
	if (!dcache_initialized)
		return;

	/**
	 * 如果该变量为true，表示设备缓存已初始化，将其设为false，
	 * 然后判断c.cache_config.dbg_en变量，该变量是一个整型变量，用于控制设备缓存的调试开关。
	 * 如果该变量为真，表示开启了调试模式，调用dcache_print_statistics函数，该函数用于打印设备缓存的统计信息，如缓存条目数，缓存命中率，缓存替换次数等。
	*/
	dcache_initialized = false;

	if (c.cache_config.dbg_en)
		dcache_print_statistics();

	/**
	 * 这些变量分别指向设备缓存中的：
	 * 数据块号数组，
	 * 最后使用时刻数组，
	 * 数据块内容数组，
	 * 数据块有效性数组。
	 * 如果这些指针不为空，表示已经分配了内存，调用free函数释放这些内存。
	*/
	if (dcache_blk != NULL)
		free(dcache_blk);
	if (dcache_lastused != NULL)
		free(dcache_lastused);
	if (dcache_buf != NULL)
		free(dcache_buf);
	if (dcache_valid != NULL)
		free(dcache_valid);
	/**
	 * 函数将dcache_config.num_cache_entry变量设为0，表示设备缓存的条目数为0，
	 * 将dcache_blk，dcache_lastused，dcache_buf，dcache_valid四个指针变量设为NULL，表示设备缓存已经清空
	*/
	dcache_config.num_cache_entry = 0;
	dcache_blk = NULL;
	dcache_lastused = NULL;
	dcache_buf = NULL;
	dcache_valid = NULL;
}

// return 0 for success, error code for failure.
/**
 * 表示为设备缓存分配所有资源，参数n表示要分配的缓存条目数。
*/
static int dcache_alloc_all(long n)
{
	if (n <= 0)
		return -1;
	if ((dcache_blk = (off64_t *) malloc(sizeof(off64_t) * n)) == NULL
		|| (dcache_lastused = (uint64_t *)
				malloc(sizeof(uint64_t) * n)) == NULL
		|| (dcache_buf = (char *) malloc (F2FS_BLKSIZE * n)) == NULL
		|| (dcache_valid = (bool *) malloc(sizeof(bool) * n)) == NULL)
	{
		dcache_release();
		return -1;
	}
	dcache_config.num_cache_entry = n;
	return 0;
}
/**
 * 表示初始化设备缓存的重定位偏移量
*/
static void dcache_relocate_init(void)
{
	int i;
	int n0 = (sizeof(dcache_relocate_offset0)
			/ sizeof(dcache_relocate_offset0[0]));
	int n = (sizeof(dcache_relocate_offset)
			/ sizeof(dcache_relocate_offset[0]));

	ASSERT(n == n0);
	for (i = 0; i < n && i < dcache_config.max_hash_collision; i++) {
		/**
		 * 函数首先使用labs函数计算dcache_relocate_offset0数组中第i个元素的绝对值，
		 * 并判断是否大于dcache_config.num_cache_entry / 2。
		 * dcache_config.num_cache_entry表示设备缓存的条目数，即缓存的大小。
		 * labs函数用于返回一个长整型数的绝对值。
		 * 
		 * 如果绝对值大于缓存条目数的一半，表示该重定位偏移量过大，可能导致缓存溢出，
		 * 函数将dcache_config.max_hash_collision设为i，并跳出循环。
		 * 
		 * 如果绝对值小于等于缓存条目数的一半，表示该重定位偏移量合理，
		 * 函数将dcache_relocate_offset数组中第i个元素设为：
		 * 		dcache_config.num_cache_entry + dcache_relocate_offset0[i]，
		 * 即在预设的偏移量的基础上，加上缓存的大小，从而得到一个随机的偏移量
		*/
		if (labs(dcache_relocate_offset0[i])
				> dcache_config.num_cache_entry / 2) {
			dcache_config.max_hash_collision = i;
			break;
		}
		dcache_relocate_offset[i] =
				dcache_config.num_cache_entry
				+ dcache_relocate_offset0[i];
	}
}


//初始化设备缓存
void dcache_init(void)
{
	long n;

	if (c.cache_config.num_cache_entry <= 0)
		return;

	/* release previous cache init, if any */
	dcache_release();

	dcache_blk = NULL;
	dcache_lastused = NULL;
	dcache_buf = NULL;
	dcache_valid = NULL;

	dcache_config = c.cache_config;

	n = max(MIN_NUM_CACHE_ENTRY, dcache_config.num_cache_entry);

	/* halve alloc size until alloc succeed, or min cache reached 分配资源 */
	while (dcache_alloc_all(n) != 0 && n !=  MIN_NUM_CACHE_ENTRY)
		n = max(MIN_NUM_CACHE_ENTRY, n/2);

	/* must be the last: data dependent on num_cache_entry  重定向初始化*/
	dcache_relocate_init();
	dcache_initialized = true;

	if (!dcache_exit_registered) {
		dcache_exit_registered = true;
		atexit(dcache_release); /* auto release */
	}
	//初始化统计信息
	dcache_raccess = 0;
	dcache_rhit = 0;
	dcache_rmiss = 0;
	dcache_rreplace = 0;
}

//定位一个条目 说明一个条目的单位就是blk
static inline char *dcache_addr(long entry)
{
	return dcache_buf + F2FS_BLKSIZE * entry;
}

/* relocate on (n+1)-th collision 重定向定位 */
static inline long dcache_relocate(long entry, int n)
{
	assert(dcache_config.num_cache_entry != 0);
	return (entry + dcache_relocate_offset[n]) %
			dcache_config.num_cache_entry;
}
/**
 * 设备缓存中查找，参数blk表示要查找的数据块号，返回值是一个长整型数，表示数据块在缓存中的索引
*/
static long dcache_find(off64_t blk)
{
	register long n = dcache_config.num_cache_entry;
	register unsigned m = dcache_config.max_hash_collision;
	long entry, least_used, target;
	unsigned try;
	//如果缓存为空的话将会终止
	assert(n > 0);

	/**
	 * 接着将target，least_used，entry都设为blk对n取模的结果，
	 * 即简单的模运算哈希，这是一种常用的哈希函数，用于将一个数据块号映射到一个缓存索引。
	*/
	target = least_used = entry = blk % n; /* simple modulo hash */

	/**
	 * 从0开始，每次递增try，直到try达到m或者找到目标数据块
	*/
	for (try = 0; try < m; try++) {
		//缓存有效且找到
		if (!dcache_valid[target] || dcache_blk[target] == blk)
			return target;  /* found target or empty cache slot */
		/**
		 * 如果上述条件都不满足，表示缓存中存在哈希冲突，需要使用重定位偏移量来解决冲突
		 * 当前的目标缓存条目比最近最少使用的缓存条目更少使用，函数将least_used设为target，更新最近最少使用的缓存索引
		*/
		if (dcache_lastused[target] < dcache_lastused[least_used])
			least_used = target;
		//接着调用dcache_relocate函数，传入entry和try两个参数，该函数用于根据重定位偏移量计算下一个目标缓存索引
		target = dcache_relocate(entry, try); /* next target */
	}
	/*表示已经达到了最大的查找次数，仍然没有找到目标数据块或者空的缓存条目，函数返回least_used，即最近最少使用的缓存索引，表示需要替换该缓存条目*/
	return least_used;  /* max search reached, return least used slot */
}

/* Physical read into cache 从设备读取到缓存*/
/**
 * fd表示设备的文件描述符，用于访问设备
 * entry：一个长整型变量，表示缓存中的索引，用于确定缓存的位置
 * offset：一个64位的偏移量，表示设备上的位置，用于确定数据块的位置
 * blk：一个64位的数据块号，表示数据块的编号，用于记录数据块的原始位置。
*/
static int dcache_io_read(int fd, long entry, off64_t offset, off64_t blk)
{
	/**
	 * 将设备的文件指针移动到offset指定的位置，
	 * 该函数是一个C语言的标准库函数，用于定位文件的读写位置
	*/
	if (lseek64(fd, offset, SEEK_SET) < 0) {
		MSG(0, "\n lseek64 fail.\n");
		return -1;
	}
	/**
	 * 使用read函数，从设备上读取F2FS_BLKSIZE个字节的数据，写入到dcache_buf数组中，
	 * 该数组是一个字符型数组，用于存储缓存中的数据块内容，
	 * entry * F2FS_BLKSIZE表示缓存中的偏移量，
	 * F2FS_BLKSIZE是一个宏，用于定义f2fs文件系统的数据块大小，为4KB
	*/
	if (read(fd, dcache_buf + entry * F2FS_BLKSIZE, F2FS_BLKSIZE) < 0) {
		MSG(0, "\n read() fail.\n");
		return -1;
	}
	dcache_lastused[entry] = ++dcache_usetick;
	dcache_valid[entry] = true;
	dcache_blk[entry] = blk;
	return 0;
}

/*
 *  - Note: Read/Write are not symmetric:
 *       For read, we need to do it block by block, due to the cache nature:
 *           some blocks may be cached, and others don't.
 *       For write, since we always do a write-thru, we can join all writes into one,
 *       and write it once at the caller.  This function updates the cache for write, but
 *       not the do a physical write.  The caller is responsible for the physical write.
 *  - Note: We concentrate read/write together, due to the fact of similar structure to find
 *          the relavant cache entries
 *  - Return values:
 *       0: success
 *       1: cache not available (uninitialized)
 *      -1: error
 * 
 * 在缓存中更新读写
 * fd：一个整型变量，表示设备的文件描述符，用于访问设备。
 * buf：一个指针变量，表示要读写的数据缓冲区，用于存储数据。
 * offset：一个64位的偏移量，表示设备上的位置，用于确定数据块的位置。
 * byte_count：一个无符号整型变量，表示要读写的数据字节数，用于控制读写的范围。
 * is_write：一个布尔型变量，表示是否是写操作，用于区分读写的行为。
 * 
 * 更新缓存：
 * 如果是写的话：
 * 缓存命中就更新缓存条目
 * 未命中的话啥也不做
 * 
 * 如果是读的话：
 * 缓存命中就直接读
 * 未命中就从物理盘读到缓存 再读
 */
static int dcache_update_rw(int fd, void *buf, off64_t offset,
		size_t byte_count, bool is_write)
{
	off64_t blk;
	int addr_in_blk;
	off64_t start;

	if (!dcache_initialized)
		dcache_init(); /* auto initialize */

	if (!dcache_initialized)
		return 1; /* not available */

	blk = offset / F2FS_BLKSIZE;         //起始块号
	addr_in_blk = offset % F2FS_BLKSIZE; //块内偏移
	start = blk * F2FS_BLKSIZE;          //块的起始地址

	while (byte_count != 0) {
		/**
		 * 计算当前块中需要读写的字节数（cur_size），
		 * 取 byte_count 和块大小（F2FS_BLKSIZE）减去块内偏移量（addr_in_blk）的较小值
		*/
		size_t cur_size = min(byte_count,
				(size_t)(F2FS_BLKSIZE - addr_in_blk));
		/**
		 * 调用 dcache_find 函数，根据块号（blk）找到对应的缓存条目（entry）
		*/
		long entry = dcache_find(blk);

		if (!is_write)
			++dcache_raccess;

		//缓存命中
		if (dcache_valid[entry] && dcache_blk[entry] == blk) {
			/* cache hit 写的话就是更新缓存*/
			if (is_write)  /* write: update cache */
				memcpy(dcache_addr(entry) + addr_in_blk,
					buf, cur_size);
			else
				++dcache_rhit;
		} 
		else //缓存未命中
		{
			/* cache miss */
			if (!is_write) {
				int err;
				++dcache_rmiss;
				if (dcache_valid[entry])
					++dcache_rreplace;
				/* read: physical I/O read into cache */
				err = dcache_io_read(fd, entry, start, blk);
				if (err)
					return err;
			}
		}

		/* read: copy data from cache */
		/* write: nothing to do, since we don't do physical write. */
		if (!is_write)
			memcpy(buf, dcache_addr(entry) + addr_in_blk,
				cur_size);

		/* next block */
		++blk;
		buf += cur_size;
		start += F2FS_BLKSIZE;
		byte_count -= cur_size;
		addr_in_blk = 0;
	}
	return 0;
}

/*
 * dcache_update_cache() just update cache, won't do physical I/O.
 * Thus even no error, we need normal non-cache I/O for actual write
 *
 * return value: 1: cache not available
 *               0: success, -1: I/O error
 */
int dcache_update_cache(int fd, void *buf, off64_t offset, size_t count)
{
	return dcache_update_rw(fd, buf, offset, count, true);
}

/* handles read into cache + read into buffer  */
int dcache_read(int fd, void *buf, off64_t offset, size_t count)
{
	return dcache_update_rw(fd, buf, offset, count, false);
}

/*
 * IO interfaces 
 从一个设备（c.kd）中读取一定长度（len）的版本信息（buf），并从一个偏移量（offset）开始读取
 */
int dev_read_version(void *buf, __u64 offset, size_t len)
{
	if (c.sparse_mode)
		return 0;
	if (lseek64(c.kd, (off64_t)offset, SEEK_SET) < 0)
		return -1;
	if (read(c.kd, buf, len) < 0)
		return -1;
	return 0;
}

/**
 * 检测是否安装了sparse库，这是一个用于处理稀疏文件的库。
 * 稀疏文件是一种不占用所有分配空间的文件，而是只存储非零数据的文件。
 * 如果安装了sparse库，就可以使用它提供的函数和结构体来操作稀疏文件。
 * 如果没有安装，就需要使用其他的方法或者忽略稀疏文件的特性
*/
#ifdef HAVE_SPARSE_SPARSE_H

/**
 * 从一个稀疏文件（blocks）中读取一定数量（count）的块（block），并将读取到的数据存储到一个缓冲区（buf）中。
 * 函数的参数和返回值的类型如下：
 * block: __u64，表示一个64位的无符号整数，用于指定读取的起始块号。
 * count: int，表示一个整数，用于指定读取的块数。
 * buf: void *，表示一个指向任意类型的指针，用于存储读取到的数据。
*/
static int sparse_read_blk(__u64 block, int count, void *buf)
{
	int i;
	char *out = buf;
	__u64 cur_block;

	for (i = 0; i < count; ++i) {
		cur_block = block + i;
		if (blocks[cur_block])
			memcpy(out + (i * F2FS_BLKSIZE),
					blocks[cur_block], F2FS_BLKSIZE);
		else if (blocks)
			memset(out + (i * F2FS_BLKSIZE), 0, F2FS_BLKSIZE);
	}
	return 0;
}

static int sparse_write_blk(__u64 block, int count, const void *buf)
{
	int i;
	__u64 cur_block;
	const char *in = buf;

	for (i = 0; i < count; ++i) {
		cur_block = block + i;
		if (blocks[cur_block] == zeroed_block)
			blocks[cur_block] = NULL;
		if (!blocks[cur_block]) {
			blocks[cur_block] = calloc(1, F2FS_BLKSIZE);
			if (!blocks[cur_block])
				return -ENOMEM;
		}
		memcpy(blocks[cur_block], in + (i * F2FS_BLKSIZE),
				F2FS_BLKSIZE);
	}
	return 0;
}

static int sparse_write_zeroed_blk(__u64 block, int count)
{
	int i;
	__u64 cur_block;

	for (i = 0; i < count; ++i) {
		cur_block = block + i;
		if (blocks[cur_block])
			continue;
		blocks[cur_block] = zeroed_block;
	}
	return 0;
}

#ifdef SPARSE_CALLBACK_USES_SIZE_T
static int sparse_import_segment(void *UNUSED(priv), const void *data,
		size_t len, unsigned int block, unsigned int nr_blocks)
#else
static int sparse_import_segment(void *UNUSED(priv), const void *data, int len,
		unsigned int block, unsigned int nr_blocks)
#endif
{
	/* Ignore chunk headers, only write the data */
	if (!nr_blocks || len % F2FS_BLKSIZE)
		return 0;

	return sparse_write_blk(block, nr_blocks, data);
}

static int sparse_merge_blocks(uint64_t start, uint64_t num, int zero)
{
	char *buf;
	uint64_t i;

	if (zero) {
		blocks[start] = NULL;
		return sparse_file_add_fill(f2fs_sparse_file, 0x0,
					F2FS_BLKSIZE * num, start);
	}

	buf = calloc(num, F2FS_BLKSIZE);
	if (!buf) {
		fprintf(stderr, "failed to alloc %llu\n",
			(unsigned long long)num * F2FS_BLKSIZE);
		return -ENOMEM;
	}

	for (i = 0; i < num; i++) {
		memcpy(buf + i * F2FS_BLKSIZE, blocks[start + i], F2FS_BLKSIZE);
		free(blocks[start + i]);
		blocks[start + i] = NULL;
	}

	/* free_sparse_blocks will release this buf. */
	blocks[start] = buf;

	return sparse_file_add_data(f2fs_sparse_file, blocks[start],
					F2FS_BLKSIZE * num, start);
}
#else
static int sparse_read_blk(__u64 UNUSED(block),
				int UNUSED(count), void *UNUSED(buf))
{
	return 0;
}

static int sparse_write_blk(__u64 UNUSED(block),
				int UNUSED(count), const void *UNUSED(buf))
{
	return 0;
}

static int sparse_write_zeroed_blk(__u64 UNUSED(block), int UNUSED(count))
{
	return 0;
}
#endif

//dev read 和 write 大致上还是 先缓存  后调用read 和 write 去写fd
int dev_read(void *buf, __u64 offset, size_t len)
{
	int fd;
	int err;

	if (c.sparse_mode)
		return sparse_read_blk(offset / F2FS_BLKSIZE,
					len / F2FS_BLKSIZE, buf);

	fd = __get_device_fd(&offset);
	if (fd < 0)
		return fd;

	/* err = 1: cache not available, fall back to non-cache R/W */
	/* err = 0: success, err=-1: I/O error */
	err = dcache_read(fd, buf, (off64_t)offset, len);
	if (err <= 0)
		return err;
	if (lseek64(fd, (off64_t)offset, SEEK_SET) < 0)
		return -1;
	if (read(fd, buf, len) < 0)
		return -1;
	return 0;
}

//预读
#ifdef POSIX_FADV_WILLNEED
int dev_readahead(__u64 offset, size_t len)
#else
int dev_readahead(__u64 offset, size_t UNUSED(len))
#endif
{
	int fd = __get_device_fd(&offset);

	if (fd < 0)
		return fd;
#ifdef POSIX_FADV_WILLNEED
	return posix_fadvise(fd, offset, len, POSIX_FADV_WILLNEED);
#else
	return 0;
#endif
}

int dev_write(void *buf, __u64 offset, size_t len)
{
	int fd;

	if (c.dry_run)
		return 0;

	if (c.sparse_mode)
		return sparse_write_blk(offset / F2FS_BLKSIZE,
					len / F2FS_BLKSIZE, buf);

	fd = __get_device_fd(&offset);
	if (fd < 0)
		return fd;

	/*
	 * dcache_update_cache() just update cache, won't do I/O.
	 * Thus even no error, we need normal non-cache I/O for actual write
	 */
	if (dcache_update_cache(fd, buf, (off64_t)offset, len) < 0)
		return -1;
	if (lseek64(fd, (off64_t)offset, SEEK_SET) < 0)
		return -1;
	if (write(fd, buf, len) < 0)
		return -1;
	return 0;
}

//写一个block大小 
int dev_write_block(void *buf, __u64 blk_addr)
{
	return dev_write(buf, blk_addr << F2FS_BLKSIZE_BITS, F2FS_BLKSIZE);
}

//备份？
int dev_write_dump(void *buf, __u64 offset, size_t len)
{
	if (lseek64(c.dump_fd, (off64_t)offset, SEEK_SET) < 0)
		return -1;
	if (write(c.dump_fd, buf, len) < 0)
		return -1;
	return 0;
}

int dev_fill(void *buf, __u64 offset, size_t len)
{
	int fd;

	if (c.sparse_mode)
		return sparse_write_zeroed_blk(offset / F2FS_BLKSIZE,
						len / F2FS_BLKSIZE);

	fd = __get_device_fd(&offset);
	if (fd < 0)
		return fd;

	/* Only allow fill to zero */
	if (*((__u8*)buf))
		return -1;
	if (lseek64(fd, (off64_t)offset, SEEK_SET) < 0)
		return -1;
	if (write(fd, buf, len) < 0)
		return -1;
	return 0;
}

int dev_fill_block(void *buf, __u64 blk_addr)
{
	return dev_fill(buf, blk_addr << F2FS_BLKSIZE_BITS, F2FS_BLKSIZE);
}

int dev_read_block(void *buf, __u64 blk_addr)
{
	return dev_read(buf, blk_addr << F2FS_BLKSIZE_BITS, F2FS_BLKSIZE);
}

int dev_reada_block(__u64 blk_addr)
{
	return dev_readahead(blk_addr << F2FS_BLKSIZE_BITS, F2FS_BLKSIZE);
}

//fsync(c.devices[i].fd) 的作用是将文件描述符上的数据同步到磁盘上 相当于强制刷新无需等待操作系统刷新
int f2fs_fsync_device(void)
{
#ifdef HAVE_FSYNC
	int i;

	for (i = 0; i < c.ndevs; i++) {
		if (fsync(c.devices[i].fd) < 0) {
			MSG(0, "\tError: Could not conduct fsync!!!\n");
			return -1;
		}
	}
#endif
	return 0;
}

int f2fs_init_sparse_file(void)
{
#ifdef HAVE_SPARSE_SPARSE_H
	if (c.func == MKFS) {
		f2fs_sparse_file = sparse_file_new(F2FS_BLKSIZE, c.device_size);
		if (!f2fs_sparse_file)
			return -1;
	} else {
		f2fs_sparse_file = sparse_file_import(c.devices[0].fd,
							true, false);
		if (!f2fs_sparse_file)
			return -1;

		c.device_size = sparse_file_len(f2fs_sparse_file, 0, 0);
		c.device_size &= (~((uint64_t)(F2FS_BLKSIZE - 1)));
	}

	if (sparse_file_block_size(f2fs_sparse_file) != F2FS_BLKSIZE) {
		MSG(0, "\tError: Corrupted sparse file\n");
		return -1;
	}
	blocks_count = c.device_size / F2FS_BLKSIZE;
	blocks = calloc(blocks_count, sizeof(char *));
	if (!blocks) {
		MSG(0, "\tError: Calloc Failed for blocks!!!\n");
		return -1;
	}

	zeroed_block = calloc(1, F2FS_BLKSIZE);
	if (!zeroed_block) {
		MSG(0, "\tError: Calloc Failed for zeroed block!!!\n");
		return -1;
	}

	return sparse_file_foreach_chunk(f2fs_sparse_file, true, false,
				sparse_import_segment, NULL);
#else
	MSG(0, "\tError: Sparse mode is only supported for android\n");
	return -1;
#endif
}

void f2fs_release_sparse_resource(void)
{
#ifdef HAVE_SPARSE_SPARSE_H
	int j;

	if (c.sparse_mode) {
		if (f2fs_sparse_file != NULL) {
			sparse_file_destroy(f2fs_sparse_file);
			f2fs_sparse_file = NULL;
		}
		for (j = 0; j < blocks_count; j++)
			free(blocks[j]);
		free(blocks);
		blocks = NULL;
		free(zeroed_block);
		zeroed_block = NULL;
	}
#endif
}

#define MAX_CHUNK_SIZE		(1 * 1024 * 1024 * 1024ULL)
#define MAX_CHUNK_COUNT		(MAX_CHUNK_SIZE / F2FS_BLKSIZE)
int f2fs_finalize_device(void)
{
	int i;
	int ret = 0;

#ifdef HAVE_SPARSE_SPARSE_H
	if (c.sparse_mode) {
		int64_t chunk_start = (blocks[0] == NULL) ? -1 : 0;
		uint64_t j;

		if (c.func != MKFS) {
			sparse_file_destroy(f2fs_sparse_file);
			ret = ftruncate(c.devices[0].fd, 0);
			ASSERT(!ret);
			lseek(c.devices[0].fd, 0, SEEK_SET);
			f2fs_sparse_file = sparse_file_new(F2FS_BLKSIZE,
							c.device_size);
		}

		for (j = 0; j < blocks_count; ++j) {
			if (chunk_start != -1) {
				if (j - chunk_start >= MAX_CHUNK_COUNT) {
					ret = sparse_merge_blocks(chunk_start,
							j - chunk_start, 0);
					ASSERT(!ret);
					chunk_start = -1;
				}
			}

			if (chunk_start == -1) {
				if (!blocks[j])
					continue;

				if (blocks[j] == zeroed_block) {
					ret = sparse_merge_blocks(j, 1, 1);
					ASSERT(!ret);
				} else {
					chunk_start = j;
				}
			} else {
				if (blocks[j] && blocks[j] != zeroed_block)
					continue;

				ret = sparse_merge_blocks(chunk_start,
						j - chunk_start, 0);
				ASSERT(!ret);

				if (blocks[j] == zeroed_block) {
					ret = sparse_merge_blocks(j, 1, 1);
					ASSERT(!ret);
				}

				chunk_start = -1;
			}
		}
		if (chunk_start != -1) {
			ret = sparse_merge_blocks(chunk_start,
						blocks_count - chunk_start, 0);
			ASSERT(!ret);
		}

		sparse_file_write(f2fs_sparse_file, c.devices[0].fd,
				/*gzip*/0, /*sparse*/1, /*crc*/0);

		f2fs_release_sparse_resource();
	}
#endif
	/*
	 * We should call fsync() to flush out all the dirty pages
	 * in the block device page cache.
	 */
	for (i = 0; i < c.ndevs; i++) {
#ifdef HAVE_FSYNC
		ret = fsync(c.devices[i].fd);
		if (ret < 0) {
			MSG(0, "\tError: Could not conduct fsync!!!\n");
			break;
		}
#endif
		ret = close(c.devices[i].fd);
		if (ret < 0) {
			MSG(0, "\tError: Failed to close device file!!!\n");
			break;
		}
		free(c.devices[i].path);
		free(c.devices[i].zone_cap_blocks);
	}
	close(c.kd);

	return ret;
}
