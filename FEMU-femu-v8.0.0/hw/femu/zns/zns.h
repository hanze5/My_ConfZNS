#ifndef __FEMU_ZNS_H
#define __FEMU_ZNS_H

#include "../nvme.h"

#define _64KB   (64 * KiB)
#define _16KB   (16 * KiB)
#define _4KB    (4 * KiB)
// #define RESOURCE_UTIL_LOG

//定义了各项操作的实验
enum {
    NAND_READ =  0,
    NAND_WRITE = 1,
    NAND_ERASE = 2,

 /* FIXME: */
    NAND_READ_LATENCY  = 65000,//NAND_READ_LATENCY  = 65000,  //65us TLC_tREAD(65us : 16K page time)
    NAND_PROG_LATENCY  = 450000,//450us TLC_tProg ,3D time
    NAND_ERASE_LATENCY = SLC_BLOCK_ERASE_LATENCY_NS,//2000000
    NAND_CHNL_PAGE_TRANSFER_LATENCY = 25000, // =2.5? 1200MT = 9600MB/s = 390ns per 4K 
    //NAND_CHNL_PAGE_TRANSFER_LATENCY = 0,
    //SK Hynix read     : 400Mb/s for 1 chip..
    //WD ZN540 4TB read : avg 80us 
    //ZEMU read         : 
    //SK Hynix write    : 100Mb/s for 1 chip..
    //WD ZN540 4TB write: 
    //ZEMU write        : 5Mb/s for 1 chip...

    ZONE_RESET_LATENCY =  SLC_BLOCK_ERASE_LATENCY_NS,
};

//dz added
#define MAX_RESET_RECORDS 1000
#define MAX_RECORDS 10000
#define MAX_WORKLOADS 4
#define MAX_ZONES 1024

uint64_t last_time=0;

/**
 * @brief 
 * inhoinno: to implement controller-level zone mapping in zns ssd, 
 * struct ssd_channel is neccesary
 * so simply extends 'struct ssd_channel' in ../bbssd/ftl.h:102
 * 用于实现zone映射以及时延
 */
typedef struct zns_ssd_channel {
    int nzones;                  //包含的区域数？
    uint64_t next_ch_avail_time; //下一个可用时间
    pthread_spinlock_t time_lock;//访问控制锁
    bool busy;                   //是否繁忙

    uint64_t transfer_time;
}zns_ssd_channel;

typedef struct zns_ssd_plane {
    uint64_t next_avail_time;   
    uint64_t nregs;              //表示寄存器数量？
    bool *is_reg_busy;           //bool类型数组 表示寄存器是否繁忙

    uint64_t rw_time;
}zns_ssd_plane;

/**
 * @brief 
 * inhoinno: to implement Multi way in ZNS, ssd_chip structure is required.
 * This zns_ssd_chip structure follows 'struct ssd_lun' in ../bbssd/ftl.h:94
 * but differnce is ZNS does not do ftl jobs such as badblock management, GC 
 * so this structure only contains minimum fields
 * 也就是说这实际上就是chip
 */
typedef struct zns_ssd_lun {
    uint64_t next_avail_time; // in nanoseconds
    pthread_spinlock_t time_lock;
}zns_ssd_lun;

/**
 * @brief 
 * dz modified
 */
typedef struct zns_ssd_die {
    uint64_t next_avail_time; // in nanoseconds
    pthread_spinlock_t time_lock;

    uint32_t *blkgrps_in_die; //die用于维护自身blkgrp idx

}zns_ssd_die;

/**
 * @brief 
 * dz added
 */
typedef struct zns_ssd_blkgrp {
    uint16_t id;   
    bool     is_being_used;
    uint8_t  belong_2_die;
    uint16_t erase_cnt;
    uint64_t bsla;
    uint64_t bela;

}zns_ssd_blkgrp;



/**
 * @brief 
 * inhoinno: to emulate latency in zns ssd, struct znsssd is needed
 * extends 'struct ssdparams' in ../bbssd/ftl.h:110
 */
struct zns_ssdparams{
    uint16_t register_model;    /* =1 single register =2 double register */
    uint64_t nchnls;            /* # of channels in the SSD */
    uint64_t ways;              /* # of ways in the SSD */
    uint64_t zones;             /* # of zones in ZNS SSD */
    uint64_t chnls_per_zone;    /* ZNS Association degree. # of channels per zone, must be divisor of nchnls */
    uint64_t ways_per_zone;     /* another ZNS Association degree. # of ways per zone, must be divisor of nways */
    uint64_t dies_per_chip;     
    uint64_t planes_per_die;      
    uint64_t csze_pages;        /* #of Pages in Chip (Inhoinno:I guess lun in femu)意思是芯片中的页面数？*/
    uint64_t nchips;            /* # of chips in SSD*/
    bool     is_another_namespace;
    uint64_t chnls_per_another_zone;    
    uint64_t pg_rd_lat;         /* NAND page read latency in nanoseconds 页的读时延*/
    uint64_t pg_wr_lat;         /* NAND page program latency in nanoseconds 页的写时延*/
    uint64_t blk_er_lat;        /* NAND block erase latency in nanoseconds 块擦除时延*/
    uint64_t zone_reset_lat;    /* ZNS SSD ZONE reset latency in nanoseconds 把一个zonereset的时延*/
    uint64_t ch_xfer_lat;       /* channel transfer latency for one page in nanoseconds 在通道上传输的时延*/
};



// 用于保存单个重置记录的结构体
typedef struct {
    int zone;
    int count;
} ResetRecord;

typedef struct {
    uint16_t *local_dies_for_workload;
    double pressure;
} Workload;

Workload workloads[MAX_WORKLOADS];


// 全局的重置记录数组
ResetRecord reset_records[MAX_RESET_RECORDS];
// 用于跟踪重置记录的起始和结束索引
int reset_start_index = 0;
int reset_end_index = 0;
// 用于跟踪实际的重置记录数量
int reset_count = 0;

uint64_t add_count[MAX_WORKLOADS];


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


enum RecourseAllocateType{
    CONFZNS                 = 0x00,

    STATIC_HORIZONTAL_FIRST = 0x01,
    STATIC_VERTICAL_FIRST   = 0x02,

    STATIC_MANUAL4411       = 0x03,
    STATIC_MANUAL4422       = 0x04,

    DYNAMIC                 = 0x05,

};


/**
 * @brief 
 * inhoinno: latency emulation with zns ssd, struct znsssd is needed
 * extends 'struct ssd' in ../bbssd/ftl.h:197 
 * 从该结构可以看出way应该是逻辑上的概念吧
 * dz modified
 * 增加了
 */
typedef struct zns {
    /*members from struct ssd*/
    char                *ssdname;              //ssd名称
    struct zns_ssdparams    sp;                //物理结构参数包括通道数 芯片数 以及分布情况 以及时延情况等
    struct zns_ssd_channel *ch;                //通道
    struct zns_ssd_lun *chips;                 //芯片
    struct zns_ssd_die *dies;                  //中间似乎还有一个die 不过这里没写  正常的物理结构是 channel->chip->die->plane->block->page
    struct zns_ssd_plane *planes;              //平面

    uint16_t ** dz_unit_allocate;
    uint16_t ** dz_unit_using;


    uint64_t blkgrp_size;
    struct zns_ssd_blkgrp *blkgrps;


    enum RecourseAllocateType allocateType;

    uint64_t dz_unit_size;                     //暂时没想好以什么作为单位 应该以逻辑块为单位 也就是512B

    /*new members for znsssd*/
    struct NvmeNamespace    * namespaces;      //FEMU only support 1 namespace For now, 
    struct NvmeZone      * zone_array;         //zone数组
    uint32_t            num_zones;             //zone总数
    /* lockless ring for communication with NVMe IO thread 这个不知说的是什么似乎并没有实现啊*/

    QemuThread          zns_thread;

}ZNS;

typedef struct QEMU_PACKED NvmeZonedResult {
    uint64_t slba;
} NvmeZonedResult;

typedef struct NvmeIdCtrlZoned {
    uint8_t     zasl;
    uint8_t     rsvd1[4095];
} NvmeIdCtrlZoned;

enum NvmeZoneAttr {
    NVME_ZA_FINISHED_BY_CTLR         = 1 << 0,
    NVME_ZA_FINISH_RECOMMENDED       = 1 << 1,
    NVME_ZA_RESET_RECOMMENDED        = 1 << 2,
    NVME_ZA_ZD_EXT_VALID             = 1 << 7,
};

typedef struct QEMU_PACKED NvmeZoneReportHeader {
    uint64_t    nr_zones;
    uint8_t     rsvd[56];
} NvmeZoneReportHeader;

enum NvmeZoneReceiveAction {
    NVME_ZONE_REPORT                 = 0,
    NVME_ZONE_REPORT_EXTENDED        = 1,
};

enum NvmeZoneReportType {
    NVME_ZONE_REPORT_ALL             = 0,
    NVME_ZONE_REPORT_EMPTY           = 1,
    NVME_ZONE_REPORT_IMPLICITLY_OPEN = 2,
    NVME_ZONE_REPORT_EXPLICITLY_OPEN = 3,
    NVME_ZONE_REPORT_CLOSED          = 4,
    NVME_ZONE_REPORT_FULL            = 5,
    NVME_ZONE_REPORT_READ_ONLY       = 6,
    NVME_ZONE_REPORT_OFFLINE         = 7,
};

enum NvmeZoneType {
    NVME_ZONE_TYPE_RESERVED          = 0x00,
    NVME_ZONE_TYPE_SEQ_WRITE         = 0x02,
};

enum NvmeZoneSendAction {
    NVME_ZONE_ACTION_RSD             = 0x00,
    NVME_ZONE_ACTION_CLOSE           = 0x01,
    NVME_ZONE_ACTION_FINISH          = 0x02,
    NVME_ZONE_ACTION_OPEN            = 0x03,
    NVME_ZONE_ACTION_RESET           = 0x04,
    NVME_ZONE_ACTION_OFFLINE         = 0x05,
    NVME_ZONE_ACTION_SET_ZD_EXT      = 0x10,
};



typedef struct QEMU_PACKED NvmeZoneDescr {
    uint8_t     zt;
    uint8_t     zs;
    uint8_t     za;
    uint8_t     rsvd3[4];

    bool is_mapped;

    uint64_t    zcap;
    uint64_t    zslba;
    uint64_t    wp;
    uint8_t     rsvd32[16];

    uint16_t   *local_dies;     //计算延迟时用的
    uint32_t   *local_blkgrps;   //用于管理磨损  
} NvmeZoneDescr;

typedef enum NvmeZoneState {
    NVME_ZONE_STATE_RESERVED         = 0x00,
    NVME_ZONE_STATE_EMPTY            = 0x01,
    NVME_ZONE_STATE_IMPLICITLY_OPEN  = 0x02,
    NVME_ZONE_STATE_EXPLICITLY_OPEN  = 0x03,
    NVME_ZONE_STATE_CLOSED           = 0x04,
    NVME_ZONE_STATE_READ_ONLY        = 0x0D,
    NVME_ZONE_STATE_FULL             = 0x0E,
    NVME_ZONE_STATE_OFFLINE          = 0x0F,
} NvmeZoneState;

#define NVME_SET_CSI(vec, csi) (vec |= (uint8_t)(1 << (csi)))

typedef struct QEMU_PACKED NvmeLBAFE {
    uint64_t    zsze;
    uint8_t     zdes;
    uint8_t     rsvd9[7];
} NvmeLBAFE;

typedef struct QEMU_PACKED NvmeIdNsZoned {
    uint16_t    zoc;
    uint16_t    ozcs;
    uint32_t    mar;
    uint32_t    mor;
    uint32_t    rrl;
    uint32_t    frl;
    uint8_t     rsvd20[2796];
    NvmeLBAFE   lbafe[16];
    uint8_t     rsvd3072[768];
    uint8_t     vs[256];
} NvmeIdNsZoned;

typedef struct NvmeZone {
    NvmeZoneDescr   d;
    uint64_t        w_ptr;
    
    uint64_t        cnt_reset;      //新加入 统计重置次数？
    pthread_spinlock_t w_ptr_lock;  //新加更新写指针锁

    //
    QTAILQ_ENTRY(NvmeZone) entry;
} NvmeZone;

typedef struct NvmeNamespaceParams {
    uint32_t nsid;                  //命名空间标识符
    QemuUUID uuid;                  //通用唯一标识符

    bool     zoned;                 //布尔值，表示命名空间是否分区
    bool     cross_zone_read;       //布尔值，表示是否允许跨区域读取
    uint64_t zone_size_bs;          //表示区域大小（以字节为单位）
    uint64_t zone_cap_bs;           //表示区域容量（以字节为单位）
    uint32_t max_active_zones;      //最大活动区域数
    uint32_t max_open_zones;        //最大打开区域数
    uint32_t zd_extension_size;     //区域描述符扩展大小
} NvmeNamespaceParams;


static inline uint32_t zns_nsid(NvmeNamespace *ns)
{
    if (ns) {
        return ns->id;
    }

    return -1;
}

static inline NvmeLBAF *zns_ns_lbaf(NvmeNamespace *ns)
{
    NvmeIdNs *id_ns = &ns->id_ns;
    return &id_ns->lbaf[NVME_ID_NS_FLBAS_INDEX(id_ns->flbas)];
}

static inline uint8_t zns_ns_lbads(NvmeNamespace *ns)
{
    /* NvmeLBAF */
    return zns_ns_lbaf(ns)->lbads;
}

/* calculate the number of LBAs that the namespace can accomodate */
static inline uint64_t zns_ns_nlbas(NvmeNamespace *ns)
{
    return ns->size >> zns_ns_lbads(ns);
}

/* convert an LBA to the equivalent in bytes */
static inline size_t zns_l2b(NvmeNamespace *ns, uint64_t lba)
{
    return lba << zns_ns_lbads(ns);
}

static inline NvmeZoneState zns_get_zone_state(NvmeZone *zone)
{
    //pthread_spin_lock(&zone->w_ptr_lock);
    uint8_t zs = zone->d.zs >> 4;
    //pthread_spin_unlock(&zone->w_ptr_lock);

    return zs;
}

static inline void zns_set_zone_state(NvmeZone *zone, NvmeZoneState state)
{
    //pthread_spin_lock(&zone->w_ptr_lock);
    zone->d.zs = state << 4;
    //pthread_spin_unlock(&zone->w_ptr_lock);

}

static inline uint64_t zns_zone_rd_boundary(NvmeNamespace *ns, NvmeZone *zone)
{
    return zone->d.zslba + ns->ctrl->zone_size;
}

static inline uint64_t zns_zone_wr_boundary(NvmeZone *zone)
{
    return zone->d.zslba + zone->d.zcap;
}

static inline bool zns_wp_is_valid(NvmeZone *zone)
{
    uint8_t st = zns_get_zone_state(zone);

    return st != NVME_ZONE_STATE_FULL &&
           st != NVME_ZONE_STATE_READ_ONLY &&
           st != NVME_ZONE_STATE_OFFLINE;
}

static inline uint8_t *zns_get_zd_extension(NvmeNamespace *ns, uint32_t zone_idx)
{
    return &ns->ctrl->zd_extensions[zone_idx * ns->ctrl->zd_extension_size];
}

static inline void zns_aor_inc_open(NvmeNamespace *ns)
{
    FemuCtrl *n = ns->ctrl;
    assert(n->nr_open_zones >= 0);
    if (n->max_open_zones) {
        n->nr_open_zones++;
        assert(n->nr_open_zones <= n->max_open_zones);
    }
}

static inline void zns_aor_dec_open(NvmeNamespace *ns)
{
    FemuCtrl *n = ns->ctrl;
    if (n->max_open_zones) {
        assert(n->nr_open_zones > 0);
        n->nr_open_zones--;
    }
    assert(n->nr_open_zones >= 0);
}

static inline void zns_aor_inc_active(NvmeNamespace *ns)
{
    FemuCtrl *n = ns->ctrl;
    assert(n->nr_active_zones >= 0);
    if (n->max_active_zones) {
        n->nr_active_zones++;
        assert(n->nr_active_zones <= n->max_active_zones);
    }
}

static inline void zns_aor_dec_active(NvmeNamespace *ns)
{
    FemuCtrl *n = ns->ctrl;
    if (n->max_active_zones) {
        assert(n->nr_active_zones > 0);
        n->nr_active_zones--;
        assert(n->nr_active_zones >= n->nr_open_zones);
    }
    assert(n->nr_active_zones >= 0);
}

void zns_ns_shutdown(NvmeNamespace *ns);
void zns_ns_cleanup(NvmeNamespace *ns);

//get_zone(Namespace, req)
//get_ch(Zone, req)

void znsssd_init(FemuCtrl * n);

//时序更新 应该是最重要的一块
static int zns_advance_status(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd, NvmeRequest *req);

static inline NvmeZone *zns_get_zone_by_slba(NvmeNamespace *ns, uint64_t slba);

#endif
