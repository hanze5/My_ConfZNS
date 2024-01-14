# RocksDB
RocksDB 是一个键值存储接口的存储引擎库，其中键和值是任意大小的字节流。RocksDB 将所有数据按排序顺序组织，常见的操作包括 Get（获取键对应的值）、NewIterator（创建新的迭代器）、Put（存储键值对）、Delete（删除键）和 SingleDelete（单个删除键）。
RocksDB 的三个基本构件是 memtable、sstfile 和 logfile。memtable 是一个内存中的数据结构，新的写入会插入到 memtable 中，并且可以选择性地写入到 logfile（即预写日志 WAL）。logfile 是在存储上顺序写入的文件。当 memtable 填满时，它会被刷新到存储上的 sstfile 中，相应的 logfile 可以安全地被删除。sstfile 中的数据是排序的，以便于快速查找键。这样的设计使 RocksDB 能够高效地处理键值数据，同时保证数据的持久性和一致性。

在 RocksDB 中，**列族（Column Families）**的主要思想是它们共享写前日志（WAL），但不共享内存表（memtables）和 SST 文件。通过共享写前日志，我们可以获得原子写操作的巨大好处。通过分离内存表和 SST 文件，我们能够独立配置各个列族，并快速删除它们。

每次单个列族被刷新时，我们会创建一个新的写前日志（WAL）。所有新的写操作都会进入新的 WAL。然而，我们仍然不能删除旧的 WAL，因为它包含其他列族的活动数据。只有当所有列族都被刷新并且该 WAL 中的所有数据都持久化到 SST 文件中时，我们才能删除旧的 WAL。这就产生了一些有趣的实现细节，并将产生有趣的调优需求。确保调整您的 RocksDB，使所有列族都定期刷新。另外，查看 Options::max_total_wal_size，可以配置它以便自动刷新陈旧的列族。



工作负载模拟可以：https://github.com/facebook/rocksdb/wiki/RocksDB-Trace%2C-Replay%2C-Analyzer%2C-and-Workload-Generation


参考：
https://zhuanlan.zhihu.com/p/49966056
https://zhuanlan.zhihu.com/p/618742569