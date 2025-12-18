/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "sys_ctrl.h"
#include "clk_cfg.h"
#include "sys_cfg.h"

static void set_vi_online_video_norm_vpss_online_qos(void)
{
    void *misc_base = sys_config_get_reg_misc();

    /*
     * write_qos:
     *         30:28        26:24        22:20        18:16        14:12        10:8        6:4        2:0
     * 0x300:  cpu_inst     cpu_data     coresight    vpss         viproc       vicap       ddrt       vedu
     * 0x304:  aiao         vgs          gzip         jpge      npu_instr     npu_smmu  npu_high_data  npu_low_data
     * 0x308:  eth          sdio1        sdio0        edma         spacc        fmc         usb2       usb2_uvc
     */
    sys_writel(misc_base, 0x0300, 0x55477705);
    sys_writel(misc_base, 0x0304, 0x70057054);
    sys_writel(misc_base, 0x0308, 0x44404447);

    /*
     * read_qos:
     *         30:28        26:24        22:20        18:16        14:12        10:8        6:4        2:0
     * 0x310:  cpu_inst     cpu_data     coresight    vpss         viproc       vicap       ddrt       vedu
     * 0x314:  aiao         vgs          gzip         jpge      npu_instr     npu_smmu  npu_high_data  npu_low_data
     * 0x318:  eth          sdio1        sdio0        edma         spacc        fmc         usb2       usb2_uvc
     */
    sys_writel(misc_base, 0x0310, 0x66477706);
    sys_writel(misc_base, 0x0314, 0x70057054);
    sys_writel(misc_base, 0x0318, 0x55505547);
}

static void set_vi_online_video_norm_vpss_offline_qos(void)
{
    void *misc_base = sys_config_get_reg_misc();

    /*
     * write_qos:
     *         30:28        26:24        22:20        18:16        14:12        10:8        6:4        2:0
     * 0x300:  cpu_inst     cpu_data     coresight    vpss         viproc       vicap       ddrt       vedu
     * 0x304:  aiao         vgs          gzip         jpge      npu_instr     npu_smmu  npu_high_data  npu_low_data
     * 0x308:  eth          sdio1        sdio0        edma         spacc        fmc         usb2       usb2_uvc
     */
    sys_writel(misc_base, 0x0300, 0x55447705);
    sys_writel(misc_base, 0x0304, 0x74057054);
    sys_writel(misc_base, 0x0308, 0x44404447);

    /*
     * read_qos:
     *         30:28        26:24        22:20        18:16        14:12        10:8        6:4        2:0
     * 0x310:  cpu_inst     cpu_data     coresight    vpss         viproc       vicap       ddrt       vedu
     * 0x314:  aiao         vgs          gzip         jpge      npu_instr     npu_smmu  npu_high_data  npu_low_data
     * 0x318:  eth          sdio1        sdio0        edma         spacc        fmc         usb2       usb2_uvc
     */
    sys_writel(misc_base, 0x0310, 0x66457706);
    sys_writel(misc_base, 0x0314, 0x75057054);
    sys_writel(misc_base, 0x0318, 0x55505547);
}

static void set_vi_offline_video_norm_vpss_online_qos(void)
{
    void *misc_base = sys_config_get_reg_misc();

    /*
     * write_qos:
     *         30:28        26:24        22:20        18:16        14:12        10:8        6:4        2:0
     * 0x300:  cpu_inst     cpu_data     coresight    vpss         viproc       vicap       ddrt       vedu
     * 0x304:  aiao         vgs          gzip         jpge      npu_instr     npu_smmu  npu_high_data  npu_low_data
     * 0x308:  eth          sdio1        sdio0        edma         spacc        fmc         usb2       usb2_uvc
     */
    sys_writel(misc_base, 0x0300, 0x55446705);
    sys_writel(misc_base, 0x0304, 0x70057054);
    sys_writel(misc_base, 0x0308, 0x44404447);

    /*
     * read_qos:
     *         30:28        26:24        22:20        18:16        14:12        10:8        6:4        2:0
     * 0x310:  cpu_inst     cpu_data     coresight    vpss         viproc       vicap       ddrt       vedu
     * 0x314:  aiao         vgs          gzip         jpge      npu_instr     npu_smmu  npu_high_data  npu_low_data
     * 0x318:  eth          sdio1        sdio0        edma         spacc        fmc         usb2       usb2_uvc
     */
    sys_writel(misc_base, 0x0310, 0x66456706);
    sys_writel(misc_base, 0x0314, 0x70057054);
    sys_writel(misc_base, 0x0318, 0x55505547);
}

static void set_vi_offline_video_norm_vpss_offline_qos(void)
{
    void *misc_base = sys_config_get_reg_misc();

    /*
     * write_qos:
     *         30:28        26:24        22:20        18:16        14:12        10:8        6:4        2:0
     * 0x300:  cpu_inst     cpu_data     coresight    vpss         viproc       vicap       ddrt       vedu
     * 0x304:  aiao         vgs          gzip         jpge      npu_instr     npu_smmu  npu_high_data  npu_low_data
     * 0x308:  eth          sdio1        sdio0        edma         spacc        fmc         usb2       usb2_uvc
     */
    sys_writel(misc_base, 0x0300, 0x55447705);
    sys_writel(misc_base, 0x0304, 0x74057054);
    sys_writel(misc_base, 0x0308, 0x44404447);

    /*
     * read_qos:
     *         30:28        26:24        22:20        18:16        14:12        10:8        6:4        2:0
     * 0x310:  cpu_inst     cpu_data     coresight    vpss         viproc       vicap       ddrt       vedu
     * 0x314:  aiao         vgs          gzip         jpge      npu_instr     npu_smmu  npu_high_data  npu_low_data
     * 0x318:  eth          sdio1        sdio0        edma         spacc        fmc         usb2       usb2_uvc
     */
    sys_writel(misc_base, 0x0310, 0x66457706);
    sys_writel(misc_base, 0x0314, 0x75057054);
    sys_writel(misc_base, 0x0318, 0x55505547);
}

void sys_ctl(int online_flag)
{
    isp_clk_ctrl();
    switch (online_flag) {
        case VI_ONLINE_VIDEO_NORM_VPSS_ONLINE:
            return set_vi_online_video_norm_vpss_online_qos();
        case VI_ONLINE_VIDEO_NORM_VPSS_OFFLINE:
            return set_vi_online_video_norm_vpss_offline_qos();
        case VI_OFFLINE_VIDEO_NORM_VPSS_ONLINE:
            return set_vi_offline_video_norm_vpss_online_qos();
        case VI_OFFLINE_VIDEO_NORM_VPSS_OFFLINE:
            return set_vi_offline_video_norm_vpss_offline_qos();
        default:
            return;
    }
}
EXPORT_SYMBOL(sys_ctl);
