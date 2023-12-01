blkzone reset /dev/nvme0n1
fio --ioengine=psync --direct=1 --filename=/dev/nvme0n1 --rw=write --bs=1m --group_reporting --zonemode=zbd --name=writeprepare --offset_increment=32z --size=32z --numjobs=4
blkzone report /dev/nvme0n1

fio isolated_fio_test.fio --output=output.json --output-format=json

python3 analyze.py output.json