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
nvme zns report-zones /dev/nvme0n1

DEBUG_LEVEL=0 ROCKSDB_PLUGINS=zenfs make clean
export CXXFLAGS="-I/usr/include -L/usr/lib/x86_64-linux-gnu"
DEBUG_LEVEL=0 ROCKSDB_PLUGINS=zenfs make -j2 db_bench install

rm -rf /home/femu/workspace/My_ConfZNS/trial/rocksdblogs/*


cd plugin/zenfs/util
make


echo mq-deadline > /sys/block/nvme0n1/queue/scheduler
./plugin/zenfs/util/zenfs mkfs --zbd=nvme0n1 --aux_path=/home/femu/workspace/My_ConfZNS/trial/rocksdblogs --force
./plugin/zenfs/util/zenfs ls-uuid
./plugin/zenfs/util/zenfs list --zbd=nvme0n1
./plugin/zenfs/util/zenfs fs-info --zbd=nvme0n1
./plugin/zenfs/util/zenfs df --zbd=nvme0n1 
./plugin/zenfs/util/zenfs backup --zbd=nvme0n1 

./db_bench --num_column_families=2 --num_hot_column_families=2 --column_families_name=1st --column_family_distribution=0,100  --fs_uri=zenfs://dev:nvme0n1 --benchmarks=fillrandom --use_direct_io_for_flush_and_compaction


./db_bench \
  --fs_uri=zenfs://dev:nvme0n1\
  --statistics\
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
  -reads=42000000 \
  -num=5000000 \
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





