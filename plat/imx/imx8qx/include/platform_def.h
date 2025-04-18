/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>

#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

#define PLATFORM_STACK_SIZE		0x400
#define CACHE_WRITEBACK_GRANULE		64

#define PLAT_PRIMARY_CPU		U(0x0)
#define PLATFORM_MAX_CPU_PER_CLUSTER	U(4)
#define PLATFORM_CLUSTER_COUNT		U(1)
#define PLATFORM_CORE_COUNT		U(4)
#define PLATFORM_CLUSTER0_CORE_COUNT	U(4)
#define PLATFORM_CLUSTER1_CORE_COUNT	U(0)

#define IMX_PWR_LVL0			MPIDR_AFFLVL0
#define PWR_DOMAIN_AT_MAX_LVL           U(1)
#define PLAT_MAX_PWR_LVL                U(2)
#define PLAT_MAX_OFF_STATE              U(2)
#define PLAT_MAX_RET_STATE              U(1)

#define PLAT_MU_SR_OFF			0x20
#define PLAT_MU_COLD_BOOT_FLG_MSK	0x40000000
#define PLAT_BOOT_MU_BASE		0x5D1B0000

#define BL31_BASE			0x80000000
#define BL31_LIMIT			0x80020000

#define OCRAM_BASE		0x100000
#define OCRAM_ALIAS_SIZE 0x18000 /* The lower 96KB is in OCRAM alias from 0x0 */

#define BL32_SHM_SIZE			0x00400000
#ifdef SPD_trusty
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE)
#else
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE - BL32_SHM_SIZE)
#endif
#define BL32_FDT_OVERLAY_ADDR		0x9d000000


#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 36)
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 36)

#ifdef SPD_trusty
#define MAX_XLAT_TABLES			10
#define MAX_MMAP_REGIONS		11
#else
#define MAX_XLAT_TABLES			8
#define MAX_MMAP_REGIONS		9
#endif

#define PLAT_GICD_BASE			0x51a00000
#define PLAT_GICR_BASE			0x51b00000

#if defined(IMX_USE_UART0)
#define IMX_BOOT_UART_BASE		0x5a060000
#elif defined(IMX_USE_UART1)
#define IMX_BOOT_UART_BASE		0x5a070000
#elif defined(IMX_USE_UART3)
#define IMX_BOOT_UART_BASE		0x5a090000
#else
#error "Provide proper UART configuration in IMX_DEBUG_UART"
#endif

#define IMX_BOOT_UART_BAUDRATE		115200
#define IMX_BOOT_UART_CLK_IN_HZ		24000000
#define PLAT_CRASH_UART_BASE		IMX_BOOT_UART_BASE
#define PLAT__CRASH_UART_CLK_IN_HZ	24000000
#define IMX_CONSOLE_BAUDRATE		115200
#define SC_IPC_BASE			0x5d1b0000
#define IMX_GPT0_LPCG_BASE		0x5d540000
#define IMX_GPT0_BASE			0x5d140000
#define IMX_WUP_IRQSTR_BASE		0x51090000
#define IMX_REG_BASE			0x50000000
#define IMX_REG_SIZE			0x10000000

#define COUNTER_FREQUENCY		8000000

/* non-secure u-boot base */
#define PLAT_NS_IMAGE_OFFSET		0x80020000

#ifdef SPD_trusty
#define DEBUG_CONSOLE_A35		1
#else
#define DEBUG_CONSOLE_A35		DEBUG_CONSOLE
#endif

#define IMX_TRUSTY_STACK_SIZE 0x100
#define TRUSTY_SHARED_MEMORY_OBJ_SIZE (12 * 1024)
#define IMX_SEPARATE_NOBITS_BASE	U(0x130000)
#define IMX_SEPARATE_NOBITS_LIMIT	U(0x140000)

#endif /* PLATFORM_DEF_H */
