/*
 * Copyright 2018-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <lib/mmio.h>

#include <dram.h>
#include <gpc_reg.h>
#include <platform_def.h>

#define SRC_DDR1_RCR		(IMX_SRC_BASE + 0x1000)
#define SRC_DDR2_RCR		(IMX_SRC_BASE + 0x1004)

#define CCM_SRC_CTRL_OFFSET     (IMX_CCM_BASE + 0x800)
#define CCM_CCGR_OFFSET         (IMX_CCM_BASE + 0x4000)
#define CCM_TARGET_ROOT_OFFSET	(IMX_CCM_BASE + 0x8000)
#define CCM_SRC_CTRL(n)		(CCM_SRC_CTRL_OFFSET + 0x10 * (n))
#define CCM_CCGR(n)		(CCM_CCGR_OFFSET + 0x10 * (n))
#define CCM_TARGET_ROOT(n)	(CCM_TARGET_ROOT_OFFSET + 0x80 * (n))

#define DBGCAM_EMPTY		0x36000000

#if defined(LPA_ENABLE)
bool imx_m4_lpa_active(void);
bool imx_is_m4_enabled(void);
#endif

void rank_setting_update(void)
{
	uint32_t i, offset;
	uint32_t pstate_num = dram_info.num_fsp;

	/* only support maximum 3 setpoints */
	pstate_num = (pstate_num > MAX_FSP_NUM) ? MAX_FSP_NUM : pstate_num;

	for (i = 0U; i < pstate_num; i++) {
		offset = i ? (i + 1) * 0x1000 : 0U;
		mmio_write_32(DDRC_DRAMTMG2(0) + offset, dram_info.rank_setting[i][0]);
		if (dram_info.dram_type != DDRC_LPDDR4) {
			mmio_write_32(DDRC_DRAMTMG9(0) + offset, dram_info.rank_setting[i][1]);
		}

#if !defined(PLAT_imx8mq)
		mmio_write_32(DDRC_RANKCTL(0) + offset,
			dram_info.rank_setting[i][2]);
#endif
	}
#if defined(PLAT_imx8mq)
		mmio_write_32(DDRC_RANKCTL(0), dram_info.rank_setting[0][2]);
#endif
}

void dram_enter_retention(void)
{
	/* Wait DBGCAM to be empty */
	while (mmio_read_32(DDRC_DBGCAM(0)) != DBGCAM_EMPTY) {
		;
	}

	/* Block AXI ports from taking anymore transactions */
	mmio_write_32(DDRC_PCTRL_0(0), 0x0);
	/* Wait until all AXI ports are idle */
	while (mmio_read_32(DDRC_PSTAT(0)) & 0x10001) {
		;
	}

	/* Enter self refresh */
	mmio_write_32(DDRC_PWRCTL(0), 0xaa);

	/* LPDDR4 & DDR4/DDR3L need to check different status */
	if (dram_info.dram_type == DDRC_LPDDR4) {
		while (0x223 != (mmio_read_32(DDRC_STAT(0)) & 0x33f)) {
			;
		}
	} else {
		while (0x23 != (mmio_read_32(DDRC_STAT(0)) & 0x3f)) {
			;
		}
	}

	mmio_write_32(DDRC_DFIMISC(0), 0x0);
	mmio_write_32(DDRC_SWCTL(0), 0x0);
	mmio_write_32(DDRC_DFIMISC(0), 0x1f00);
	mmio_write_32(DDRC_DFIMISC(0), 0x1f20);

	while (mmio_read_32(DDRC_DFISTAT(0)) & 0x1) {
		;
	}

	mmio_write_32(DDRC_DFIMISC(0), 0x1f00);
	/* wait DFISTAT.dfi_init_complete to 1 */
	while (!(mmio_read_32(DDRC_DFISTAT(0)) & 0x1)) {
		;
	}

	mmio_write_32(DDRC_SWCTL(0), 0x1);

	/* should check PhyInLP3 pub reg */
	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	if (!(dwc_ddrphy_apb_rd(0x90028) & 0x1)) {
		INFO("PhyInLP3 = 1\n");
	}
	dwc_ddrphy_apb_wr(0xd0000, 0x1);

	/* pwrdnreqn_async adbm/adbs of ddr */
	mmio_clrbits_32(IMX_GPC_BASE + GPC_PU_PWRHSK, DDRMIX_ADB400_SYNC);
	while (mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & DDRMIX_ADB400_ACK)
		;
	mmio_setbits_32(IMX_GPC_BASE + GPC_PU_PWRHSK, DDRMIX_ADB400_SYNC);

	/* remove PowerOk */
	mmio_write_32(SRC_DDR1_RCR, 0x8F000008);

	mmio_write_32(CCM_CCGR(5), 0);
	mmio_write_32(CCM_SRC_CTRL(15), 2);

#if defined(LPA_ENABLE)
        if (imx_is_m4_enabled() && imx_m4_lpa_active()) {
                /* disable the DRAM PLL */
                mmio_clrbits_32(0x30360050, (0x1 << 9));
        }
#else
	/* enable the phy iso */
	mmio_setbits_32(IMX_GPC_BASE + DDRMIX_PGC, 1);
	mmio_setbits_32(IMX_GPC_BASE + PU_PGC_DN_TRG, DDRMIX_PWR_REQ);
#endif

	VERBOSE("dram enter retention\n");
}

void dram_exit_retention(void)
{
	VERBOSE("dram exit retention\n");
	/* assert all reset */
#if defined(PLAT_imx8mq)
	mmio_write_32(SRC_DDR2_RCR, 0x8F000003);
	mmio_write_32(SRC_DDR1_RCR, 0x8F00000F);
	mmio_write_32(SRC_DDR2_RCR, 0x8F000000);
#else
	mmio_write_32(SRC_DDR1_RCR, 0x8F00001F);
	mmio_write_32(SRC_DDR1_RCR, 0x8F00000F);
#endif
	mmio_write_32(CCM_CCGR(5), 2);
	mmio_write_32(CCM_SRC_CTRL(15), 2);

	/* change the clock source of dram_apb_clk_root */
	mmio_write_32(CCM_TARGET_ROOT(65) + 0x8, (0x7 << 24) | (0x7 << 16));
	mmio_write_32(CCM_TARGET_ROOT(65) + 0x4, (0x4 << 24) | (0x3 << 16));

#if !defined(LPA_ENABLE)
	/* disable iso */
	mmio_setbits_32(IMX_GPC_BASE + PU_PGC_UP_TRG, DDRMIX_PWR_REQ);
#endif

	mmio_write_32(SRC_DDR1_RCR, 0x8F000006);

	/* wait dram pll locked */
	while (!(mmio_read_32(DRAM_PLL_CTRL) & BIT(31))) {
		;
	}

	/* ddrc re-init */
	dram_umctl2_init(dram_info.timing_info);

	/*
	 * Skips the DRAM init routine and starts up in selfrefresh mode
	 * Program INIT0.skip_dram_init = 2'b11
	 */
	mmio_setbits_32(DDRC_INIT0(0), 0xc0000000);
	/* Keeps the controller in self-refresh mode */
	mmio_write_32(DDRC_PWRCTL(0), 0xaa);
	mmio_write_32(DDRC_DBG1(0), 0x0);
	mmio_write_32(SRC_DDR1_RCR, 0x8F000004);
	mmio_write_32(SRC_DDR1_RCR, 0x8F000000);

	/* before write Dynamic reg, sw_done should be 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x0);

#if !PLAT_imx8mn
	if (dram_info.dram_type == DDRC_LPDDR4) {
		mmio_write_32(DDRC_DDR_SS_GPR0, 0x01); /*LPDDR4 mode */
	}
#endif /* !PLAT_imx8mn */

	mmio_write_32(DDRC_DFIMISC(0), 0x0);

	/* dram phy re-init */
	dram_phy_init(dram_info.timing_info);

	/* workaround for rank-to-rank issue */
	rank_setting_update();

	/* DWC_DDRPHYA_APBONLY0_MicroContMuxSel */
	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	while (dwc_ddrphy_apb_rd(0x20097)) {
		;
	}
	dwc_ddrphy_apb_wr(0xd0000, 0x1);

	/* before write Dynamic reg, sw_done should be 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x0);
	mmio_write_32(DDRC_DFIMISC(0), 0x20);
	/* wait DFISTAT.dfi_init_complete to 1 */
	while (!(mmio_read_32(DDRC_DFISTAT(0)) & 0x1)) {
		;
	}

	/* clear DFIMISC.dfi_init_start */
	mmio_write_32(DDRC_DFIMISC(0), 0x0);
	/* set DFIMISC.dfi_init_complete_en */
	mmio_write_32(DDRC_DFIMISC(0), 0x1);

	/* set SWCTL.sw_done to enable quasi-dynamic register programming */
	mmio_write_32(DDRC_SWCTL(0), 0x1);
	/* wait SWSTAT.sw_done_ack to 1 */
	while (!(mmio_read_32(DDRC_SWSTAT(0)) & 0x1)) {
		;
	}

	mmio_write_32(DDRC_PWRCTL(0), 0x88);
	/* wait STAT to normal state */
	while (0x1 != (mmio_read_32(DDRC_STAT(0)) & 0x7)) {
		;
	}

	mmio_write_32(DDRC_PCTRL_0(0), 0x1);
	 /* dis_auto-refresh is set to 0 */
	mmio_write_32(DDRC_RFSHCTL3(0), 0x0);

	/* should check PhyInLP3 pub reg */
	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	if (!(dwc_ddrphy_apb_rd(0x90028) & 0x1)) {
		VERBOSE("PHYInLP3 = 0\n");
	}
	dwc_ddrphy_apb_wr(0xd0000, 0x1);
}
