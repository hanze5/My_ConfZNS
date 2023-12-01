import argparse
import json

# 创建一个解析器
parser = argparse.ArgumentParser(description='处理 fio 输出的 JSON 文件')
# 添加一个命令行参数
parser.add_argument('json_file', type=str, help='fio 输出的 JSON 文件的路径')

# 解析命令行参数
args = parser.parse_args()

# 打开并读取 JSON 文件
with open(args.json_file, 'r') as f:
    data = json.load(f)

# 访问 JSON 数据
for job in data['jobs']:
    print("============================================================================")
    print('Job name:', job['jobname'])
    if(job['job options']['rw']=="write" ):
        print('Write IOPS:', job['write']['iops'])
        print(f"      写速率: {job['write']['bw']/1024} MB/s")
        print(f"      写延迟: {job['write']['lat_ns']['mean']/1000}us")
        print(f"   99%尾延迟: {job['write']['clat_ns']['percentile']['99.000000']/1000} us" )
        print(f" 99.9%尾延迟: {job['write']['clat_ns']['percentile']['99.900000']/1000} us")
        print(f"99.99%尾延迟: {job['write']['clat_ns']['percentile']['99.990000']/1000} us")
    elif(job['job options']['rw']=="read" or job['job options']['rw']=="randread"):
        print('Read IOPS:', job['read']['iops'])
        print(f"      读速率: {job['read']['bw']/1024} MB/s")
        print(f"      读延迟: {job['read']['lat_ns']['mean']/1000}us")
        print(f"   99%尾延迟: {job['read']['clat_ns']['percentile']['99.000000']/1000} us" )
        print(f" 99.9%尾延迟: {job['read']['clat_ns']['percentile']['99.900000']/1000} us")
        print(f"99.99%尾延迟: {job['read']['clat_ns']['percentile']['99.990000']/1000} us")

