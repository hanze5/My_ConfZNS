# My_ConfZNS
designed based on confzns
## How to Configure?
The current version of ConfZNS supports built-in configuration, so user have to compile confZNS for each setting(this will be change soon)
```c++
//Example in zns.c:1787
    spp->pg_rd_lat = NAND_READ_LATENCY;
    spp->pg_wr_lat = NAND_PROG_LATENCY;
    spp->blk_er_lat = NAND_ERASE_LATENCY;
    spp->ch_xfer_lat = NAND_CHNL_PAGE_TRANSFER_LATENCY;

    spp->nchnls         = 8;   //default : 8
    spp->chnls_per_zone = 1;   
    spp->zones          = n->num_zones;     
    spp->ways           = 2;    //default : 2
    spp->ways_per_zone  = 2;    //default :==spp->ways
    spp->dies_per_chip  = 1;    //default : 1
    spp->planes_per_die = 4;    //default : 4
    spp->register_model = 1;    
```
* to change configurations, 
  * SU-zone
  ```c++
      spp->chnls_per_zone = 1;   
      spp->ways_per_zone  = 1;    //default :==spp->ways
  ```
  * MU4-zone
  ```c++
      spp->chnls_per_zone = 4;    
      spp->ways_per_zone  = 1;    //default :==spp->ways
  ```
  * MU8-zone
  ```c++
      spp->chnls_per_zone = 8;    
      spp->ways_per_zone  = 1;    //default :==spp->ways
  ```
  * FU-zone(16)
  ```c++
      spp->chnls_per_zone = 8;    
      spp->ways_per_zone  = 2;    //default :==spp->ways
  ```


实验参数：
模拟硬盘16G  8个channel 4个way  zone大小512MB  因此刚好与芯片大小一致   一个硬盘共有32个芯片 也有32个zone  当一个zone只占据一个并行单元时就刚好独占一个chip

TODO：
磁盘性能理论值计算
读延迟 65000  写延迟450000 传输延迟25000

读延迟模拟 写延迟模拟


一些常用的命令
```shell
blkzone reset /dev/nvme0n1
blkzone report /dev/nvme0n1 > output_report.txt
fio --direct=1 --zonemode=zbd --name=diff_bs --iodepth=1 --size=1z --filename=/dev/nvme0n1 --ioengine=psync --bs=16k --rw=write --offset=8z



nvme zns report-zones /dev/nvme0n1
cat /sys/block/nvme0n1/queue/max_open_zones

echo mq-deadline > /sys/block/nvme0n1/queue/scheduler

DEBUG_LEVEL=0 ROCKSDB_PLUGINS=zenfs make clean
export CXXFLAGS="-I/usr/include -L/usr/lib/x86_64-linux-gnu"
DEBUG_LEVEL=0 ROCKSDB_PLUGINS=zenfs make -j4 db_bench install

cd plugin/zenfs/util
make

rm -rf /home/femu/workspace/My_ConfZNS/trial/rocksdblogs/*

./plugin/zenfs/util/zenfs mkfs --zbd=nvme0n1 --aux_path=/home/femu/workspace/My_ConfZNS/trial/rocksdblogs --force

./db_bench --fs_uri=zenfs://dev:nvme0n1 -use_direct_io_for_flush_and_compaction=true --disable_wal  -benchmarks=fillseq -num=327680 -key_size=48 -value_size=65488 >  output.txt


  # -num_multi_db=4 \
  # --duration=600 \
  # -reads=100000 \

  --benchmarks="mixgraph_together" \
  --benchmarks="ycsb-a" \



./db_bench \
  --fs_uri=zenfs://dev:nvme0n1 \
  --benchmarks="mixgraph_together" \
  --statistics \
  --disable_wal \
  --use_direct_io_for_flush_and_compaction \
  -use_direct_reads=true \
  -keyrange_dist_a=14.18 \
  -keyrange_dist_b=-2.917 \
  -keyrange_dist_c=0.0164 \
  -keyrange_dist_d=-0.08082 \
  -keyrange_num=30 \
  -iter_k=2.517 \
  -iter_sigma=14.236 \
  -sine_mix_rate \
  -sine_mix_rate_interval_milliseconds=5000 \
  -sine_a=100 \
  -sine_b=0.000073 \
  -sine_d=450 \
  --duration=180 \
  -num=1000000 \
  -key_size=48 > outputa.txt



./plugin/zenfs/util/zenfs ls-uuid
./plugin/zenfs/util/zenfs list --zbd=nvme0n1
./plugin/zenfs/util/zenfs fs-info --zbd=nvme0n1
./plugin/zenfs/util/zenfs df --zbd=nvme0n1 
./plugin/zenfs/util/zenfs backup --zbd=nvme0n1 
 

./db_bench \
  --fs_uri=zenfs://dev:nvme0n1 \
  --db=1st \
  --statistics \
  --disable_wal \
  --benchmarks=fillrandom \
  --use_direct_io_for_flush_and_compaction \
  --num=20000000


mixgraph_together :    1543.369 micros/op 647 ops/sec 60.190 seconds 38999 operations;   40.8 MB/s ( Gets:19375 Puts:19624 Seek:0, reads 0 in 19375 found, avg size: 131072.0 value, -nan scan)

mixgraph_together :    1876.106 micros/op 533 ops/sec 61.910 seconds 32999 operations;   33.5 MB/s ( Gets:16390 Puts:16609 Seek:0, reads 0 in 16390 found, avg size: 131072.0 value, -nan scan)

mixgraph_together :    2619.180 micros/op 381 ops/sec 60.239 seconds 22999 operations;   23.9 MB/s ( Gets:11504 Puts:11495 Seek:0, reads 0 in 11504 found, avg size: 131072.0 value, -nan scan)

mixgraph_together :    2522.561 micros/op 396 ops/sec 60.539 seconds 23999 operations;   24.7 MB/s ( Gets:12036 Puts:11963 Seek:0, reads 0 in 12036 found, avg size: 131072.0 value, -nan scan)


43.7
35.4
24.0
12.7 

```

zenfs 是基于libzbd实现的。
先读 zbd_zenfs.h 里面定义了最基础最主要的类的功能：
ZonedBlockDevice ZonedBlockDeviceBackend  Zone

zbdlib_zenfs 对ZonedBlockDeviceBackend这个基类进行了实现 class ZbdlibBackend

看完 zbdlib_zenfs zbd_zenfs 就看

zonefs_zenfs则是与zbd_zenfs同级别的另一种实现
其中ZoneFsBackend是对ZonedBlockDeviceBackend这个基类进行了另一种实现

snapshot好像是zdb才有的功能


计算file与zone生命周期差异以及选择最合适zone的代码在zbd_zenfs.cc里面
IOStatus ZonedBlockDevice::GetBestOpenZoneMatch()
在 TakeMigrateZone与AllocateIOZone函数中都有使用到这块应该是关注重点

void ZenFS::GCWorker()也要额外关注一下


AllocateIOZone函数 会在 io_zenfs.cc ZoneFile::AllocateNewZone()中被调用


zenfs主要提供了  get  delete  创建（创建这里调用的rockdb里面的）  

读源码过程中：
ZoneList 里面存放的数据就是下面这个结构体  是libzbd 里面定义的
```c
struct zbd_zone {
	unsigned long long	start;		/* Zone start */
	unsigned long long	len;		/* Zone length */
	unsigned long long	capacity;	/* Zone capacity */
	unsigned long long	wp;		/* Zone write pointer */
	unsigned int		flags;		/* Zone state flags */
	unsigned int		type;		/* Zone type */
	unsigned int		cond;		/* Zone condition */
	uint8_t			reserved[20];	/* Padding to 64B */
} __attribute__((packed));
```
pread和pwrite函数与read和write函数的主要区别在于以下几点：

1. 原子操作：pread和pwrite函数是原子操作，这意味着它们的定位和读/写操作在一个原子操作中完成，期间不可中断。而read和write函数可能在定位和读/写操作之间被其他程序中断。

2. 文件偏移量：pread和pwrite函数不会改变当前文件的偏移量，也就是说不改变当前文件指针的位置。而read和write函数在操作后会改变文件的偏移量。

3. 指定位置：pread和pwrite函数可以从文件的任意位置开始读取或写入数据，而无需改变文件的当前偏移量。而read和write函数则从文件的当前偏移量开始读取或写入数据。

总的来说，pread和pwrite函数提供了更高级的文件读写控制，使得程序能够更灵活地处理文件的读写操作。因此在使用zbd打开了设备如：/dev/nvme0n1之后，使用pread和pwrite进行真正的数据读写。


defer IO 一种延迟和重新利用IO的方式。它使用主机内存作为缓冲区，并使用MMU页面错误作为执行设备IO的预触发器


IOType::kWAL是一个枚举值，通常在数据库系统中使用，特别是在RocksDB这样的键值存储系统中。WAL是Write-Ahead Logging的缩写，这是一种用于保证数据完整性和持久性的技术。

在Write-Ahead Logging策略中，所有的修改操作（如插入、更新或删除）在被应用到数据库之前，都会先被写入到一个称为WAL的日志中。如果在操作完成之前系统崩溃，WAL会提供一个恢复到一致状态的方法，因为它记录了所有未完成的修改操作。

因此，IOType::kWAL通常表示一个与Write-Ahead Logging相关的IO操作


在ZenFS的实现中，每个ZoneFile都有自己的active_zone_，并且这个active_zone_在使用完毕后会被释放1。ZenFS可以在单个区域中存储多个文件，通过使用一个范围分配方案。一个文件可以由一个或多个范围组成，组成该文件的所有范围可以存储在设备的同一区域（或不同区域）中。

因此，虽然理论上多个ZoneFile对象的active_zone_可能指向同一个Zone，但在ZenFS的实现中，这种情况似乎不太可能发生。这是因为每个ZoneFile都管理自己的active_zone_，并且这个active_zone_在使用完毕后会被释放。这种设计可以简化管理并避免并发问题。这个可能需要看一下zenfs是如何实现的

 
# 如何实现呢？
zbd的open是可以独占或者非独占的，**BUT**
ZenFS是一个为RocksDB设计的文件系统插件，它使用RocksDB的文件系统接口将文件放置在原始的分区块设备（ZBD）上的区域中12。以下是ZenFS需要一个独立的ZBD设备的主要原因：
- 数据生命周期管理：ZenFS通过将文件分离到不同的区域，并利用写入生命周期提示（WLTH）将具有相似生命周期的数据放在一起，从而大大减少了与传统块设备相比的系统写入放大12。
- 垃圾回收：在ZenFS中，只有当RocksDB启动LSM-tree表压缩过程时，才会执行数据垃圾回收。ZenFS和ZNS设备控制器都不执行垃圾回收1。
- 空间利用：ZenFS可以通过使用区域分配方案在单个区域中存储多个文件。一个文件可以由一个或多个区域组成，所有组成该文件的区域可以存储在设备的同一区域（或不同区域）中1。
因此，ZenFS需要一个独立的ZBD设备，以便能够有效地管理数据的生命周期，减少写入放大，并优化空间利用。
# 代码实现思路形成过程
我们的最主要目标还是要初步实现对zone的分配，目前zenfs的zone分配基于生命周期。
**step 1**，我们把目光放到`ZonedBlockDevice::AllocateIOZone`与`ZonedBlockDevice::AllocateMetaZone`.分配MetaZone就是遍历所有MetaZone找到没有被使用的进行分配。至于zone是MetaZone还是IOzone在`ZonedBlockDevice::Open`的时候就被规定好了。目前的实现是取前3个zone为MetaZone（*后面可以根据工作负载数量来进行调整*），后面都是IOzone。IOZone通过调用`ZonedBlockDevice::GetBestOpenZoneMatch`和`ZonedBlockDevice::FinishCheapestIOZone`来遍历每一个IOZone来比对生命周期找到最适合的那个以及结束最廉价的那个,发现这个函数不止被`ZonedBlockDevice::AllocateIOZone`调用。还被`ZonedBlockDevice::TakeMigrateZone`调用。
目前的想法是修改`ZonedBlockDevice::GetBestOpenZoneMatch`和`ZonedBlockDevice::FinishCheapestIOZone`，在选择IOZone时候来增加限制。也就是说目前这个思路需要修改三个函数.
**step 2**，接着我们来看是谁在调用`ZonedBlockDevice::AllocateIOZone`与`ZonedBlockDevice::TakeMigrateZone`。
- `ZonedBlockDevice::AllocateIOZone`被`ZoneFile::AllocateNewZone`调用，那么想法是给zonefile增加一个成员属性叫做appID_。只需要在`ZoneFile`实例化时候传入，因此需要修改该类的构造函数。
- `ZonedBlockDevice::TakeMigrateZone`被`ZenFS::MigrateFileExtents`调用。该函数为传入的名为fname的ZoneFile的某些需要被迁移的extent选择适合的IOZone，因此吾认为此处的修改和上面是一样的。即给zonefile增加一个成员属性叫做appID_。只需要在`ZoneFile`实例化时候传入，因此需要修改该类的构造函数。
目前的想法是给`ZoneFile`增加新的成员属性来表示改file属于哪一个工作负载。然后只需要改动上述相应的函数最终实现限制IOZone的分配。
**step 3**那么现在需要关注的就是ZoneFile的实例化过程。可以参考ZoneFile的生命周期是如何传入的。ZoneFile是作为成员变量被**封装**在`ZonedWritableFile`、`ZonedSequentialFile`和`ZonedRandomAccessFile`。这三个类分别继承自RocksDB文件系统接口`FSSequentialFile`、`FSSequentialFile`和`FSRandomAccessFile`，均在RocksDB的file_system中有所定义。也就是说我们要看`ZonedWritableFile`、`ZonedSequentialFile`和`ZonedRandomAccessFile`这三个东西是如何被实例化的。此三者，分别 `ZenFS::NewSequentialFile`、`ZenFS::NewRandomAccessFile`和`ZenFS::NewWritableFile`中被创建，而这三个函数名将会在RocksDB中被调用，他们三个只是`ZenFS`的定义，因此RocksDB并不能感知ZoneFile类型。那么问题又来了，ZoneFile的实例化在哪里？以`ZenFS::NewSequentialFile`的源码为例：
```c++
IOStatus ZenFS::NewSequentialFile(const std::string& filename,
                                  const FileOptions& file_opts,
                                  std::unique_ptr<FSSequentialFile>* result,
                                  IODebugContext* dbg) {
  std::string fname = FormatPathLexically(filename);
  std::shared_ptr<ZoneFile> zoneFile = GetFile(fname);

  Debug(logger_, "New sequential file: %s direct: %d\n", fname.c_str(),
        file_opts.use_direct_reads);

  if (zoneFile == nullptr) {
    return target()->NewSequentialFile(ToAuxPath(fname), file_opts, result,
                                       dbg);
  }

  result->reset(new ZonedSequentialFile(zoneFile, file_opts));
  return IOStatus::OK();
}
```
函数的流程：
首先，它尝试获取一个名fname为ZoneFile 的文件，就是从`files_`里面找。`files_`是`ZenFS`的map类型的成员变量，`ZenFS`继承自`FileSystemWrapper`继承自`FileSystem`，后面这俩都是RocksDB封装了文件系统操作的类或接口。它提供了一种抽象，使得代码可以在不直接处理底层文件系统细节的情况下进行文件操作,目前感觉不用管。
说回来，如果说文件系统没有这个文件，会转而调用`target()`的`NewSequentialFile`函数，而target到底是什么类型由ZenFS实例化的时候决定，我觉的应该跑不出别的文件系统，应该就是ZenFS。那么`GetFile(fname)`就一定会有结果。那么files_是什么时候被设置的呢,ZenFS被初始化的时候files_肯定时空的。通过搜索`files_.insert`得到了以下插入files_的情况。
- `ZenFS::DeleteFileNoLock`删除文件的时候，如果无法将删除记录持久化无法保持一致性，那么就会重新插入。
- `ZenFS::OpenWritableFile`打开一个可写文件，根据`bool reopen`参数来决定重用文件还是重新创建文件。这里面会有。`std::make_shared<ZoneFile>(zbd_, next_file_id_++, &metadata_writer_);`
- `ZenFS::RenameFileNoLock`重命名文件
- `ZenFS::LinkFile` 创建硬链接
- `ZenFS::DecodeFileUpdateFrom` 从slice对象中解码文件更新 这里面会有`std::shared_ptr<ZoneFile> update(new ZoneFile(zbd_, 0, &metadata_writer_));`
- `ZenFS::DecodeSnapshotFrom` 目的是将文件删除操作编码到一个字符串中，里面会有`std::shared_ptr<ZoneFile> zoneFile(new ZoneFile(zbd_, 0, &metadata_writer_));`

需要重点关注的三个：
- `ZenFS::OpenWritableFile` 被 ZenFS::NewWritableFile ZenFS::ReuseWritableFile ReopenWritableFile调用 

- ZenFS::DecodeFileUpdateFrom与ZenFS::DecodeSnapshotFrom 则是会在崩溃回复时被 ZenFS::RecoverFrom 调用 暂且不管



到现在反应过来了，ZoneFile的实例化根本就不需要传入lifetime_,而是会在构造函数中自动被设置为`Env::WLTH_NOT_SET`。那appID应该可以参考这个去设计，通过`ZoneFile::SetWriteLifeTimeHint` 或者 `ZoneFile::DecodeFrom` 进行设置。到时候就在搜索lifetime_对照着看看加什么就好了。
**step 4**那么接下来就看看是谁在调用`ZoneFile::SetWriteLifeTimeHint`和 `ZoneFile::DecodeFrom`就好了。而对于RocksDB来说是感知不到ZoneFile的，他调用的是上层接口的`SetWriteLifeTimeHint`。例如：
```c++
void ZonedWritableFile::SetWriteLifeTimeHint
```
与生命周期不同的是，appid应该是创建的时候就确定好，后面不会更改，而这也让我意识到一个问题，`ZenFS`统一管理所有文件，那么多个用户程序似乎不能有相同名称的文件。这样的话似乎要在应用程序创建文件的时候加上一点后缀用于标识了，是不是似乎反而更加容易了呢？那么现在决定反向来看了，去看看dbbench的代码。在`tools`目录下
。。。。。。
目前了解到RocksDB有，Column Families  在RocksDB中，每个键值对都与一个确切的列族（Column Family）相关联。如果没有指定列族，键值对就会与默认的列族（“default”）相关联。列族提供了一种逻辑上划分数据库的方式。这意味着你可以根据不同的应用场景或数据类型，将数据分配到不同的列族中，从而实现数据的逻辑隔离和独立管理。不同的列族可以有相同的键（key）。每个键值对都与一个确切的列族相关联，这意味着即使多个列族中有相同的键，它们也是独立存储和管理的。
。。。。。。
目前大概的路线是，运行工作负载时指定Column Families，这样不同的Column Families 的数据会被写入到不同的SSTFile中。不管怎么说，先去看看SSTFile的构建过程以及合并过程确定是隔离的。
。。。。。。
现在在创建SSTFile时候会加入Column Families name信息，现在目标就是让benchmark运行命令可以指定Columun Families就好了。Benchmark::db_何时初始化,应该查找 `db_->CreateColumnFamily` 或者  `DB::Open`。已在`void OpenDb`中找到.
。。。。。。
假设colmun_family name是 1st 2nd 3rd 4th
。。。。。。
`ZonedBlockDevice::GetBestOpenZoneMatch`和`ZonedBlockDevice::FinishCheapestIOZone`和`ZonedBlockDevice::AllocateEmptyZone`已经重载。需要改调用他们的，需要改 
- `ZonedBlockDevice::AllocateIOZone`
- `ZonedBlockDevice::TakeMigrateZone` 
改完了 然后再看看 谁调用了这俩,有下面这两个
- `ZenFS::MigrateFileExtents` 该函数里面可以直接获取到fname 因此改动不大
- `ZoneFile::AllocateNewZone` 似乎需要给ZoneFile加一个 fname的成员. fname与ZoneFile似乎并不是1对1的关系而是n对1的关系. 通过一个fname确实可以确定一个ZoneFile,那是否可以通过其中一个fname就确定ZoneFile属于哪一个app呢，这个需要实验
。。。。。。
看了这么多  发现有一个方法就可以直接修改文件名  其实是文件路径  就是参数上加
`DEFINE_string(db, "", "Use the db with the following name.");`
这样文件名称就会是：
`rocksdbtest/<db>/*.*`  好无语啊。。。不过离真相更近了
。。。。。。
`ZonedBlockDevice::AllocateIOZone` 与 `ZenFS::MigrateFileExtents` 中加上提取db函数


# benchmark.sh里面的一些负载：
## benchmark_bulkload_fillrandom
- benchmarks=fillrandom,stats：运行 fillrandom 和 stats 两种基准测试。fillrandom 测试会随机填充数据库，stats 测试会输出数据库的统计信息。
- use_existing_db=0：这意味着每次运行基准测试时，都会创建一个新的数据库。
- disable_auto_compactions=1：禁用自动压缩。在 RocksDB 中，压缩是一种重要的后台操作，用于合并多个数据文件以提高读取效率。但在填充数据库时，可能希望禁用它以提高写入速度。
- sync=0：禁用同步写入。这意味着写入操作可能不会立即写入磁盘，这可以提高写入速度，但在系统崩溃时可能会丢失数据。
- $params_bulkload：这是一个变量，包含了一组额外的参数，这些参数在执行批量加载操作时会用到。
- threads=1：这意味着基准测试将在单个线程上运行。
- memtablerep=vector：这设置了内存表的实现方式，vector 表示使用向量作为内存表的数据结构。
- allow_concurrent_memtable_write=false：这禁止了并发的内存表写入。
- disable_wal=1：这禁用了写前日志（WAL）。WAL 是一种用于在系统崩溃时恢复数据的机制。禁用它可以提高写入速度，但在系统崩溃时可能会丢失数据。
- seed=$( date +%s )：这设置了随机数生成器的种子，使得每次运行测试时的随机数序列都不同。



Slice 是一个重要的数据结构，它代表了一个不可变的字节数组。它通常用于表示键（key）和值（value），以及在数据库操作中传递数据。Slice 由一个指向数据的指针和数据长度组成，这使得它可以高效地引用数据，而无需进行复制。




通过write_buffer_size参数设置MemTable的大小，这将影响刷新到SST文件的频率和大小

target_file_size_base和target_file_size_multiplier参数用于控制SST文件的目标大小以及随着层级增长的大小变化。这些参数对于优化数据库的性能和存储效率至关重要。

target_file_size_base参数设置了L1层SST文件的目标大小。这是创建SST文件时的基础大小，通常用于控制最初写入的SST文件的大小。
**目前都设置的是32MB**

target_file_size_multiplier参数决定了每一层SST文件大小相对于上一层的增长倍数。这意味着，如果target_file_size_multiplier大于1，每向下一层，SST文件的大小就会按照这个倍数增加。
**目前设置的是1**

max_bytes_for_level_base  为L1的数据存储总量上限。超过将会触发压实
**目前设置的是 256MB**


l0层文件大小就是 write_buffer_size 也就是32MB

文件数量限制方面：
level0_file_num_compaction_trigger 4
level0_slowdown_writes_trigger 10
level0_stop_writes_trigger 16

**设置为不压缩**
static enum ROCKSDB_NAMESPACE::CompressionType FLAGS_compression_type_e =
    ROCKSDB_NAMESPACE::kNoCompression;

compression_ratio 设置为 1.0

DEFINE_int32(num_bottom_pri_threads, 4,
             "The number of threads in the bottom-priority thread pool (used "
             "by universal compaction only).");

DEFINE_int32(num_high_pri_threads, 4,
             "The maximum number of concurrent background compactions"
             " that can occur in parallel.");

DEFINE_int32(num_low_pri_threads, 4,
             "The maximum number of concurrent background compactions"
             " that can occur in parallel.");


max_background_jobs 设置为 默认
max_background_compactions 设置为 16
max_background_flushes 设置为 16



mixgraph_together :    1433.007 micros/op 697 ops/sec 120.371 seconds 83999 operations;   22.0 MB/s ( Gets:41746 Puts:42253 Seek:0, reads 0 in 41746 found, avg size: 65536.0 value, -nan scan)

mixgraph_together :    1552.863 micros/op 643 ops/sec 121.122 seconds 77999 operations;   20.0 MB/s ( Gets:39246 Puts:38753 Seek:0, reads 0 in 39246 found, avg size: 65536.0 value, -nan scan)

mixgraph_together :    1726.489 micros/op 579 ops/sec 120.852 seconds 69999 operations;   18.0 MB/s ( Gets:35138 Puts:34861 Seek:0, reads 0 in 35138 found, avg size: 65536.0 value, -nan scan)

mixgraph_together :    2464.068 micros/op 405 ops/sec 120.737 seconds 48999 operations;   12.7 MB/s ( Gets:24503 Puts:24496 Seek:0, reads 0 in 24503 found, avg size: 65536.0 value, -nan scan)

mixgraph_together :    1719.179 micros/op 2319 ops/sec 121.122 seconds 280996 operations;   72.5 MB/s ( Gets:41746 Puts:42253 Seek:0, reads 0 in 41746 found, avg size: 65536.0 value, -nan scan)

<===================================前0个线程报告===================================>
STATISTICS:
rocksdb.memtable.hit COUNT : 0
rocksdb.memtable.miss COUNT : 41746
rocksdb.l0.hit COUNT : 0
rocksdb.l1.hit COUNT : 0
rocksdb.l2andup.hit COUNT : 0
rocksdb.number.keys.written COUNT : 42253
rocksdb.number.keys.read COUNT : 41746
rocksdb.number.keys.updated COUNT : 0
rocksdb.bytes.written COUNT : 2771839053
rocksdb.bytes.read COUNT : 0
rocksdb.db.iter.bytes.read COUNT : 0
rocksdb.stall.micros COUNT : 0
rocksdb.flush.write.bytes COUNT : 65676045
rocksdb.number.direct.load.table.properties COUNT : 0
rocksdb.merge.operation.time.nanos COUNT : 0
rocksdb.db.get.micros P50 : 1373.790040 P95 : 9193.979174 P99 : 13715.998299 P100 : 33874.000000 COUNT : 41746 SUM : 116724917
rocksdb.db.write.micros P50 : 20.175792 P95 : 75.295157 P99 : 139.518636 P100 : 9510.000000 COUNT : 42253 SUM : 1238538
rocksdb.table.sync.micros P50 : 3730.357143 P95 : 7825.714286 P99 : 8243.000000 P100 : 8243.000000 COUNT : 88 SUM : 339365
rocksdb.db.seek.micros P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.db.write.stall P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.sst.read.micros P50 : 234.030330 P95 : 4119.321314 P99 : 6095.541636 P100 : 26519.000000 COUNT : 164763 SUM : 138359653
rocksdb.file.read.flush.micros P50 : 202.000000 P95 : 3666.666667 P99 : 3739.000000 P100 : 3739.000000 COUNT : 88 SUM : 58728
rocksdb.file.read.get.micros P50 : 230.218566 P95 : 3807.848359 P99 : 5456.688845 P100 : 26519.000000 COUNT : 156680 SUM : 111753139
rocksdb.db.flush.micros P50 : 158000.000000 P95 : 239333.333333 P99 : 247420.000000 P100 : 247420.000000 COUNT : 88 SUM : 15622271

<===================================前1个线程报告===================================>
STATISTICS:
rocksdb.memtable.hit COUNT : 0
rocksdb.memtable.miss COUNT : 39246
rocksdb.l0.hit COUNT : 0
rocksdb.l1.hit COUNT : 0
rocksdb.l2andup.hit COUNT : 0
rocksdb.number.keys.written COUNT : 38753
rocksdb.number.keys.read COUNT : 39246
rocksdb.number.keys.updated COUNT : 0
rocksdb.bytes.written COUNT : 2542235553
rocksdb.bytes.read COUNT : 0
rocksdb.db.iter.bytes.read COUNT : 0
rocksdb.stall.micros COUNT : 0
rocksdb.flush.write.bytes COUNT : 60501814
rocksdb.number.direct.load.table.properties COUNT : 0
rocksdb.merge.operation.time.nanos COUNT : 0
rocksdb.db.get.micros P50 : 1330.629371 P95 : 8950.373102 P99 : 13449.965616 P100 : 32220.000000 COUNT : 39246 SUM : 104274722
rocksdb.db.write.micros P50 : 20.160287 P95 : 84.723446 P99 : 147.274684 P100 : 12187.000000 COUNT : 38753 SUM : 1168382
rocksdb.table.sync.micros P50 : 3571.875000 P95 : 8208.750000 P99 : 24090.000000 P100 : 25469.000000 COUNT : 81 SUM : 316611
rocksdb.db.seek.micros P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.db.write.stall P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.sst.read.micros P50 : 237.987902 P95 : 4086.786484 P99 : 6072.080309 P100 : 30523.000000 COUNT : 146651 SUM : 123477253
rocksdb.file.read.flush.micros P50 : 171.739130 P95 : 867.100000 P99 : 4494.000000 P100 : 4494.000000 COUNT : 81 SUM : 33173
rocksdb.file.read.get.micros P50 : 233.953577 P95 : 3775.022917 P99 : 5419.283784 P100 : 30523.000000 COUNT : 139345 SUM : 99858753
rocksdb.db.flush.micros P50 : 146818.181818 P95 : 237384.615385 P99 : 257060.000000 P100 : 257060.000000 COUNT : 81 SUM : 13300144

<===================================前2个线程报告===================================>
STATISTICS:
rocksdb.memtable.hit COUNT : 0
rocksdb.memtable.miss COUNT : 35138
rocksdb.l0.hit COUNT : 0
rocksdb.l1.hit COUNT : 0
rocksdb.l2andup.hit COUNT : 0
rocksdb.number.keys.written COUNT : 34861
rocksdb.number.keys.read COUNT : 35138
rocksdb.number.keys.updated COUNT : 0
rocksdb.bytes.written COUNT : 2286916461
rocksdb.bytes.read COUNT : 0
rocksdb.db.iter.bytes.read COUNT : 0
rocksdb.stall.micros COUNT : 0
rocksdb.flush.write.bytes COUNT : 55087226
rocksdb.number.direct.load.table.properties COUNT : 0
rocksdb.merge.operation.time.nanos COUNT : 0
rocksdb.db.get.micros P50 : 1153.938735 P95 : 8279.041591 P99 : 12976.927573 P100 : 35875.000000 COUNT : 35138 SUM : 82005917
rocksdb.db.write.micros P50 : 19.654194 P95 : 60.255345 P99 : 109.612265 P100 : 10726.000000 COUNT : 34861 SUM : 895147
rocksdb.table.sync.micros P50 : 3491.346154 P95 : 5142.500000 P99 : 7299.000000 P100 : 7299.000000 COUNT : 73 SUM : 243088
rocksdb.db.seek.micros P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.db.write.stall P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.sst.read.micros P50 : 231.479600 P95 : 3964.020029 P99 : 6001.689388 P100 : 28840.000000 COUNT : 129040 SUM : 98535801
rocksdb.file.read.flush.micros P50 : 169.189189 P95 : 2575.000000 P99 : 4994.000000 P100 : 5820.000000 COUNT : 73 SUM : 35536
rocksdb.file.read.get.micros P50 : 227.895349 P95 : 3600.303086 P99 : 5150.011043 P100 : 28840.000000 COUNT : 122731 SUM : 78177573
rocksdb.db.flush.micros P50 : 147118.644068 P95 : 229142.857143 P99 : 238094.000000 P100 : 238094.000000 COUNT : 73 SUM : 12154704

<===================================前3个线程报告===================================>
STATISTICS:
rocksdb.memtable.hit COUNT : 0
rocksdb.memtable.miss COUNT : 24503
rocksdb.l0.hit COUNT : 0
rocksdb.l1.hit COUNT : 0
rocksdb.l2andup.hit COUNT : 0
rocksdb.number.keys.written COUNT : 24496
rocksdb.number.keys.read COUNT : 24503
rocksdb.number.keys.updated COUNT : 0
rocksdb.bytes.written COUNT : 1606962096
rocksdb.bytes.read COUNT : 0
rocksdb.db.iter.bytes.read COUNT : 0
rocksdb.stall.micros COUNT : 0
rocksdb.flush.write.bytes COUNT : 37946884
rocksdb.number.direct.load.table.properties COUNT : 0
rocksdb.merge.operation.time.nanos COUNT : 0
rocksdb.db.get.micros P50 : 913.909995 P95 : 6414.664329 P99 : 11494.042484 P100 : 29523.000000 COUNT : 24503 SUM : 43692650
rocksdb.db.write.micros P50 : 19.624459 P95 : 67.791547 P99 : 115.711732 P100 : 4167.000000 COUNT : 24496 SUM : 648084
rocksdb.table.sync.micros P50 : 3525.000000 P95 : 4344.642857 P99 : 4634.000000 P100 : 4634.000000 COUNT : 51 SUM : 163346
rocksdb.db.seek.micros P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.db.write.stall P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.sst.read.micros P50 : 229.919940 P95 : 3606.009615 P99 : 5799.559089 P100 : 27713.000000 COUNT : 79556 SUM : 52641485
rocksdb.file.read.flush.micros P50 : 222.307692 P95 : 1077.833333 P99 : 1556.000000 P100 : 1556.000000 COUNT : 51 SUM : 19229
rocksdb.file.read.get.micros P50 : 226.750334 P95 : 3152.808294 P99 : 4432.906250 P100 : 27713.000000 COUNT : 75947 SUM : 41348032
rocksdb.db.flush.micros P50 : 144772.727273 P95 : 184341.000000 P99 : 184341.000000 P100 : 184341.000000 COUNT : 51 SUM : 8052308