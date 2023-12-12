blkzone reset /dev/nvme0n1
fio fios/0_1_write_together.fio --output=results/0_1_write_together.json --output-format=json
fio fios/0_1_read_together.fio --output=results/0_1_read_together.json --output-format=json

blkzone reset /dev/nvme0n1
fio fios/0_2_write_together.fio --output=results/0_2_write_together.json --output-format=json
fio fios/0_2_read_together.fio --output=results/0_2_read_together.json --output-format=json

blkzone reset /dev/nvme0n1
fio fios/0_4_write_together.fio --output=results/0_4_write_together.json --output-format=json
fio fios/0_4_read_together.fio --output=results/0_4_read_together.json --output-format=json

blkzone reset /dev/nvme0n1
fio fios/0_8_write_together.fio --output=results/0_8_write_together.json --output-format=json
fio fios/0_8_read_together.fio --output=results/0_8_read_together.json --output-format=json

blkzone reset /dev/nvme0n1
fio fios/0_16_write_together.fio --output=results/0_16_write_together.json --output-format=json
fio fios/0_16_read_together.fio --output=results/0_16_read_together.json --output-format=json

blkzone reset /dev/nvme0n1
fio fios/all_write_together.fio --output=results/all_write_together.json --output-format=json
fio fios/all_read_together.fio --output=results/all_read_together.json --output-format=json
