#!/bin/bash

# 启动第一个 db_bench 命令
./db_bench --fs_uri=zenfs://dev:nvme0n1 --db=1st --statistics --disable_wal --benchmarks=fillrandom --use_direct_io_for_flush_and_compaction --num=2000000 > output1.txt &

# 启动第二个 db_bench 命令
./db_bench --fs_uri=zenfs://dev:nvme0n1 --db=2nd --statistics --disable_wal --benchmarks=fillrandom --use_direct_io_for_flush_and_compaction --num=2000000 > output2.txt&

# 等待所有后台进程完成
wait