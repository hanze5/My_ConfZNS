#include "./zns.h"
#include <math.h>
#include <signal.h>

#define MIN_DISCARD_GRANULARITY     (4 * KiB)
// #define NVME_DEFAULT_ZONE_SIZE      (128 * MiB)
#define NVME_DEFAULT_MAX_AZ_SIZE    (128 * KiB)
#define ZNS_PAGE_SIZE               (16 * KiB)
#define NVME_DEFAULT_ZONE_SIZE      (256 * MiB) //72 * MiB
uint64_t lag = 0;
//union signal sv;

static inline uint32_t zns_zone_idx(NvmeNamespace *ns, uint64_t slba)
{
    FemuCtrl *n = ns->ctrl;

    return (n->zone_size_log2 > 0 ? slba >> n->zone_size_log2 : slba /
            n->zone_size);
}
//dz_added
static inline int zns_workload_idx(NvmeNamespace *ns, uint64_t slba)
{
    uint32_t zone_idx = zns_zone_idx(ns,slba);
    FemuCtrl *n = ns->ctrl;
    

    return zone_idx/(n->num_zones/MAX_WORKLOADS);
}

static inline uint64_t zns_get_multichnlway_ppn_idx(NvmeNamespace *ns, uint64_t slba){

    //@inho : ppa(4K) distributed to 1. channels and 2. ways in interleaving manner(considering actual pagesize).
    
    FemuCtrl *n = ns->ctrl;
    struct zns * zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;

    uint64_t zone_size = NVME_DEFAULT_ZONE_SIZE / ZNS_PAGE_SIZE;        //double check (O) 看看zone有多少页 64MB/16KB = 2^12
 
    uint64_t zidx= zns_zone_idx(ns, slba);                              //double check (O) 获取该逻辑块所属的zone

    uint64_t slpa = (slba >> 3) / (ZNS_PAGE_SIZE/MIN_DISCARD_GRANULARITY); //相当于将slba 右移了5位 说明要找到该页（page 16KiB）的逻辑地址

    uint64_t slpa_origin = slpa;

    slpa = slpa / spp->planes_per_die;  //为什么要这么做？ 难不成如果spp->planes_per_die为4的话  slpa要划分为4个独立计算

    /**
     * 假设我们有一个存储设备，它有8个通道（channels），每个通道有4个路径（ways）。
     * 我们将这个设备划分为多个区域（zones），每个区域包含4个通道和4个路径。
     * 那么，我们的设备就有2个并发区域（concurrent zones），因为(8通道 / 4通道/区域) * (4路径 / 4路径/区域) = 2
     * 
     * 所属同一并发区域的zone不能并行，不同并发区域的zone 可以并行
     * 
     * 如果把可以并行的zone分成一组叫做并行组的话 那么就有 8×4/2=16 个并行组
    */
    uint64_t num_of_concurrent_zones = (spp->nchnls / spp->chnls_per_zone) * (spp->ways / spp->ways_per_zone);//计算并发区域总数 假设总共4个channel 4个way 然后zone 全都特别分散那么  这些zone就无法并行。

    /**
     * 计算zone在第几个并行组里
     * 假设是第3个zone（zidx = 2）
     * BIG_ITER = zidx / num_of_concurrent_zones = 2 / 2 = 1。表示是第2个并行组
     * 
     * BIG_ITER_VAL = zone_size * num_of_concurrent_zones * spp->planes_per_die。这个值表示一个并行组里面有多少个
     * 
    */
    uint64_t BIG_ITER = zidx / num_of_concurrent_zones ;

    uint64_t BIG_ITER_VAL = zone_size * num_of_concurrent_zones * spp->planes_per_die;  

    //可以设想三位数 但是每一位进制不一样  最低位进制是  ((spp->ways/spp->ways_per_zone))   中位进制是(spp->nchnls/spp->chnls_per_zone)  如果他们都是10 那么就好理解了

    uint64_t small_iter = zidx % (spp->nchnls/spp->chnls_per_zone);   //(spp->nchnls/spp->chnls_per_zone)为通道的并行组数，small_iter代表在哪一个通道并行组里
    uint64_t small_iter_val =((spp->chnls_per_zone) % (spp->nchnls)) * spp->planes_per_die;//取模主要是为了 当他俩相等的时候 small_iter_val为0 否则就是 spp->chnls_per_zone


    /**
     * zidx/(spp->nchnls/spp->chnls_per_zone)   med_iter就表示第几个way的并行组里面 如果说
     * 假设zidx 为11  总共有 8/2= 4个通道并行组 我们可以得知 zidx/(spp->nchnls/spp->chnls_per_zone)=2 在第3个通道并行组
     * (spp->ways/spp->ways_per_zone) 是way 并行组    这时候对它取模是何用意呢
    */
    
    uint64_t med_iter = (zidx/(spp->nchnls/spp->chnls_per_zone))%((spp->ways/spp->ways_per_zone));
    uint64_t med_iter_val = spp->nchnls * spp->ways_per_zone * spp->planes_per_die;

    //也就是说可以这样理解  BIG_ITER表示在第几个并行组  small_iter表示在第几个通道并行组  med_iter表示在第几个way并行组
    /**
     * BIG_ITER*BIG_ITER_VAL 代表总共有多少页   
    */

    uint64_t start = BIG_ITER*BIG_ITER_VAL + med_iter*med_iter_val + small_iter*small_iter_val; //后面这个使用乘法分配律改过来的

    //channel
    uint64_t iter_chnl_way = (slpa / spp->chnls_per_zone / spp->ways_per_zone) % (zone_size / spp->chnls_per_zone  / spp->ways_per_zone);
    uint64_t iter_chnl_way_val = spp->nchnls * spp->ways *spp->planes_per_die;   //总共有多少plane

    uint64_t iter_chnl = (slpa / spp->chnls_per_zone) % (spp->ways_per_zone);
    uint64_t iter_chnl_val = spp->nchnls * spp->planes_per_die;

    uint64_t incre = (slpa % spp->chnls_per_zone) * spp->planes_per_die;
    uint64_t increp= slpa_origin % spp->planes_per_die;
    //femu_err("[TEST] zns.c:99 zidx:%lu start:%lu iter_chnl_way %lu iter_chnl %lu\n", zidx, start, iter_chnl_way,iter_chnl);

    return ((start + (iter_chnl_way*iter_chnl_way_val) + (iter_chnl*iter_chnl_val) + incre + increp));
}

static inline uint64_t zns_get_ns0_zone_ppn_idx(NvmeNamespace *ns, uint64_t slba){

    //inho : should be revised by anothoer ns_association & new chnnl
    FemuCtrl *n = ns->ctrl;
    struct zns * zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;
    uint64_t zidx= zns_zone_idx(ns, slba);
    uint64_t slpa = (slba >> 3) / (ZNS_PAGE_SIZE/MIN_DISCARD_GRANULARITY);
    uint64_t zone_size = NVME_DEFAULT_ZONE_SIZE / ZNS_PAGE_SIZE;
    uint64_t now_avail_chnls = spp->chnls_per_another_zone;
    if(spp->chnls_per_another_zone > spp->nchnls){
        femu_err("Wrong setting : spp->chnls_per_another_zone > spp->nchnls \n");
        return 0;
    }

    uint64_t num_of_concurrent_zones = (spp->nchnls / now_avail_chnls) * (spp->ways / spp->ways_per_zone);
    uint64_t BIG_ITER = zidx / num_of_concurrent_zones ;
    uint64_t BIG_ITER_VAL = zone_size * num_of_concurrent_zones;
    uint64_t small_iter = zidx % num_of_concurrent_zones;
    uint64_t small_iter_val =  (now_avail_chnls * spp->ways_per_zone) % (spp->ways*spp->nchnls);
    //inho : i think small_iter_val is wrong here, please correct

    uint64_t start = BIG_ITER*BIG_ITER_VAL + small_iter*small_iter_val;

    uint64_t iter_chnl_way = (slpa / (now_avail_chnls * spp->ways));
    uint64_t iter_chnl_way_val = spp->nchnls * spp->ways ;
    uint64_t iter_chnl = (slpa / now_avail_chnls) % (spp->ways);
    uint64_t iter_chnl_val = spp->nchnls;
    uint64_t incre = slpa % now_avail_chnls;

    return (start + (iter_chnl_way*iter_chnl_way_val) + (iter_chnl*iter_chnl_val) + incre);
}
static inline uint64_t zns_another_ns1_zone_ppn_idx(NvmeNamespace *ns, uint64_t slba){
    FemuCtrl *n = ns->ctrl;
    struct zns * zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;
    uint64_t zidx= zns_zone_idx(ns, slba);
    uint64_t slpa = (slba >> 3) / (ZNS_PAGE_SIZE/MIN_DISCARD_GRANULARITY);
    uint64_t zone_size = NVME_DEFAULT_ZONE_SIZE / ZNS_PAGE_SIZE;
    uint64_t now_avail_chnls = spp->nchnls - spp->chnls_per_another_zone;
    for(uint64_t i =0 ; i > 10; i++){
        femu_err("zns_another_ns_elastic:zns.c:172 [ spp->nchnls < spp->chnls_per_another_zone ] \n");
    }
    uint64_t num_of_concurrent_zones = (now_avail_chnls / spp->chnls_per_zone) * (spp->ways / spp->ways_per_zone);

    uint64_t BIG_ITER = zidx / num_of_concurrent_zones ;
    uint64_t BIG_ITER_VAL = zone_size * (spp->nchnls / spp->chnls_per_zone) * (spp->ways / spp->ways_per_zone);
    uint64_t small_iter = zidx % (now_avail_chnls/spp->chnls_per_zone);
    uint64_t small_iter_val = (spp->chnls_per_zone) % (spp->nchnls);
    uint64_t med_iter = (zidx/(now_avail_chnls/spp->chnls_per_zone))%((spp->ways/spp->ways_per_zone));
    uint64_t med_iter_val = spp->nchnls * spp->ways_per_zone;

    uint64_t start = BIG_ITER*BIG_ITER_VAL + med_iter*med_iter_val + small_iter*small_iter_val;

    uint64_t iter_chnl_way = (slpa / spp->chnls_per_zone / spp->ways_per_zone) % (zone_size/spp->chnls_per_zone/spp->ways_per_zone);
    uint64_t iter_chnl_way_val = spp->nchnls * spp->ways ;
    uint64_t iter_chnl = (slpa / spp->chnls_per_zone) % (spp->ways_per_zone);
    uint64_t iter_chnl_val = spp->nchnls;
    uint64_t incre = slpa % spp->chnls_per_zone;
    uint64_t base = spp->chnls_per_another_zone;

    return (start + (iter_chnl_way*iter_chnl_way_val) + (iter_chnl*iter_chnl_val) + incre + base);
}
//找到所在的plane
static inline uint64_t zns_advanced_plane_idx(NvmeNamespace *ns, uint64_t slba){
    FemuCtrl *n = ns->ctrl;
    struct zns * zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;
    uint64_t ppn = zns_get_multichnlway_ppn_idx(ns, slba);
    return (ppn % (spp->nchnls * spp->ways * spp->dies_per_chip * spp->planes_per_die));
}

//找到所在的chip
static inline uint64_t zns_get_multiway_chip_idx(NvmeNamespace *ns, uint64_t slba){
    FemuCtrl *n = ns->ctrl;
    struct zns * zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;
    uint64_t zidx= zns_zone_idx(ns, slba);

    if (spp->is_another_namespace)
        return (zidx < 16) ? (zns_get_ns0_zone_ppn_idx(ns,slba)% (spp->nchnls * spp->ways)) : (zns_another_ns1_zone_ppn_idx(ns,slba) % (spp->nchnls * spp->ways)); 
    else{
        uint64_t ppn = zns_get_multichnlway_ppn_idx(ns,slba);
        return ((ppn/spp->planes_per_die) % (spp->nchnls * spp->ways));
    }
}

/**
 * @brief Inhoinno, get slba, return chnl index considerring controller-level zone mapping(static zone mapping)
 *  
 * @param ns        namespace
 * @param slba      start lba
 * @param association    1-to-N, N is zone-channel association
 * @return chnl_idx
 */
static inline uint64_t zns_advanced_chnl_idx(NvmeNamespace *ns, uint64_t slba)
{    
    FemuCtrl *n = ns->ctrl;
    struct zns * zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;
    return zns_get_multiway_chip_idx(ns,slba) % spp->nchnls;
}

//dz added
static inline uint64_t dz_zns_advanced_chnl_idx(FemuCtrl *n,NvmeNamespace *ns ,uint64_t slba)
{
    struct zns *zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;

    // uint32_t zone_idx = zns_zone_idx(ns,slba);
    NvmeZone * zone = zns_get_zone_by_slba(ns, slba);
    //计算slba在该zone中的相对地址
    uint64_t zone_rela_slba = slba - zone->d.zslba;

    //计算出应该写第几个页请求了  
    uint64_t zone_rela_slpa = zone_rela_slba/(ZNS_PAGE_SIZE / 512);

    // int big_iter = zone_rela_slpa / (spp->nchnls*spp->ways*spp->dies_per_chip*spp->planes_per_die);
    int big_offset = zone_rela_slpa % (spp->nchnls*spp->ways*spp->dies_per_chip*spp->planes_per_die);

    int die_idx = big_offset/spp->planes_per_die;

    uint16_t die;
    if(zns->allocateType == DYNAMIC){
        die= workloads[zone->d.stealing].local_dies_for_workload[die_idx];
    }
    else if(zns->allocateType == STATIC){
        die= zone->d.local_dies[die_idx];
    }
    else{
        uint32_t zone_idx = zns_zone_idx(ns,slba);
        die= workloads[zone_idx/MAX_WORKLOADS].local_dies_for_workload[die_idx];
    }

    uint64_t channel =  die / (spp->dies_per_chip*spp->ways);
    return channel;
}

static inline uint64_t dz_zns_advanced_plane_idx(FemuCtrl *n,NvmeNamespace *ns ,uint64_t slba)
{
    struct zns *zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;

    // uint32_t zone_idx = zns_zone_idx(ns,slba);
    NvmeZone * zone = zns_get_zone_by_slba(ns, slba);

    //计算slba在该zone中的相对地址
    uint64_t zone_rela_slba = slba - zone->d.zslba;
    //计算出应该写第几个页请求了  
    uint64_t zone_rela_slpa = zone_rela_slba/(ZNS_PAGE_SIZE / 512);

    // int big_iter = zone_rela_slpa / (spp->nchnls*spp->ways*spp->dies_per_chip*spp->planes_per_die);
    int big_offset = zone_rela_slpa % (spp->nchnls*spp->ways*spp->dies_per_chip*spp->planes_per_die);

    int die_idx = big_offset/spp->planes_per_die;
    uint64_t plane_idx =big_offset%spp->planes_per_die;

    uint16_t die;
    if(zns->allocateType == DYNAMIC){
        die= workloads[zone->d.stealing].local_dies_for_workload[die_idx];
    }
    else if(zns->allocateType == STATIC){
        die= zone->d.local_dies[die_idx];
    }
    else{
        uint32_t zone_idx = zns_zone_idx(ns,slba);
        die= workloads[zone_idx/MAX_WORKLOADS].local_dies_for_workload[die_idx];
    }


    uint64_t plane = plane_idx + die*spp->planes_per_die;

    return plane;

}

static inline void dz_zns_advanced_resource(FemuCtrl *n, NvmeNamespace *ns, uint64_t slba, uint64_t *channel, uint64_t *plane) {
    struct zns *zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;

    // 通过slba获取相应的zone
    NvmeZone *zone = zns_get_zone_by_slba(ns, slba);

    // 计算slba在该zone中的相对地址
    uint64_t zone_rela_slba = slba - zone->d.zslba;

    // 计算出应该写第几个页请求了
    uint64_t zone_rela_slpa = zone_rela_slba / (ZNS_PAGE_SIZE / 512);

    // 根据整体配置计算offset
    int big_offset = zone_rela_slpa % (spp->nchnls * spp->ways * spp->dies_per_chip * spp->planes_per_die);

    // 从offset中计算die索引和平面索引
    int die_idx = big_offset / spp->planes_per_die;
    uint64_t plane_idx = big_offset % spp->planes_per_die;

    uint16_t die;
    if (zns->allocateType == DYNAMIC) {
        // 动态分配，根据工作负载来分配die
        die = workloads[zone->d.stealing].local_dies_for_workload[die_idx];
    } else if (zns->allocateType == STATIC) {
        // 静态分配，直接从zone信息中获取die
        die = zone->d.local_dies[die_idx];
    } else {
        // 默认情况，根据zone索引和工作负载计算die
        uint32_t zone_idx = zns_zone_idx(ns, slba);
        zone = &n->zone_array[(zone_idx % MAX_WORKLOADS)*(zns->num_zones/MAX_WORKLOADS)];
        die = zone->d.local_dies[die_idx];
    }

    // 计算平面索引和通道
    *plane = plane_idx + die * spp->planes_per_die;
    *channel = die / (spp->dies_per_chip * spp->ways);
}




//返回die上磨损最少的 blkgrp_idx
static inline uint32_t get_youngest_blkgrp(ZNS *zns, int die_idx) {
    uint32_t best_blkgrp = 0;
    uint16_t min_erase_cnt = UINT16_MAX;

    int find = 0;

    for (int i = 0; i < zns->sp.zones; i++) {
        uint32_t blkgrp_idx = zns->dies[die_idx].blkgrps_in_die[i];

        if(find == 0 && !(zns->blkgrps[blkgrp_idx].is_being_used)){
            find = 1;
            best_blkgrp = blkgrp_idx;
            min_erase_cnt = zns->blkgrps[blkgrp_idx].erase_cnt;
        }else if (!(zns->blkgrps[blkgrp_idx].is_being_used) && (zns->blkgrps[blkgrp_idx].erase_cnt < min_erase_cnt)) {
            best_blkgrp = blkgrp_idx;
            min_erase_cnt = zns->blkgrps[best_blkgrp].erase_cnt;
        }
    }

    return best_blkgrp;
}


//返回平均值+1或者+2的
static inline uint32_t get_oldest_blkgrp(ZNS *zns, int die_idx ,int workload_idx ) {
    uint32_t oldest_blkgrp = 0;
    uint16_t max_erase_cnt = 0;

    int find = 0;

    for (int i = 0; i < zns->sp.zones; i++) {
        uint32_t blkgrp_idx = zns->dies[die_idx].blkgrps_in_die[i];

        if(find == 0 && !(zns->blkgrps[blkgrp_idx].is_being_used)){
            find = 1;
            oldest_blkgrp = blkgrp_idx;
            max_erase_cnt = zns->blkgrps[blkgrp_idx].erase_cnt;
        }else if (!(zns->blkgrps[blkgrp_idx].is_being_used) && (zns->blkgrps[blkgrp_idx].erase_cnt == (workloads[workload_idx].reset_count/256)+1 || zns->blkgrps[blkgrp_idx].erase_cnt == (workloads[workload_idx].reset_count/256)+2)) {
            oldest_blkgrp = blkgrp_idx;
            max_erase_cnt = zns->blkgrps[oldest_blkgrp].erase_cnt;
            break;
        }
    }

    return oldest_blkgrp;
}

//返回平均值 或者-1的
static inline uint32_t get_older_blkgrp(ZNS *zns, int die_idx ,int workload_idx ) {
    uint32_t oldest_blkgrp = 0;
    uint16_t max_erase_cnt = 0;

    int find = 0;

    for (int i = 0; i < zns->sp.zones; i++) {
        uint32_t blkgrp_idx = zns->dies[die_idx].blkgrps_in_die[i];

        if(find == 0 && !(zns->blkgrps[blkgrp_idx].is_being_used)){
            find = 1;
            oldest_blkgrp = blkgrp_idx;
            max_erase_cnt = zns->blkgrps[blkgrp_idx].erase_cnt;
        }else if (!(zns->blkgrps[blkgrp_idx].is_being_used) && (zns->blkgrps[blkgrp_idx].erase_cnt == (workloads[workload_idx].reset_count/256)-1 || zns->blkgrps[blkgrp_idx].erase_cnt == (workloads[workload_idx].reset_count/256))) {
            oldest_blkgrp = blkgrp_idx;
            max_erase_cnt = zns->blkgrps[oldest_blkgrp].erase_cnt;
            break;
        }
    }

    return oldest_blkgrp;
}

//返回die上磨损最少的 blkgrp_idx
static inline uint32_t get_next_blkgrp(ZNS *zns, int die_idx) {
    uint32_t next_blkgrp = 0;

    for (int i = 0; i < zns->sp.zones; i++) {
        uint32_t blkgrp_idx = zns->dies[die_idx].blkgrps_in_die[(zns->dies[die_idx].next_free_blkgrp++)%zns->sp.zones];

        if(!(zns->blkgrps[blkgrp_idx].is_being_used)){
            return blkgrp_idx;
        }
    }
    printf("不可能\n");
    return -1;
}



static inline void zns_allocate_youngest_blkgrp(ZNS *zns,NvmeZone *zone)
{

    // struct zns * zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;
    uint32_t best_blkgrp=0;
    for (int i = 0; i < spp->nchnls*spp->ways*spp->dies_per_chip; i++) {
        int die_idx;
        if(zns->allocateType ==DYNAMIC){
            die_idx = workloads[zone->d.stealing].local_dies_for_workload[i];  
        }else{
            die_idx = zone->d.local_dies[i];  
        }
          
        best_blkgrp = get_youngest_blkgrp(zns, die_idx);
        zone->d.local_blkgrps[i] = best_blkgrp;
        zns->blkgrps[best_blkgrp].is_being_used=true;
    }
    zone->d.is_mapped=true;

}

static inline void zns_allocate_older_blkgrp(ZNS *zns,NvmeZone *zone)
{
    // struct zns * zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;
    uint32_t best_blkgrp=0;
    for (int i = 0; i < spp->nchnls*spp->ways*spp->dies_per_chip; i++) {
        int die_idx;
        if(zns->allocateType ==DYNAMIC){
            die_idx = workloads[zone->d.stealing].local_dies_for_workload[i];  
        }else{
            die_idx = zone->d.local_dies[i];  
        }
            
        // best_blkgrp = get_oldest_blkgrp(zns, die_idx);
        best_blkgrp = get_older_blkgrp(zns, die_idx,zone->d.stealing);
        zone->d.local_blkgrps[i] = best_blkgrp;
        zns->blkgrps[best_blkgrp].is_being_used=true;
    }
    zone->d.is_mapped=true;

}

static inline void zns_allocate_oldest_blkgrp(ZNS *zns,NvmeZone *zone)
{
    // struct zns * zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;
    uint32_t best_blkgrp=0;
    for (int i = 0; i < spp->nchnls*spp->ways*spp->dies_per_chip; i++) {
        int die_idx;
        if(zns->allocateType ==DYNAMIC){
            die_idx = workloads[zone->d.stealing].local_dies_for_workload[i];  
        }else{
            die_idx = zone->d.local_dies[i];  
        }
            
        best_blkgrp = get_oldest_blkgrp(zns, die_idx,zone->d.stealing);
        zone->d.local_blkgrps[i] = best_blkgrp;
        zns->blkgrps[best_blkgrp].is_being_used=true;
    }
    zone->d.is_mapped=true;

}

static inline void zns_allocate_next_blkgrp(ZNS *zns,NvmeZone *zone)
{
    // struct zns * zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp;
    uint32_t best_blkgrp=0;
    for (int i = 0; i < spp->nchnls*spp->ways*spp->dies_per_chip; i++) {
        int die_idx;
        if(zns->allocateType ==DYNAMIC){
            die_idx = workloads[zone->d.stealing].local_dies_for_workload[i];  
        }else{
            die_idx = zone->d.local_dies[i];  
        }
             
        best_blkgrp = get_next_blkgrp(zns, die_idx);
        zone->d.local_blkgrps[i] = best_blkgrp;
        zns->blkgrps[best_blkgrp].is_being_used=true;
    }
    zone->d.is_mapped=true;

}


static inline void zns_reclaim_blkgrp(ZNS *zns,NvmeZone *zone)
{

    if(zone->d.is_mapped==false) return;
    struct zns_ssdparams *spp = &zns->sp;
    // printf("擦除了");
    for (int i = 0; i < spp->nchnls*spp->ways*spp->dies_per_chip; i++) {
        // uint16_t  die_idx = zone->d.local_dies[i];
        uint32_t  blkgrp_idx =  zone->d.local_blkgrps[i];

        zns->blkgrps[blkgrp_idx].is_being_used=dont_reclaim_;
        zns->blkgrps[blkgrp_idx].erase_cnt++;
        // printf("%u ",blkgrp_idx);
    }
    zone->d.is_mapped=dont_reclaim_;  
}


//60
void dz_reset_zone(ZNS *zns,int zone_idx,NvmeZone *zone) {
    // ... 其他重置逻辑 ...
    //判断属于是哪个workload
    if(zone->d.is_mapped==false) return;
    struct zns_ssdparams *spp = &zns->sp;
    int workload_idx = zone_idx/(zns->sp.zones/MAX_WORKLOADS);
    if(workload_idx!=zone->d.stealing){
        printf("======> reset workload %d s zone %d stealing workload %d \n",workload_idx,zone_idx,zone->d.stealing);
    }else{
        printf("======> reset workload %d s zone %d \n",workload_idx,zone_idx);
    }
    int queue_head = get_queue_head(workloads[workload_idx].openzones);
        
    NvmeZone *zone_head = &zns->namespaces->ctrl->zone_array[queue_head];

    //冷数据迁移逻辑
    if(!emptyQueue(workloads[workload_idx].openzones) && zone_idx!=queue_head)
    {
        workloads[workload_idx].same_cnt++;
    }else
    {
        workloads[workload_idx].same_cnt=0;
    }

    dequeue(workloads[workload_idx].openzones,zone_idx);
    printf("workload %d queue:",workload_idx);
    printQueue(workloads[workload_idx].openzones);



    if(workloads[workload_idx].same_cnt>=16 && zns->blkgrps[zone_head->d.local_blkgrps[0]].erase_cnt < ((workloads[workload_idx].reset_count/256)-2)){



        int zone_need_moration_idx = dequeue_head(workloads[workload_idx].openzones);
        
        NvmeZone *zone_need_moration = &zns->namespaces->ctrl->zone_array[zone_need_moration_idx];

        printf("迁移%d \n",zone_need_moration_idx);
        zns_reclaim_blkgrp(zns,zone_need_moration);
        workloads[workload_idx].reset_count++;

        // memcpy(zone_need_moration->d.local_dies, workloads[workload_idx].local_dies_for_workload, die_num * sizeof(*zone_need_moration->d.local_dies));
        zns_allocate_oldest_blkgrp(zns,zone_need_moration);
        enqueue(workloads[workload_idx].openzones,zone_need_moration_idx);

        workloads[workload_idx].same_cnt = 0;

    }

    
    //主要是擦除计数
    zns_reclaim_blkgrp(zns,zone);

    //用户reset次数++
    workloads[zone->d.stealing].reset_count++;

    //找到第一个die
    uint16_t die_index =  workloads[workload_idx].local_dies_for_workload[1];


    // printf("当前blkgrps磨损情况:\n");
    for(int i = 0;i<spp->zones;i+=4)
    {
        // printf("%u:%u ", zns->dies[die_index].blkgrps_in_die[i],zns->blkgrps[zns->dies[die_index].blkgrps_in_die[i]].erase_cnt);
        printf("%u ",zns->blkgrps[zns->dies[die_index].blkgrps_in_die[i]].erase_cnt);
    }
    printf("\n");


    // // 更新重置记录
    // if (workloads[workload_idx].reset_count < MAX_RESET_RECORDS) {
    //     // 如果还没有达到最大记录数，直接添加新记录
    //     workloads[workload_idx].reset_records[workloads[workload_idx].reset_end_index]= zone_idx;
    //     workloads[workload_idx].zone_reset_count[zone_idx]++;

    //     workloads[workload_idx].reset_end_index = (workloads[workload_idx].reset_end_index + 1) % MAX_RESET_RECORDS;
        
    //     workloads[workload_idx].reset_count++;
    // } else {
    //     // 如果已经达到最大记录数，需要删除最早的记录，然后添加新记录
    //     int delete_zone_idx = workloads[workload_idx].reset_records[workloads[workload_idx].reset_start_index];
    //     workloads[workload_idx].zone_reset_count[delete_zone_idx]--;

    //     workloads[workload_idx].reset_records[workloads[workload_idx].reset_end_index] = zone_idx;
    //     workloads[workload_idx].zone_reset_count[zone_idx]++;

    //     workloads[workload_idx].reset_end_index = (workloads[workload_idx].reset_end_index + 1) % MAX_RESET_RECORDS;
    //     workloads[workload_idx].reset_start_index = (workloads[workload_idx].reset_start_index + 1) % MAX_RESET_RECORDS;
    // }
}

static inline NvmeZone *zns_get_zone_by_slba(NvmeNamespace *ns, uint64_t slba)
{
    FemuCtrl *n = ns->ctrl;
    uint32_t zone_idx = zns_zone_idx(ns, slba);

    assert(zone_idx < n->num_zones);
    return &n->zone_array[zone_idx];
}

static int zns_init_zone_geometry(NvmeNamespace *ns, Error **errp)
{
    FemuCtrl *n = ns->ctrl;
    uint64_t zone_size, zone_cap;
    uint32_t lbasz = 1 << zns_ns_lbads(ns);

    if (n->zone_size_bs) {
        zone_size = n->zone_size_bs;
    } else {
        zone_size = NVME_DEFAULT_ZONE_SIZE;
    }

    if (n->zone_cap_bs) {
        zone_cap = n->zone_cap_bs;
    } else {
        zone_cap = zone_size;
    }

    if (zone_cap > zone_size) {
        femu_err("zone capacity %luB > zone size %luB", zone_cap, zone_size);
        return -1;
    }
    if (zone_size < lbasz) {
        femu_err("zone size %luB too small, must >= %uB", zone_size, lbasz);
        return -1;
    }
    if (zone_cap < lbasz) {
        femu_err("zone capacity %luB too small, must >= %uB", zone_cap, lbasz);
        return -1;
    }

    n->zone_size = zone_size / lbasz;
    n->zone_capacity = zone_cap / lbasz;
    n->num_zones = ns->size / lbasz / n->zone_size;

    if (n->max_open_zones > n->num_zones) {
        femu_err("max_open_zones value %u exceeds the number of zones %u",
                 n->max_open_zones, n->num_zones);
        return -1;
    }
    if (n->max_active_zones > n->num_zones) {
        femu_err("max_active_zones value %u exceeds the number of zones %u",
                 n->max_active_zones, n->num_zones);
        return -1;
    }

    if (n->zd_extension_size) {
        if (n->zd_extension_size & 0x3f) {
            femu_err("zone descriptor extension size must be multiples of 64B");
            return -1;
        }
        if ((n->zd_extension_size >> 6) > 0xff) {
            femu_err("zone descriptor extension size is too large");
            return -1;
        }
    }

    return 0;
}

static void zns_init_zoned_state(NvmeNamespace *ns)
{
    FemuCtrl *n = ns->ctrl;
    uint64_t start = 0, zone_size = n->zone_size;
    uint64_t capacity = n->num_zones * zone_size;
    NvmeZone *zone;
    int i;

    n->zone_array = g_new0(NvmeZone, n->num_zones);
    if (n->zd_extension_size) {
        n->zd_extensions = g_malloc0(n->zd_extension_size * n->num_zones);
    }

    QTAILQ_INIT(&n->exp_open_zones);
    QTAILQ_INIT(&n->imp_open_zones);
    QTAILQ_INIT(&n->closed_zones);
    QTAILQ_INIT(&n->full_zones);

    zone = n->zone_array;
    for (i = 0; i < n->num_zones; i++, zone++) {
        if (start + zone_size > capacity) {
            zone_size = capacity - start;
        }
        zone->d.zt = NVME_ZONE_TYPE_SEQ_WRITE;
#if MK_ZONE_CONVENTIONAL   //默认在我们这里是不支持传统zone的
        if( (i & (UINT32_MAX << MK_ZONE_CONVENTIONAL)) == 0){
            zone->d.zt = NVME_ZONE_TYPE_CONVENTIONAL;}
#endif
        zns_set_zone_state(zone, NVME_ZONE_STATE_EMPTY);
        zone->d.za = 0;
        zone->d.zcap = n->zone_capacity;
        zone->d.zslba = start;
        zone->d.wp = start;
        zone->w_ptr = start;
        // zone->d.lifetime = SHORT;
        start += zone_size;
    }

    n->zone_size_log2 = 0;
    if (is_power_of_2(n->zone_size)) {
        n->zone_size_log2 = 63 - clz64(n->zone_size);
    }
}

static void zns_init_zone_identify(FemuCtrl *n, NvmeNamespace *ns, int lba_index)
{
    NvmeIdNsZoned *id_ns_z;

    zns_init_zoned_state(ns);

    id_ns_z = g_malloc0(sizeof(NvmeIdNsZoned));

    /* MAR/MOR are zeroes-based, 0xffffffff means no limit */
    id_ns_z->mar = cpu_to_le32(n->max_active_zones - 1);
    id_ns_z->mor = cpu_to_le32(n->max_open_zones - 1);
    id_ns_z->zoc = 0;
    id_ns_z->ozcs = n->cross_zone_read ? 0x01 : 0x00;

    id_ns_z->lbafe[lba_index].zsze = cpu_to_le64(n->zone_size);
    id_ns_z->lbafe[lba_index].zdes = n->zd_extension_size >> 6; /* Units of 64B */

    n->csi = NVME_CSI_ZONED;
    ns->id_ns.nsze = cpu_to_le64(n->num_zones * n->zone_size);
    ns->id_ns.ncap = ns->id_ns.nsze;
    ns->id_ns.nuse = ns->id_ns.ncap;

    /* NvmeIdNs */
    /*
     * The device uses the BDRV_BLOCK_ZERO flag to determine the "deallocated"
     * status of logical blocks. Since the spec defines that logical blocks
     * SHALL be deallocated when then zone is in the Empty or Offline states,
     * we can only support DULBE if the zone size is a multiple of the
     * calculated NPDG.
     */
    if (n->zone_size % (ns->id_ns.npdg + 1)) {
        femu_err("the zone size (%"PRIu64" blocks) is not a multiple of the"
                 "calculated deallocation granularity (%"PRIu16" blocks); DULBE"
                 "support disabled", n->zone_size, ns->id_ns.npdg + 1);
        ns->id_ns.nsfeat &= ~0x4;
    }

    n->id_ns_zoned = id_ns_z;
}

static void zns_clear_zone(NvmeNamespace *ns, NvmeZone *zone)
{
    FemuCtrl *n = ns->ctrl;
    uint8_t state;

    zone->w_ptr = zone->d.wp;
    state = zns_get_zone_state(zone);
    if (zone->d.wp != zone->d.zslba ||
        (zone->d.za & NVME_ZA_ZD_EXT_VALID)) {
        if (state != NVME_ZONE_STATE_CLOSED) {
            zns_set_zone_state(zone, NVME_ZONE_STATE_CLOSED);
        }
        zns_aor_inc_active(ns);
        QTAILQ_INSERT_HEAD(&n->closed_zones, zone, entry);
    } else {
        zns_set_zone_state(zone, NVME_ZONE_STATE_EMPTY);
    }
}

static void zns_zoned_ns_shutdown(NvmeNamespace *ns)
{
    FemuCtrl *n = ns->ctrl;
    NvmeZone *zone, *next;

    QTAILQ_FOREACH_SAFE(zone, &n->closed_zones, entry, next) {
        QTAILQ_REMOVE(&n->closed_zones, zone, entry);
        zns_aor_dec_active(ns);
        zns_clear_zone(ns, zone);
    }
    QTAILQ_FOREACH_SAFE(zone, &n->imp_open_zones, entry, next) {
        QTAILQ_REMOVE(&n->imp_open_zones, zone, entry);
        zns_aor_dec_open(ns);
        zns_aor_dec_active(ns);
        zns_clear_zone(ns, zone);
    }
    QTAILQ_FOREACH_SAFE(zone, &n->exp_open_zones, entry, next) {
        QTAILQ_REMOVE(&n->exp_open_zones, zone, entry);
        zns_aor_dec_open(ns);
        zns_aor_dec_active(ns);
        zns_clear_zone(ns, zone);
    }

    assert(n->nr_open_zones == 0);
}

void zns_ns_shutdown(NvmeNamespace *ns)
{
    FemuCtrl *n = ns->ctrl;
    if (n->zoned) {
        zns_zoned_ns_shutdown(ns);
    }
}

// void zns_ns_cleanup(NvmeNamespace *ns)
// {
//     FemuCtrl *n = ns->ctrl;
//     if (n->zoned) {
//         g_free(n->id_ns_zoned);
//         g_free(n->zone_array);
//         g_free(n->zd_extensions);
//     }
// }

static void zns_assign_zone_state(NvmeNamespace *ns, NvmeZone *zone,
                                  NvmeZoneState state)
{
    FemuCtrl *n = ns->ctrl;

    if (QTAILQ_IN_USE(zone, entry)) {
        switch (zns_get_zone_state(zone)) {
        case NVME_ZONE_STATE_EXPLICITLY_OPEN:
            QTAILQ_REMOVE(&n->exp_open_zones, zone, entry);
            break;
        case NVME_ZONE_STATE_IMPLICITLY_OPEN:
            QTAILQ_REMOVE(&n->imp_open_zones, zone, entry);
            break;
        case NVME_ZONE_STATE_CLOSED:
            QTAILQ_REMOVE(&n->closed_zones, zone, entry);
            break;
        case NVME_ZONE_STATE_FULL:
            QTAILQ_REMOVE(&n->full_zones, zone, entry);
        default:
            ;
        }
    }

    zns_set_zone_state(zone, state);

    switch (state) {
    case NVME_ZONE_STATE_EXPLICITLY_OPEN:
        QTAILQ_INSERT_TAIL(&n->exp_open_zones, zone, entry);
        break;
    case NVME_ZONE_STATE_IMPLICITLY_OPEN:
        QTAILQ_INSERT_TAIL(&n->imp_open_zones, zone, entry);
        break;
    case NVME_ZONE_STATE_CLOSED:
        QTAILQ_INSERT_TAIL(&n->closed_zones, zone, entry);
        break;
    case NVME_ZONE_STATE_FULL:
        QTAILQ_INSERT_TAIL(&n->full_zones, zone, entry);
    case NVME_ZONE_STATE_READ_ONLY:
        break;
    default:
        zone->d.za = 0;
    }
}

/*
 * Check if we can open a zone without exceeding open/active limits.
 * AOR stands for "Active and Open Resources" (see TP 4053 section 2.5).
 */
static int zns_aor_check(NvmeNamespace *ns, uint32_t act, uint32_t opn)
{
    FemuCtrl *n = ns->ctrl;
    if (n->max_active_zones != 0 &&
        n->nr_active_zones + act > n->max_active_zones) {
        return NVME_ZONE_TOO_MANY_ACTIVE | NVME_DNR;
    }
    if (n->max_open_zones != 0 &&
        n->nr_open_zones + opn > n->max_open_zones) {
        return NVME_ZONE_TOO_MANY_OPEN | NVME_DNR;
    }

    return NVME_SUCCESS;
}

static uint16_t zns_check_zone_state_for_write(NvmeZone *zone)
{
    uint16_t status;

    switch (zns_get_zone_state(zone)) {
    case NVME_ZONE_STATE_EMPTY:
    case NVME_ZONE_STATE_IMPLICITLY_OPEN:
    case NVME_ZONE_STATE_EXPLICITLY_OPEN:
    case NVME_ZONE_STATE_CLOSED:
        status = NVME_SUCCESS;
        break;
    case NVME_ZONE_STATE_FULL:
        status = NVME_ZONE_FULL;
        break;
    case NVME_ZONE_STATE_OFFLINE:
        status = NVME_ZONE_OFFLINE;
        break;
    case NVME_ZONE_STATE_READ_ONLY:
        status = NVME_ZONE_READ_ONLY;
        break;
    default:
        assert(false);
    }

    return status;
}

static uint16_t zns_check_zone_write(FemuCtrl *n, NvmeNamespace *ns,
                                      NvmeZone *zone, uint64_t slba,
                                      uint32_t nlb, bool append)
{
    uint16_t status;
    uint32_t zidx = zns_zone_idx(ns, slba);
    if (unlikely((slba + nlb) > zns_zone_wr_boundary(zone))) {
        status = NVME_ZONE_BOUNDARY_ERROR;
    } else {
        status = zns_check_zone_state_for_write(zone);
    }

    if (status != NVME_SUCCESS) {
    } else {
        assert(zns_wp_is_valid(zone));
        if (append) {
            if (unlikely(slba != zone->d.zslba)) {
                status = NVME_INVALID_FIELD;
            }
            if (zns_l2b(ns, nlb) > (n->page_size << n->zasl)) {
                status = NVME_INVALID_FIELD;
            }
            if((zidx == 0) || (zidx == 1) || (zidx == 2) || (zidx == 3))//为什么zidx为1 2 3就是错误呢
            {
                femu_err("append wp error(%d) in zidx=%d",status, zidx);
            }
        } else if (unlikely(slba != zone->w_ptr)) {
            status = NVME_ZONE_INVALID_WRITE;
#if MK_ZONE_CONVENTIONAL  //不支持
            if( (zidx < ( 1 << MK_ZONE_CONVENTIONAL)) ){
                //zidx & (UINT32_MAX << 3) == 0 //2^3 convs
                //(zidx == 0) || (zidx == 1) || (zidx == 2) || (zidx == 3)
                //NVME_ZONE_TYPE_CONVENTIONAL;
                zone->w_ptr = slba;
                //zone->w_ptr = zone->d.zslba;
                status = NVME_SUCCESS;
            }
#endif
        }
    }

    return status;
}

/**
 * 检查zone是否是可读的状态  只要不是离线都可读
*/
static uint16_t zns_check_zone_state_for_read(NvmeZone *zone)
{
    uint16_t status;

    switch (zns_get_zone_state(zone)) {
    case NVME_ZONE_STATE_EMPTY:
    case NVME_ZONE_STATE_IMPLICITLY_OPEN:
    case NVME_ZONE_STATE_EXPLICITLY_OPEN:
    case NVME_ZONE_STATE_FULL:
    case NVME_ZONE_STATE_CLOSED:
    case NVME_ZONE_STATE_READ_ONLY:
        status = NVME_SUCCESS;
        break;
    case NVME_ZONE_STATE_OFFLINE:
        status = NVME_ZONE_OFFLINE;
        break;
    default:
        assert(false);
    }

    return status;
}

/**
 * 检查本次读操作是否有效   如果是可跨区域读 则要多检查几次范围这些都系
*/
static uint16_t zns_check_zone_read(NvmeNamespace *ns, uint64_t slba,
                                    uint32_t nlb)
{
    FemuCtrl *n = ns->ctrl;
    NvmeZone *zone = zns_get_zone_by_slba(ns, slba);
    uint64_t bndry = zns_zone_rd_boundary(ns, zone);
    uint64_t end = slba + nlb;
    uint16_t status;

    status = zns_check_zone_state_for_read(zone);
    if (status != NVME_SUCCESS) {
        ;
    } else if (unlikely(end > bndry)) {
        if (!n->cross_zone_read) {
            status = NVME_ZONE_BOUNDARY_ERROR;
        } else {
            /*
             * Read across zone boundary - check that all subsequent
             * zones that are being read have an appropriate state.
             */
            do {
                zone++;
                status = zns_check_zone_state_for_read(zone);
                if (status != NVME_SUCCESS) {
                    break;
                }
            } while (end > zns_zone_rd_boundary(ns, zone));
        }
    }

    return status;
}

/**
 * 检查当前打开的区域数量是否达到了最大值。如果是，则自动关闭第一个隐式打开的区域。
 *  在ZNS（Zoned Namespace）设备中，区域可以处于多种状态之一，包括显式打开和隐式打开。
 * 显式打开区域是通过发送特定的命令（例如ZONE MANAGEMENT SEND命令）来打开的。隐式打开区域是在执行写入操作时自动打开的。
 * 例如，如果向空区域执行写入操作，则该区域将自动从空状态转换为隐式打开状态。
 * 这两种类型的打开区域都可以执行写入操作，但它们之间的主要区别在于关闭方式。显式打开区域必须通过发送特定的命令来关闭，而隐式打开区域可以在达到最大打开区域数量时自动关闭。
*/
static void zns_auto_transition_zone(NvmeNamespace *ns)
{
    FemuCtrl *n = ns->ctrl;
    NvmeZone *zone;

    if (n->max_open_zones &&
        n->nr_open_zones == n->max_open_zones) {
        zone = QTAILQ_FIRST(&n->imp_open_zones);
        if (zone) {
             /* Automatically close this implicitly open zone */
            QTAILQ_REMOVE(&n->imp_open_zones, zone, entry);
            zns_aor_dec_open(ns);
            zns_assign_zone_state(ns, zone, NVME_ZONE_STATE_CLOSED);
        }
    }
}
/**
 * 会被写操作调用 应该是隐式打开
*/
static uint16_t zns_auto_open_zone(NvmeNamespace *ns, NvmeZone *zone)
{
    uint16_t status = NVME_SUCCESS;
    uint8_t zs = zns_get_zone_state(zone);

    if (zs == NVME_ZONE_STATE_EMPTY) {
        zns_auto_transition_zone(ns);
        status = zns_aor_check(ns, 1, 1);
    } else if (zs == NVME_ZONE_STATE_CLOSED) {
        zns_auto_transition_zone(ns);
        status = zns_aor_check(ns, 0, 1);
    }

    return status;
}
//该函数在执行分区写入操作后被调用，用于更新区域状态。
static void zns_finalize_zoned_write(NvmeNamespace *ns, NvmeRequest *req,
                                     bool failed)
{
    NvmeRwCmd *rw = (NvmeRwCmd *)&req->cmd;
    NvmeZone *zone;
    NvmeZonedResult *res = (NvmeZonedResult *)&req->cqe;
    uint64_t slba;
    uint32_t nlb;
    //该区域的写入指针增加nlb个逻辑块。如果写入操作失败，则将res->slba设置为0
    slba = le64_to_cpu(rw->slba);
    nlb = le16_to_cpu(rw->nlb) + 1;
    zone = zns_get_zone_by_slba(ns, slba);

    zone->d.wp += nlb;

    if (failed) {
        res->slba = 0;
    }
    //它检查该区域的写入指针是否达到了区域的写入边界。如果是，则根据当前区域状态执行不同的操作。
    if (zone->d.wp == zns_zone_wr_boundary(zone)) {
        switch (zns_get_zone_state(zone)) {
        case NVME_ZONE_STATE_IMPLICITLY_OPEN:
        case NVME_ZONE_STATE_EXPLICITLY_OPEN:
            zns_aor_dec_open(ns);
            /* fall through */
        case NVME_ZONE_STATE_CLOSED:
            zns_aor_dec_active(ns);
            /* fall through */
        case NVME_ZONE_STATE_EMPTY:
            zns_assign_zone_state(ns, zone, NVME_ZONE_STATE_FULL);
            /* fall through */
        case NVME_ZONE_STATE_FULL:
            break;
        default:
            assert(false);
        }
    }
}

/**
 * 用于更新zone的写指针    此外如果zone此时处于空 或者 关闭状态 还会将其状态修改为活动状态
*/
static uint64_t zns_advance_zone_wp(NvmeNamespace *ns, NvmeZone *zone,
                                    uint32_t nlb)
{
    uint64_t result = zone->w_ptr;
    uint8_t zs;

    zone->w_ptr += nlb;

    if (zone->w_ptr < zns_zone_wr_boundary(zone)) {
        zs = zns_get_zone_state(zone);
        switch (zs) {
        case NVME_ZONE_STATE_EMPTY:
            zns_aor_inc_active(ns);
            /* fall through */
        case NVME_ZONE_STATE_CLOSED:
            zns_aor_inc_open(ns);
            zns_assign_zone_state(ns, zone, NVME_ZONE_STATE_IMPLICITLY_OPEN);
        }
    }

    return result;
}

/**
 * 0 used
 * 这个结构体通常用于在执行区域重置操作时传递上下文信息。 
 * 它包含两个成员：req和zone。req是一个指向NvmeRequest类型的指针，表示与区域重置操作相关联的请求。zone是一个指向NvmeZone类型的指针，表示要重置的区域。
*/
struct zns_zone_reset_ctx {
    NvmeRequest *req;
    NvmeZone    *zone;
};

/**
 * 用于在执行区域重置操作时更新区域状态。该函数接受NvmeRequest和NvmeZone作为参数。
 * 首先获取与请求相关联的命名空间。
 * 然后，使用zns_get_zone_state函数获取区域状态，并根据状态执行不同的操作。
 * 例如，
 * 如果区域状态为NVME_ZONE_STATE_EXPLICITLY_OPEN或NVME_ZONE_STATE_IMPLICITLY_OPEN，则调用zns_aor_dec_open减少打开区域的数量。
 * 如果区域状态为NVME_ZONE_STATE_CLOSED，则调用zns_aor_dec_active减少活动区域的数量。
 * 如果区域状态为NVME_ZONE_STATE_FULL，则将zone->w_ptr和zone->d.wp设置为zone->d.zslba，并调用zns_assign_zone_state将该区域的状态更改为NVME_ZONE_STATE_EMPTY。
*/
static void zns_aio_zone_reset_cb(NvmeRequest *req, NvmeZone *zone)
{
    NvmeNamespace *ns = req->ns;

    /* FIXME, We always assume reset SUCCESS */
    switch (zns_get_zone_state(zone)) {
    case NVME_ZONE_STATE_EXPLICITLY_OPEN:
        /* fall through */
    case NVME_ZONE_STATE_IMPLICITLY_OPEN:
        zns_aor_dec_open(ns);
        /* fall through */
    case NVME_ZONE_STATE_CLOSED:
        zns_aor_dec_active(ns);
        /* fall through */
    case NVME_ZONE_STATE_FULL:
        zone->w_ptr = zone->d.zslba;
        zone->d.wp = zone->w_ptr;
        zns_assign_zone_state(ns, zone, NVME_ZONE_STATE_EMPTY);
    default:
        break;
    }
}

/**
 * 这段代码定义了一个名为op_handler_t的类型。
 * 这种类型的函数通常用于处理区域操作，例如打开区域，关闭区域或完成区域。
*/
typedef uint16_t (*op_handler_t)(NvmeNamespace *, NvmeZone *, NvmeZoneState,
                                 NvmeRequest *);

/**
 * 这段代码定义了一个名为NvmeZoneProcessingMask的枚举类型。它用于指定在执行区域操作时要处理的区域类型。
 * 例如，
 * NVME_PROC_CURRENT_ZONE表示仅处理当前区域，
 * NVME_PROC_OPENED_ZONES表示处理所有打开的区域，
 * NVME_PROC_CLOSED_ZONES表示处理所有关闭的区域，
 * NVME_PROC_READ_ONLY_ZONES表示处理所有只读区域，
 * NVME_PROC_FULL_ZONES表示处理所有满区域。
*/
enum NvmeZoneProcessingMask {
    NVME_PROC_CURRENT_ZONE    = 0,
    NVME_PROC_OPENED_ZONES    = 1 << 0,
    NVME_PROC_CLOSED_ZONES    = 1 << 1,
    NVME_PROC_READ_ONLY_ZONES = 1 << 2,
    NVME_PROC_FULL_ZONES      = 1 << 3,
};

static uint16_t zns_open_zone(NvmeNamespace *ns, NvmeZone *zone,
                              NvmeZoneState state, NvmeRequest *req)
{
    uint16_t status;

    switch (state) {
    case NVME_ZONE_STATE_EMPTY:
        status = zns_aor_check(ns, 1, 0);
        if (status != NVME_SUCCESS) {
            return status;
        }
        zns_aor_inc_active(ns);
        /* fall through */
    case NVME_ZONE_STATE_CLOSED:
        status = zns_aor_check(ns, 0, 1);
        if (status != NVME_SUCCESS) {
            if (state == NVME_ZONE_STATE_EMPTY) {
                zns_aor_dec_active(ns);
            }
            return status;
        }
        zns_aor_inc_open(ns);
        /* fall through */
    case NVME_ZONE_STATE_IMPLICITLY_OPEN:
        zns_assign_zone_state(ns, zone, NVME_ZONE_STATE_EXPLICITLY_OPEN);
        /* fall through */
    case NVME_ZONE_STATE_EXPLICITLY_OPEN:
        return NVME_SUCCESS;
    default:
        return NVME_ZONE_INVAL_TRANSITION;
    }
}

static uint16_t zns_close_zone(NvmeNamespace *ns, NvmeZone *zone,
                               NvmeZoneState state, NvmeRequest *req)
{
    switch (state) {
    case NVME_ZONE_STATE_EXPLICITLY_OPEN:
        /* fall through */
    case NVME_ZONE_STATE_IMPLICITLY_OPEN:
        zns_aor_dec_open(ns);
        zns_assign_zone_state(ns, zone, NVME_ZONE_STATE_CLOSED);
        /* fall through */
    case NVME_ZONE_STATE_CLOSED:
        return NVME_SUCCESS;
    default:
        return NVME_ZONE_INVAL_TRANSITION;
    }
}

/**
 * 用于完成一个区域。 反正就是把zone的状态改成满状态
 * 首先根据区域状态执行不同的操作。
 * 例如，
 * 如果区域状态为NVME_ZONE_STATE_EXPLICITLY_OPEN或NVME_ZONE_STATE_IMPLICITLY_OPEN，则调用zns_aor_dec_open减少打开区域的数量。
 * 如果区域状态为NVME_ZONE_STATE_CLOSED，则调用zns_aor_dec_active减少活动区域的数量。
 * 如果区域状态为NVME_ZONE_STATE_EMPTY，则将zone->w_ptr和zone->d.wp设置为zns_zone_wr_boundary(zone)，并调用zns_assign_zone_state将该区域的状态更改为NVME_ZONE_STATE_FULL。
*/
static uint16_t zns_finish_zone(NvmeNamespace *ns, NvmeZone *zone,
                                NvmeZoneState state, NvmeRequest *req)
{
    switch (state) {
    case NVME_ZONE_STATE_EXPLICITLY_OPEN:
        /* fall through */
    case NVME_ZONE_STATE_IMPLICITLY_OPEN:
        zns_aor_dec_open(ns);
        /* fall through */
    case NVME_ZONE_STATE_CLOSED:
        zns_aor_dec_active(ns);
        /* fall through */
    case NVME_ZONE_STATE_EMPTY:
        zone->w_ptr = zns_zone_wr_boundary(zone);
        zone->d.wp = zone->w_ptr;
        zns_assign_zone_state(ns, zone, NVME_ZONE_STATE_FULL);
        /* fall through */
    case NVME_ZONE_STATE_FULL:
        return NVME_SUCCESS;
    default:
        return NVME_ZONE_INVAL_TRANSITION;
    }
}

/**
 * 重置一个zone  不管是啥状态都调用 zns_aio_zone_reset_cb 将一块区域重置
*/
static uint16_t zns_reset_zone(NvmeNamespace *ns, NvmeZone *zone,
                               NvmeZoneState state, NvmeRequest *req)
{
    switch (state) {
    case NVME_ZONE_STATE_EMPTY:
        return NVME_SUCCESS;
    case NVME_ZONE_STATE_EXPLICITLY_OPEN:
    case NVME_ZONE_STATE_IMPLICITLY_OPEN:
    case NVME_ZONE_STATE_CLOSED:
    case NVME_ZONE_STATE_FULL:
        break;
    default:
        return NVME_ZONE_INVAL_TRANSITION;
    }

    zns_aio_zone_reset_cb(req, zone);

    return NVME_SUCCESS;
}

static uint16_t zns_offline_zone(NvmeNamespace *ns, NvmeZone *zone,
                                 NvmeZoneState state, NvmeRequest *req)
{
    switch (state) {
    case NVME_ZONE_STATE_READ_ONLY:
        zns_assign_zone_state(ns, zone, NVME_ZONE_STATE_OFFLINE);
        /* fall through */
    case NVME_ZONE_STATE_OFFLINE:
        return NVME_SUCCESS;
    default:
        return NVME_ZONE_INVAL_TRANSITION;
    }
}

/**
 * 这个函数用来设置 NvmeZone 的扩展描述符。
 * 如果 NvmeZone 的状态为空，他会激活该区域 并 将zone属性里的 NVME_ZA_ZD_EXT_VALID 位置1 代表启用扩展zone描述符存储额外的元数据
 * 如果 NvmeZone 的状态不为空，则返回 NVME_ZONE_INVAL_TRANSITION 状态。
*/
static uint16_t zns_set_zd_ext(NvmeNamespace *ns, NvmeZone *zone)
{
    uint16_t status;
    uint8_t state = zns_get_zone_state(zone);

    if (state == NVME_ZONE_STATE_EMPTY) {
        status = zns_aor_check(ns, 1, 0);
        if (status != NVME_SUCCESS) {
            return status;
        }
        zns_aor_inc_active(ns);
        zone->d.za |= NVME_ZA_ZD_EXT_VALID;
        zns_assign_zone_state(ns, zone, NVME_ZONE_STATE_CLOSED);
        return NVME_SUCCESS;
    }

    return NVME_ZONE_INVAL_TRANSITION;
}

/**
 * 目的是根据给定的处理掩码和操作处理程序批量处理区域。
 * 它接受一个 NvmeNamespace，一个 NvmeZone，一个处理掩码，一个操作处理程序和一个 NvmeRequest 作为参数。
 * 函数首先获取区域的状态，并根据状态确定是否需要处理该区域。
 * 如果需要处理该区域，则调用操作处理程序并返回其返回的状态。
 * 否则，返回 NVME_SUCCESS 状态。
*/
static uint16_t zns_bulk_proc_zone(NvmeNamespace *ns, NvmeZone *zone,
                                   enum NvmeZoneProcessingMask proc_mask,
                                   op_handler_t op_hndlr, NvmeRequest *req)
{
    uint16_t status = NVME_SUCCESS;
    NvmeZoneState zs = zns_get_zone_state(zone);
    bool proc_zone;

    switch (zs) {
    case NVME_ZONE_STATE_IMPLICITLY_OPEN:
    case NVME_ZONE_STATE_EXPLICITLY_OPEN:
        proc_zone = proc_mask & NVME_PROC_OPENED_ZONES;
        break;
    case NVME_ZONE_STATE_CLOSED:
        proc_zone = proc_mask & NVME_PROC_CLOSED_ZONES;
        break;
    case NVME_ZONE_STATE_READ_ONLY:
        proc_zone = proc_mask & NVME_PROC_READ_ONLY_ZONES;
        break;
    case NVME_ZONE_STATE_FULL:
        proc_zone = proc_mask & NVME_PROC_FULL_ZONES;
        break;
    default:
        proc_zone = false;
    }

    if (proc_zone) {
        status = op_hndlr(ns, zone, zs, req);
    }

    return status;
}

/**
 * 根据给定的处理掩码和操作处理程序对区域执行操作。
 * 如果处理掩码为零，则直接调用操作处理程序并返回其返回的状态。
 * 否则，根据处理掩码，遍历所有需要处理的区域，并对每个区域调用 zns_bulk_proc_zone 函数进行批量处理。
 * 如果任何区域的处理失败，则返回错误状态。
*/
static uint16_t zns_do_zone_op(NvmeNamespace *ns, NvmeZone *zone,
                               enum NvmeZoneProcessingMask proc_mask,
                               op_handler_t op_hndlr, NvmeRequest *req)
{
    FemuCtrl *n = ns->ctrl;
    NvmeZone *next;
    uint16_t status = NVME_SUCCESS;
    int i;

    if (!proc_mask) {
        status = op_hndlr(ns, zone, zns_get_zone_state(zone), req);
    } else {
        if (proc_mask & NVME_PROC_CLOSED_ZONES) {
            QTAILQ_FOREACH_SAFE(zone, &n->closed_zones, entry, next) {
                status = zns_bulk_proc_zone(ns, zone, proc_mask, op_hndlr,
                                             req);
                if (status && status != NVME_NO_COMPLETE) {
                    goto out;
                }
            }
        }
        if (proc_mask & NVME_PROC_OPENED_ZONES) {
            QTAILQ_FOREACH_SAFE(zone, &n->imp_open_zones, entry, next) {
                status = zns_bulk_proc_zone(ns, zone, proc_mask, op_hndlr,
                                             req);
                if (status && status != NVME_NO_COMPLETE) {
                    goto out;
                }
            }

            QTAILQ_FOREACH_SAFE(zone, &n->exp_open_zones, entry, next) {
                status = zns_bulk_proc_zone(ns, zone, proc_mask, op_hndlr,
                                             req);
                if (status && status != NVME_NO_COMPLETE) {
                    goto out;
                }
            }
        }
        if (proc_mask & NVME_PROC_FULL_ZONES) {
            QTAILQ_FOREACH_SAFE(zone, &n->full_zones, entry, next) {
                status = zns_bulk_proc_zone(ns, zone, proc_mask, op_hndlr,
                                             req);
                if (status && status != NVME_NO_COMPLETE) {
                    goto out;
                }
            }
        }

        if (proc_mask & NVME_PROC_READ_ONLY_ZONES) {
            for (i = 0; i < n->num_zones; i++, zone++) {
                status = zns_bulk_proc_zone(ns, zone, proc_mask, op_hndlr,
                                             req);
                if (status && status != NVME_NO_COMPLETE) {
                    goto out;
                }
            }
        }
    }

out:
    return status;
}
/**
 * 这个函数用于获取管理区域的起始逻辑块地址（SLBA）和区域索引。它接受一个 FemuCtrl，一个 NvmeCmd，一个指向 SLBA 的指针和一个指向区域索引的指针作为参数。
 * 函数首先检查控制器是否支持分区。如果不支持，则返回 NVME_INVALID_OPCODE 状态。
 * 然后，它从命令中获取 SLBA 并检查其是否在命名空间的范围内。如果不在范围内，则返回 NVME_LBA_RANGE 状态。
 * 最后，它计算区域索引并将其存储在给定的指针中。
 * 
 * 也就是从前面两个参数  获取  后面两个参数
*/
static uint16_t zns_get_mgmt_zone_slba_idx(FemuCtrl *n, NvmeCmd *c,
                                           uint64_t *slba, uint32_t *zone_idx)
{
    NvmeNamespace *ns = &n->namespaces[0];
    uint32_t dw10 = le32_to_cpu(c->cdw10);
    uint32_t dw11 = le32_to_cpu(c->cdw11);

    if (!n->zoned) {
        return NVME_INVALID_OPCODE | NVME_DNR;
    }

    *slba = ((uint64_t)dw11) << 32 | dw10;
    if (unlikely(*slba >= ns->id_ns.nsze)) {
        *slba = 0;
        return NVME_LBA_RANGE | NVME_DNR;
    }

    *zone_idx = zns_zone_idx(ns, *slba);
    assert(*zone_idx < n->num_zones);

    return NVME_SUCCESS;
}

/**
 * 用于处理区域管理命令 它接受一个 FemuCtrl 和一个 NvmeRequest 作为参数。
 * 首先从命令中获取操作和是否应用于所有区域的标志。然后，如果不应用于所有区域，则调用 zns_get_mgmt_zone_slba_idx 函数获取管理区域的起始逻辑块地址和区域索引。
 * 接下来，它检查给定的 SLBA 是否与区域的起始逻辑块地址匹配。如果不匹配，则返回 NVME_INVALID_FIELD 状态。
 * 然后，根据操作类型，调用 zns_do_zone_op 函数对区域执行相应的操作。如果操作成功，则返回 NVME_SUCCESS 状态。
 * 这段代码是从一个名为 zns_zone_mgmt_send 的函数中选出来的。这个函数用于处理区域管理发送命令。它接受一个 FemuCtrl 和一个 NvmeRequest 作为参数。
*/
static uint16_t zns_zone_mgmt_send(FemuCtrl *n, NvmeRequest *req)
{
    NvmeCmd *cmd = (NvmeCmd *)&req->cmd;
    NvmeNamespace *ns = req->ns;
    uint64_t prp1 = le64_to_cpu(cmd->dptr.prp1);
    uint64_t prp2 = le64_to_cpu(cmd->dptr.prp2);
    NvmeZone *zone;
    uintptr_t *resets;
    uint8_t *zd_ext;
    uint32_t dw13 = le32_to_cpu(cmd->cdw13);
    uint64_t slba = 0;
    uint32_t zone_idx = 0;
    uint16_t status;
    uint8_t action;
    bool all;
    enum NvmeZoneProcessingMask proc_mask = NVME_PROC_CURRENT_ZONE;

    action = dw13 & 0xff;
    all = dw13 & 0x100;

    req->status = NVME_SUCCESS;

    if (!all) {
        status = zns_get_mgmt_zone_slba_idx(n, cmd, &slba, &zone_idx);
        if (status) {
            return status;
        }
    }

    zone = &n->zone_array[zone_idx];
    if (slba != zone->d.zslba) {
        return NVME_INVALID_FIELD | NVME_DNR;
    }

    switch (action) {
    case NVME_ZONE_ACTION_OPEN:
        if (all) {
            proc_mask = NVME_PROC_CLOSED_ZONES;
        }
        status = zns_do_zone_op(ns, zone, proc_mask, zns_open_zone, req);
        break;
    case NVME_ZONE_ACTION_CLOSE:
        if (all) {
            proc_mask = NVME_PROC_OPENED_ZONES;
        }
        status = zns_do_zone_op(ns, zone, proc_mask, zns_close_zone, req);
        break;
    case NVME_ZONE_ACTION_FINISH:
        if (all) {
            proc_mask = NVME_PROC_OPENED_ZONES | NVME_PROC_CLOSED_ZONES;
        }
        status = zns_do_zone_op(ns, zone, proc_mask, zns_finish_zone, req);
        break;
    case NVME_ZONE_ACTION_RESET:
        resets = (uintptr_t *)&req->opaque;

        if (all) {
            proc_mask = NVME_PROC_OPENED_ZONES | NVME_PROC_CLOSED_ZONES |
                NVME_PROC_FULL_ZONES;
        }
        *resets = 1;
        status = zns_do_zone_op(ns, zone, proc_mask, zns_reset_zone, req);
        req->expire_time += zns_advance_status(n, ns, cmd, req); //reset操作需要增加
        (*resets)--;
        // femu_log("zone reset    action:%c   slba:%ld     zone_idx:%d    req->expire_time(%lu) - req->stime(%lu):%lu\n",action, req->slba ,zone_idx,req->expire_time,req->stime,(req->expire_time - req->stime));
        return NVME_SUCCESS;
    case NVME_ZONE_ACTION_OFFLINE:
        if (all) {
            proc_mask = NVME_PROC_READ_ONLY_ZONES;
        }
        status = zns_do_zone_op(ns, zone, proc_mask, zns_offline_zone, req);
        break;
    case NVME_ZONE_ACTION_SET_ZD_EXT:
        if (all || !n->zd_extension_size) {
            return NVME_INVALID_FIELD | NVME_DNR;
        }
        zd_ext = zns_get_zd_extension(ns, zone_idx);
        status = dma_write_prp(n, (uint8_t *)zd_ext, n->zd_extension_size, prp1,
                               prp2);
        if (status) {
            return status;
        }
        status = zns_set_zd_ext(ns, zone);
        if (status == NVME_SUCCESS) {
            return status;
        }
        break;
    default:
        status = NVME_INVALID_FIELD;
    }

    if (status) {
        status |= NVME_DNR;
    }

    return status;
}

/**
 * 判断zl的状态是不是 zafs  一般用于过滤指定状态的zone
*/
static bool zns_zone_matches_filter(uint32_t zafs, NvmeZone *zl)
{
    NvmeZoneState zs = zns_get_zone_state(zl);

    switch (zafs) {
    case NVME_ZONE_REPORT_ALL:
        return true;
    case NVME_ZONE_REPORT_EMPTY:
        return zs == NVME_ZONE_STATE_EMPTY;
    case NVME_ZONE_REPORT_IMPLICITLY_OPEN:
        return zs == NVME_ZONE_STATE_IMPLICITLY_OPEN;
    case NVME_ZONE_REPORT_EXPLICITLY_OPEN:
        return zs == NVME_ZONE_STATE_EXPLICITLY_OPEN;
    case NVME_ZONE_REPORT_CLOSED:
        return zs == NVME_ZONE_STATE_CLOSED;
    case NVME_ZONE_REPORT_FULL:
        return zs == NVME_ZONE_STATE_FULL;
    case NVME_ZONE_REPORT_READ_ONLY:
        return zs == NVME_ZONE_STATE_READ_ONLY;
    case NVME_ZONE_REPORT_OFFLINE:
        return zs == NVME_ZONE_STATE_OFFLINE;
    default:
        return false;
    }
}

/**
 * 总之，这个函数用于处理区域管理接收命令，它根据命令中指定的过滤器类型和区域范围来生成区域报告，并将报告传输回主机。
 * 首先，函数从 `req` 参数中获取命令并提取相关字段。然后，它检查命令是否有效并执行必要的错误检查。例如，它检查区域报告操作是否有效，数据大小是否足够大，以及是否满足最大数据传输大小限制。
 * 接下来，函数根据命令中指定的过滤器类型和区域范围来确定要报告的区域数量。然后，它分配一个缓冲区并填充区域报告头信息。
 * 最后，函数遍历所有匹配过滤器的区域，并将它们的信息添加到缓冲区中。然后，它使用 `dma_read_prp` 函数将缓冲区中的数据传输回主机。
 * 
*/
static uint16_t zns_zone_mgmt_recv(FemuCtrl *n, NvmeRequest *req)
{
    NvmeCmd *cmd = (NvmeCmd *)&req->cmd;
    NvmeNamespace *ns = req->ns;
    uint64_t prp1 = le64_to_cpu(cmd->dptr.prp1);
    uint64_t prp2 = le64_to_cpu(cmd->dptr.prp2);
    /* cdw12 is zero-based number of dwords to return. Convert to bytes */
    uint32_t data_size = (le32_to_cpu(cmd->cdw12) + 1) << 2;
    uint32_t dw13 = le32_to_cpu(cmd->cdw13);
    uint32_t zone_idx, zra, zrasf, partial;
    uint64_t max_zones, nr_zones = 0;
    uint16_t status;
    uint64_t slba, capacity = zns_ns_nlbas(ns);
    NvmeZoneDescr *z;
    NvmeZone *zone;
    NvmeZoneReportHeader *header;
    void *buf, *buf_p;
    size_t zone_entry_sz;
    //然后，它检查命令是否有效并执行必要的错误检查。
    //例如，它检查区域报告操作是否有效，数据大小是否足够大，以及是否满足最大数据传输大小限制。

    req->status = NVME_SUCCESS;

    status = zns_get_mgmt_zone_slba_idx(n, cmd, &slba, &zone_idx);
    if (status) {
        return status;
    }

    zra = dw13 & 0xff;
    if (zra != NVME_ZONE_REPORT && zra != NVME_ZONE_REPORT_EXTENDED) {
        return NVME_INVALID_FIELD | NVME_DNR;
    }
    if (zra == NVME_ZONE_REPORT_EXTENDED && !n->zd_extension_size) {
        return NVME_INVALID_FIELD | NVME_DNR;
    }

    zrasf = (dw13 >> 8) & 0xff;
    if (zrasf > NVME_ZONE_REPORT_OFFLINE) {
        return NVME_INVALID_FIELD | NVME_DNR;
    }

    if (data_size < sizeof(NvmeZoneReportHeader)) {
        return NVME_INVALID_FIELD | NVME_DNR;
    }

    status = nvme_check_mdts(n, data_size);
    if (status) {
        return status;
    }

    partial = (dw13 >> 16) & 0x01;

    zone_entry_sz = sizeof(NvmeZoneDescr);
    if (zra == NVME_ZONE_REPORT_EXTENDED) {
        zone_entry_sz += n->zd_extension_size;
    }

    max_zones = (data_size - sizeof(NvmeZoneReportHeader)) / zone_entry_sz;
    buf = g_malloc0(data_size);

    zone = &n->zone_array[zone_idx];
    for (; slba < capacity; slba += n->zone_size) {
        if (partial && nr_zones >= max_zones) {
            break;
        }
        if (zns_zone_matches_filter(zrasf, zone++)) {
            nr_zones++;
        }
    }
    header = (NvmeZoneReportHeader *)buf;
    header->nr_zones = cpu_to_le64(nr_zones);
    //找到buf的payload部分
    buf_p = buf + sizeof(NvmeZoneReportHeader);
    //然后循环填充每一个NvmeZoneDescr
    for (; zone_idx < n->num_zones && max_zones > 0; zone_idx++) {
        zone = &n->zone_array[zone_idx];
        if (zns_zone_matches_filter(zrasf, zone)) {
            z = (NvmeZoneDescr *)buf_p;
            buf_p += sizeof(NvmeZoneDescr);

            z->zt = zone->d.zt;
            z->zs = zone->d.zs;
            z->zcap = cpu_to_le64(zone->d.zcap);
            z->zslba = cpu_to_le64(zone->d.zslba);
            z->za = zone->d.za;

            if (zns_wp_is_valid(zone)) {
                z->wp = cpu_to_le64(zone->d.wp);
            } else {
                z->wp = cpu_to_le64(~0ULL);
            }

            if (zra == NVME_ZONE_REPORT_EXTENDED) {
                if (zone->d.za & NVME_ZA_ZD_EXT_VALID) {
                    memcpy(buf_p, zns_get_zd_extension(ns, zone_idx),
                           n->zd_extension_size);
                }
                buf_p += n->zd_extension_size;
            }

            max_zones--;
        }
    }

    status = dma_read_prp(n, (uint8_t *)buf, data_size, prp1, prp2);

    g_free(buf);

    return status;
}

/**
 * 该函数用于检查命名空间是否支持NVM（非易失性内存）。它接受一个指向NvmeNamespace结构体的指针作为参数。
 * 函数通过检查命名空间控制器的CSI（命令集标识符）字段来确定命名空间是否支持NVM。
 * 如果CSI字段的值为NVME_CSI_NVM或NVME_CSI_ZONED，则表示命名空间支持NVM，函数返回true。否则，函数返回false。
*/
static inline bool nvme_csi_has_nvm_support(NvmeNamespace *ns)
{
    switch (ns->ctrl->csi) {

        
    case NVME_CSI_NVM:
    case NVME_CSI_ZONED:
        return true;
    }
    return false;
}
/**
 * 该函数用于检查给定的起始逻辑块地址（slba）和逻辑块数量（nlb）是否在命名空间的范围内。
 * 首先计算命名空间的大小（nsze），然后检查slba + nlb是否超出了命名空间的范围。
*/
static inline uint16_t zns_check_bounds(NvmeNamespace *ns, uint64_t slba,
                                        uint32_t nlb)
{
    uint64_t nsze = le64_to_cpu(ns->id_ns.nsze);

    if (unlikely(UINT64_MAX - slba < nlb || slba + nlb > nsze)) {
        return NVME_LBA_RANGE | NVME_DNR;
    }

    return NVME_SUCCESS;
}


/**
 * 不用管其实  是为了进行地址转换  
 * 该函数用于根据请求中的数据指针（dptr）字段映射PRP（物理区域页）列表。
 * 函数首先检查请求中的psdt字段，以确定数据传输类型。
 * 如果psdt字段的值为NVME_PSDT_PRP，则表示使用PRP列表进行数据传输。
 * 函数从请求中获取PRP1和PRP2字段的值，并调用nvme_map_prp函数来映射PRP列表。
 * 如果psdt字段的值不是NVME_PSDT_PRP，则返回NVME_INVALID_FIELD错误代码。
 * 
 * 其实就是从req的cmd.psdt 转换成 req->qsg req->iov  这样后续的读写操作就都可以使用 req->qsg req->iov
 * 
*/
static uint16_t zns_map_dptr(FemuCtrl *n, size_t len, NvmeRequest *req)
{
    uint64_t prp1, prp2;

    switch (req->cmd.psdt) {
    case NVME_PSDT_PRP:
        prp1 = le64_to_cpu(req->cmd.dptr.prp1);
        prp2 = le64_to_cpu(req->cmd.dptr.prp2);

        return nvme_map_prp(&req->qsg, &req->iov, prp1, prp2, len, n);
    default:
        return NVME_INVALID_FIELD;
    }
}

static uint16_t zns_do_write(FemuCtrl *n, NvmeRequest *req, bool append,
                             bool wrz)
{
    NvmeRwCmd *rw = (NvmeRwCmd *)&req->cmd;
    NvmeNamespace *ns = req->ns;
    uint64_t slba = le64_to_cpu(rw->slba);
    uint32_t nlb = (uint32_t)le16_to_cpu(rw->nlb) + 1;
    uint64_t data_size = zns_l2b(ns, nlb);
    uint64_t data_offset;
    NvmeZone *zone;
    NvmeZonedResult *res = (NvmeZonedResult *)&req->cqe;
    uint16_t status;
    uint64_t zidx = zns_zone_idx(ns, slba);

    femu_log("zns_do_write: zone%lu 上%u \n",zidx, nlb);

    assert(n->zoned);
    req->is_write = true;

    if (!wrz) {
        status = nvme_check_mdts(n, data_size);
        if (status) {
            goto err;
        }
    }
    //函数检查slba和nlb是否在命名空间的范围内，并检查写入操作是否符合分区命名空间的要求。
    status = zns_check_bounds(ns, slba, nlb);
    if (status) {
        goto err;
    }

    zone = zns_get_zone_by_slba(ns, slba);

    status = zns_check_zone_write(n, ns, zone, slba, nlb, append);
    if (status) {
        goto err;
    }

    status = zns_auto_open_zone(ns, zone);
    if (status) {
        goto err;
    }

    if (append) {
        slba = zone->w_ptr;
    }

    res->slba = zns_advance_zone_wp(ns, zone, nlb);

    data_offset = zns_l2b(ns, slba);

    if (!wrz) {
        status = zns_map_dptr(n, data_size, req);
        if (status) {
            goto err;
        }
        //更新操作
        req->expire_time += zns_advance_status(n,ns,&req->cmd,req);

        backend_rw(n->mbe, &req->qsg, &data_offset, req->is_write);
    }

    zns_finalize_zoned_write(ns, req, false);
    return NVME_SUCCESS;

err:
    printf("****************Append Failed***************\n");
    return status | NVME_DNR;
}

/**
 * 该函数检查 `cmd` 中的操作码，并根据操作码执行相应的操作。  疑问？
 * 然而，在这段代码中，该函数并未实现任何特定的操作，而是对所有操作码都返回 `NVME_INVALID_OPCODE | NVME_DNR`，
 * 表示无效的操作码。
*/
static uint16_t zns_admin_cmd(FemuCtrl *n, NvmeCmd *cmd)
{
    switch (cmd->opcode) {
    default:
        return NVME_INVALID_OPCODE | NVME_DNR;
    }
}

static inline uint16_t zns_zone_append(FemuCtrl *n, NvmeRequest *req)
{
    return zns_do_write(n, req, true, false);
}

//默认支持dulbe
static uint16_t zns_check_dulbe(NvmeNamespace *ns, uint64_t slba, uint32_t nlb)
{
    return NVME_SUCCESS;
}

//dz added
double update_workload_pressure(struct zns * zns,uint64_t current_time) {
    struct zns_ssdparams * spp = &zns->sp;
    uint64_t during_time = 0;
    // uint64_t current_time = qemu_clock_get_ns(QEMU_CLOCK_REALTIME);

    //计算上次到现在时间
    if(last_time != 0){
        during_time = current_time - last_time;
    }
    // printf("距离上次打印过了 %lu \n",during_time);
    if(during_time != 0)
    {
        //更新每一个workload的压力
        for(int i = 0 ;i<MAX_WORKLOADS;i++ ){
            Workload* workload = &workloads[i];
            //收集die 的繁忙时间
            uint64_t die_busytime= 0;
            uint64_t chnl_busytime= 0;
            for(int j = 0; j<spp->nchnls*spp->ways*spp->dies_per_chip;j++)
            {
                uint16_t die_index = workload->local_dies_for_workload[j];
                uint16_t chnnl_index = die_index/(spp->dies_per_chip*spp->ways);
                uint64_t plane_busytime = 0;
                for(int k = 0;k<spp->planes_per_die;k++)
                {
                    uint16_t plane_index = die_index*spp->planes_per_die+k;
                    zns_ssd_plane *plane = &(zns->planes[plane_index]);
                    plane_busytime+=plane->rw_time;

                }
                die_busytime+=plane_busytime/spp->planes_per_die;
                zns_ssd_channel *chnl = &(zns->ch[chnnl_index]);
                chnl_busytime+=chnl->transfer_time;
            }
            die_busytime = die_busytime / (spp->nchnls*spp->ways*spp->dies_per_chip);
            chnl_busytime = chnl_busytime / (spp->nchnls*spp->ways*spp->dies_per_chip);
            //压力为繁忙时间与空闲时间的比值
            double lagger = die_busytime > chnl_busytime ? die_busytime : chnl_busytime;
            // printf("%.3f ",lagger);
            workload->pressure = (double)lagger/(during_time);
        }
        //重置
        for(int i = 0 ;i<MAX_WORKLOADS;i++ ){
            Workload* workload = &workloads[i];
            for(int j = 0; j<spp->nchnls*spp->ways*spp->dies_per_chip;j++)
            {
                uint16_t die_index = workload->local_dies_for_workload[j];
                uint16_t chnnl_index = die_index/(spp->dies_per_chip*spp->ways);
                for(int k = 0;k<spp->planes_per_die;k++)
                {
                    uint16_t plane_index = die_index*spp->planes_per_die+k;
                    zns_ssd_plane *plane = &(zns->planes[plane_index]);
                    plane->rw_time = 0;
                }      
                zns_ssd_channel *chnl = &(zns->ch[chnnl_index]);
                chnl->transfer_time = 0;
            }
        }
        // printf("\n");
        //更新时间
        last_time = current_time;
    }
}

//每次打印的时候更新
void print_pressure_info(struct zns * zns){

    // printf("距离上次打印 添加记录数:");
    // for(int i = 0;i<MAX_WORKLOADS;i++){
    //     printf("Workload%d: %lu |",i,add_count[i]);
    //     add_count[i]=0;
    // }
    // printf("\n");
    printf("workload 压力情况:");
    for(int i = 0;i<MAX_WORKLOADS;i++)
    {
        printf("Workload%d: %.3f |",i,workloads[i].pressure);
    }
    printf("\n");
}
int compare(const void *a, const void *b) {
    return ((ZoneReset*)b)->reset_count - ((ZoneReset*)a)->reset_count;
}




//新加的   这里说明了  femu的zns实现是单线程所以加锁是无意义的
static uint64_t znsssd_write(FemuCtrl *n, NvmeRequest *req){
    //FEMU only supports 1 namespace for now (see femu.c:365)
    //and FEMU ZNS Extension use a single thread which mean lockless operations(ch->available_time += ~~) if thread increased
    
    //最重要的就是拿到参数然后  其实逻辑块地址以及要操作的数量
    struct zns * zns = n->zns;
    NvmeRwCmd *rw = (NvmeRwCmd *)&req->cmd;
    uint64_t slba = le64_to_cpu(rw->slba);
    uint32_t nlb = (uint32_t)le16_to_cpu(rw->nlb) + 1;
    struct NvmeNamespace *ns = req->ns;
    struct zns_ssdparams * spp = &zns->sp; 

    //初始化一系列时延时间参数
    //zns_ssd_lun *chip = NULL;
    zns_ssd_plane *plane = NULL;

    uint64_t currlat = 0, maxlat= 0;

    //uint32_t my_chip_idx = 0;
    uint32_t my_plane_idx = 0;
    uint64_t nand_stime =0;

    uint64_t cmd_stime = 0;
    zns_ssd_channel *chnl =NULL;


    uint32_t my_chnl_idx = 0;
    uint64_t chnl_stime =0;

    //更新请求开始的时间
    if (req->stime == 0) {
        cmd_stime = qemu_clock_get_ns(QEMU_CLOCK_REALTIME);
    }else{
        cmd_stime = req->stime;
    }

    //femu_err("PROFILING znsssd_write %lu\n", (req->expire_time -req->stime));
    // 384 = 192K 45 0us 65us
    // 96  = 48K  45 0/4 65/4
    // 32  = 16K  
    // 8   = 
    //说明一次写16KiB   一页   不是512
    for (uint64_t i = 0; i<nlb ; i+=(ZNS_PAGE_SIZE / 512)){
        
        slba += i;
        //femu_err("[TEST] zns.c:1295 i:%lu slba:%lu nlb:%u ppa:%lu chidx%lu chnnl:%lu \n",
        //i, slba, nlb,zns_get_multiway_ppn_idx(req->ns,slba), zns_get_multiway_chip_idx(req->ns,slba), 
        //zns_advanced_chnl_idx(req->ns,slba));
        /*
        #if SK_HYNIX_VALIDATION
                my_chip_idx=hynix_zns_get_lun_idx(ns,slba); //SK Hynix
        #endif
        #if !(SK_HYNIX_VALIDATION)
                my_chip_idx=zns_get_multiway_chip_idx(ns, slba); 
        #endif
                chip = &(zns->chips[my_chip_idx]);
        #if !(ADVANCE_PER_CH_ENDTIME)
                //Inhoinno:  Single thread emulation so assume we dont need lock per chnl
                nand_stime = (chip->next_avail_time < cmd_stime) ? cmd_stime : \
                            chip->next_avail_time;
                chip->next_avail_time = nand_stime + spp->pg_wr_lat;
                currlat= chip->next_avail_time - cmd_stime ; //Inhoinno : = T_channel + T_chip(=chnl->next_available_time) - stime; // FIXME like this 
                maxlat = (maxlat < currlat)? currlat : maxlat;
        #endif
        */
#if ADVANCE_PER_CH_ENDTIME
#if SK_HYNIX_VALIDATION
        my_chnl_idx = hynix_zns_get_chnl_idx(ns, slba); //SK Hynix
#endif

        if(zns->allocateType == STATIC || zns->allocateType == DYNAMIC){
            // my_chnl_idx = dz_zns_advanced_chnl_idx(n,ns, slba);
            // my_plane_idx = dz_zns_advanced_plane_idx(n,ns, slba); 
            dz_zns_advanced_resource(n, ns, slba, &my_chnl_idx, &my_plane_idx);
            // femu_log("读了 chnl [%u] die[%u] plane[%u]\n",my_chnl_idx,my_plane_idx/4,my_plane_idx);
        }else{
            my_chnl_idx=zns_advanced_chnl_idx(ns, slba); 
            my_plane_idx=zns_advanced_plane_idx(ns, slba);
        }
        chnl = &(zns->ch[my_chnl_idx]);
        plane= &(zns->planes[my_plane_idx]);

        chnl->transfer_time+=spp->ch_xfer_lat;
        plane->rw_time+=spp->pg_wr_lat;
        int workload = zns_workload_idx(ns, slba);
        add_count[workload]++;


        if(last_time==0){
            last_time = cmd_stime;
        }else{
            //1s = 1000000000
            if(cmd_stime-last_time>=10000000){ 
                update_workload_pressure(zns,cmd_stime);
                // print_pressure_info(zns);
            }
        }


        //pthread_spin_lock(&(chnl->time_lock));
        
        //更新通道时间就是传输时延
        chnl_stime = (chnl->next_ch_avail_time < cmd_stime) ? cmd_stime : \
                     chnl->next_ch_avail_time;

        // int64_t chnl_free_time = (cmd_stime - chnl->next_ch_avail_time);
        // double Response_Ratio = ((double)chnl_free_time/spp->ch_xfer_lat);       

        chnl->next_ch_avail_time = chnl_stime + spp->ch_xfer_lat;
        //pthread_spin_unlock(&(chnl->time_lock));
        #ifdef RESOURCE_UTIL_LOG 
        femu_log("chnl [%u] status busy [%lu] from %lu to %lu [sqid %u] write", my_chnl_idx, qemu_clock_get_ns(QEMU_CLOCK_REALTIME), chnl_stime, chnl->next_ch_avail_time, req->sq->sqid);
        #endif

        //write: then do NAND program
        //pthread_spin_lock(&(chip->time_lock)); 
        //写命令的话是先传
        nand_stime = (plane->next_avail_time < chnl->next_ch_avail_time) ? \
            chnl->next_ch_avail_time : plane->next_avail_time;

        // int64_t plane_free_time = chnl->next_ch_avail_time - plane->next_avail_time;
        // Response_Ratio = Response_Ratio > ((double)plane_free_time/spp->pg_wr_lat)?Response_Ratio:((double)plane_free_time/spp->pg_wr_lat);
        // int workload = zns_workload_idx(ns, slba);
        // if(Response_Ratio<-20.0) Response_Ratio = -20.0;
        // else if(Response_Ratio>20.0) Response_Ratio = 20.0;

        // uint32_t zone_idx = zns_zone_idx(ns, slba);
        // if((zone_idx%(zns->num_zones/MAX_WORKLOADS))!=0){
        //     add_request_record(workload,Response_Ratio);
        // }

        plane->next_avail_time = nand_stime + spp->pg_wr_lat;


        currlat = plane->next_avail_time - cmd_stime;
        //pthread_spin_unlock(&(chip->time_lock));
        #ifdef RESOURCE_UTIL_LOG 
        femu_log("plane [%u] status busy [%lu] from %lu to %lu [sqid %u] write\n", my_plane_idx, qemu_clock_get_ns(QEMU_CLOCK_REALTIME), nand_stime, plane->next_avail_time, req->sq->sqid );
        #endif
        maxlat = (maxlat < currlat)? currlat : maxlat;
#endif
    //femu_err("PROFILING znsssd_write %lu\n", (req->expire_time -req->stime));

    }
    return maxlat;

}
static uint64_t  znsssd_read(FemuCtrl *n, NvmeRequest *req){
    // FEMU only supports 1 namespace for now (see femu.c:365) 
    // and FEMU ZNS Extension use a single thread which mean lockless operations(ch->available_time += ~~) if thread increased 
    //最重要的就是拿到参数然后  其实逻辑块地址以及要操作的数量
    struct zns * zns = n->zns;
    NvmeRwCmd *rw = (NvmeRwCmd *)&req->cmd;
    uint64_t slba = le64_to_cpu(rw->slba);
    uint32_t nlb = (uint32_t)le16_to_cpu(rw->nlb) + 1;
    struct NvmeNamespace *ns = req->ns;
    struct zns_ssdparams * spp = &zns->sp; 
    // uint32_t zone_idx = zns_zone_idx(ns,slba);
    // femu_log("znsssd_read: zone %lu上 %lu \n",zone_idx,nlb);

    //初始化一系列时延时间参数
    //zns_ssd_lun *chip = NULL;
    zns_ssd_plane *plane = NULL;

    uint64_t currlat = 0, maxlat= 0;

    //uint32_t my_chip_idx = 0;
    uint32_t my_plane_idx = 0;
    uint64_t nand_stime =0;

    uint64_t cmd_stime = (req->stime == 0) ? qemu_clock_get_ns(QEMU_CLOCK_REALTIME) : req->stime ;
    zns_ssd_channel *chnl =NULL;

    uint32_t my_chnl_idx = 0;
    uint64_t chnl_stime =0;
    //uint64_t zidx= zns_zone_idx(ns, slba);
    //uint64_t slpa = (slba >> 3) / (ZNS_PAGE_SIZE/MIN_DISCARD_GRANULARITY);
    // 8:4K 32:16K 64:32K 128:64K
    for (uint64_t i = 0; i<nlb ; i+=(ZNS_PAGE_SIZE / 512)){
        slba += i;


        if(zns->allocateType == STATIC || zns->allocateType == DYNAMIC){
            // my_chnl_idx = dz_zns_advanced_chnl_idx(n,ns, slba);
            // my_plane_idx = dz_zns_advanced_plane_idx(n,ns, slba); 
            dz_zns_advanced_resource(n, ns, slba, &my_chnl_idx, &my_plane_idx);
            // femu_log("读了 chnl [%u] die[%u] plane[%u]\n",my_chnl_idx,my_plane_idx/4,my_plane_idx);
        }else{
            my_chnl_idx=zns_advanced_chnl_idx(ns, slba); 
            my_plane_idx=zns_advanced_plane_idx(ns, slba);
        }
        //chip = &(zns->chips[my_chip_idx]);
        chnl = &(zns->ch[my_chnl_idx]);
        plane= &(zns->planes[my_plane_idx]);

        chnl->transfer_time+=spp->ch_xfer_lat;
        plane->rw_time+=spp->pg_rd_lat;
        int workload = zns_workload_idx(ns, slba);
        add_count[workload]++;
        if(last_time==0){
            last_time = cmd_stime;
        }else{
            //1s = 1000000000
            if(cmd_stime-last_time>=10000000){ 
                update_workload_pressure(zns,cmd_stime);
                // print_pressure_info(zns);
            }
        }

        //Inhoinno:  Single thread emulation so assume we dont need lock per chnl
             
        //pthread_spin_lock(&(chip->time_lock));
        
        //GET PLANE AVAILABLE TIME
        nand_stime = (plane->next_avail_time < cmd_stime) ? cmd_stime : \
                     plane->next_avail_time;

        // int64_t plane_free_time = (cmd_stime - plane->next_avail_time);
        // double Response_Ratio = (double)plane_free_time / spp->pg_rd_lat;
        
        //pthread_spin_unlock(&(chip->time_lock));
        //NAND READ OPERATION HERE
        //pthread_spin_lock(&(chip->time_lock));
        
        plane->next_avail_time = nand_stime + spp->pg_rd_lat;
        //pthread_spin_unlock(&(chip->time_lock));
        //

        //read: then data transfer through channel
        //pthread_spin_lock(&(chnl->time_lock));
        chnl_stime = (chnl->next_ch_avail_time < plane->next_avail_time) ? \
            plane->next_avail_time : chnl->next_ch_avail_time;
        
        // int64_t chnl_free_time = (plane->next_avail_time - chnl->next_ch_avail_time);
        // Response_Ratio = Response_Ratio > ((double)chnl_free_time/spp->ch_xfer_lat)?Response_Ratio:((double)chnl_free_time/spp->ch_xfer_lat);
        // int workload = zns_workload_idx(ns, slba);
        // if(Response_Ratio<-20.0) Response_Ratio = -20.0;
        // else if(Response_Ratio>20.0) Response_Ratio = 20.0;

        // uint32_t zone_idx = zns_zone_idx(ns, slba);
        // if((zone_idx%(zns->num_zones/MAX_WORKLOADS))!=0){
        //     add_request_record(workload,Response_Ratio);
        // }
        

        chnl->next_ch_avail_time = chnl_stime + spp->ch_xfer_lat;
        //想实现缓存机制
        //IF REGISTER IS AVAIL THEN = 
        //ELSE REGISTER IS FULL THEN PLANE NEXT AVAIL TIME = chnl->next_ch_avail_time
        //pthread_spin_unlock(&(chnl->time_lock));

        //if register full in plane i 
        //      then plane->next_avail_time = chnl->next_ch_avail_time
        //      
        //else if register is not full 

        //femu_log("chnl %u status busy [%lu] from %lu to %lu ", my_chnl_idx, qemu_clock_get_ns(QEMU_CLOCK_REALTIME),chnl_stime, chnl->next_ch_avail_time);
        //femu_log("chip [%u] status busy from %lu to %lu (r)\n", my_chip_idx, nand_stime,chip->next_avail_time );
        #ifdef RESOURCE_UTIL_LOG 
        femu_log("chnl [%u] status busy [%lu] from %lu to %lu [sqid %u] read", my_chnl_idx, qemu_clock_get_ns(QEMU_CLOCK_REALTIME), chnl_stime, chnl->next_ch_avail_time, req->sq->sqid);
        femu_log("plane [%u] status busy [%lu] from %lu to %lu [sqid %u] read\n", my_plane_idx, qemu_clock_get_ns(QEMU_CLOCK_REALTIME), nand_stime, plane->next_avail_time,req->sq->sqid );
        #endif
        currlat = chnl->next_ch_avail_time - cmd_stime;
        maxlat = (maxlat < currlat)? currlat : maxlat;
        //femu_log("ztrace %lu zidx %lu slpa %lu cidx %u \n", qemu_clock_get_ns(QEMU_CLOCK_REALTIME), zidx, slpa, my_chip_idx);

    }

    return maxlat;
}

/**
 * @brief 
 * 
 * @param n 
 * @param ns 
 * @param cmd 
 * @param req 
}*/
static uint64_t znssd_reset_zones(ZNS *zns, NvmeRequest *req){
    NvmeCmd *cmd = (NvmeCmd *)&req->cmd;
    NvmeNamespace *ns = req->ns;
    FemuCtrl *n = ns->ctrl;
    struct zns_ssdparams * spp = &zns->sp;
    //NvmeZone *zone;
    uint32_t zone_idx = 0;
    zns_ssd_lun *chip = NULL;
    uint32_t chip_idx=0;
    uint32_t chip_start_idx=0;
    //zns_ssd_channel *chnl =NULL;

    uint64_t slba = 0;
    uint64_t cmd_stime = (req->stime == 0) ? qemu_clock_get_ns(QEMU_CLOCK_REALTIME) : req->stime ;

    uint64_t maxlat=0;
    uint64_t lat =0;
    //uint16_t status;

    zns_get_mgmt_zone_slba_idx(n, cmd, &slba, &zone_idx);
#if SK_HYNIX_VALIDATION
    chip_idx = zone_idx % (spp->nchnls * spp->ways);
    chip = &(zns->chips[chip_idx]);
    chip->next_avail_time = (chip->next_avail_time > cmd_stime)? chip->next_avail_time + ZONE_RESET_LATENCY : cmd_stime + ZONE_RESET_LATENCY;
    return (chip->next_avail_time - cmd_stime);
#endif
    //default
    chip_start_idx = zone_idx % (spp->nchnls / spp->chnls_per_zone);
    chip_idx = chip_start_idx;
    n->zone_array[zone_idx].cnt_reset +=1;
    if(zns->allocateType == DYNAMIC || zns->allocateType ==STATIC){
        dz_reset_zone(zns,zone_idx,&n->zone_array[zone_idx]);
    }


    for(uint64_t ass=0; ass < spp->chnls_per_zone; ass++){
        for(uint64_t i =0 ; i < spp->ways ; i++){
            chip_idx += (i*spp->nchnls);
            chip = &(zns->chips[chip_idx]);
            chip->next_avail_time = (chip->next_avail_time > cmd_stime)? chip->next_avail_time + ZONE_RESET_LATENCY : cmd_stime + ZONE_RESET_LATENCY;
            lat = chip->next_avail_time - cmd_stime;
            maxlat = (maxlat < lat) ? lat : maxlat;
        }
        chip_idx += 1; 
    }
    return maxlat;
}
//延迟模拟三种情况  读 写 和reset  返回的是 请求完成的时间
static int zns_advance_status(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd, NvmeRequest *req){
    
    NvmeRwCmd *rw = (NvmeRwCmd *)&req->cmd;
    uint8_t opcode = rw->opcode;
    uint32_t dw13 = le32_to_cpu(cmd->cdw13);

    uint8_t action;
    action = dw13 & 0xff;

    // Zone Reset 
    if (action == NVME_ZONE_ACTION_RESET){
        //reset zone->wp and zone->status=Empty
        //reset zone, causing every chip lat +
        return znssd_reset_zones(n->zns,req);
    }
    // Read, Write 
    assert(opcode == NVME_CMD_WRITE || opcode == NVME_CMD_READ || opcode == NVME_CMD_ZONE_APPEND);
    if(req->is_write)
        return znsssd_write(n, req);
    return znsssd_read(n, req);
}

static uint16_t zns_read(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd,
                         NvmeRequest *req)
{
    NvmeRwCmd *rw = (NvmeRwCmd *)&req->cmd;
    uint64_t slba = le64_to_cpu(rw->slba);
    uint32_t nlb = (uint32_t)le16_to_cpu(rw->nlb) + 1;
    uint64_t data_size = zns_l2b(ns, nlb);
    uint64_t data_offset;
    uint16_t status;
    struct zns *zns = n->zns;

    // uint32_t zone_idx = zns_zone_idx(ns,slba);
    // //判断是否是元数据区域
    // if((zone_idx%(zns->num_zones/MAX_WORKLOADS))!=0){
    //     add_record(zone_idx,nlb,'r',zone_idx/(zns->num_zones/MAX_WORKLOADS));
    // }
    // if(zns_workload_idx(ns,slba)>=1){
    //     printf("workload %d zone %u 读了",zns_workload_idx(ns,slba),zone_idx);
    // }

    
#if PCIe_TIME_SIMULATION
    uint64_t nk = nlb/2;
    uint64_t delta_time = (uint64_t)nk*pow(10,9);   //n KB > 4096*1KB*2^10:10^9ns = 1KB : (10^9 / 2^10 / 4096)ns
    //femu_err("[Inho ] delt : %lx            ",delta_time);
    delta_time = delta_time/pow(2,10)/(Interface_PCIeGen3x4_bw);
    PCIe_Gen3_x4 * pcie = n->pci_simulation;
#endif
    assert(n->zoned);
    req->is_write = false;

    status = nvme_check_mdts(n, data_size);
    if (status) {
        femu_err("nvme_check_mdts status %d %x\n",status,status);
        goto err;
    }

    status = zns_check_bounds(ns, slba, nlb);
    if (status) {
        femu_err("zns_check_bounds status d:%d x:%x slba:%lu nlb:%u\n",status,status,slba,nlb);
        goto err;
    }

    status = zns_check_zone_read(ns, slba, nlb);
    
    if (status) {
        femu_err("zns_check_zone_read status %d %x\n",status,status);
        goto err;
    }

    status = zns_map_dptr(n, data_size, req);
    
    if (status) {
        femu_err("zns_map_dptr status %d %x\n",status,status);
        goto err;
    }

    if (NVME_ERR_REC_DULBE(n->features.err_rec)) {
        femu_err("n->features.err_rec %d %x\n",status,status);
        status = zns_check_dulbe(ns, slba, nlb);
        if (status) {
            femu_err("zns_check_dulbe %d %x\n",status,status);
            goto err;
        }
    }
    // femu_err("read success?? slba %lu\n",slba);
    data_offset = zns_l2b(ns, slba);
    req->expire_time += zns_advance_status(n,ns,cmd,req);
    /*PCI latency model here*/

#if PCIe_TIME_SIMULATION
    //lock
    //pthread_spin_lock(&n->pci_lock);
    if(pcie->ntime + 2000 <  req->stime ){
        lag=0;
        pcie->stime = req->stime;
        pcie->ntime = pcie->stime + Interface_PCIeGen3x4_bwmb/NVME_DEFAULT_MAX_AZ_SIZE/1000 * delta_time;
        req->expire_time += 968*(req->nlb/8);
    }else if(pcie->ntime < (pcie->stime + delta_time)){
        //update lag
        lag = (pcie->ntime - req->stime);
        pcie->stime = pcie->ntime;
        pcie->ntime = pcie->stime + Interface_PCIeGen3x4_bwmb/NVME_DEFAULT_MAX_AZ_SIZE/1000 * delta_time; //1ms
        req->expire_time += lag;
        pcie->stime += delta_time;
    }else if (req->stime < pcie->ntime && lag != 0 ){
        req->expire_time+=lag;
    }
    pcie->stime += delta_time;
    //femu_err("[inho] lag : %lx\n", lag);
    //pthread_spin_unlock(&n->pci_lock);
#endif
    //unlock
    backend_rw(n->mbe, &req->qsg, &data_offset, req->is_write);
    return NVME_SUCCESS;

err:
    return status | NVME_DNR;
}


int find_low_pressure_workload()
{
    int min_pressure_index = 0;
    double min_pressure = workloads[0].pressure;

    for(int i = 1; i < MAX_WORKLOADS; i++) {
        if(workloads[i].pressure < min_pressure) {
            // if(i == last) continue;
            min_pressure = workloads[i].pressure;
            min_pressure_index = i;
        }
    }
    if(min_pressure<=0.4){
        last = min_pressure_index;
        return min_pressure_index;
    }
    else{
        last = -1;
        return -1;
    } 
}

int find_min_wear_workload(int workload){
    int min_wear_index = workload;
    double min_wear = workloads[workload].reset_count;

    for(int i = 0; i < MAX_WORKLOADS; i++) {
        if(workloads[i].reset_count < min_wear*0.95 && workloads[i].pressure<0.1) {
            // if(i == last) continue;
            min_wear = workloads[i].reset_count;
            min_wear_index = i;
        }
    }

    last = min_wear_index;
    return min_wear_index;

}

//dz added
void zone_remapping(uint64_t zidx,struct zns * zns ,NvmeZone *zone,int workload){
    // printf("====================================为zone %ld 分配资源====================================\n",zidx);
    // print_pressure_info(zns);

    if(zone->d.stealing!=workload){
        // printf("属于workload %d 的 zone %d 之前窃取了workload%d 后者被偷了%u\n",workload,zidx,zone->d.stealing,workloads[zone->d.stealing].stolen);
        workloads[zone->d.stealing].stolen--;
        zone->d.stealing = workload;
    }

    // 判断是否需要窃取  压力很大  且   zone reset 频繁
    if(workloads[workload].pressure>0)
    {
        // printf("workload %d 可能会压力过大\n",workload);
        // int min_pressure_index = find_low_pressure_workload();
        int min_pressure_index = find_min_wear_workload(workload);

        if(min_pressure_index!=-1){

            // if(zidx % (zns->num_zones/MAX_WORKLOADS)<65 && workloads[min_pressure_index].reset_count< workloads[workload].reset_count ){
            if(zidx % (zns->num_zones/MAX_WORKLOADS)<=128){
                zone->d.stealing = min_pressure_index;
                workloads[min_pressure_index].stolen++;
                // printf("属于workload %d 的 zone %d 窃取了workload%d 后者被偷了%u\n",workload,zidx,min_pressure_index,workloads[min_pressure_index].stolen);
            }

        }
    }
    //80

    if(zone->d.is_mapped==false){
        // zns_allocate_next_blkgrp(zns,zone);
        if(zidx % (zns->num_zones/MAX_WORKLOADS)<=16){
            zns_allocate_youngest_blkgrp(zns,zone);
        }else if(zidx % (zns->num_zones/MAX_WORKLOADS)>16&&zidx % (zns->num_zones/MAX_WORKLOADS)<128){
            // zns_allocate_youngest_blkgrp(zns,zone);
            zns_allocate_older_blkgrp(zns,zone);
        }else{
            // zns_allocate_youngest_blkgrp(zns,zone);
            zns_allocate_older_blkgrp(zns,zone);
            //zns_allocate_oldest_blkgrp(zns,zone);
            
        }
    }




    ////冷数据迁移逻辑
    enqueue(workloads[workload].openzones,zidx);
    printf(" workload %d queue add %d :",workload,zidx);
    printQueue(workloads[workload].openzones);

    // printf("zone %d 分配完后:",zidx);
    // for(int j = 0;j<die_num;j++){
    //     printf(" %u ", zone->d.local_dies[j]);
    // }
    // printf("\n");
    
    // for(int j = 0;j<die_num;j++){
    //     printf(" %u ", zone->d.local_blkgrps[j]);
    // }
    // printf("\n");

}

static uint16_t zns_write(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd,
                          NvmeRequest *req)
{
    struct zns *zns = n->zns;
    NvmeRwCmd *rw = (NvmeRwCmd *)cmd;
    uint64_t slba = le64_to_cpu(rw->slba);
    uint32_t nlb = (uint32_t)le16_to_cpu(rw->nlb) + 1;
    uint64_t data_size = zns_l2b(ns, nlb);
    uint64_t data_offset;
    NvmeZone *zone;
    NvmeZonedResult *res = (NvmeZonedResult *)&req->cqe;
    uint16_t status;
    uint64_t zidx = zns_zone_idx(ns, slba);
    uint64_t err_zidx = 0;

    // femu_log("zns_write: zone%lu 上%lu \n",zidx,nlb);
    zone = zns_get_zone_by_slba(ns, slba);


    // if(zns->allocateType == DYNAMIC && zone->d.is_mapped==false)
    // {
    //     zone_remapping(zidx,zns,zone,zidx/(zns->num_zones/MAX_WORKLOADS));
    // }

    if((zns->allocateType == DYNAMIC ||zns->allocateType == STATIC ||zns->allocateType == CONFZNS) && zone->d.is_mapped==false)
    {
        zone_remapping(zidx,zns,zone,zidx/(zns->num_zones/MAX_WORKLOADS));
    }




    assert(n->zoned);
    req->is_write = true;

    status = nvme_check_mdts(n, data_size);
    if (status) {
        goto err;
    }

    status = zns_check_bounds(ns, slba, nlb);
    if (status) {
        femu_err("zns check bounds [pid %x] slba : %lx , nlb : %x\n", getpid(), slba, nlb);
        goto err;
    }

    

    status = zns_check_zone_write(n, ns, zone, slba, nlb, false);
    if (status) {
        err_zidx = zidx;
        femu_err("in zns_check_zone_write [pid %x] Zidx : %lx z.wtp : %lx , slba : %lx , nlb : %x\n", getpid() ,zidx, zone->w_ptr, slba, nlb);
        goto err;
    }
    // //默认不会进来
    // if(err_zidx > (1<<MK_ZONE_CONVENTIONAL)){
    //     femu_err("in errzidx:%lx [pid %x] Zidx : %lx z.wtp : %lx , slba : %lx, nlb : %x \n", err_zidx, getpid() ,zidx, zone->w_ptr, slba, nlb);
    // }

    status = zns_auto_open_zone(ns, zone);
    if (status) {
        goto err;
    }

    res->slba = zns_advance_zone_wp(ns, zone, nlb);

    data_offset = zns_l2b(ns, slba);

    status = zns_map_dptr(n, data_size, req);
    if (status) {
        goto err;
    }
    req->expire_time += zns_advance_status(n,ns,cmd,req);
    backend_rw(n->mbe, &req->qsg, &data_offset, req->is_write);
    zns_finalize_zoned_write(ns, req, false);

    return NVME_SUCCESS;

err:
    femu_err("*********ZONE WRITE FAILED*********\n");
    return status | NVME_DNR;
}

static uint16_t zns_io_cmd(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd,
                           NvmeRequest *req)
{
    switch (cmd->opcode) {
    case NVME_CMD_READ:
        // femu_log("ZNS READ cmd->opcode %d %x\n",cmd->opcode, cmd->opcode);
        return zns_read(n, ns, cmd, req);
    case NVME_CMD_WRITE:
        // femu_log("ZNS WRITE cmd->opcode %d %x\n",cmd->opcode, cmd->opcode);
        return zns_write(n, ns, cmd, req);
    case NVME_CMD_ZONE_MGMT_SEND:
        return zns_zone_mgmt_send(n, req);
    case NVME_CMD_ZONE_MGMT_RECV:
        return zns_zone_mgmt_recv(n, req);
    case NVME_CMD_ZONE_APPEND:
        return zns_zone_append(n, req);
    }

    return NVME_INVALID_OPCODE | NVME_DNR;
}

static void zns_set_ctrl_str(FemuCtrl *n)
{
    static int fsid_zns = 0;
    const char *zns_mn = "FEMU ZNS-SSD Controller";
    const char *zns_sn = "vZNSSD";

    nvme_set_ctrl_name(n, zns_mn, zns_sn, &fsid_zns);
}

static void zns_set_ctrl(FemuCtrl *n)
{
    uint8_t *pci_conf = n->parent_obj.config;

    zns_set_ctrl_str(n);
    pci_config_set_vendor_id(pci_conf, PCI_VENDOR_ID_INTEL);
    pci_config_set_device_id(pci_conf, 0x5845);
}

static int zns_init_zone_cap(FemuCtrl *n)
{
    n->zoned = true;
    n->zasl_bs = NVME_DEFAULT_MAX_AZ_SIZE;
    n->zone_size_bs = NVME_DEFAULT_ZONE_SIZE;
    n->zone_cap_bs = 0;
    n->cross_zone_read = false;
    n->max_active_zones = 0;
    n->max_open_zones = 0;
    n->zd_extension_size = 0;

    return 0;
}

static int zns_start_ctrl(FemuCtrl *n)
{
    /* Coperd: let's fail early before anything crazy happens */
    assert(n->page_size == 4096);

    if (!n->zasl_bs) {
        n->zasl = n->mdts;
    } else {
        if (n->zasl_bs < n->page_size) {
            femu_err("ZASL too small (%dB), must >= 1 page (4K)\n", n->zasl_bs);
            return -1;
        }
        n->zasl = 31 - clz32(n->zasl_bs / n->page_size);
    }

    return 0;
}

static void zns_init(FemuCtrl *n, Error **errp)
{
    NvmeNamespace *ns = &n->namespaces[0];

    zns_set_ctrl(n);

    zns_init_zone_cap(n);

    if (zns_init_zone_geometry(ns, errp) != 0) {
        return;
    }

    zns_init_zone_identify(n, ns, 0);
    //延迟模拟所需 配置zns相关信息。
    znsssd_init(n);
}
static void znsssd_init_params(FemuCtrl * n, struct zns_ssdparams *spp){
    int wear_leveling = 1;
    spp->pg_rd_lat = NAND_READ_LATENCY/wear_leveling;
    spp->pg_wr_lat = NAND_PROG_LATENCY/wear_leveling;
    spp->blk_er_lat = NAND_ERASE_LATENCY/wear_leveling;
    spp->ch_xfer_lat = NAND_CHNL_PAGE_TRANSFER_LATENCY/wear_leveling;
    /**
     * 1. SSD size  2. zone size 3. # of chnls 4. # of chnls per zone
    */
    spp->nchnls         = 8;   //default : 8                                                   /* FIXME : = ZNS_MAX_CHANNEL channel configuration like this */
    spp->chnls_per_zone = 2;   
    spp->zones          = n->num_zones;     
    spp->ways           = 4;    //default : 2
    spp->ways_per_zone  = 4;    //default :==spp->ways

    spp->dies_per_chip  = 1;    //default : 1
    spp->planes_per_die = 1;    //default : 4  //因为plane默认不可并行 可能是confzns的错误  因此先设置为1
    spp->register_model = 1;    
    /*Inho @ Temporarly, FEMU doesn't support more than 1 namespace. Parameters below is for supporting different zone configurations temporarly*/

    spp->is_another_namespace = false;
    spp->chnls_per_another_zone = 7;
    /* TO REAL STORAGE SIZE */
    spp->csze_pages     = (((int64_t)n->memsz) * 1024 * 1024) / MIN_DISCARD_GRANULARITY / spp->nchnls / spp->ways;
    spp->nchips         = (((int64_t)n->memsz) * 1024 * 1024) / MIN_DISCARD_GRANULARITY / spp->csze_pages;
    femu_log("===========================================\n");
    femu_log("|        ConfZNS HW Configuration()       |\n");      
    femu_log("===========================================\n");
    femu_log("| nchnl       : %lu   | nway      : %lu   |\n",spp->nchnls, spp->ways);
    femu_log("| nchnl/zone  : %lu   | nway/zone : %lu   |\n",spp->chnls_per_zone, spp->ways_per_zone);
    femu_log("| die/chip    : %lu   |           :       |\n",spp->dies_per_chip);
    femu_log("| plane/die   : %lu   |           :       |\n",spp->planes_per_die);
    femu_log("| block       :       |           :       |\n");
    femu_log("| page        : %ldKiB|           :       |\n",(ZNS_PAGE_SIZE/KiB));
    femu_log("===========================================\n");
}

/**
 * @brief 
 * @Inhoinno: we need to make zns ssd latency emulation
 * in order to emulate controller-level mapping in ZNS
 * for example, 1-to-1 mapping or 1-to-All mapping (zone-channel) 
 * @param FemuCtrl for mapping channel for zones
 * @return none 
 */
static void zns_init_ch(struct zns_ssd_channel *ch, struct zns_ssdparams *spp)
{
    ch->next_ch_avail_time = 0;
    ch->transfer_time =0;
    int ret = pthread_spin_init(&(ch->time_lock), PTHREAD_PROCESS_SHARED);
    if(ret)
        femu_err("zns.c:1754 znssd_init(): lock alloc failed, to inhoinno \n");        
}
static void zns_init_chip(struct zns_ssd_lun *ch, struct zns_ssdparams *spp)
{
    ch->next_avail_time = 0;
    
    int ret = pthread_spin_init(&(ch->time_lock), PTHREAD_PROCESS_SHARED);
    if(ret)
        femu_err("zns.c:1754 znssd_init(): lock alloc failed, to inhoinno \n");
}
static void zns_init_plane(struct zns_ssd_plane *pl, struct zns_ssdparams *spp){

    pl->next_avail_time=0;
    pl->nregs=spp->register_model;
    pl->rw_time = 0;

}

void test_print(FemuCtrl * n){
    struct zns *zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp; 


    femu_log("Initial values of dz_unit_allocate:\n");
    for (int j = 0; j < spp->ways * spp->dies_per_chip; j++) {
        for (int i = 0; i < spp->nchnls; i++) {
            printf("%d ", zns->dz_unit_allocate[i][j]);
        }
        printf("\n");
    }

    femu_log("Initial values of dz_unit_using:\n"); 
    for (int j = 0; j < spp->ways * spp->dies_per_chip; j++) {
        for (int i = 0; i < spp->nchnls; i++) {
            printf("%d ", zns->dz_unit_using[i][j]);
        }
        printf("\n");
    }

    // femu_log("Initial values of every die:\n"); 
    // for(int i = 0;i<spp->nchnls*spp->ways*spp->dies_per_chip;i++){
    //     printf("die  %d 上的:blkgrp",i);
    //     for(int j = 0;j<zns->num_zones;j++){
    //         printf(" %u ", zns->dies[i].blkgrps_in_die[j]);
    //     }
    //     printf("\n");
    // }

    femu_log("Initial values of every zone:\n"); 
    for(int i = 0;i<zns->num_zones;i++){
        printf("zone %d :",i);
        for(int j = 0;j<spp->nchnls*spp->ways*spp->dies_per_chip;j++){
            printf(" %u ", zns->zone_array[i].d.local_dies[j]);
        }
        printf("\n");
        
        for(int j = 0;j<spp->nchnls*spp->ways*spp->dies_per_chip;j++){
            printf(" %u ", zns->zone_array[i].d.local_blkgrps[j]);
        }
        printf("\n");
    }


    // femu_log("Initial values of local_dies_for_workload:\n"); 
    // for (int i = 0; i < MAX_WORKLOADS; i++) {
    //     printf("Workload %d: ", i);
    //     for (int j = 0; j < spp->nchnls*spp->ways*spp->dies_per_chip; j++) {
    //         printf("%u ", workloads[i].local_dies_for_workload[j]);
    //     }
    //     printf("\n");
    // }

    // print_workloads();
}

void znsssd_init(FemuCtrl * n){
    struct zns *zns = n->zns = g_malloc0(sizeof(struct zns));
    struct zns_ssdparams *spp = &zns->sp; 
    zns->namespaces = n->namespaces;
    znsssd_init_params(n, spp);
    uint64_t nplanes = (spp->ways * spp->planes_per_die* spp->dies_per_chip * spp->nchnls);
    last = -1;
    femu_log("zns.c:1820 znssd_init(): nplanes %ld spp->ways %ld spp->planes_per_die %ld\
             spp->dies_per_chip %ld \
             spp->nchnls %ld \n ", nplanes, spp->ways, spp->planes_per_die, spp->dies_per_chip, spp->nchnls);
    /* initialize zns ssd internal layout architecture */
    zns->ch     = g_malloc0(sizeof(struct zns_ssd_channel) * spp->nchnls);
    zns->chips  = g_malloc0(sizeof(struct zns_ssd_lun) * spp->nchnls*spp->ways);
    zns->planes = g_malloc0(sizeof(struct zns_ssd_plane) * nplanes);
    zns->zone_array = n->zone_array;
    zns->num_zones = spp->zones;

    /*=================================dz added===============================*/
    zns->dies   = g_malloc0(sizeof(struct zns_ssd_die) * spp->nchnls*spp->ways*spp->dies_per_chip);
    zns->dz_unit_allocate = g_malloc(spp->nchnls * sizeof(uint16_t*));
    for(int i = 0; i < spp->nchnls; i++) {
        zns->dz_unit_allocate[i] = g_malloc(spp->ways*spp->dies_per_chip* sizeof(uint16_t));
        for(int j =0;j<spp->ways*spp->dies_per_chip;j++){
            zns->dz_unit_allocate[i][j]=zns->num_zones;
        }
    }
    zns->dz_unit_using = g_malloc(spp->nchnls * sizeof(uint16_t*));
    for(int i = 0; i < spp->nchnls; i++) {
       zns->dz_unit_using[i] = g_malloc0(spp->ways*spp->dies_per_chip* sizeof(uint16_t));
    }
    for(int i = 0;i<zns->num_zones;i++)
    {
        zns->zone_array[i].d.stealing = i / (spp->zones/MAX_WORKLOADS);
        zns->zone_array[i].d.is_mapped = false;
        zns->zone_array[i].d.local_dies = g_malloc0(sizeof(uint16_t) * spp->nchnls*spp->ways*spp->dies_per_chip);
        zns->zone_array[i].d.local_blkgrps = g_malloc0(sizeof(uint32_t) * spp->nchnls*spp->ways*spp->dies_per_chip);
        // zns->zone_array[i].d.num_unit = spp->nchnls*spp->ways*spp->dies_per_chip;
    }
    zns->blkgrps = g_malloc(sizeof(struct zns_ssd_blkgrp)*spp->nchnls*spp->ways*spp->dies_per_chip*spp->zones);
    zns->blkgrp_size= (n->num_zones * n->zone_size)/(spp->nchnls*spp->ways*spp->dies_per_chip*spp->zones);
    for(int i = 0;i<spp->nchnls*spp->ways*spp->dies_per_chip*spp->zones;i++){
        zns->blkgrps[i].id = i;
        zns->blkgrps[i].is_being_used = false;
        zns->blkgrps[i].belong_2_die = i/(spp->nchnls*spp->ways*spp->dies_per_chip);
        zns->blkgrps[i].erase_cnt = 0;
        zns->blkgrps[i].bsla = i*zns->blkgrp_size;
        zns->blkgrps[i].bela = (i+1)*zns->blkgrp_size;
    }
    //初始化workloads
    for(int i = 0;i<MAX_WORKLOADS;i++)
    {
        workloads[i].zone_reset_count = g_malloc0(sizeof(uint16_t) * spp->zones);
        workloads[i].reset_start_index = 0;
        workloads[i].reset_end_index = 0;
        workloads[i].reset_count = 0;
        workloads[i].pressure = 0;


        workloads[i].openzones = createQueue();

        workloads[i].stolen = 0;
        workloads[i].same_cnt = 0;
        
        workloads[i].local_dies_for_workload = g_malloc0(sizeof(uint16_t) * spp->nchnls*spp->ways*spp->dies_per_chip);
        // zns->zone_array[i].d.num_unit = spp->nchnls*spp->ways*spp->dies_per_chip;
    }
    for(uint32_t i=0 ; i < n->num_zones; i++){
        int ret = pthread_spin_init(&(zns->zone_array[i].w_ptr_lock), PTHREAD_PROCESS_SHARED);
        n->zone_array[i].cnt_reset=0;
        if(ret)
            femu_err("zns.c:1687 znssd_init(): lock alloc failed, to inhoinno \n");
    }

    for (int i = 0; i < spp->nchnls; i++) {
        zns_init_ch(&zns->ch[i], spp);
    }
    for (int i = 0; i < spp->nchnls * spp->ways; i++) {
        zns_init_chip(&zns->chips[i], spp);
    }
    femu_log("初始化每个die\n");
    for (int i = 0; i < spp->nchnls * spp->ways * spp->dies_per_chip; i++) {
        zns->dies[i].next_free_blkgrp = 0;

        zns->dies[i].blkgrps_in_die =  g_malloc0(sizeof(uint32_t) * spp->zones);

        for(int j = 0;j<spp->zones;j++ )
        {
            zns->dies[i].blkgrps_in_die[j]= i * spp->zones +j;
        }
    }
    printf("%lu\n",&zns->dies[0].blkgrps_in_die[0]);

    for (uint64_t i=0; i<nplanes; i++){
        zns_init_plane(&zns->planes[i], spp);
    }
    /*自定义分配 70*/
    zns->allocateType=DYNAMIC;
    switch (zns->allocateType) {
        case STATIC_HORIZONTAL_FIRST:
        {
            // 当type为STATIC_HORIZONTAL_FIRST时执行的代码
            int zone_idx = 0;
            for(int j = 0 ;j < spp->ways;  j += spp->ways_per_zone){
                for(int i = 0 ;i < spp->nchnls;i += spp->chnls_per_zone){
                    while(zns->dz_unit_allocate[i][j]!=0){
                        // test_print(n);
                        uint16_t* dies_arr = malloc(spp->chnls_per_zone * spp->ways_per_zone * sizeof(uint16_t));
                        int cnt = 0;
                        for(int m = 0;m< spp->ways_per_zone; m++){
                            for(int n = 0;n< spp->chnls_per_zone; n++){
                                dies_arr[cnt++]=(i+n)*spp->ways+j+m;
                                zns->dz_unit_allocate[i+n][j+m]-=(spp->nchnls*spp->ways*spp->dies_per_chip)/(spp->chnls_per_zone * spp->ways_per_zone*spp->dies_per_chip);
                                zns->dz_unit_using[i+n][j+m]+=(spp->nchnls*spp->ways*spp->dies_per_chip)/(spp->chnls_per_zone * spp->ways_per_zone*spp->dies_per_chip);
                            }
                        }
                        struct NvmeZone* zone = &zns->zone_array[zone_idx++];
                        for(int m = 0;m<spp->nchnls*spp->ways*spp->dies_per_chip;m++){
                            zone->d.local_dies[m] = dies_arr[m%cnt];
                        }
                        zns_allocate_youngest_blkgrp(zns,zone);
                        free(dies_arr);
                    }          
                }
            }
            break;
        }
        case STATIC_VERTICAL_FIRST:
        {
            // 当type为STATIC_HORIZONTAL_FIRST时执行的代码
            int zone_idx = 0;
            for(int i = 0     ;i < spp->nchnls;i += spp->chnls_per_zone){
                for(int j = 0 ;j < spp->ways;  j += spp->ways_per_zone){
                    while(zns->dz_unit_allocate[i][j]!=0){
                        test_print(n);
                        uint16_t* dies_arr = malloc(spp->chnls_per_zone * spp->ways_per_zone * sizeof(uint16_t));
                        int cnt = 0;
                        for(int n = 0;n< spp->chnls_per_zone;n++){
                            for(int m = 0;m< spp->ways_per_zone;m++){                           
                                dies_arr[cnt++]=(i+n)*spp->ways+j+m;
                                zns->dz_unit_allocate[i+n][j+m]-=(spp->nchnls*spp->ways*spp->dies_per_chip)/(spp->chnls_per_zone * spp->ways_per_zone*spp->dies_per_chip);
                                zns->dz_unit_using[i+n][j+m]+=(spp->nchnls*spp->ways*spp->dies_per_chip)/(spp->chnls_per_zone * spp->ways_per_zone*spp->dies_per_chip);
                            }
                        }
                        struct NvmeZone* zone = &zns->zone_array[zone_idx++];
                        for(int m = 0;m<spp->nchnls*spp->ways*spp->dies_per_chip;m++){
                            zone->d.local_dies[m] = dies_arr[m%cnt];
                        }
                        zns_allocate_youngest_blkgrp(zns,zone);
                        free(dies_arr);
                    }             
                }
            }
            break; 
        }
        case STATIC_MANUAL4411:
        {
            int zone_idx = 0;  
            uint64_t chnls_per_zone;  
            uint64_t ways_per_zone; 

            chnls_per_zone = spp->nchnls/2;
            ways_per_zone = spp->ways;
            //先划分 
            for(int i = 0     ;i < spp->nchnls;i += chnls_per_zone){
                for(int j = 0 ;j < spp->ways;  j += ways_per_zone){
                    //更新构造
                    if(zone_idx>=spp->zones/2){
                        chnls_per_zone = spp->nchnls/4;
                        ways_per_zone = spp->ways;
                    }
                    while(zns->dz_unit_allocate[i][j]!=0){
                        uint16_t* dies_arr = malloc(chnls_per_zone * ways_per_zone * sizeof(uint16_t));
                        int cnt = 0;
                        for(int n = 0;n< chnls_per_zone;n++){
                            for(int m = 0;m< ways_per_zone;m++){                           
                                dies_arr[cnt++]=(i+n)*spp->ways+j+m;
                                zns->dz_unit_allocate[i+n][j+m]-=(spp->nchnls*spp->ways*spp->dies_per_chip)/(chnls_per_zone * ways_per_zone*spp->dies_per_chip);
                                zns->dz_unit_using[i+n][j+m]+=(spp->nchnls*spp->ways*spp->dies_per_chip)/(chnls_per_zone * ways_per_zone*spp->dies_per_chip);
                            }
                        }
                        struct NvmeZone* zone = &zns->zone_array[zone_idx++];
                        for(int m = 0;m<spp->nchnls*spp->ways*spp->dies_per_chip;m++){
                            zone->d.local_dies[m] = dies_arr[m%cnt];
                        }
                        test_print(n);
                        zns_allocate_youngest_blkgrp(zns,zone);
                        free(dies_arr);
                    }   
                }
            }
            for (int i = 0; i < MAX_WORKLOADS; i++) {
                memcpy(workloads[i].local_dies_for_workload, zns->zone_array[i*(spp->zones/MAX_WORKLOADS)].d.local_dies, sizeof(uint16_t) * spp->nchnls*spp->ways*spp->dies_per_chip);
            }
            femu_log("Initial values of local_dies_for_workload:\n"); 
            for (int i = 0; i < MAX_WORKLOADS; i++) {
                printf("Workload %d: ", i);
                for (int j = 0; j < spp->nchnls*spp->ways*spp->dies_per_chip; j++) {
                    printf("%u ", workloads[i].local_dies_for_workload[j]);
                }
                printf("\n");
            }
            break;
        }
        case STATIC_MANUAL4422:
        {
            int zone_idx = 0;  
            uint64_t chnls_per_zone;  
            uint64_t ways_per_zone; 

            chnls_per_zone = spp->nchnls/2;
            ways_per_zone = spp->ways;
            //先划分 
            for(int i = 0     ;i < spp->nchnls;i += chnls_per_zone){
                for(int j = 0 ;j < spp->ways;  j += ways_per_zone){
                    //更新构造
                    if(zone_idx>=spp->zones/2){
                        chnls_per_zone = spp->nchnls/2;
                        ways_per_zone = spp->ways/2;
                    }
                    while(zns->dz_unit_allocate[i][j]!=0){
                        test_print(n);
                        uint16_t* dies_arr = malloc(chnls_per_zone * ways_per_zone * sizeof(uint16_t));
                        int cnt = 0;
                        for(int n = 0;n< chnls_per_zone;n++){
                            for(int m = 0;m< ways_per_zone;m++){                           
                                dies_arr[cnt++]=(i+n)*spp->ways+j+m;
                                zns->dz_unit_allocate[i+n][j+m]-=(spp->nchnls*spp->ways*spp->dies_per_chip)/(chnls_per_zone * ways_per_zone*spp->dies_per_chip);
                                zns->dz_unit_using[i+n][j+m]+=(spp->nchnls*spp->ways*spp->dies_per_chip)/(chnls_per_zone * ways_per_zone*spp->dies_per_chip);
                            }
                        }
                        struct NvmeZone* zone = &zns->zone_array[zone_idx++];
                        for(int m = 0;m<spp->nchnls*spp->ways*spp->dies_per_chip;m++){
                            zone->d.local_dies[m] = dies_arr[m%cnt];
                        }
                        zns_allocate_youngest_blkgrp(zns,zone);
                        free(dies_arr);
                    }   
                }
            }
            for (int i = 0; i < MAX_WORKLOADS; i++) {
                memcpy(workloads[i].local_dies_for_workload, zns->zone_array[i*(spp->zones/MAX_WORKLOADS)].d.local_dies, sizeof(uint16_t) * spp->nchnls*spp->ways*spp->dies_per_chip);
            }
            femu_log("Initial values of local_dies_for_workload:\n"); 
            for (int i = 0; i < MAX_WORKLOADS; i++) {
                printf("Workload %d: ", i);
                for (int j = 0; j < spp->nchnls*spp->ways*spp->dies_per_chip; j++) {
                    printf("%u ", workloads[i].local_dies_for_workload[j]);
                }
                printf("\n");
            }
            break;
        }
        case DYNAMIC:
        {   //50
            dont_reclaim_ = false;
            printf("Size of enum: %zu bytes\n", sizeof(enum ZoneLifetime));
            int zone_idx = 0;  
            uint64_t chnls_per_zone;  
            uint64_t ways_per_zone; 

            chnls_per_zone = spp->nchnls/MAX_WORKLOADS;
            ways_per_zone = spp->ways;
            //先划分 
            for(int i = 0     ;i < spp->nchnls;i += chnls_per_zone){
                for(int j = 0 ;j < spp->ways;  j += ways_per_zone){
                    //更新构造
                    while(zns->dz_unit_allocate[i][j]!=0){                      
                        uint16_t* dies_arr = malloc(chnls_per_zone * ways_per_zone * sizeof(uint16_t));
                        int cnt = 0;
                        for(int m = 0;m< ways_per_zone;m++){  
                            for(int n = 0;n< chnls_per_zone;n++){                         
                                dies_arr[cnt++]=(i+n)*spp->ways+j+m;
                                zns->dz_unit_allocate[i+n][j+m]-=(spp->nchnls*spp->ways*spp->dies_per_chip)/(chnls_per_zone * ways_per_zone*spp->dies_per_chip);
                                zns->dz_unit_using[i+n][j+m]+=(spp->nchnls*spp->ways*spp->dies_per_chip)/(chnls_per_zone * ways_per_zone*spp->dies_per_chip);
                            }
                        }
                        struct NvmeZone* zone = &zns->zone_array[zone_idx++];
                        for(int m = 0;m<spp->nchnls*spp->ways*spp->dies_per_chip;m++){
                            zone->d.local_dies[m] = dies_arr[m%cnt];
                        }
                        // test_print(n);
                        // zns_allocate_next_blkgrp(zns,zone);

                        free(dies_arr);
                    }   
                }
            }
            for (int i = 0; i < MAX_WORKLOADS; i++) {
                memcpy(workloads[i].local_dies_for_workload, zns->zone_array[i*(spp->zones/MAX_WORKLOADS)].d.local_dies, sizeof(uint16_t) * spp->nchnls*spp->ways*spp->dies_per_chip);
            }
            femu_log("Initial values of local_dies_for_workload:\n"); 
            for (int i = 0; i < MAX_WORKLOADS; i++) {
                printf("Workload %d: ", i);
                for (int j = 0; j < spp->nchnls*spp->ways*spp->dies_per_chip; j++) {
                    printf("%u ", workloads[i].local_dies_for_workload[j]);
                }
                printf("\n");
            }           
            break;
        }
        case CONFZNS:
        case STATIC:
        {
            dont_reclaim_ = true;
            printf("Size of enum: %zu bytes\n", sizeof(enum ZoneLifetime));
            int zone_idx = 0;  
            uint64_t chnls_per_zone;  
            uint64_t ways_per_zone; 

            chnls_per_zone = spp->nchnls/MAX_WORKLOADS;
            ways_per_zone = spp->ways;
            //先划分 
            for(int i = 0     ;i < spp->nchnls;i += chnls_per_zone){
                for(int j = 0 ;j < spp->ways;  j += ways_per_zone){
                    //更新构造
                    while(zns->dz_unit_allocate[i][j]!=0){                      
                        uint16_t* dies_arr = malloc(chnls_per_zone * ways_per_zone * sizeof(uint16_t));
                        int cnt = 0;
                        for(int m = 0;m< ways_per_zone;m++){  
                            for(int n = 0;n< chnls_per_zone;n++){                         
                                dies_arr[cnt++]=(i+n)*spp->ways+j+m;
                                zns->dz_unit_allocate[i+n][j+m]-=(spp->nchnls*spp->ways*spp->dies_per_chip)/(chnls_per_zone * ways_per_zone*spp->dies_per_chip);
                                zns->dz_unit_using[i+n][j+m]+=(spp->nchnls*spp->ways*spp->dies_per_chip)/(chnls_per_zone * ways_per_zone*spp->dies_per_chip);
                            }
                        }
                        struct NvmeZone* zone = &zns->zone_array[zone_idx++];
                        for(int m = 0;m<spp->nchnls*spp->ways*spp->dies_per_chip;m++){
                            zone->d.local_dies[m] = dies_arr[m%cnt];
                        }
                        // test_print(n);
                        zns_allocate_next_blkgrp(zns,zone);

                        free(dies_arr);
                    }   
                }
            }
            for (int i = 0; i < MAX_WORKLOADS; i++) {
                memcpy(workloads[i].local_dies_for_workload, zns->zone_array[i*(spp->zones/MAX_WORKLOADS)].d.local_dies, sizeof(uint16_t) * spp->nchnls*spp->ways*spp->dies_per_chip);
            }
            femu_log("Initial values of local_dies_for_workload:\n"); 
            for (int i = 0; i < MAX_WORKLOADS; i++) {
                printf("Workload %d: ", i);
                for (int j = 0; j < spp->nchnls*spp->ways*spp->dies_per_chip; j++) {
                    printf("%u ", workloads[i].local_dies_for_workload[j]);
                }
                printf("\n");
            }           
            break;
        }     
        default:         
            break;
    }

    // test_print(n);

    /*=================================++++++++===============================*/
   
    // for (uint64_t i =0; i < 16 00; i+=16){
    //     femu_err("[TEST] zns.c:1767 slba:%lu  ppa:%lu plane:%lu chidx:%lu chnnl:%lu \n",
    //     i, zns_get_multichnlway_ppn_idx(n->namespaces,i), 
    //     zns_advanced_plane_idx(n->namespaces, i), 
    //     zns_get_multiway_chip_idx(n->namespaces, i), 
    //     zns_advanced_chnl_idx(n->namespaces,i));
    // }
}
static void zns_exit(FemuCtrl *n)
{
    /*
     * Release any extra resource (zones) allocated for ZNS mode
     */   

    struct zns *zns = n->zns;
    struct zns_ssdparams *spp = &zns->sp; 
    g_free(zns->ch);
    g_free(zns->chips);

    for (int i = 0; i < spp->nchnls * spp->ways * spp->dies_per_chip; i++) {

        g_free(zns->dies[i].blkgrps_in_die);

    }
    g_free(zns->dies);
    g_free(zns->planes);
    g_free(zns->blkgrps);
    g_free(zns);
    

    // 释放dz_unit_allocate
    for(int i = 0; i < spp->nchnls; i++) {
        g_free(zns->dz_unit_allocate[i]);
    }
    g_free(zns->dz_unit_allocate);

    // 释放dz_unit_using
    for(int i = 0; i < spp->nchnls; i++) {
        g_free(zns->dz_unit_using[i]);
    }
    g_free(zns->dz_unit_using);

    for(int i = 0;i<zns->num_zones;i++)
    {
        g_free(zns->zone_array[i].d.local_blkgrps);
        g_free(zns->zone_array[i].d.local_dies);
    }

    for(int i = 0;i<MAX_WORKLOADS;i++)
    {
        g_free(workloads[i].zone_reset_count);
        g_free(workloads[i].local_dies_for_workload);
        freeQueue(workloads[i].openzones);
        // zns->zone_array[i].d.num_unit = spp->nchnls*spp->ways*spp->dies_per_chip;
    }

    g_free(n->id_ns_zoned);
    g_free(n->zone_array);
    g_free(n->zd_extensions);

}

int nvme_register_znssd(FemuCtrl *n)
{
    n->ext_ops = (FemuExtCtrlOps) {
        .state            = NULL,
        .init             = zns_init,
        .exit             = zns_exit,
        .rw_check_req     = NULL,
        .start_ctrl       = zns_start_ctrl,
        .admin_cmd        = zns_admin_cmd,
        .io_cmd           = zns_io_cmd,
        .get_log          = NULL,
    };

    return 0;
}
