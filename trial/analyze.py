import argparse
import json


parser_folder = "results/mu11/"

parser_files=[
    "0_1_write_together.json",
    "0_1_read_together.json",
    "0_2_write_together.json",
    "0_2_read_together.json",
    "0_4_write_together.json",
    "0_4_read_together.json",
    "0_8_write_together.json",
    "0_8_read_together.json",
    "0_16_write_together.json",
    "0_16_read_together.json"
    # ,
    # "all_write_together.json",
    # "all_read_together.json"
]

output = []
for parser_files in parser_files:
    full_path = parser_folder+parser_files
    with open(full_path, 'r') as f:
        data = json.load(f)
    # 访问 JSON 数据
    for job in data['jobs']:
        job_data = {}
        job_data['Job name'] = job['jobname']
        # if(job_data['Job name']=="zone0_read" or job_data['Job name']=="zone0_write"):
        #     continue
        if(job['job options']['rw']=="write" ):
            # job_data['IOPS'] = job['write']['iops']
            job_data['速率'] = job['write']['bw']/1024
            job_data['延迟'] = job['write']['lat_ns']['mean']/1000
            # job_data['99%尾延迟'] = job['write']['clat_ns']['percentile']['99.000000']/1000
            job_data['99.9%尾延迟'] = job['write']['clat_ns']['percentile']['99.900000']/1000
            # job_data['99.99%尾延迟'] = job['write']['clat_ns']['percentile']['99.990000']/1000
        elif(job['job options']['rw']=="read" or job['job options']['rw']=="randread"):
            # job_data['IOPS'] = job['read']['iops']
            job_data['速率'] = job['read']['bw']/1024
            job_data['延迟'] = job['read']['lat_ns']['mean']/1000
            # job_data['99%尾延迟'] = job['read']['clat_ns']['percentile']['99.000000']/1000
            job_data['99.9%尾延迟'] = job['read']['clat_ns']['percentile']['99.900000']/1000
            # job_data['99.99%尾延迟'] = job['read']['clat_ns']['percentile']['99.990000']/1000
        output.append(job_data)


# # 创建一个解析器
# parser = argparse.ArgumentParser(description='处理 fio 输出的 JSON 文件')
# # 添加一个命令行参数
# parser.add_argument('json_file', type=str, help='fio 输出的 JSON 文件的路径')

# # 解析命令行参数
# args = parser.parse_args()
# # 打开并读取 JSON 文件
# with open(args.json_file, 'r') as f:
#     data = json.load(f)
# output = []

# # 访问 JSON 数据
# for job in data['jobs']:
#     job_data = {}
#     job_data['Job name'] = job['jobname']
#     if(job['job options']['rw']=="write" ):
#         job_data['IOPS'] = job['write']['iops']
#         job_data['速率'] = job['write']['bw']/1024
#         job_data['延迟'] = job['write']['lat_ns']['mean']/1000
#         job_data['99%尾延迟'] = job['write']['clat_ns']['percentile']['99.000000']/1000
#         job_data['99.9%尾延迟'] = job['write']['clat_ns']['percentile']['99.900000']/1000
#         job_data['99.99%尾延迟'] = job['write']['clat_ns']['percentile']['99.990000']/1000
#     elif(job['job options']['rw']=="read" or job['job options']['rw']=="randread"):
#         job_data['IOPS'] = job['read']['iops']
#         job_data['速率'] = job['read']['bw']/1024
#         job_data['延迟'] = job['read']['lat_ns']['mean']/1000
#         job_data['99%尾延迟'] = job['read']['clat_ns']['percentile']['99.000000']/1000
#         job_data['99.9%尾延迟'] = job['read']['clat_ns']['percentile']['99.900000']/1000
#         job_data['99.99%尾延迟'] = job['read']['clat_ns']['percentile']['99.990000']/1000
#     output.append(job_data)


# 将结果保存为 JSON 文件
with open('output.json', 'w') as f:
    json.dump(output, f, ensure_ascii=False, indent=4)