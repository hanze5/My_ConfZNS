def parse_results(file_path, fields):
    results = {}
    with open(file_path, 'r') as file:
        for line in file:
            # 去除行尾的换行符
            line = line.strip()
            # 分割行以获取统计信息的名称和值
            name, values = line.split(' ', 1)
            # 如果这个名称不在你感兴趣的字段中，就跳过这一行
            if name not in fields:
                continue
            # 进一步分割值字符串以获取各个统计值
            values = values.split(' ')
            # 创建一个字典来存储统计值
            value_dict = {}
            for value in values:
                # 分割统计值字符串以获取统计值的名称和数值
                value_name, value_number = value.split(' : ')
                # 将统计值添加到字典中
                value_dict[value_name] = float(value_number)
            # 将统计信息添加到结果字典中
            results[name] = value_dict
    return results

# 使用函数
fields = ['rocksdb.db.get.micros', 'rocksdb.db.write.micros']  # 你感兴趣的字段
results = parse_results('path_to_your_file.txt', fields)
print(results)
