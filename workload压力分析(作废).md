```c++
typedef struct {
    // int die_idx;
    double Response_Ratio;
    uint64_t timestamp;
} Record;

// 用于保存单个重置记录的结构体
typedef struct {
    int zone;
    int count;
} ResetRecord;

typedef struct {
    Record records[MAX_RECORDS];
    int start_index;
    int end_index;

    double avg_response_ratio;  // 新增的变量
    int record_count;           // 新增的变量，用于跟踪实际的记录数量

    uint16_t *local_dies_for_workload;
} Workload;

Workload workloads[MAX_WORKLOADS];

uint64_t add_count[MAX_WORKLOADS];

uint64_t total_count = 0;

// 全局的重置记录数组
ResetRecord reset_records[MAX_RESET_RECORDS];
// 用于跟踪重置记录的起始和结束索引
int reset_start_index = 0;
int reset_end_index = 0;
// 用于跟踪实际的重置记录数量
int reset_count = 0;


void add_request_record(int i,  double Response_Ratio) {
    // 创建一个新的记录
    add_count[i]++;
    if(total_count++>=10000){
        printf("    workload 压力情况:");
        for(int j = 0;j<MAX_WORKLOADS;j++)
        {
            printf("Workload%d: %.3f |",j,workloads[j].avg_response_ratio);
        }
        printf("\n");
        total_count = 0;
    }
    uint64_t current_time = qemu_clock_get_ns(QEMU_CLOCK_REALTIME);
    Record new_record;
    // new_record.die_idx = die_idx;
    new_record.Response_Ratio = Response_Ratio;
    new_record.timestamp = current_time;

    int start_index = workloads[i].start_index;
    int end_index = workloads[i].end_index;

    // 如果循环队列已满，那么 start_index 也需要向前移动，并从平均值中减去被替换的记录的响应比
    if (workloads[i].record_count==MAX_RECORDS) {
        workloads[i].avg_response_ratio -= workloads[i].records[start_index].Response_Ratio / workloads[i].record_count;
        workloads[i].start_index = (workloads[i].start_index + 1) % MAX_RECORDS;
    } else {
        // 如果循环队列未满，那么记录数量增加
        workloads[i].record_count++;
    }

    // 将新的记录添加到循环队列中
    workloads[i].records[end_index] = new_record;

    // 更新 end_index
    workloads[i].end_index = (workloads[i].end_index + 1) % MAX_RECORDS;

    // 更新平均响应比
    workloads[i].avg_response_ratio += (new_record.Response_Ratio - workloads[i].avg_response_ratio) / workloads[i].record_count;
}

// 重置区的函数
void reset_zone(int zone) {
    // ... 其他重置逻辑 ...

    // 更新重置记录
    if (reset_count < MAX_RESET_RECORDS) {
        // 如果还没有达到最大记录数，直接添加新记录
        reset_records[reset_end_index].zone = zone;
        reset_records[reset_end_index].count++;
        reset_end_index = (reset_end_index + 1) % MAX_RESET_RECORDS;
        reset_count++;
    } else {
        // 如果已经达到最大记录数，需要删除最早的记录，然后添加新记录
        reset_records[reset_start_index].zone = zone;
        reset_records[reset_start_index].count++;
        reset_start_index = (reset_start_index + 1) % MAX_RESET_RECORDS;
    }
}

// 比较函数，用于按照重置次数进行降序排序
int compare(const void* a, const void* b) {
    ResetRecord* record_a = (ResetRecord*)a;
    ResetRecord* record_b = (ResetRecord*)b;
    return record_b->count - record_a->count;
}

// 打印每个区的重置次数
void print_zone_reset_counts() {
    // 对数组进行排序
    qsort(reset_records, MAX_RESET_RECORDS, sizeof(ResetRecord), compare);

    // 打印排序后的结果
    printf("Zone reset 次数\n");
    for (int i = 0; i < MAX_RESET_RECORDS; i++) {
        printf("Zone %d: %d resets\n", reset_records[i].zone, reset_records[i].count);
    }
}

void print_workload_info(int i) {
    Workload workload = workloads[i];
    printf("<=========================Workload: %d=========================>\n", i);

    // printf("Start Index: %d\n", workload.start_index);
    // printf("End Index: %d\n", workload.end_index);
    printf("Average Response Ratio: %.2f\n", workload.avg_response_ratio);
    // printf("Record Count: %d\n", workload.record_count);

    printf("Records:\n");
    int index = workload.start_index;
    int count = 0;
    while (count++<workload.record_count) {
        printf("Record %d:  Response Ratio: %.2f, Timestamp: %llu\n",
               index, workload.records[index].Response_Ratio,workload.records[index].timestamp);
        index = (index + 1) % MAX_RECORDS;
    }

    printf("Local Dies for Workload:\n");
    for (int j = 0; j < workload.record_count; j++) {
        printf("Die %d: %u", j, workload.local_dies_for_workload[j]);
    }
    printf("\n");
}

```