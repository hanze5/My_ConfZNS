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
blkzone report /dev/nvme0n1
fio --direct=1 --zonemode=zbd --name=diff_bs --iodepth=1 --size=1z --filename=/dev/nvme0n1 --ioengine=psync --bs=16k --rw=write --offset=0z



nvme zns report-zones /dev/nvme0n1
cat /sys/block/nvme0n1/queue/max_open_zones

DEBUG_LEVEL=0 ROCKSDB_PLUGINS=zenfs make clean
export CXXFLAGS="-I/usr/include -L/usr/lib/x86_64-linux-gnu"
DEBUG_LEVEL=0 ROCKSDB_PLUGINS=zenfs make -j2 db_bench install

cd plugin/zenfs/util
make

echo mq-deadline > /sys/block/nvme0n1/queue/scheduler

rm -rf /home/femu/workspace/My_ConfZNS/trial/rocksdblogs/*

./plugin/zenfs/util/zenfs mkfs --zbd=nvme0n1 --aux_path=/home/femu/workspace/My_ConfZNS/trial/rocksdblogs --force

  # -num_multi_db=4 \
  # --duration=60 \
  # -reads=100000 \

  --benchmarks="mixgraph_together" \


./db_bench \
  --fs_uri=zenfs://dev:nvme0n1 \
  --benchmarks="mixgraph_together" \
  --statistics \
  --disable_wal \
  -use_direct_io_for_flush_and_compaction=true \
  -use_direct_reads=true \
  -keyrange_dist_a=14.18 \
  -keyrange_dist_b=-2.917 \
  -keyrange_dist_c=0.0164 \
  -keyrange_dist_d=-0.08082 \
  -keyrange_num=30 \
  -iter_k=2.517 \
  -iter_sigma=14.236 \
  -sine_mix_rate_interval_milliseconds=5000 \
  -sine_a=1000 \
  -sine_b=0.000073 \
  -sine_d=4500 \
  -reads=2000000 \
  -num=10000000 \
  -key_size=48 > output.txt

./db_bench \
  --fs_uri=zenfs://dev:nvme0n1 \
  --benchmarks="mixgraph_together" \
  --statistics \
  --disable_wal \
  -use_direct_io_for_flush_and_compaction=true \
  -use_direct_reads=true \
  -keyrange_dist_a=14.18 \
  -keyrange_dist_b=-2.917 \
  -keyrange_dist_c=0.0164 \
  -keyrange_dist_d=-0.08082 \
  -keyrange_num=30 \
  -iter_k=2.517 \
  -iter_sigma=14.236 \
  -sine_mix_rate_interval_milliseconds=5000 \
  -sine_a=1000 \
  -sine_b=0.000073 \
  -sine_d=4500 \
  -reads=20000 \
  -num=100000 \
  -key_size=48 




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




#YCSB 
./db_bench \
  --fs_uri=zenfs://dev:nvme0n1 \
  --db=1st \
  --statistics \
  --disable_wal \
  --benchmarks=ycsb-a \
  --use_direct_io_for_flush_and_compaction \
  --value_size_distribution_type=normal \
  --value_size_min=8 \
  --value_size_max=256 \
  --sine_write_rate=true \
  --duration=600


./db_bench \
  --benchmarks="mixgraph" \
  -use_direct_io_for_flush_and_compaction=true \
  -use_direct_reads=true \
  -cache_size=268435456 \
  -keyrange_dist_a=14.18 \
  -keyrange_dist_b=-2.917 \
  -keyrange_dist_c=0.0164 \
  -keyrange_dist_d=-0.08082 \
  -keyrange_num=30 \
  -value_k=0.2615 \
  -value_sigma=25.45 \
  -iter_k=2.517 \
  -iter_sigma=14.236 \
  -mix_get_ratio=0.85 \
  -mix_put_ratio=0.14 \
  -mix_seek_ratio=0.01 \
  -sine_mix_rate_interval_milliseconds=5000 \
  -sine_a=1000 \
  -sine_b=0.000073 \
  -sine_d=4500 \
  --perf_level=2 \
  -reads=4200000 \
  -num=500000 \
  -key_size=48

./db_bench \
  --benchmarks="mixgraph_together" \
  -num_multi_db=4 \
  -use_direct_io_for_flush_and_compaction=true \
  -use_direct_reads=true \
  -cache_size=268435456 \
  -keyrange_dist_a=14.18 \
  -keyrange_dist_b=-2.917 \
  -keyrange_dist_c=0.0164 \
  -keyrange_dist_d=-0.08082 \
  -keyrange_num=30 \
  -value_k=0.2615 \
  -value_sigma=25.45 \
  -iter_k=2.517 \
  -iter_sigma=14.236 \
  -mix_get_ratio=0.85 \
  -mix_put_ratio=0.14 \
  -mix_seek_ratio=0.01 \
  -sine_mix_rate_interval_milliseconds=5000 \
  -sine_a=1000 \
  -sine_b=0.000073 \
  -sine_d=4500 \
  --perf_level=2 \
  -reads=4200000 \
  -num=500000 \
  -key_size=48



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

target_file_size_base参数设置了L0层SST文件的目标大小。这是创建SST文件时的基础大小，通常用于控制最初写入的SST文件的大小。

target_file_size_multiplier参数决定了每一层SST文件大小相对于上一层的增长倍数。这意味着，如果target_file_size_multiplier大于1，每向下一层，SST文件的大小就会按照这个倍数增加。

例如，如果target_file_size_base设置为2MB，target_file_size_multiplier设置为10，那么：

L0层的SST文件大小将是2MB。
L1层的SST文件大小将是2MB * 10 = 20MB。
L2层的SST文件大小将是20MB * 10 = 200MB。
以此类推，每一层的SST文件大小都是上一层的10倍。
这种设置允许RocksDB在不同层级上存储不同大小的SST文件，从而优化读写性能和存储空间的使用。较小的SST文件可以减少读取时的查找范围，而较大的SST文件可以减少层级之间的文件数量，从而减少合并操作的复杂性。




TATISTICS:
rocksdb.memtable.hit COUNT : 1584
rocksdb.memtable.miss COUNT : 998622
rocksdb.l0.hit COUNT : 66157
rocksdb.l1.hit COUNT : 932465
rocksdb.l2andup.hit COUNT : 0
rocksdb.number.keys.written COUNT : 999794
rocksdb.number.keys.read COUNT : 1000206
rocksdb.number.keys.updated COUNT : 0
rocksdb.bytes.written COUNT : 230334553
rocksdb.bytes.read COUNT : 166897410
rocksdb.db.iter.bytes.read COUNT : 0
rocksdb.stall.micros COUNT : 0
rocksdb.flush.write.bytes COUNT : 142486367
rocksdb.number.direct.load.table.properties COUNT : 0
rocksdb.db.get.micros P50 : 348.360388 P95 : 570.535803 P99 : 799.750790 P100 : 23619.000000 COUNT : 1000206 SUM : 357943671
rocksdb.db.write.micros P50 : 4.996809 P95 : 13.169486 P99 : 21.005648 P100 : 12874.000000 COUNT : 999794 SUM : 6154183
rocksdb.table.sync.micros P50 : 2900.000000 P95 : 6082.000000 P99 : 6082.000000 P100 : 6082.000000 COUNT : 4 SUM : 15185
rocksdb.db.seek.micros P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.db.write.stall P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.sst.read.micros P50 : 132.871388 P95 : 167.479253 P99 : 211.073102 P100 : 23575.000000 COUNT : 2633256 SUM : 320612520
rocksdb.file.read.flush.micros P50 : 480.000000 P95 : 535.000000 P99 : 535.000000 P100 : 535.000000 COUNT : 4 SUM : 2014
rocksdb.file.read.get.micros P50 : 132.869519 P95 : 167.475701 P99 : 210.799460 P100 : 23575.000000 COUNT : 2633128 SUM : 320175922
rocksdb.db.flush.micros P50 : 613498.000000 P95 : 613498.000000 P99 : 613498.000000 P100 : 613498.000000 COUNT : 4 SUM : 2395689



STATISTICS:
rocksdb.memtable.hit COUNT : 1608
rocksdb.memtable.miss COUNT : 998989
rocksdb.l0.hit COUNT : 61719
rocksdb.l1.hit COUNT : 937270
rocksdb.l2andup.hit COUNT : 0
rocksdb.number.keys.written COUNT : 999403
rocksdb.number.keys.read COUNT : 1000597
rocksdb.number.keys.updated COUNT : 0
rocksdb.bytes.written COUNT : 230249576
rocksdb.bytes.read COUNT : 166900961
rocksdb.db.iter.bytes.read COUNT : 0
rocksdb.stall.micros COUNT : 0
rocksdb.flush.write.bytes COUNT : 106590346
rocksdb.number.direct.load.table.properties COUNT : 0
rocksdb.db.get.micros P50 : 394.252779 P95 : 781.481897 P99 : 943.819015 P100 : 30937.000000 COUNT : 1000597 SUM : 401473002
rocksdb.db.write.micros P50 : 5.435824 P95 : 14.995345 P99 : 27.116073 P100 : 17037.000000 COUNT : 999403 SUM : 7596556
rocksdb.table.sync.micros P50 : 5500.000000 P95 : 6692.000000 P99 : 6692.000000 P100 : 6692.000000 COUNT : 3 SUM : 13401
rocksdb.db.seek.micros P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.db.write.stall P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.sst.read.micros P50 : 140.295053 P95 : 214.703980 P99 : 338.329774 P100 : 30609.000000 COUNT : 2616922 SUM : 360677285
rocksdb.file.read.flush.micros P50 : 480.000000 P95 : 527.000000 P99 : 527.000000 P100 : 527.000000 COUNT : 3 SUM : 1466
rocksdb.file.read.get.micros P50 : 140.293307 P95 : 214.648407 P99 : 337.935962 P100 : 30609.000000 COUNT : 2616795 SUM : 360254049
rocksdb.db.flush.micros P50 : 613794.000000 P95 : 613794.000000 P99 : 613794.000000 P100 : 613794.000000 COUNT : 3 SUM : 1818778


STATISTICS:
rocksdb.memtable.hit COUNT : 0
rocksdb.memtable.miss COUNT : 9997
rocksdb.l0.hit COUNT : 327
rocksdb.l1.hit COUNT : 6205
rocksdb.l2andup.hit COUNT : 3465
rocksdb.number.keys.written COUNT : 10003
rocksdb.number.keys.read COUNT : 9997
rocksdb.number.keys.updated COUNT : 0
rocksdb.bytes.written COUNT : 1711106377
rocksdb.bytes.read COUNT : 1705036240
rocksdb.db.iter.bytes.read COUNT : 0
rocksdb.stall.micros COUNT : 25524659
rocksdb.flush.write.bytes COUNT : 965933360
rocksdb.number.direct.load.table.properties COUNT : 0
rocksdb.db.get.micros P50 : 5768.524748 P95 : 14213.387424 P99 : 20702.312373 P100 : 38221.000000 COUNT : 9997 SUM : 64888829
rocksdb.db.write.micros P50 : 53.975621 P95 : 20164.912281 P99 : 65332.386364 P100 : 119910.000000 COUNT : 10003 SUM : 26160517
rocksdb.table.sync.micros P50 : 4950.000000 P95 : 7670.000000 P99 : 7670.000000 P100 : 7670.000000 COUNT : 28 SUM : 126855
rocksdb.db.seek.micros P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.db.write.stall P50 : 27269.461078 P95 : 91112.068966 P99 : 111050.000000 P100 : 119828.000000 COUNT : 786 SUM : 25524659
rocksdb.sst.read.micros P50 : 292.498000 P95 : 786.242593 P99 : 3603.078740 P100 : 12980.000000 COUNT : 148909 SUM : 57991273
rocksdb.file.read.flush.micros P50 : 154.210526 P95 : 246.000000 P99 : 581.000000 P100 : 581.000000 COUNT : 28 SUM : 5192
rocksdb.file.read.get.micros P50 : 291.157401 P95 : 706.075529 P99 : 2281.368209 P100 : 12980.000000 COUNT : 147292 SUM : 52653197
rocksdb.db.flush.micros P50 : 475000.000000 P95 : 479154.000000 P99 : 479154.000000 P100 : 479154.000000 COUNT : 28 SUM : 12136964

STATISTICS:
rocksdb.memtable.hit COUNT : 1
rocksdb.memtable.miss COUNT : 10061
rocksdb.l0.hit COUNT : 282
rocksdb.l1.hit COUNT : 9474
rocksdb.l2andup.hit COUNT : 305
rocksdb.number.keys.written COUNT : 9938
rocksdb.number.keys.read COUNT : 10062
rocksdb.number.keys.updated COUNT : 0
rocksdb.bytes.written COUNT : 1715506466
rocksdb.bytes.read COUNT : 1740280045
rocksdb.db.iter.bytes.read COUNT : 0
rocksdb.stall.micros COUNT : 46150532
rocksdb.flush.write.bytes COUNT : 965076784
rocksdb.number.direct.load.table.properties COUNT : 0
rocksdb.db.get.micros P50 : 6862.857143 P95 : 18789.463019 P99 : 22708.888889 P100 : 38556.000000 COUNT : 10062 SUM : 76261566
rocksdb.db.write.micros P50 : 57.917905 P95 : 31969.058296 P99 : 110372.000000 P100 : 154882.000000 COUNT : 9938 SUM : 46929335
rocksdb.table.sync.micros P50 : 5342.857143 P95 : 8217.000000 P99 : 8217.000000 P100 : 8217.000000 COUNT : 28 SUM : 140833
rocksdb.db.seek.micros P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.db.write.stall P50 : 29963.800905 P95 : 137780.000000 P99 : 154460.000000 P100 : 154460.000000 COUNT : 1074 SUM : 46150532
rocksdb.sst.read.micros P50 : 316.666226 P95 : 1153.148065 P99 : 4087.920832 P100 : 19114.000000 COUNT : 149134 SUM : 69066202
rocksdb.file.read.flush.micros P50 : 166.000000 P95 : 553.333333 P99 : 1520.000000 P100 : 1520.000000 COUNT : 28 SUM : 7322
rocksdb.file.read.get.micros P50 : 315.354106 P95 : 966.351081 P99 : 3677.115385 P100 : 19114.000000 COUNT : 147632 SUM : 64024652
rocksdb.db.flush.micros P50 : 478518.518519 P95 : 567185.185185 P99 : 621520.000000 P100 : 621520.000000 COUNT : 28 SUM : 12855889

===========================================================

STATISTICS:
rocksdb.memtable.hit COUNT : 1708
rocksdb.memtable.miss COUNT : 998458
rocksdb.l0.hit COUNT : 63107
rocksdb.l1.hit COUNT : 935351
rocksdb.l2andup.hit COUNT : 0
rocksdb.number.keys.written COUNT : 999834
rocksdb.number.keys.read COUNT : 1000166
rocksdb.number.keys.updated COUNT : 0
rocksdb.bytes.written COUNT : 230348606
rocksdb.bytes.read COUNT : 166913558
rocksdb.db.iter.bytes.read COUNT : 0
rocksdb.stall.micros COUNT : 0
rocksdb.flush.write.bytes COUNT : 142487391
rocksdb.number.direct.load.table.properties COUNT : 0
rocksdb.db.get.micros P50 : 398.043281 P95 : 636.756170 P99 : 862.023156 P100 : 37009.000000 COUNT : 1000166 SUM : 388788966
rocksdb.db.write.micros P50 : 4.913389 P95 : 13.587592 P99 : 21.357197 P100 : 8704.000000 COUNT : 999834 SUM : 6338537
rocksdb.table.sync.micros P50 : 870.000000 P95 : 1436.000000 P99 : 1436.000000 P100 : 1436.000000 COUNT : 4 SUM : 4255
rocksdb.db.seek.micros P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.db.write.stall P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.sst.read.micros P50 : 139.685178 P95 : 169.088712 P99 : 248.704517 P100 : 30684.000000 COUNT : 2627686 SUM : 344717337
rocksdb.file.read.flush.micros P50 : 315.000000 P95 : 315.000000 P99 : 315.000000 P100 : 315.000000 COUNT : 4 SUM : 1215
rocksdb.file.read.get.micros P50 : 139.683587 P95 : 169.085688 P99 : 248.559963 P100 : 30684.000000 COUNT : 2627558 SUM : 344493250
rocksdb.db.flush.micros P50 : 320006.000000 P95 : 363666.000000 P99 : 363666.000000 P100 : 363666.000000 COUNT : 4 SUM : 1369759

STATISTICS:
rocksdb.memtable.hit COUNT : 1665
rocksdb.memtable.miss COUNT : 998291
rocksdb.l0.hit COUNT : 60094
rocksdb.l1.hit COUNT : 938197
rocksdb.l2andup.hit COUNT : 0
rocksdb.number.keys.written COUNT : 1000044
rocksdb.number.keys.read COUNT : 999956
rocksdb.number.keys.updated COUNT : 0
rocksdb.bytes.written COUNT : 230535540
rocksdb.bytes.read COUNT : 166800996
rocksdb.db.iter.bytes.read COUNT : 0
rocksdb.stall.micros COUNT : 0
rocksdb.flush.write.bytes COUNT : 106534538
rocksdb.number.direct.load.table.properties COUNT : 0
rocksdb.db.get.micros P50 : 387.650262 P95 : 758.408319 P99 : 867.614932 P100 : 32395.000000 COUNT : 999956 SUM : 389024610
rocksdb.db.write.micros P50 : 4.664938 P95 : 12.827385 P99 : 21.263094 P100 : 15476.000000 COUNT : 1000044 SUM : 6012279
rocksdb.table.sync.micros P50 : 1600.000000 P95 : 2007.000000 P99 : 2007.000000 P100 : 2007.000000 COUNT : 3 SUM : 4239
rocksdb.db.seek.micros P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.db.write.stall P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.sst.read.micros P50 : 139.015351 P95 : 204.801817 P99 : 322.983756 P100 : 32082.000000 COUNT : 2619857 SUM : 351056784
rocksdb.file.read.flush.micros P50 : 347.500000 P95 : 502.000000 P99 : 502.000000 P100 : 502.000000 COUNT : 3 SUM : 1139
rocksdb.file.read.get.micros P50 : 139.013583 P95 : 204.738322 P99 : 322.516462 P100 : 32082.000000 COUNT : 2619730 SUM : 350816161
rocksdb.db.flush.micros P50 : 350571.000000 P95 : 363767.000000 P99 : 363767.000000 P100 : 363767.000000 COUNT : 3 SUM : 1066421


rocksdb.memtable.hit COUNT : 0
rocksdb.memtable.miss COUNT : 10085
rocksdb.l0.hit COUNT : 145
rocksdb.l1.hit COUNT : 7037
rocksdb.l2andup.hit COUNT : 2903
rocksdb.number.keys.written COUNT : 9915
rocksdb.number.keys.read COUNT : 10085
rocksdb.number.keys.updated COUNT : 0
rocksdb.bytes.written COUNT : 1691601351
rocksdb.bytes.read COUNT : 1741057255
rocksdb.db.iter.bytes.read COUNT : 0
rocksdb.stall.micros COUNT : 3174979
rocksdb.flush.write.bytes COUNT : 929956956
rocksdb.number.direct.load.table.properties COUNT : 0
rocksdb.db.get.micros P50 : 3960.540234 P95 : 9686.284599 P99 : 13494.985955 P100 : 31033.000000 COUNT : 10085 SUM : 45071730
rocksdb.db.write.micros P50 : 46.843066 P95 : 160.964215 P99 : 15573.333333 P100 : 32271.000000 COUNT : 9915 SUM : 3720084
rocksdb.table.sync.micros P50 : 1415.384615 P95 : 2725.000000 P99 : 3263.000000 P100 : 3263.000000 COUNT : 27 SUM : 34383
rocksdb.db.seek.micros P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.db.write.stall P50 : 17911.111111 P95 : 31498.412698 P99 : 32246.000000 P100 : 32246.000000 COUNT : 172 SUM : 3174979
rocksdb.sst.read.micros P50 : 207.972209 P95 : 930.310345 P99 : 1736.997179 P100 : 27581.000000 COUNT : 132046 SUM : 40230783
rocksdb.file.read.flush.micros P50 : 155.000000 P95 : 836.166667 P99 : 924.000000 P100 : 924.000000 COUNT : 27 SUM : 6782
rocksdb.file.read.get.micros P50 : 206.988572 P95 : 823.930477 P99 : 1429.818620 P100 : 27581.000000 COUNT : 130141 SUM : 36857364
rocksdb.db.flush.micros P50 : 203043.478261 P95 : 233997.000000 P99 : 233997.000000 P100 : 233997.000000 COUNT : 27 SUM : 5141130

rocksdb.memtable.hit COUNT : 0
rocksdb.memtable.miss COUNT : 10105
rocksdb.l0.hit COUNT : 209
rocksdb.l1.hit COUNT : 8841
rocksdb.l2andup.hit COUNT : 1055
rocksdb.number.keys.written COUNT : 9895
rocksdb.number.keys.read COUNT : 10105
rocksdb.number.keys.updated COUNT : 0
rocksdb.bytes.written COUNT : 1703535400
rocksdb.bytes.read COUNT : 1733310760
rocksdb.db.iter.bytes.read COUNT : 0
rocksdb.stall.micros COUNT : 9009187
rocksdb.flush.write.bytes COUNT : 968389424
rocksdb.number.direct.load.table.properties COUNT : 0
rocksdb.db.get.micros P50 : 5939.894636 P95 : 13590.158546 P99 : 20127.019499 P100 : 44644.000000 COUNT : 10105 SUM : 63526510
rocksdb.db.write.micros P50 : 49.258667 P95 : 475.000000 P99 : 28114.705882 P100 : 49684.000000 COUNT : 9895 SUM : 9631941
rocksdb.table.sync.micros P50 : 1400.000000 P95 : 2850.000000 P99 : 3253.000000 P100 : 3253.000000 COUNT : 28 SUM : 41379
rocksdb.db.seek.micros P50 : 0.000000 P95 : 0.000000 P99 : 0.000000 P100 : 0.000000 COUNT : 0 SUM : 0
rocksdb.db.write.stall P50 : 18403.669725 P95 : 37222.580645 P99 : 47444.516129 P100 : 49590.000000 COUNT : 466 SUM : 9009187
rocksdb.sst.read.micros P50 : 243.526015 P95 : 1134.105259 P99 : 1913.884220 P100 : 20957.000000 COUNT : 151813 SUM : 56515329
rocksdb.file.read.flush.micros P50 : 204.285714 P95 : 1620.000000 P99 : 1776.000000 P100 : 1776.000000 COUNT : 28 SUM : 10641
rocksdb.file.read.get.micros P50 : 242.391868 P95 : 1001.620684 P99 : 1759.560900 P100 : 20957.000000 COUNT : 149842 SUM : 52869053
rocksdb.db.flush.micros P50 : 211481.481481 P95 : 248814.814815 P99 : 255417.000000 P100 : 255417.000000 COUNT : 28 SUM : 6200826



zone的容量除以 die的总数量   就是划分资源的最小单元   假如 4个 channel 一个channel 4个die  那么总共16个die  那么一个zone 256 MB的话  那么  相当于划分资源的最小单位就是16MB

