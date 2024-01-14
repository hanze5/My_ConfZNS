DEBUG_LEVEL=0 ROCKSDB_PLUGINS=zenfs make -j2 db_bench install
rm -rf /home/femu/workspace/My_ConfZNS/trial/rocksdblogs/*
./plugin/zenfs/util/zenfs mkfs --zbd=nvme0n1 --aux_path=/home/femu/workspace/My_ConfZNS/trial/rocksdblogs --force
./db_bench --num_column_families=2 --num_hot_column_families=2 --column_families_name=1st --column_family_distribution=0,100  --fs_uri=zenfs://dev:nvme0n1 --benchmarks=fillrandom --use_direct_io_for_flush_and_compaction