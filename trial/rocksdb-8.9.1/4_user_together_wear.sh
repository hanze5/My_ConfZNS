# blkzone reset /dev/nvme0n1

# echo mq-deadline > /sys/block/nvme0n1/queue/scheduler

# DEBUG_LEVEL=0 ROCKSDB_PLUGINS=zenfs make -j4 db_bench install

# rm -rf /home/femu/workspace/My_ConfZNS/trial/rocksdblogs/*

# ./plugin/zenfs/util/zenfs mkfs --zbd=nvme0n1 --aux_path=/home/femu/workspace/My_ConfZNS/trial/rocksdblogs --force
 
for i in {1..3}
do
  blkzone reset /dev/nvme0n1

  echo mq-deadline > /sys/block/nvme0n1/queue/scheduler

  DEBUG_LEVEL=0 ROCKSDB_PLUGINS=zenfs make -j4 db_bench install

  rm -rf /home/femu/workspace/My_ConfZNS/trial/rocksdblogs/*

  ./plugin/zenfs/util/zenfs mkfs --zbd=nvme0n1 --aux_path=/home/femu/workspace/My_ConfZNS/trial/rocksdblogs --force
 
  ./db_bench \
    --fs_uri=zenfs://dev:nvme0n1 \
    --benchmarks="mixgraph_together" \
    --statistics \
    --disable_wal \
    --use_direct_io_for_flush_and_compaction \
    -use_direct_reads=true \
    -keyrange_dist_a=24.18 \
    -keyrange_dist_b=-8.917 \
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
    --duration=7200 \
    -num=300000 \
    -key_size=48 > "output_$i.txt"

  sleep 10
done

echo "结束了"