# RocksDB
RocksDB 是一个键值存储接口的存储引擎库，其中键和值是任意大小的字节流。RocksDB 将所有数据按排序顺序组织，常见的操作包括 Get（获取键对应的值）、NewIterator（创建新的迭代器）、Put（存储键值对）、Delete（删除键）和 SingleDelete（单个删除键）。
RocksDB 的三个基本构件是 memtable、sstfile 和 logfile。memtable 是一个内存中的数据结构，新的写入会插入到 memtable 中，并且可以选择性地写入到 logfile（即预写日志 WAL）。logfile 是在存储上顺序写入的文件。当 memtable 填满时，它会被刷新到存储上的 sstfile 中，相应的 logfile 可以安全地被删除。sstfile 中的数据是排序的，以便于快速查找键。这样的设计使 RocksDB 能够高效地处理键值数据，同时保证数据的持久性和一致性。

在 RocksDB 中，**列族（Column Families）**的主要思想是它们共享写前日志（WAL），但不共享内存表（memtables）和 SST 文件。通过共享写前日志，我们可以获得原子写操作的巨大好处。通过分离内存表和 SST 文件，我们能够独立配置各个列族，并快速删除它们。

每次单个列族被刷新时，我们会创建一个新的写前日志（WAL）。所有新的写操作都会进入新的 WAL。然而，我们仍然不能删除旧的 WAL，因为它包含其他列族的活动数据。只有当所有列族都被刷新并且该 WAL 中的所有数据都持久化到 SST 文件中时，我们才能删除旧的 WAL。这就产生了一些有趣的实现细节，并将产生有趣的调优需求。确保调整您的 RocksDB，使所有列族都定期刷新。另外，查看 Options::max_total_wal_size，可以配置它以便自动刷新陈旧的列族。

在RocksDB中，每个列族（Column Family）都有自己的Memtable12。所有的写操作都是在Memtable进行，当Memtable空间不足时，会创建一块新的Memtable来继续接收写操作，原先的内存将被标识为只读模式，等待被刷入SST文件1。在任何时刻，一个列族中，只存在一块活动的Memtable和0或多块只读的Memtable1。

刷入SST文件的时机由以下三个条件来确定1：

write_buffer_size设置一块Memtable的容量,一旦写满，就标记为只读，然后创建一块新的1。
max_write_buffer_number设置Memtable的最大存在数 (活动的和只读的共享)，一旦活动的Memtable被写满了，并且Memtable的数量大于max_write_buffer_number, 此时会阻塞写操作1。
min_write_buffer_number_to_merge设置刷入SST文件之前，最小可合并的Memtable数1。

假设我们有以下设置：

write_buffer_size = 64MB
max_write_buffer_number = 3
memtable_memory_budget = 128MB
当你向RocksDB写入数据时，数据首先会被写入到活动的Memtable中。当活动的Memtable的大小达到write_buffer_size（64MB）时，它会变成只读，然后创建一个新的Memtable来接收新的写入。

此时，我们有一个活动的Memtable和一个只读的Memtable，总共占用了128MB的内存，这已经达到了memtable_memory_budget的限制。然后，如果你继续写入数据，新的Memtable在达到64MB之前，就会因为超过了memtable_memory_budget而被刷入磁盘。这就是memtable_memory_budget参数的作用。

max_bytes_for_level_base和max_bytes_for_level_multiplier 也需要注意！！！

参考：
https://zhuanlan.zhihu.com/p/49966056
https://zhuanlan.zhihu.com/p/618742569


#dbbench修改 

```c++
//运行某一个benchmark
void Run()
```

在这里可以创建多个db 重点是OpenDb
```c++
if (FLAGS_num_multi_db <= 1) {
    OpenDb(options, FLAGS_db, &db_);
} else {
    multi_dbs_.clear();
    multi_dbs_.resize(FLAGS_num_multi_db);
    auto wal_dir = options.wal_dir;
    for (int i = 0; i < FLAGS_num_multi_db; i++) {
    if (!wal_dir.empty()) {
        options.wal_dir = GetPathForMultiple(wal_dir, i);
    }
    OpenDb(options, GetPathForMultiple(FLAGS_db, i), &multi_dbs_[i]);
    }
    options.wal_dir = wal_dir;
}
```

在OpenDb中,我们默认 每个db只用默认列族所以 就是用的Open函数
```c++
...
} else {
    s = DB::Open(options, db_name, &db->db);
}
...
//实现 如下
Status DB::Open(const Options& options, const std::string& dbname, DB** dbptr) {
  DBOptions db_options(options);
  ColumnFamilyOptions cf_options(options);
  std::vector<ColumnFamilyDescriptor> column_families;
  column_families.push_back(
      ColumnFamilyDescriptor(kDefaultColumnFamilyName, cf_options));
  if (db_options.persist_stats_to_disk) {
    column_families.push_back(
        ColumnFamilyDescriptor(kPersistentStatsColumnFamilyName, cf_options));
  }
  std::vector<ColumnFamilyHandle*> handles;
  Status s = DB::Open(db_options, dbname, column_families, &handles, dbptr);
  if (s.ok()) {
    if (db_options.persist_stats_to_disk) {
      assert(handles.size() == 2);
    } else {
      assert(handles.size() == 1);
    }
    // i can delete the handle since DBImpl is always holding a reference to
    // default column family
    if (db_options.persist_stats_to_disk && handles[1] != nullptr) {
      delete handles[1];
    }
    delete handles[0];
  }
  return s;
}
```

```c++ 从上面的代码中我们可以提取出  两点  也就是要看我们要修改的参数  是DBOptions  里面的还是 ColumnFamilyOptions里面的了
  column_families.push_back(
      ColumnFamilyDescriptor(kDefaultColumnFamilyName, cf_options));

  Status s = DB::Open(db_options, dbname, column_families, &handles, dbptr);
```



Stats stats = RunBenchmark(num_threads, name, method);


调用关系main->Run->Open->OpenDb
                ->RunBenchmark

在rocksdb-8.9.1/include/rocksdb/options.h中我们可以得知
```c++

  ColumnFamilyOptions* OptimizeLevelStyleCompaction(
      uint64_t memtable_memory_budget = 512 * 1024 * 1024);

ColumnFamilyOptions::write_buffer_size   

/*在RocksDB中，每个数据库实例（DB）都有自己的默认列族（Default Column Family）12。默认情况下，所有的记录都会存储在这个默认列族里12。

因此，如果你有两个RocksDB数据库实例，那么每个数据库实例都会有自己的默认列族。这两个默认列族是独立的，它们不会共享SST文件或Memtable12。所以，每个默认列族的level-0都可以有最多4个SST文件（假设level0_file_num_compaction_trigger设置为默认值4）。这意味着总共可以有8个level-0的SST文件，但这8个文件是分布在两个数据库实例中的。*/
ColumnFamilyOptions::level0_file_num_compaction_trigger = 4

uint64_t max_bytes_for_level_base = 256 * 1048576;


//DBOptions里    
std::shared_ptr<RateLimiter> rate_limiter = nullptr;  //速率限制

uint32_t max_subcompactions = 1;//执行压缩任务的最大线程数

size_t manifest_preallocation_size = 4 * 1024 * 1024; 好像可以设置的再大一点 

// Use O_DIRECT for user and compaction reads.
// Default: false
bool use_direct_reads = false;

// Use O_DIRECT for writes in background flush and compactions.
// Default: false
bool use_direct_io_for_flush_and_compaction = false;

uint64_t max_write_batch_group_size_bytes = 1 << 20;  //似乎是写入磁盘的限制

//ReadOptions里


//WriteOptions里

```

正弦函数最适合QPS。F(x)=A∗sin(B∗x+C)+D  

示例：


dbstats是如何进行？

首先进行定义声明:
```c++
static class std::shared_ptr<ROCKSDB_NAMESPACE::Statistics> dbstats;
>>
std::vector<std::shared_ptr<ROCKSDB_NAMESPACE::Statistics>> multi_dbstats(4);
```


结果时候：
```c++
fprintf(stdout, "STATISTICS:\n%s\n", dbstats->ToString().c_str());
>>
for(auto& item : multi_dbstats){
  fprintf(stdout, "STATISTICS:\n%s\n", item->ToString().c_str());
}
```

在初始化参数的时候：  这个应该不用改  options 是在 OpenDb 被当作参数传进去的
```c++
if (options.statistics == nullptr) {
  options.statistics = dbstats;
}
```

main函数里：
```c++
  if (!FLAGS_statistics_string.empty()) {
    Status s = Statistics::CreateFromString(config_options,
                                            FLAGS_statistics_string, &dbstats);
    if (dbstats == nullptr) {
      fprintf(stderr,
              "No Statistics registered matching string: %s status=%s\n",
              FLAGS_statistics_string.c_str(), s.ToString().c_str());
      exit(1);
    }
  }
  if (FLAGS_statistics) {
    dbstats = ROCKSDB_NAMESPACE::CreateDBStatistics();
  }
  if (dbstats) {
    dbstats->set_stats_level(static_cast<StatsLevel>(FLAGS_stats_level));
  }
```
>>
```c++
  if (!FLAGS_statistics_string.empty()) {
    Status s = Statistics::CreateFromString(config_options,
                                            FLAGS_statistics_string, &dbstats);
    if (dbstats == nullptr) {
      fprintf(stderr,
              "No Statistics registered matching string: %s status=%s\n",
              FLAGS_statistics_string.c_str(), s.ToString().c_str());
      exit(1);
    }
  }
  if (FLAGS_statistics) {
    dbstats = ROCKSDB_NAMESPACE::CreateDBStatistics();
  }
  if (dbstats) {
    dbstats->set_stats_level(static_cast<StatsLevel>(FLAGS_stats_level));
  }
```

  void OpenDb(Options options, const std::string& db_name,
              DBWithColumnFamilies* db)  函数是按值传递  可以修改各自的options



  DBOptions           有statistics
  ColumnFamilyOptions 也有