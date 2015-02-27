/* Copyright (c) 2010-2014, The Linux Foundation. All rights reserved.
 * Copyright (c) 2011-2014, Xiaomi Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <Base.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/ArmLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>

#include <Library/qcom_lk.h>

#include <mipi_dsi.h>
#include <fbcon.h>

#include <display.h>
#include <Library/qcom_msm8960_iomap.h>
#include <Library/qcom_msm8960_clock.h>
#include <Library/qcom_msm8960_timer.h>
#include <msm_panel.h>
#include <Library/qcom_msm8960_machtype.h>
#include <mdp4.h>

extern void mdp_disable(void);
extern int mipi_dsi_cmd_config(struct fbcon_config mipi_fb_cfg, unsigned short num_of_lanes);
extern void mdp_shutdown(void);
extern void mdp_start_dma(void);
extern int target_cont_splash_screen(void);

static UINT32 manu_id;
static UINT32 manu_id0;

//DISPLAY_MIPI_PANEL_RENESAS_HT
static struct fbcon_config mipi_fb_cfg = {
	.height = REN_MIPI_FB_HEIGHT,
	.width = REN_MIPI_FB_WIDTH,
	.stride = REN_MIPI_FB_WIDTH,
	.format = FB_FORMAT_RGB888,
	.bpp = 24,
	.update_start = NULL,
	.update_done = NULL,
};

struct mipi_dsi_panel_config renesas_panel_info = {
	.mode = MIPI_CMD_MODE,
	.num_of_lanes = 4,
	.dsi_phy_config = &mipi_dsi_renesas_panel_phy_ctrl,
	.panel_cmds = renesas_panel_cmd_mode_cmds,
	.num_of_panel_cmds = ARRAY_SIZE(renesas_panel_cmd_mode_cmds),
	.lane_swap = 1,
};


static int cmd_mode_status = 0;

struct mipi_dsi_panel_config *get_panel_info(void)
{
	return &renesas_panel_info;

}

static UINT32 response_value = 0;

UINT32 mdss_dsi_read_panel_signature(UINT32 panel_signature)
{
	UINT32 rec_buf[1];
	UINT32 *lp = rec_buf, data;
	int ret = response_value;


	return ret;
}

int mdss_dual_dsi_cmd_dma_trigger_for_panel()
{
	UINT32 ReadValue;
	UINT32 count = 0;
	int status = 0;

	return status;
}

int dsi_cmd_dma_trigger_for_panel()
{
	unsigned long ReadValue;
	unsigned long count = 0;
	int status = 0;

	writel(0x03030303, DSI_INT_CTRL);
	dsb();
	writel(0x1, DSI_CMD_MODE_DMA_SW_TRIGGER);
	dsb();
	ReadValue = readl(DSI_INT_CTRL) & 0x00000001;
	while (ReadValue != 0x00000001) {
		ReadValue = readl(DSI_INT_CTRL) & 0x00000001;
		count++;
		if (count > 0xffff) {
			status = FAIL;
			DEBUG((EFI_D_WARN, "Panel CMD: command mode dma test failed\n"));
			return status;
		}
	}

	writel((readl(DSI_INT_CTRL) | 0x01000001), DSI_INT_CTRL);
	dsb();
	//DEBUG((EFI_D_WARN, "Panel CMD: command mode dma tested successfully\n"));
	return status;
}

int mdss_dual_dsi_cmds_tx(struct mipi_dsi_cmd *cmds, int count)
{
	int ret = 0;
	struct mipi_dsi_cmd *cm;
	int i = 0;
	char pload[256];
	UINT32 off;


	return ret;
}

int mdss_dsi_cmds_rx(UINT32 **rp, int rp_len, int rdbk_len)
{
	UINT32 *lp, data;
	char *dp;
	int i, off;
	int rlen, res;

	if (rdbk_len > rp_len) {
		return 0;
	}

	if (rdbk_len <= 2)
		rlen = 4;	/* short read */
	else
		rlen = MIPI_DSI_MRPS + 6;	/* 4 bytes header + 2 bytes crc */

	if (rlen > MIPI_DSI_REG_LEN) {
		return 0;
	}

	res = rlen & 0x03;

	rlen += res;		/* 4 byte align */
	lp = *rp;

	rlen += 3;
	rlen >>= 2;

	if (rlen > 4)
		rlen = 4;	/* 4 x 32 bits registers only */

	off = RDBK_DATA0;
	off += ((rlen - 1) * 4);

	for (i = 0; i < rlen; i++) {
		data = readl(MIPI_DSI_BASE + off);
		*lp = ntohl(data);	/* to network byte order */
		lp++;

		off -= 4;
	}

	if (rdbk_len > 2) {
		/*First 4 bytes + paded bytes will be header next len bytes would be payload */
		for (i = 0; i < rdbk_len; i++) {
			dp = (char *)(*rp);
			dp[i] = dp[(res + i) >> 2];
		}
	}
	return rdbk_len;
}

int mipi_dsi_cmds_tx(struct mipi_dsi_cmd *cmds, int count)
{
	int ret = 0;
	struct mipi_dsi_cmd *cm;
	int i = 0;
	char pload[256];
	UINT32 off;

	/* Align pload at 8 byte boundry */
	off = (UINT32)(&pload[0]);
	
	off &= 0x07;
	if (off)
		off = 8 - off;
	off += (UINT32)(&pload[0]);

	cm = cmds;
	for (i = 0; i < count; i++) {
		CopyMem((void *)off, (cm->payload), cm->size);
		writel(off, DSI_DMA_CMD_OFFSET);
		writel(cm->size, DSI_DMA_CMD_LENGTH);	// reg 0x48 for this build
		dsb();
		ret += dsi_cmd_dma_trigger_for_panel();
		dsb();

		if (count == sizeof(cmd_delay)/sizeof(int)) {
			if (i < (sizeof(cmd_delay)/sizeof(int)))
				mdelay(cmd_delay[i]);
			else
				mdelay(100);
		}

		if (count == sizeof(cmd_delay_lgd)/sizeof(int)) 
		{
			if (i < (sizeof(cmd_delay_lgd)/sizeof(int)))
				mdelay(cmd_delay_lgd[i]);
			else
				mdelay(100);
		}

		if (count == sizeof(cmd_delay_auo)/sizeof(int)) {
			if (i < (sizeof(cmd_delay_auo)/sizeof(int)))
				mdelay(cmd_delay_auo[i]);
			else
				mdelay(100);
		}

		if (count == sizeof(cmd_delay_jdi)/sizeof(int)) {
			if (i < (sizeof(cmd_delay_jdi)/sizeof(int)))
				mdelay(cmd_delay_jdi[i]);
			else
				mdelay(100);
		}


		cm++;
	}
	return ret;
}

/*
 * mipi_dsi_cmd_rx: can receive at most 16 bytes
 * per transaction since it only have 4 32bits reigsters
 * to hold data.
 * therefore Maximum Return Packet Size need to be set to 16.
 * any return data more than MRPS need to be break down
 * to multiple transactions.
 */
int mipi_dsi_cmds_rx(char **rp, int len)
{
	UINT32 *lp, data;
	char *dp;
	int i, off, cnt;
	int rlen, res;

	if (len <= 2)
		rlen = 4;	/* short read */
	else
		rlen = MIPI_DSI_MRPS + 6;	/* 4 bytes header + 2 bytes crc */

	if (rlen > MIPI_DSI_REG_LEN) {
		return 0;
	}

	res = rlen & 0x03;

	rlen += res;		/* 4 byte align */
	lp = (UINT32 *) (*rp);

	cnt = rlen;
	cnt += 3;
	cnt >>= 2;

	if (cnt > 4)
		cnt = 4;	/* 4 x 32 bits registers only */

	off = 0x068;		/* DSI_RDBK_DATA0 */
	off += ((cnt - 1) * 4);

	for (i = 0; i < cnt; i++) {
		data = (UINT32) readl(MIPI_DSI_BASE + off);
		*lp++ = ntohl(data);	/* to network byte order */
		off -= 4;
	}

	if (len > 2) {
		/*First 4 bytes + paded bytes will be header next len bytes would be payload */
		for (i = 0; i < len; i++) {
			dp = *rp;
			dp[i] = dp[4 + res + i];
		}
	}

	return len;
}

static int mipi_dsi_cmd_bta_sw_trigger(void)
{
	UINT32 data;
	int cnt = 0;
	int err = 0;

	writel(0x01, MIPI_DSI_BASE + 0x094);	/* trigger */
	while (cnt < 10000) {
		data = readl(MIPI_DSI_BASE + 0x0004);	/*DSI_STATUS */
		if ((data & 0x0010) == 0)
			break;
		cnt++;
	}
	if (cnt == 10000)
		err = 1;
	return err;
}

static UINT32 mipi_novatek_manufacture_id(void)
{
	char rec_buf[24];
	char *rp = rec_buf;
	UINT32 *lp, data;


	mipi_dsi_cmds_tx(&novatek_panel_max_packet_cmd, 1);

	mipi_dsi_cmds_tx(&novatek_panel_manufacture_id_cmd, 1);
	mipi_dsi_cmds_rx(&rp, 2);


	lp = (UINT32 *) rp;
	data = (UINT32) * lp;
	data = ntohl(data);
	data = data >> 8;
	return data;
}


static UINT32 mipi_novatek_manufacture_id0(void)
{
	char rec_buf[24];
	char *rp = rec_buf;
	UINT32 *lp, data;

	mipi_dsi_cmds_tx(&novatek_panel_manufacture_id0_cmd, 1);
	mipi_dsi_cmds_rx(&rp, 3);

	lp = (UINT32 *) rp;
	data = (UINT32) * lp;
	data = ntohl(data);
	data = data >> 8;
	return data;
}


int mdss_dsi_host_init(struct mipi_dsi_panel_config *pinfo, UINT32
		dual_dsi, UINT32 broadcast)
{
	UINT8 DMA_STREAM1 = 0;	// for mdp display processor path
	UINT8 EMBED_MODE1 = 1;	// from frame buffer
	UINT8 POWER_MODE2 = 1;	// from frame buffer
	UINT8 PACK_TYPE1;		// long packet
	UINT8 VC1 = 0;
	UINT8 DT1 = 0;	// non embedded mode
	UINT8 WC1 = 0;	// for non embedded mode only
	UINT8 DLNx_EN;
	UINT8 lane_swap = 0;
	UINT32 timing_ctl = 0;
	UINT32 lane_swap_dsi1 = 0;


	return 0;
}

int mdss_dsi_panel_initialize(struct mipi_dsi_panel_config *pinfo, UINT32
		broadcast)
{
	int status = 0;


	return status;
}


int mipi_dsi_panel_initialize_prepare(struct mipi_dsi_panel_config *pinfo)
{
	unsigned char DMA_STREAM1 = 0;	// for mdp display processor path
	unsigned char EMBED_MODE1 = 1;	// from frame buffer
	unsigned char POWER_MODE2 = 1;	// from frame buffer
	unsigned char PACK_TYPE1 = 1;	// long packet
	unsigned char VC1 = 0;
	unsigned char DT1 = 0;	// non embedded mode
	unsigned short WC1 = 0;	// for non embedded mode only
	int status = 0;
	unsigned char DLNx_EN;

	switch (pinfo->num_of_lanes) {
	default:
	case 1:
		DLNx_EN = 1;	// 1 lane
		break;
	case 2:
		DLNx_EN = 3;	// 2 lane
		break;
	case 3:
		DLNx_EN = 7;	// 3 lane
		break;
	case 4:
		DLNx_EN = 0x0F;	/* 4 lanes */
		break;
	}

	writel(0x0001, DSI_SOFT_RESET);
	writel(0x0000, DSI_SOFT_RESET);

	writel((0 << 16) | 0x3f, DSI_CLK_CTRL);	/* Turn on all DSI Clks */
	writel(DMA_STREAM1 << 8 | 0x04, DSI_TRIG_CTRL);	// reg 0x80 dma trigger: sw
	// trigger 0x4; dma stream1

	writel(0 << 30 | DLNx_EN << 4 | 0x105, DSI_CTRL);	// reg 0x00 for this
	// build
	writel(EMBED_MODE1 << 28 | POWER_MODE2 << 26
	       | PACK_TYPE1 << 24 | VC1 << 22 | DT1 << 16 | WC1,
	       DSI_COMMAND_MODE_DMA_CTRL);

	return status;
}

int mipi_dsi_panel_initialize_cmds(struct mipi_dsi_panel_config *pinfo)
{
	int status = 0;

	if (pinfo->panel_cmds)
		status = mipi_dsi_cmds_tx(pinfo->panel_cmds,
					  pinfo->num_of_panel_cmds);

	return status;
}

void trigger_mdp_dsi(void)
{
	dsb();
	mdp_start_dma();
	mdelay(10);
	dsb();
	writel(0x1, DSI_CMD_MODE_MDP_SW_TRIGGER);
}

int mipi_dsi_panel_initialize(struct mipi_dsi_panel_config *pinfo)
{
	UINT8 DMA_STREAM1 = 0;	// for mdp display processor path
	UINT8 EMBED_MODE1 = 1;	// from frame buffer
	UINT8 POWER_MODE2 = 1;	// from frame buffer
	UINT8 PACK_TYPE1;		// long packet
	UINT8 VC1 = 0;
	UINT8 DT1 = 0;	// non embedded mode
	UINT8 WC1 = 0;	// for non embedded mode only
	int status = 0;
	UINT8 DLNx_EN;

	switch (pinfo->num_of_lanes) {
	default:
	case 1:
		DLNx_EN = 1;	// 1 lane
		break;
	case 2:
		DLNx_EN = 3;	// 2 lane
		break;
	case 3:
		DLNx_EN = 7;	// 3 lane
		break;
	case 4:
		DLNx_EN = 0x0F;	/* 4 lanes */
		break;
	}

	PACK_TYPE1 = pinfo->pack;

	writel(0x0001, DSI_SOFT_RESET);
	writel(0x0000, DSI_SOFT_RESET);

	writel((0 << 16) | 0x3f, DSI_CLK_CTRL);	/* Turn on all DSI Clks */
	writel(DMA_STREAM1 << 8 | 0x04, DSI_TRIG_CTRL);	// reg 0x80 dma trigger: sw
	// trigger 0x4; dma stream1

	writel(0 << 30 | DLNx_EN << 4 | 0x105, DSI_CTRL);	// reg 0x00 for this
	// build
	writel(EMBED_MODE1 << 28 | POWER_MODE2 << 26
	       | PACK_TYPE1 << 24 | VC1 << 22 | DT1 << 16 | WC1,
	       DSI_COMMAND_MODE_DMA_CTRL);

	if (pinfo->panel_cmds)
		status = mipi_dsi_cmds_tx(pinfo->panel_cmds,
					  pinfo->num_of_panel_cmds);

	return status;
}

//TODO: Clean up arguments being passed in not being used
int
config_dsi_video_mode(unsigned short disp_width, unsigned short disp_height,
		      unsigned short img_width, unsigned short img_height,
		      unsigned short hsync_porch0_fp,
		      unsigned short hsync_porch0_bp,
		      unsigned short vsync_porch0_fp,
		      unsigned short vsync_porch0_bp,
		      unsigned short hsync_width,
		      unsigned short vsync_width, unsigned short dst_format,
		      unsigned short traffic_mode, unsigned short datalane_num)
{

	unsigned char DST_FORMAT;
	unsigned char TRAFIC_MODE;
	unsigned char DLNx_EN;
	// video mode data ctrl
	int status = 0;
	unsigned long low_pwr_stop_mode = 0;
	unsigned char eof_bllp_pwr = 0x9;
	unsigned char interleav = 0;

	// disable mdp first
	mdp_disable();

	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000002, DSI_CLK_CTRL);
	writel(0x00000006, DSI_CLK_CTRL);
	writel(0x0000000e, DSI_CLK_CTRL);
	writel(0x0000001e, DSI_CLK_CTRL);
	writel(0x0000003e, DSI_CLK_CTRL);

	writel(0, DSI_CTRL);

	writel(0x13ff3fe0, DSI_ERR_INT_MASK0);

	DST_FORMAT = 0;		// RGB565
	DEBUG((EFI_D_WARN, "DSI_Video_Mode - Dst Format: RGB565\n"));

	DLNx_EN = 1;		// 1 lane with clk programming
	DEBUG((EFI_D_WARN,  "Data Lane: 1 lane\n"));

	TRAFIC_MODE = 0;	// non burst mode with sync pulses
	DEBUG((EFI_D_WARN, "Traffic mode: non burst mode with sync pulses\n"));

	writel(0x02020202, DSI_INT_CTRL);

	writel(((hsync_width + img_width + hsync_porch0_bp) << 16)
	       | (hsync_width + hsync_porch0_bp),
	       DSI_VIDEO_MODE_ACTIVE_H);

	writel(((vsync_width + img_height + vsync_porch0_bp) << 16)
	       | (vsync_width + vsync_porch0_bp),
	       DSI_VIDEO_MODE_ACTIVE_V);

	writel(((vsync_width + img_height + vsync_porch0_fp + vsync_porch0_bp - 1) << 16)
	       | (hsync_width + img_width + hsync_porch0_fp + hsync_porch0_bp - 1),
	       DSI_VIDEO_MODE_TOTAL);

	writel((hsync_width << 16) | 0, DSI_VIDEO_MODE_HSYNC);

	writel(0 << 16 | 0, DSI_VIDEO_MODE_VSYNC);

	writel(vsync_width << 16 | 0, DSI_VIDEO_MODE_VSYNC_VPOS);

	writel(1, DSI_EOT_PACKET_CTRL);

	writel(0x00000100, DSI_MISR_VIDEO_CTRL);

	writel(low_pwr_stop_mode << 16 | eof_bllp_pwr << 12 | TRAFIC_MODE << 8
	       | DST_FORMAT << 4 | 0x0, DSI_VIDEO_MODE_CTRL);

	writel(0x67, DSI_CAL_STRENGTH_CTRL);

	writel(0x80006711, DSI_CAL_CTRL);

	writel(0x00010100, DSI_MISR_VIDEO_CTRL);

	writel(0x00010100, DSI_INT_CTRL);
	writel(0x02010202, DSI_INT_CTRL);

	writel(0x02030303, DSI_INT_CTRL);

	writel(interleav << 30 | 0 << 24 | 0 << 20 | DLNx_EN << 4
	       | 0x103, DSI_CTRL);
	mdelay(10);

	return status;
}

int
config_dsi_cmd_mode(unsigned short disp_width, unsigned short disp_height,
		    unsigned short img_width, unsigned short img_height,
		    unsigned short dst_format,
		    unsigned short traffic_mode, unsigned short datalane_num)
{
	unsigned char DST_FORMAT;
	unsigned char TRAFIC_MODE;
	unsigned char DLNx_EN;
	// video mode data ctrl
	int status = 0;
	unsigned char interleav = 0;
	unsigned char ystride = 0x03;
	// disable mdp first

	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000002, DSI_CLK_CTRL);
	writel(0x00000006, DSI_CLK_CTRL);
	writel(0x0000000e, DSI_CLK_CTRL);
	writel(0x0000001e, DSI_CLK_CTRL);
	writel(0x0000003e, DSI_CLK_CTRL);

	writel(0x13ff3fe0, DSI_ERR_INT_MASK0);

	// writel(0, DSI_CTRL);

	// writel(0, DSI_ERR_INT_MASK0);

	DST_FORMAT = 8;		// RGB888
	DEBUG((EFI_D_WARN, "DSI_Cmd_Mode - Dst Format: RGB888\n"));

	switch (datalane_num) {
	default:
	case 1:
		DLNx_EN = 1;
		break;
	case 2:
		DLNx_EN = 3;
		break;
	case 3:
		DLNx_EN = 7;
		break;
	case 4:
		DLNx_EN = 0xF;
		break;
	}

	TRAFIC_MODE = 0;	// non burst mode with sync pulses
	DEBUG((EFI_D_WARN, "Traffic mode: non burst mode with sync pulses\n"));

	writel(0x02020202, DSI_INT_CTRL);

	writel(0x00100000 | DST_FORMAT, DSI_COMMAND_MODE_MDP_CTRL);
	writel((img_width * ystride + 1) << 16 | 0x0039,
	       DSI_COMMAND_MODE_MDP_STREAM0_CTRL);
	writel((img_width * ystride + 1) << 16 | 0x0039,
	       DSI_COMMAND_MODE_MDP_STREAM1_CTRL);
	writel(img_height << 16 | img_width,
	       DSI_COMMAND_MODE_MDP_STREAM0_TOTAL);
	writel(img_height << 16 | img_width,
	       DSI_COMMAND_MODE_MDP_STREAM1_TOTAL);
	writel(0xEE, DSI_CAL_STRENGTH_CTRL);
	writel(0x80000000, DSI_CAL_CTRL);
	writel(0x40, DSI_TRIG_CTRL);
	writel(0x13c2c, DSI_COMMAND_MODE_MDP_DCS_CMD_CTRL);
	writel(interleav << 30 | 0 << 24 | 0 << 20 | DLNx_EN << 4 | 0x105,
	       DSI_CTRL);
	mdelay(10);
	writel(0x14000000, DSI_COMMAND_MODE_DMA_CTRL);
	writel(0x10000000, DSI_MISR_CMD_CTRL);
	writel(0x13ff3fe0, DSI_ERR_INT_MASK0);
	writel(0x1, DSI_EOT_PACKET_CTRL);
	// writel(0x0, MDP_OVERLAYPROC0_START);
	dsb();
	mdp_start_dma();
	mdelay(10);
	dsb();
	writel(0x1, DSI_CMD_MODE_MDP_SW_TRIGGER);

	status = 1;
	return status;
}

int mipi_dsi_video_config(unsigned short num_of_lanes)
{

	int status = 0;
	unsigned long ReadValue;
	unsigned long count = 0;
	unsigned long low_pwr_stop_mode = 0;	// low power mode 0x1111 start from
	// bit16, high spd mode 0x0
	unsigned char eof_bllp_pwr = 0x9;	// bit 12, 15, 1:low power stop mode or
	// let cmd mode eng send packets in hs
	// or lp mode
	unsigned short image_wd = mipi_fb_cfg.width;
	unsigned short image_ht = mipi_fb_cfg.height;
	unsigned short display_wd = mipi_fb_cfg.width;
	unsigned short display_ht = mipi_fb_cfg.height;
	unsigned short hsync_porch_fp = MIPI_HSYNC_FRONT_PORCH_DCLK;
	unsigned short hsync_porch_bp = MIPI_HSYNC_BACK_PORCH_DCLK;
	unsigned short vsync_porch_fp = MIPI_VSYNC_FRONT_PORCH_LINES;
	unsigned short vsync_porch_bp = MIPI_VSYNC_BACK_PORCH_LINES;
	unsigned short hsync_width = MIPI_HSYNC_PULSE_WIDTH;
	unsigned short vsync_width = MIPI_VSYNC_PULSE_WIDTH;
	unsigned short dst_format = 0;
	unsigned short traffic_mode = 0;
	unsigned short pack_pattern = 0x12;	//BGR
	unsigned char ystride = 3;

	low_pwr_stop_mode = 0x1111;	// low pwr mode bit16:HSA, bit20:HBA,
	// bit24:HFP, bit28:PULSE MODE, need enough
	// time for swithc from LP to HS
	eof_bllp_pwr = 0x9;	// low power stop mode or let cmd mode eng send
	// packets in hs or lp mode

	status +=
	    config_dsi_video_mode(display_wd, display_ht, image_wd, image_ht,
				  hsync_porch_fp, hsync_porch_bp,
				  vsync_porch_fp, vsync_porch_bp, hsync_width,
				  vsync_width, dst_format, traffic_mode,
				  num_of_lanes);

	status +=
	    mdp_setup_dma_p_video_mode(display_wd, display_ht, image_wd,
				       image_ht, hsync_porch_fp, hsync_porch_bp,
				       vsync_porch_fp, vsync_porch_bp,
				       hsync_width, vsync_width, MIPI_FB_ADDR,
				       image_wd, pack_pattern, ystride);

	ReadValue = readl(DSI_INT_CTRL) & 0x00010000;
	while (ReadValue != 0x00010000) {
		ReadValue = readl(DSI_INT_CTRL) & 0x00010000;
		count++;
		if (count > 0xffff) {
			status = FAIL;
			DEBUG((EFI_D_WARN, "Video lane test failed\n"));
			return status;
		}
	}

	DEBUG((EFI_D_WARN, "Video lane tested successfully\n"));
	return status;
}

int is_cmd_mode_enabled(void)
{
	return cmd_mode_status;
}

void mipi_dsi_cmd_mode_trigger(void)
{
	int status = 0;
	unsigned short display_wd = mipi_fb_cfg.width;
	unsigned short display_ht = mipi_fb_cfg.height;
	unsigned short image_wd = mipi_fb_cfg.width;
	unsigned short image_ht = mipi_fb_cfg.height;
	unsigned short dst_format = 0;
	unsigned short traffic_mode = 0;
	struct mipi_dsi_panel_config *panel_info = get_panel_info();
	status += mipi_dsi_cmd_config(mipi_fb_cfg, panel_info->num_of_lanes);
	mdelay(50);
	config_dsi_cmd_mode(display_wd, display_ht, image_wd, image_ht,
			    dst_format, traffic_mode,
			    panel_info->num_of_lanes /* num_of_lanes */ );
}


void mipi_dsi_cmd_trigger(struct msm_fb_panel_data *pdata)
{
	struct msm_panel_info *pinfo = NULL;
	struct fbcon_config mipi_fb_cfg;

	if (pdata)
		pinfo = &(pdata->panel_info);
	else
		return;

	unsigned short display_wd = pinfo->xres;
	unsigned short display_ht = pinfo->yres;
	unsigned short image_wd = pinfo->xres;
	unsigned short image_ht = pinfo->yres;
	unsigned short dst_format = 0;
	unsigned short traffic_mode = 0;
	unsigned short num_of_lanes = pinfo->mipi.num_of_lanes;

	mipi_fb_cfg.width = display_wd;
	mipi_fb_cfg.height = display_ht;

	mipi_dsi_cmd_config(mipi_fb_cfg, num_of_lanes);
	mdelay(50);
	config_dsi_cmd_mode(display_wd, display_ht, image_wd, image_ht,dst_format, traffic_mode,num_of_lanes /* num_of_lanes */ );
}

void panel_manu_id_detection(void)
{
	mipi_dsi_cmd_bta_sw_trigger();
	manu_id = mipi_novatek_manufacture_id();
	manu_id0 = mipi_novatek_manufacture_id0();
}


void mipi_dsi_shutdown(void)
{
	if(!target_cont_splash_screen())
	{
		mdp_shutdown();
		writel(0x01010101, DSI_INT_CTRL);
		writel(0x13FF3BFF, DSI_ERR_INT_MASK0);

		/* Disable branch clocks */
		writel(0x0, DSI1_BYTE_CC_REG);
		writel(0x0, DSI_PIXEL_CC_REG);
		writel(0x0, DSI1_ESC_CC_REG);
		/* Disable root clock */
		writel(0x0, DSI_CC_REG);


		writel(0, DSI_CLK_CTRL);
		writel(0, DSI_CTRL);
		writel(0, DSIPHY_PLL_CTRL(0));
	}
	else
	{
        /* To keep the splash screen displayed till kernel driver takes
        control, do not turn off the video mode engine and clocks.
        Only disabling the MIPI DSI IRQs */
        writel(0x01010101, DSI_INT_CTRL);
        writel(0x13FF3BFF, DSI_ERR_INT_MASK0);
	}
}

struct fbcon_config *mipi_init(void)
{
	int status = 0;
	struct mipi_dsi_panel_config *panel_info = get_panel_info();

	if (panel_info == NULL) {
		DEBUG((EFI_D_WARN, "Panel info is null\n"));
		return NULL;
	}

	/* Enable MMSS_AHB_ARB_MATER_PORT_E for arbiter master0 and master 1 request */

	writel(0x00001800, MMSS_SFPB_GPREG);


	mipi_dsi_phy_init(panel_info);


	status += mipi_dsi_panel_initialize(panel_info);


	mipi_fb_cfg.base = (void *)MIPI_FB_ADDR;

	if (panel_info->mode == MIPI_VIDEO_MODE)
		status += mipi_dsi_video_config(panel_info->num_of_lanes);

	if (panel_info->mode == MIPI_CMD_MODE)
		cmd_mode_status = 1;

	return &mipi_fb_cfg;
}

int mipi_config(struct msm_fb_panel_data *panel)
{
	int ret = NO_ERROR;
	struct msm_panel_info *pinfo;
	struct mipi_dsi_panel_config mipi_pinfo;

	if (!panel)
		return ERR_INVALID_ARGS;

	pinfo = &(panel->panel_info);
	mipi_pinfo.mode = pinfo->mipi.mode;
	mipi_pinfo.num_of_lanes = pinfo->mipi.num_of_lanes;
	mipi_pinfo.dsi_phy_config = pinfo->mipi.dsi_phy_db;
	mipi_pinfo.panel_cmds = pinfo->mipi.panel_cmds;
	mipi_pinfo.num_of_panel_cmds = pinfo->mipi.num_of_panel_cmds;
	mipi_pinfo.lane_swap = pinfo->mipi.lane_swap;
	mipi_pinfo.pack = 1;

	/* Enable MMSS_AHB_ARB_MATER_PORT_E for
	   arbiter master0 and master 1 request */

	writel(0x00001800, MMSS_SFPB_GPREG);


	mipi_dsi_phy_init(&mipi_pinfo);

	ret = mipi_dsi_panel_initialize_prepare(&mipi_pinfo);
	
	if (ret) {
		DEBUG((EFI_D_WARN, "mipi_dsi_panel_initialize_prepare!\n"));
		return ret;
	}


		panel_manu_id_detection();
		DEBUG((EFI_D_WARN, "manu_id=%d,manu_id0=%d!\n",manu_id,manu_id0));
		//if ((manu_id) && (manu_id0)) {
			mipi_pinfo.panel_cmds = lgd_panel_cmd_mode_cmds;
			mipi_pinfo.num_of_panel_cmds = ARRAY_SIZE(lgd_panel_cmd_mode_cmds);
		//}
		//if ((manu_id)&& !manu_id0) {
		//	mipi_pinfo.panel_cmds = auo_panel_cmd_mode_cmds;
		//	mipi_pinfo.num_of_panel_cmds = ARRAY_SIZE(auo_panel_cmd_mode_cmds);
		//}
		//if (manu_id == 0x60 && manu_id0 == 0x141304) {
		//	mipi_pinfo.panel_cmds = jdi_panel_cmd_mode_cmds;
		//	mipi_pinfo.num_of_panel_cmds = ARRAY_SIZE(jdi_panel_cmd_mode_cmds);
		//}
	

	ret = mipi_dsi_panel_initialize_cmds(&mipi_pinfo);
	
	if (ret) {
		DEBUG((EFI_D_WARN, "mipi_dsi_panel_initialize_cmds!\n"));
		//return ret;
		ret = 0;
	}


	if (pinfo->rotate && panel->rotate)
		pinfo->rotate();

	return ret;
}

int mdss_dsi_video_mode_config(UINT16 disp_width,
	UINT16 disp_height,
	UINT16 img_width,
	UINT16 img_height,
	UINT16 hsync_porch0_fp,
	UINT16 hsync_porch0_bp,
	UINT16 vsync_porch0_fp,
	UINT16 vsync_porch0_bp,
	UINT16 hsync_width,
	UINT16 vsync_width,
	UINT16 dst_format,
	UINT16 traffic_mode,
	UINT8 lane_en,
	UINT16 low_pwr_stop_mode,
	UINT8 eof_bllp_pwr,
	UINT8 interleav,
	UINT32 ctl_base)
{
	int status = 0;



	return status;
}

int mdss_dsi_config(struct msm_fb_panel_data *panel)
{
	int ret = NO_ERROR;
	struct msm_panel_info *pinfo;
	struct mipi_dsi_panel_config mipi_pinfo;
error:
	return ret;
}

int mipi_dsi_video_mode_config(unsigned short disp_width,
	unsigned short disp_height,
	unsigned short img_width,
	unsigned short img_height,
	unsigned short hsync_porch0_fp,
	unsigned short hsync_porch0_bp,
	unsigned short vsync_porch0_fp,
	unsigned short vsync_porch0_bp,
	unsigned short hsync_width,
	unsigned short vsync_width,
	unsigned short dst_format,
	unsigned short traffic_mode,
	unsigned char lane_en,
	unsigned low_pwr_stop_mode,
	unsigned char eof_bllp_pwr,
	unsigned char interleav)
{
	int status = 0;

	/* disable mdp first */
	mdp_disable();

	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000002, DSI_CLK_CTRL);
	writel(0x00000006, DSI_CLK_CTRL);
	writel(0x0000000e, DSI_CLK_CTRL);
	writel(0x0000001e, DSI_CLK_CTRL);
	writel(0x0000023f, DSI_CLK_CTRL);

	writel(0, DSI_CTRL);

	writel(0x13ff3fe0, DSI_ERR_INT_MASK0);

	writel(0x02020202, DSI_INT_CTRL);

	writel(((disp_width + hsync_porch0_bp) << 16) | hsync_porch0_bp,
			DSI_VIDEO_MODE_ACTIVE_H);

	writel(((disp_height + vsync_porch0_bp) << 16) | (vsync_porch0_bp),
			DSI_VIDEO_MODE_ACTIVE_V);

	if (mdp_get_revision() >= MDP_REV_41) {
		writel(((disp_height + vsync_porch0_fp
			+ vsync_porch0_bp - 1) << 16)
			| (disp_width + hsync_porch0_fp
			+ hsync_porch0_bp - 1),
			DSI_VIDEO_MODE_TOTAL);
	} else {
		writel(((disp_height + vsync_porch0_fp
			+ vsync_porch0_bp) << 16)
			| (disp_width + hsync_porch0_fp
			+ hsync_porch0_bp),
			DSI_VIDEO_MODE_TOTAL);
	}

	writel((hsync_width << 16) | 0, DSI_VIDEO_MODE_HSYNC);

	writel(0 << 16 | 0, DSI_VIDEO_MODE_VSYNC);

	writel(vsync_width << 16 | 0, DSI_VIDEO_MODE_VSYNC_VPOS);

	writel(0x0, DSI_EOT_PACKET_CTRL);

	writel(0x00000100, DSI_MISR_VIDEO_CTRL);

	if (mdp_get_revision() >= MDP_REV_41) {
		writel(low_pwr_stop_mode << 16 |
				eof_bllp_pwr << 12 | traffic_mode << 8
				| dst_format << 4 | 0x0, DSI_VIDEO_MODE_CTRL);
	} else {
		writel(1 << 28 | 1 << 24 | 1 << 20 | low_pwr_stop_mode << 16 |
				eof_bllp_pwr << 12 | traffic_mode << 8
				| dst_format << 4 | 0x0, DSI_VIDEO_MODE_CTRL);
	}

	writel(0x3fd08, DSI_HS_TIMER_CTRL);
	writel(0x67, DSI_CAL_STRENGTH_CTRL);
	writel(0x80006711, DSI_CAL_CTRL);
	writel(0x00010100, DSI_MISR_VIDEO_CTRL);

	writel(0x00010100, DSI_INT_CTRL);
	writel(0x02010202, DSI_INT_CTRL);
	writel(0x02030303, DSI_INT_CTRL);

	writel(interleav << 30 | 0 << 24 | 0 << 20 | lane_en << 4
			| 0x103, DSI_CTRL);

	return status;
}

int mdss_dsi_cmd_mode_config(UINT16 disp_width,
	UINT16 disp_height,
	UINT16 img_width,
	UINT16 img_height,
	UINT16 dst_format,
	UINT8 ystride,
	UINT8 lane_en,
	UINT8 interleav,
	UINT32 ctl_base)
{
	UINT16 dst_fmt = 0;

	switch (dst_format) {
	case DSI_VIDEO_DST_FORMAT_RGB565:
		dst_fmt = DSI_CMD_DST_FORMAT_RGB565;
		break;
	case DSI_VIDEO_DST_FORMAT_RGB666:
	case DSI_VIDEO_DST_FORMAT_RGB666_LOOSE:
		dst_fmt = DSI_CMD_DST_FORMAT_RGB666;
		break;
	case DSI_VIDEO_DST_FORMAT_RGB888:
		dst_fmt = DSI_CMD_DST_FORMAT_RGB888;
		break;
	default:
		DEBUG((EFI_D_WARN, "unsupported dst format\n"));
		return ERROR;
	}
	return 0;
}

int mipi_dsi_cmd_mode_config(unsigned short disp_width,
	unsigned short disp_height,
	unsigned short img_width,
	unsigned short img_height,
	unsigned short dst_format,
	unsigned short traffic_mode)
{
	unsigned char DST_FORMAT;
	unsigned char TRAFIC_MODE;
	unsigned char DLNx_EN;
	// video mode data ctrl
	int status = 0;
	unsigned char interleav = 0;
	unsigned char ystride = 0x03;
	// disable mdp first

	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000000, DSI_CLK_CTRL);
	writel(0x00000002, DSI_CLK_CTRL);
	writel(0x00000006, DSI_CLK_CTRL);
	writel(0x0000000e, DSI_CLK_CTRL);
	writel(0x0000001e, DSI_CLK_CTRL);
	writel(0x0000003e, DSI_CLK_CTRL);

	writel(0x13ff3fe0, DSI_ERR_INT_MASK0);


	DST_FORMAT = 8;		// RGB888
	DEBUG((EFI_D_WARN,  "DSI_Cmd_Mode - Dst Format: RGB888\n"));

	DLNx_EN = 3;		// 2 lane with clk programming
	DEBUG((EFI_D_WARN,  "Data Lane: 2 lane\n"));

	TRAFIC_MODE = 0;	// non burst mode with sync pulses
	DEBUG((EFI_D_WARN,  "Traffic mode: non burst mode with sync pulses\n"));

	writel(0x02020202, DSI_INT_CTRL);

	writel(0x00100000 | DST_FORMAT, DSI_COMMAND_MODE_MDP_CTRL);
	writel((img_width * ystride + 1) << 16 | 0x0039,
	       DSI_COMMAND_MODE_MDP_STREAM0_CTRL);
	writel((img_width * ystride + 1) << 16 | 0x0039,
	       DSI_COMMAND_MODE_MDP_STREAM1_CTRL);
	writel(img_height << 16 | img_width,
	       DSI_COMMAND_MODE_MDP_STREAM0_TOTAL);
	writel(img_height << 16 | img_width,
	       DSI_COMMAND_MODE_MDP_STREAM1_TOTAL);
	writel(0xEE, DSI_CAL_STRENGTH_CTRL);
	writel(0x80000000, DSI_CAL_CTRL);
	writel(0x40, DSI_TRIG_CTRL);
	writel(0x13c2c, DSI_COMMAND_MODE_MDP_DCS_CMD_CTRL);
	writel(interleav << 30 | 0 << 24 | 0 << 20 | DLNx_EN << 4 | 0x105,
	       DSI_CTRL);
	writel(0x14000000, DSI_COMMAND_MODE_DMA_CTRL);
	writel(0x10000000, DSI_MISR_CMD_CTRL);
	writel(0x13ff3fe0, DSI_ERR_INT_MASK0);
	writel(0x1, DSI_EOT_PACKET_CTRL);

	return NO_ERROR;
}

int mipi_dsi_on()
{
	int ret = NO_ERROR;
	unsigned long ReadValue;
	unsigned long count = 0;

	ReadValue = readl(DSI_INT_CTRL) & 0x00010000;

	mdelay(10);

	while (ReadValue != 0x00010000) {
		ReadValue = readl(DSI_INT_CTRL) & 0x00010000;
		count++;
		if (count > 0xffff) {
			DEBUG((EFI_D_WARN,  "Video lane test failed\n"));
			return ERROR;
		}
	}

	//DEBUG((EFI_D_WARN, "Video lane tested successfully\n"));
	return ret;
}

int mipi_dsi_off(struct msm_panel_info *pinfo)
{
	if(!target_cont_splash_screen())
	{
		writel(0, DSI_CLK_CTRL);
		writel(0x1F1, DSI_CTRL);
		mdelay(10);
		writel(0x0001, DSI_SOFT_RESET);
		writel(0x0000, DSI_SOFT_RESET);
		writel(0x1115501, DSI_INT_CTRL);
		writel(0, DSI_CTRL);
	}

	writel(0x1115501, DSI_INT_CTRL);
	if (pinfo->mipi.broadcast)
		writel(0x1115501, DSI_INT_CTRL + 0x600);

	return NO_ERROR;
}

int mipi_cmd_trigger()
{
	writel(0x1, DSI_CMD_MODE_MDP_SW_TRIGGER);

	return NO_ERROR;
}
