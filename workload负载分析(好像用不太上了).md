```c++
#define MAX_RECORDS 10000
#define MAX_WORKLOADS 4
#define MAX_ZONES 1024

typedef struct {
    int zone;
    int size;
    char type; // 'r' for read, 'w' for write
    int workload;
    uint64_t timestamp;
} Record;



typedef struct {
    int zone;
    int count;
} ZoneCount;

typedef struct {
    ZoneCount read_zone[MAX_ZONES];
    ZoneCount write_zone[MAX_ZONES];
    int total_read_size;
    int total_write_size;
    int read_count;
    int write_count;
    int read_zone_count;
    int write_zone_count;
} Workload;

Record records[MAX_RECORDS];
Workload workloads[MAX_WORKLOADS];
int start_index = 0;
int end_index = 0;
int compare(const void* a, const void* b);
int compare(const void* a, const void* b) {
    ZoneCount* zone_count_a = (ZoneCount*)a;
    ZoneCount* zone_count_b = (ZoneCount*)b;
    return zone_count_b->count - zone_count_a->count;
}

// struct{}


void add_record(int zone, int size, char type, int workload);
void add_record(int zone, int size, char type, int workload) {
    // 获取当前时间戳
    uint64_t current_time = qemu_clock_get_ns(QEMU_CLOCK_REALTIME);

    // 如果记录已满，移除最旧的记录
    if ((end_index + 1) % MAX_RECORDS == start_index) {
        Record old_record = records[start_index];
        int old_workload = old_record.workload;
        ZoneCount* old_zone = old_record.type == 'r' ? workloads[old_workload].read_zone : workloads[old_workload].write_zone;
        int* old_zone_count = old_record.type == 'r' ? &workloads[old_workload].read_zone_count : &workloads[old_workload].write_zone_count;
        for (int i = 0; i < *old_zone_count; i++) {
            if (old_zone[i].zone == old_record.zone) {
                old_zone[i].count--;
                if (old_zone[i].count == 0) {
                    memmove(old_zone + i, old_zone + i + 1, sizeof(ZoneCount) * (*old_zone_count - i - 1));
                    (*old_zone_count)--;
                }
                break;
            }
        }
        if (old_record.type == 'r') {
            workloads[old_workload].total_read_size -= old_record.size;
            workloads[old_workload].read_count--;
        } else {
            workloads[old_workload].total_write_size -= old_record.size;
            workloads[old_workload].write_count--;
        }
        start_index = (start_index + 1) % MAX_RECORDS;
    }

    // 添加新的记录
    records[end_index].zone = zone;
    records[end_index].size = size;
    records[end_index].type = type;
    records[end_index].workload = workload;
    records[end_index].timestamp = current_time;
    
    ZoneCount* current_zone = type == 'r' ? workloads[workload].read_zone : workloads[workload].write_zone;
    int* current_zone_count = type == 'r' ? &workloads[workload].read_zone_count : &workloads[workload].write_zone_count;
    int found = 0;
    for (int i = 0; i < *current_zone_count; i++) {
        if (current_zone[i].zone == zone) {
            current_zone[i].count++;
            found = 1;
            break;
        }
    }
    if (!found) {
        current_zone[*current_zone_count].zone = zone;
        current_zone[*current_zone_count].count = 1;
        (*current_zone_count)++;
    }
    if (type == 'r') {
        workloads[workload].total_read_size += size;
        workloads[workload].read_count++;
    } else {
        workloads[workload].total_write_size += size;
        workloads[workload].write_count++;
    }
    end_index = (end_index + 1) % MAX_RECORDS;
}


void print_workload(int i) {
    printf("<=========================Workload: %d=========================>\n", i);
    printf("读取的区域访问频数:\n");
    qsort(workloads[i].read_zone, workloads[i].read_zone_count, sizeof(ZoneCount), compare);
    for (int j = 0; j < workloads[i].read_zone_count; j++) {
        printf("Zone: %d, 访问频数: %d\n", workloads[i].read_zone[j].zone, workloads[i].read_zone[j].count);
    }
    printf("写入的区域访问频数:\n");
    qsort(workloads[i].write_zone, workloads[i].write_zone_count, sizeof(ZoneCount), compare);
    for (int j = 0; j < workloads[i].write_zone_count; j++) {
        printf("Zone: %d, 访问频数: %d\n", workloads[i].write_zone[j].zone, workloads[i].write_zone[j].count);
    }
    printf("读写比例: %.2f\n", workloads[i].read_count > 0 ? (double)workloads[i].read_count /(workloads[i].read_count+ workloads[i].write_count) : 0);
    printf("Read平均Size: %.2f KB\n", workloads[i].read_count > 0 ? (double)workloads[i].total_read_size / (workloads[i].read_count*2) : 0);
    printf("Write平均Size: %.2f KB\n", workloads[i].write_count > 0 ? (double)workloads[i].total_write_size / (workloads[i].write_count*2) : 0);
    uint64_t dur = records[(end_index - 1 + MAX_RECORDS) % MAX_RECORDS].timestamp - records[start_index].timestamp; //单位是秒？
    // printf("历经时间:从 %ld 到 %ld,总共%ld\n",records[start_index].timestamp,records[(end_index - 1 + MAX_RECORDS) % MAX_RECORDS].timestamp,dur);
    if(dur>0){
        printf("Read 吞吐率: %.2f MB/s\n", workloads[i].read_count > 0 ? (double)workloads[i].total_read_size *1.0e9/ (dur*2048) : 0);
        printf("Write 吞吐率: %.2f MB/s\n", workloads[i].write_count > 0 ? (double)workloads[i].total_write_size*1.0e9/ (dur*2048) : 0);
    }
}
void print_workloads(void) {
    // 打印每个workload的访问每个zone的频数（按频数降序排序），以及读和写操作的平均size
    for (int i = 0; i < MAX_WORKLOADS; i++) {
        print_workload(i);
    }
}
 
```