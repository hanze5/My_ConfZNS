# 导入所需的库
import csv

# 初始化字典来存储每个ASU的最大LBA，最小LBA，读请求和写请求的数量，以及请求的大小
max_lba_per_asu = {}
min_lba_per_asu = {}
read_requests_per_asu = {}
write_requests_per_asu = {}
timestamps_per_asu = {}
read_sizes_per_asu = {}
write_sizes_per_asu = {}

file_name = "Financial1"
input_file = file_name+".spc"
output_file = file_name+"_characteristic.txt"

# 打开并读取文件
with open(input_file, 'r') as file:
    reader = csv.reader(file)
    for row in reader:
        # 分割每一行并获取ASU，LBA，大小，操作类型和时间戳
        if(len(row)!=5):
            continue
        asu, lba, size, op, timestamp = int(row[0]), int(row[1]), int(row[2]), row[3], float(row[4])
        if(size*512 >1024**3 or size==0):
            continue
        
        # 更新ASU的最大和最小LBA
        if asu not in max_lba_per_asu or lba + size > max_lba_per_asu[asu]:
            max_lba_per_asu[asu] = lba + size
        if asu not in min_lba_per_asu or lba < min_lba_per_asu[asu]:
            min_lba_per_asu[asu] = lba
        
        # 更新ASU的读写请求数量，并存储每个ASU的请求大小
        if op.lower() == 'r':
            read_requests_per_asu[asu] = read_requests_per_asu.get(asu, 0) + 1
            if asu not in read_sizes_per_asu:
                read_sizes_per_asu[asu] = [size]
            else:
                read_sizes_per_asu[asu].append(size)
        elif op.lower() == 'w':
            write_requests_per_asu[asu] = write_requests_per_asu.get(asu, 0) + 1
            if asu not in write_sizes_per_asu:
                write_sizes_per_asu[asu] = [size]
            else:
                write_sizes_per_asu[asu].append(size)
        
        # 存储每个ASU的请求时间戳
        if asu not in timestamps_per_asu:
            timestamps_per_asu[asu] = [timestamp]
        else:
            timestamps_per_asu[asu].append(timestamp)

# 定义一个函数来根据大小选择合适的单位
def format_size(size):
    # Convert size to bytes
    size_bytes = size * 512
    # If size is more than 1 GB
    if size_bytes >= (1024 ** 3):
        return f'{round(size_bytes / (1024 ** 3), 3)} GB'
    # If size is more than 1 MB
    elif size_bytes >= (1024 ** 2):
        return f'{round(size_bytes / (1024 ** 2), 3)} MB'
    # If size is more than 1 KB
    elif size_bytes >= 1024:
        return f'{round(size_bytes / 1024, 3)} KB'
    # If size is less than 1 KB
    else:
        return f'{size_bytes} bytes'

# 打开输出文件
with open(output_file, 'w') as outfile:
    # 写入每个ASU的最大LBA，最小LBA，读请求和写请求的数量，以及请求大小的最小值、最大值和平均值
    for asu in max_lba_per_asu.keys():
        outfile.write(f'ASU {asu}:\n')
        outfile.write(f'Min LBA = {min_lba_per_asu[asu]}\n')
        outfile.write(f'Max LBA = {max_lba_per_asu[asu]}\n')
        range_gb = round((max_lba_per_asu[asu] - min_lba_per_asu[asu]) * 512 / (1024 ** 3), 3)  # 1GB = 1024^3 bytes
        outfile.write(f'Range = {range_gb} GB\n')
        read_requests = read_requests_per_asu.get(asu, 0)
        write_requests = write_requests_per_asu.get(asu, 0)
        outfile.write(f'Read Requests = {read_requests}\n')
        outfile.write(f'Write Requests = {write_requests}\n')
        rw_ratio = round(read_requests / (read_requests+write_requests), 3) if write_requests != 0 else "N/A"
        outfile.write(f'Read/Write Ratio = {rw_ratio}\n')
        read_sizes = read_sizes_per_asu.get(asu, [])
        write_sizes = write_sizes_per_asu.get(asu, [])
        min_read_size = format_size(min(read_sizes)) if read_sizes else "N/A"
        max_read_size = format_size(max(read_sizes)) if read_sizes else "N/A"
        avg_read_size = format_size(sum(read_sizes) / len(read_sizes)) if read_sizes else "N/A"
        min_write_size = format_size(min(write_sizes)) if write_sizes else "N/A"
        max_write_size = format_size(max(write_sizes)) if write_sizes else "N/A"
        avg_write_size = format_size(sum(write_sizes) / len(write_sizes)) if write_sizes else "N/A"
        outfile.write(f'Min Read Request Size = {min_read_size}\n')
        outfile.write(f'Max Read Request Size = {max_read_size}\n')
        outfile.write(f'Average Read Request Size = {avg_read_size}\n')
        outfile.write(f'Min Write Request Size = {min_write_size}\n')
        outfile.write(f'Max Write Request Size = {max_write_size}\n')
        outfile.write(f'Average Write Request Size = {avg_write_size}\n')
        timestamps = sorted(timestamps_per_asu[asu])
        intervals = [timestamps[i+1] - timestamps[i] for i in range(len(timestamps)-1)]
        avg_interval = round(sum(intervals) / len(intervals), 3) if intervals else 0
        outfile.write(f'Average Request Interval = {avg_interval} seconds\n')
        outfile.write('='*50 + '\n')
