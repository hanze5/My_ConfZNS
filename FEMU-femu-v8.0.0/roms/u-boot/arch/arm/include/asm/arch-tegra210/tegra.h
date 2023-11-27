/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013-2015
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA210_TEGRA_H_
#define _TEGRA210_TEGRA_H_

#define GICD_BASE		0x50041000	/* Generic Int Cntrlr Distrib */
#define GICC_BASE		0x50042000	/* Generic Int Cntrlr CPU I/F */
#define NV_PA_AHB_BASE		0x6000C000	/* System regs (AHB, etc.) */
#define NV_PA_TSC_BASE		0x700F0000	/* System Counter TSC regs */
#define NV_PA_MC_BASE		0x70019000	/* Mem Ctlr regs (MCB, etc.) */
#define NV_PA_SDRAM_BASE	0x80000000

#include <asm/arch-tegra/tegra.h>

#define BCT_ODMDATA_OFFSET	1288	/* offset to ODMDATA word */

#undef NVBOOTINFOTABLE_BCTSIZE
#undef NVBOOTINFOTABLE_BCTPTR
#define NVBOOTINFOTABLE_BCTSIZE	0x48	/* BCT size in BIT in IRAM */
#define NVBOOTINFOTABLE_BCTPTR	0x4C	/* BCT pointer in BIT in IRAM */

#define MAX_NUM_CPU		4
#define MCB_EMEM_ARB_OVERRIDE	(NV_PA_MC_BASE + 0xE8)

#define TEGRA_USB1_BASE		0x7D000000

#endif /* _TEGRA210_TEGRA_H_ */
