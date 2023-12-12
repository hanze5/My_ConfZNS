blkzone reset /dev/nvme0n1
nvme zns report-zones /dev/nvme0n1

python3 diff_bs_gen.py 

python3 diff_io_depth_gen.py 

fio fios/diff_bs.fio --output=results/diff_bs.json --output-format=json

fio fios/diff_io_depth.fio --output=results/diff_io_depth.json --output-format=json

fio fios/diff_bs.fio --output=results/diff_bs.json --output-format=json && blkzone reset /dev/nvme0n1 && fio fios/diff_io_depth.fio --output=results/diff_io_depth.json --output-format=json


python3 analyze.py results/diff_bs.json  

python3 analyze.py results/diff_io_depth.json


clear && python3 analyze.py results/mu11/diff_bs.json && python3 extract.py

clear && python3 analyze.py results/mu11/0_1_together.json && python3 extract.py

clear && python3 analyze.py results/mu11/0_2_together.json && python3 extract.py

clear && python3 analyze.py results/mu11/0_4_together.json && python3 extract.py

clear && python3 analyze.py results/mu11/0_8_together.json && python3 extract.py

clear && python3 analyze.py results/mu11/0_16_together.json && python3 extract.py

clear && python3 analyze.py results/mu11/all_together.json && python3 extract.py