operations = ['write', 'read', 'randread']  # 操作类型
ioengines = {'write': 'psync', 'read': 'psync', 'randread': 'io_uring'}  # I/O引擎

with open('diff_io_depth.fio', 'w') as f:
    f.write("[global]\n")
    f.write("direct=1\n")
    f.write("zonemode=zbd\n")
    f.write("name=diff_io_depth\n")
    f.write(f"bs={1}m\n")
    f.write("size=16z\n")
    f.write("filename=/dev/nvme0n1\n")
    f.write("\n")

    offset = 0
    for i in range(6):
        for op in operations:
            f.write(f"[{2**i}depth_{op}]\n")
            f.write(f"ioengine={ioengines[op]}\n")
            f.write(f"iodepth={2**i}\n")
            f.write(f"rw={op}\n")
            f.write(f"offset={offset}z\n")
            if op != 'write':
                f.write("time_based\n")
                f.write("runtime=20\n")
            f.write("stonewall\n")
            f.write("\n")
        offset += 16