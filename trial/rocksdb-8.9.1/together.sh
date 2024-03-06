DEBUG_LEVEL=0 ROCKSDB_PLUGINS=zenfs make -j2 db_bench install
echo mq-deadline > /sys/block/nvme0n1/queue/scheduler
rm -rf /home/femu/workspace/My_ConfZNS/trial/rocksdblogs/*
./plugin/zenfs/util/zenfs mkfs --zbd=nvme0n1 --aux_path=/home/femu/workspace/My_ConfZNS/trial/rocksdblogs --force
./db_bench --fs_uri=zenfs://dev:nvme0n1 -use_direct_io_for_flush_and_compaction=true --disable_wal --min_level_to_compress=0 -benchmarks=fillseq -num=655360 -key_size=48 -value_size=32720 >  output.txt

sleep 10

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
  -sine_a=100 \
  -sine_b=0.000073 \
  -sine_d=450 \
  --duration=600 \
  -num=655360 \
  -key_size=48 > outputtogether.txt


echo "结束了"