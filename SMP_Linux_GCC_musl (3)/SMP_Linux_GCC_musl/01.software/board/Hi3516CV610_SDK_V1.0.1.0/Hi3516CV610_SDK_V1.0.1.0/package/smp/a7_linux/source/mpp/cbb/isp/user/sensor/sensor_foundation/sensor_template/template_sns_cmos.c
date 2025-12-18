/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ot_mpi_isp.h"
#include "ot_mpi_ae.h"
#include "ot_mpi_awb.h"
#include "template_sns_cmos.h"
#include "template_sns_cmos_param.h"

/****************************************************************************
 * global variables                                                         *
 ****************************************************************************/
static ot_isp_fswdr_mode g_fswdr_mode[OT_ISP_MAX_PIPE_NUM] = {
    [0 ... OT_ISP_MAX_PIPE_NUM - 1] = OT_ISP_FSWDR_NORMAL_MODE
};

static td_u32 g_max_time_get_cnt[OT_ISP_MAX_PIPE_NUM] = {0};
static td_u32 g_init_exposure[OT_ISP_MAX_PIPE_NUM]  = {0};
static td_u32 g_lines_per500ms[OT_ISP_MAX_PIPE_NUM] = {0};
static td_u32 g_init_int_time[OT_ISP_MAX_PIPE_NUM]  = {0};
static td_u32 g_init_again[OT_ISP_MAX_PIPE_NUM]  = {0};
static td_u32 g_init_dgain[OT_ISP_MAX_PIPE_NUM]  = {0};
static td_u32 g_init_isp_dgain[OT_ISP_MAX_PIPE_NUM]  = {0};

static td_u8 g_ae_stat_pos[OT_ISP_MAX_PIPE_NUM] = {0};

static td_u16 g_init_wb_gain[OT_ISP_MAX_PIPE_NUM][OT_ISP_RGB_CHN_NUM] = {{0}};
static td_u16 g_sample_r_gain[OT_ISP_MAX_PIPE_NUM] = {0};
static td_u16 g_sample_b_gain[OT_ISP_MAX_PIPE_NUM] = {0};
static td_bool g_quick_start_en[OT_ISP_MAX_PIPE_NUM] = {TD_FALSE};

static td_bool g_ae_route_ex_valid[OT_ISP_MAX_PIPE_NUM] = {0};
static ot_isp_ae_route g_init_ae_route[OT_ISP_MAX_PIPE_NUM] = {{0}};
static ot_isp_ae_route_ex g_init_ae_route_ex[OT_ISP_MAX_PIPE_NUM] = {{0}};
static ot_isp_ae_route g_init_ae_route_sf[OT_ISP_MAX_PIPE_NUM] = {{0}};
static ot_isp_ae_route_ex g_init_ae_route_sf_ex[OT_ISP_MAX_PIPE_NUM] = {{0}};

static ot_isp_sns_commbus g_bus_info[OT_ISP_MAX_PIPE_NUM] = {
    [0] = { .i2c_dev = 0 },
    [1 ... OT_ISP_MAX_PIPE_NUM - 1] = { .i2c_dev = -1 }
};

static ot_isp_sns_state *g_sns_state[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

static td_bool blc_clamp_info[OT_ISP_MAX_PIPE_NUM] = {[0 ...(OT_ISP_MAX_PIPE_NUM - 1)] = TD_TRUE};

#ifdef SENSOR_SLAVE_MODE
td_s32 g_template_sns_slave_bind_dev[OT_ISP_MAX_PIPE_NUM] = {0, 0, 1, 1};
static td_u32 g_template_sns_slave_sensor_mode_time[OT_ISP_MAX_PIPE_NUM] = {0, 0, 0, 0};
ot_isp_slave_sns_sync g_template_sns_slave_sync[OT_ISP_MAX_PIPE_NUM];
#endif

/* key params of your sensor modes */
const template_sns_video_mode_tbl g_template_sns_mode_tbl[TEMPLATE_SNS_MODE_MAX] = {
    /* mode 0: linear */
    {
#ifdef SENSOR_SLAVE_MODE
        TEMPLATE_SNS_SLAVE_INCK,
        TEMPLATE_SNS_SLAVE_INCK_PER_HS,
        TEMPLATE_SNS_SLAVE_INCK_PER_VS,
#endif
        TEMPLATE_SNS_VMAX_LINEAR,
        TEMPLATE_SNS_FULL_LINES_MAX_LINEAR,
        TEMPLATE_SNS_FPS_MAX_LINEAR,
        TEMPLATE_SNS_FPS_MIN_LINEAR,
        TEMPLATE_SNS_WIDTH_LINEAR,
        TEMPLATE_SNS_HEIGHT_LINEAR,
        TEMPLATE_SNS_MODE_LINEAR,
        OT_WDR_MODE_NONE,
        "TEMPLATE_SNS_4M_30FPS_12BIT_LINEAR_MODE"
     },
    /* mode 1: WDR */
    {
#ifdef SENSOR_SLAVE_MODE
        TEMPLATE_SNS_SLAVE_INCK,
        TEMPLATE_SNS_SLAVE_INCK_PER_HS,
        TEMPLATE_SNS_SLAVE_INCK_PER_VS,
#endif
        TEMPLATE_SNS_VMAX_2TO1_LINE_WDR,
        TEMPLATE_SNS_FULL_LINES_MAX_2TO1_LINE_WDR,
        TEMPLATE_SNS_FPS_MAX_2TO1_LINE_WDR,
        TEMPLATE_SNS_FPS_MIN_2TO1_LINE_WDR,
        TEMPLATE_SNS_WIDTH_2TO1_LINE_WDR,
        TEMPLATE_SNS_HEIGHT_2TO1_LINE_WDR,
        TEMPLATE_SNS_MODE_2TO1_LINE_WDR,
        OT_WDR_MODE_2To1_LINE,
        "TEMPLATE_SNS_4M_30FPS_12BIT_2TO1_LINE_WDR_MODE"
     },
};

/****************************************************************************
  Again & Dgain table for TABLE Mode   NEED CHECK OR REPLACE                *
 ****************************************************************************/
#define GAIN_NODE_NUM 64
static td_u32 g_gain_table[GAIN_NODE_NUM] = {
    1024, 1088, 1152, 1216, 1280, 1344, 1408, 1472, 1536, 1600, 1664, 1728, 1792, 1856, 1920, 1984, 2048, 2176,
    2304, 2432, 2560, 2688, 2816, 2944, 3072, 3200, 3328, 3456, 3584, 3712, 3840, 3968, 4096, 4352, 4608, 4864,
    5120, 5376, 5632, 5888, 6144, 6400, 6656, 6912, 7168, 7424, 7680, 7936, 8192, 8704, 9216, 9728, 10240, 10752,
    11264, 11776, 12288, 12800, 13312, 13824, 14336, 14848, 15360, 15872
};

/****************************************************************************
 * common functions                                                         *
 ****************************************************************************/
ot_isp_sns_commbus *template_sns_get_bus_info(ot_vi_pipe vi_pipe)
{
    if (vi_pipe < 0 || vi_pipe >= OT_ISP_MAX_PIPE_NUM) {
        return TD_NULL;
    }
    return &g_bus_info[vi_pipe];
}

ot_isp_sns_state *template_sns_get_ctx(ot_vi_pipe vi_pipe)
{
    if (vi_pipe < 0 || vi_pipe >= OT_ISP_MAX_PIPE_NUM) {
        return TD_NULL;
    }
    return g_sns_state[vi_pipe];
}

static void template_sns_set_ctx(ot_vi_pipe vi_pipe, ot_isp_sns_state *ctx)
{
    if (vi_pipe < 0 || vi_pipe >= OT_ISP_MAX_PIPE_NUM) {
        return;
    }

    g_sns_state[vi_pipe] = ctx;
}

static void template_sns_reset_ctx(ot_vi_pipe vi_pipe)
{
    if (vi_pipe < 0 || vi_pipe >= OT_ISP_MAX_PIPE_NUM) {
        return;
    }

    g_sns_state[vi_pipe] = TD_NULL;
}

static td_s32 template_sns_check_pipe(ot_vi_pipe vi_pipe)
{
    if ((vi_pipe < 0) || (vi_pipe >= OT_ISP_MAX_PIPE_NUM)) {
        isp_err_trace("Err isp pipe %d!\n", vi_pipe);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

#define template_sns_check_pipe_return(pipe)                          \
    do {                                                            \
        if (template_sns_check_pipe(pipe) != TD_SUCCESS) {            \
            return OT_ERR_ISP_ILLEGAL_PARAM;                        \
        }                                                           \
    } while (0)

static td_void template_sns_set_blc_clamp_value(ot_vi_pipe vi_pipe, td_bool clamp_en)
{
    blc_clamp_info[vi_pipe] = clamp_en;
}

static void template_sns_err_mode_print(const ot_isp_cmos_sns_image_mode *sns_image_mode,
    const ot_isp_sns_state *sns_state)
{
    isp_err_trace("Not support! Width:%u, Height:%u, Fps:%f, WDRMode:%d\n",
        (sns_image_mode)->width, (sns_image_mode)->height, (sns_image_mode)->fps, (sns_state)->wdr_mode);
}

#ifdef SENSOR_SLAVE_MODE
ot_isp_slave_sns_sync *template_sns_get_slave_sync(ot_vi_pipe vi_pipe)
{
    return &g_template_sns_slave_sync[vi_pipe];
}

td_s32 template_sns_get_slave_bind_dev(ot_vi_pipe vi_pipe)
{
    return g_template_sns_slave_bind_dev[vi_pipe];
}

td_u32 template_sns_get_slave_sensor_mode_time(ot_vi_pipe vi_pipe)
{
    return g_template_sns_slave_sensor_mode_time[vi_pipe];
}

const template_sns_video_mode_tbl *template_sns_get_slave_mode_tbl(td_u8 img_mode)
{
    return &g_template_sns_mode_tbl[img_mode];
}
#endif


/****************************************************************************
 * sensor functions                                                         *
 ****************************************************************************/

static td_void cmos_get_ae_comm_default(ot_vi_pipe vi_pipe, ot_isp_ae_sensor_default *ae_sns_dft,
    const ot_isp_sns_state *sns_state)
{
    ae_sns_dft->full_lines_std = sns_state->fl_std;
    ae_sns_dft->flicker_freq = FLICKER_FREQ;
    ae_sns_dft->full_lines_max = TEMPLATE_SNS_FULL_LINES_MAX_LINEAR;
    ae_sns_dft->hmax_times = (1000000000) / (sns_state->fl_std * STANDARD_FPS); /* 1000000000ns, 30fps */

    /* 1.LINEAR mode refers to the linear increase of exposure time or gain in fixed steps.
       For example, each step increases by 0.325 times, or exposure time increases by 1 step.
       The step size is determined by accuracy.
       2.TABLE mode is generally used for gain, which means that the gain that can be achieved in each step
       is calculated by looking up a table in the cmos_again_calc_table() or cmos_dgain_calc_table() function.
       At this time, accuracy loses its meaning and does not take effect. */
    ae_sns_dft->int_time_accu.accu_type = OT_ISP_AE_ACCURACY_LINEAR; /* NEED CHECK OR REPLACE */
    ae_sns_dft->int_time_accu.accuracy = INT_TIME_ACCURACY;
    ae_sns_dft->int_time_accu.offset = 0;

    ae_sns_dft->again_accu.accu_type = OT_ISP_AE_ACCURACY_LINEAR; /* NEED CHECK OR REPLACE */
    ae_sns_dft->again_accu.accuracy  = AGAIN_ACCURACY; /* accuracy: 0.0625 */

    ae_sns_dft->dgain_accu.accu_type = OT_ISP_AE_ACCURACY_LINEAR; /* NEED CHECK OR REPLACE */
    ae_sns_dft->dgain_accu.accuracy = DGAIN_ACCURACY; /* accuracy: 0.0009765625 */

    ae_sns_dft->isp_dgain_shift = ISP_DGAIN_SHIFT; /* accuracy: 8 */
    ae_sns_dft->min_isp_dgain_target = ISP_DGAIN_TARGET_MIN << ae_sns_dft->isp_dgain_shift; /* min 1 */
    ae_sns_dft->max_isp_dgain_target = ISP_DGAIN_TARGET_MAX << ae_sns_dft->isp_dgain_shift; /* max 32 */
    if (g_lines_per500ms[vi_pipe] == 0) {
        ae_sns_dft->lines_per500ms = sns_state->fl_std * STANDARD_FPS / 2; /* 30fps, div 2 */
    } else {
        ae_sns_dft->lines_per500ms = g_lines_per500ms[vi_pipe];
    }

    (td_void)memcpy_s(&ae_sns_dft->piris_attr, sizeof(ot_isp_piris_attr), &g_piris, sizeof(ot_isp_piris_attr));
    ae_sns_dft->max_iris_fno = OT_ISP_IRIS_F_NO_1_4;
    ae_sns_dft->min_iris_fno = OT_ISP_IRIS_F_NO_5_6;

    ae_sns_dft->ae_route_ex_valid = TD_FALSE;
    ae_sns_dft->ae_route_attr.total_num = 0;
    ae_sns_dft->ae_route_attr_ex.total_num = 0;
    ae_sns_dft->quick_start.quick_start_enable = g_quick_start_en[vi_pipe];
    ae_sns_dft->quick_start.black_frame_num = 0;
    ae_sns_dft->ae_stat_pos = g_ae_stat_pos[vi_pipe]; /* 1 use be stat to AE */
    return;
}

static td_void cmos_get_ae_linear_default(ot_vi_pipe vi_pipe, ot_isp_ae_sensor_default *ae_sns_dft,
    const ot_isp_sns_state *sns_state)
{
    ae_sns_dft->max_again = TEMPLATE_SNS_AGAIN_MAX; /* max 992 */
    ae_sns_dft->min_again = TEMPLATE_SNS_AGAIN_MIN;  /* min 16 */
    ae_sns_dft->max_again_target = ae_sns_dft->max_again;
    ae_sns_dft->min_again_target = ae_sns_dft->min_again;

    ae_sns_dft->max_dgain = TEMPLATE_SNS_DGAIN_MAX; /* max 16383 */
    ae_sns_dft->min_dgain = TEMPLATE_SNS_DGAIN_MIN;  /* min 1024 */
    ae_sns_dft->max_dgain_target = ae_sns_dft->max_dgain;
    ae_sns_dft->min_dgain_target = ae_sns_dft->min_dgain;

    ae_sns_dft->ae_compensation = AE_COMENSATION_DEFAULT;
    ae_sns_dft->ae_exp_mode = OT_ISP_AE_EXP_HIGHLIGHT_PRIOR;
    ae_sns_dft->init_exposure = g_init_exposure[vi_pipe] ? g_init_exposure[vi_pipe] : INIT_EXP_DEFAULT_LINEAR;
    ae_sns_dft->init_int_time = g_init_int_time[vi_pipe];
    ae_sns_dft->init_again = g_init_again[vi_pipe];
    ae_sns_dft->init_dgain = g_init_dgain[vi_pipe];
    ae_sns_dft->init_isp_dgain = g_init_isp_dgain[vi_pipe];

    ae_sns_dft->max_int_time = sns_state->fl_std - FL_OFFSET_LINEAR; /* NEED CHECK OR REPLACE */
    ae_sns_dft->min_int_time = 1; /* min int 1 */
    ae_sns_dft->max_int_time_target = MAX_INT_TIME_TARGET;
    ae_sns_dft->min_int_time_target = ae_sns_dft->min_int_time;
    ae_sns_dft->ae_route_ex_valid = g_ae_route_ex_valid[vi_pipe];
    (td_void)memcpy_s(&ae_sns_dft->ae_route_attr, sizeof(ot_isp_ae_route),
                      &g_init_ae_route[vi_pipe],  sizeof(ot_isp_ae_route));
    (td_void)memcpy_s(&ae_sns_dft->ae_route_attr_ex, sizeof(ot_isp_ae_route_ex),
                      &g_init_ae_route_ex[vi_pipe], sizeof(ot_isp_ae_route_ex));
    return;
}

static td_void cmos_get_ae_2to1_line_wdr_default(ot_vi_pipe vi_pipe, ot_isp_ae_sensor_default *ae_sns_dft,
    const ot_isp_sns_state *sns_state)
{
    ae_sns_dft->max_int_time = sns_state->fl_std - FL_OFFSET_WDR; /* NEED CHECK OR REPLACE */
    ae_sns_dft->min_int_time = 2; /* min_int_time 2 */
    ae_sns_dft->int_time_accu.offset = -0.66; /* -0.66 line for stagger */

    ae_sns_dft->max_int_time_target = MAX_INT_TIME_TARGET;
    ae_sns_dft->min_int_time_target = ae_sns_dft->min_int_time;

    ae_sns_dft->max_again = TEMPLATE_SNS_AGAIN_MAX; /* max 992:63488 */
    ae_sns_dft->min_again = TEMPLATE_SNS_AGAIN_MIN;  /* min 16:1024 */
    ae_sns_dft->max_again_target = ae_sns_dft->max_again;
    ae_sns_dft->min_again_target = ae_sns_dft->min_again;

    ae_sns_dft->max_dgain = TEMPLATE_SNS_DGAIN_MAX; /* max 16383 */
    ae_sns_dft->min_dgain = TEMPLATE_SNS_DGAIN_MIN;  /* min 1024 */
    ae_sns_dft->max_dgain_target = ae_sns_dft->max_dgain;
    ae_sns_dft->min_dgain_target = ae_sns_dft->min_dgain;
    ae_sns_dft->max_isp_dgain_target = ISP_DGAIN_TARGET_WDR_MAX << ae_sns_dft->isp_dgain_shift; /* max 4 << shift */
    ae_sns_dft->diff_gain_support = TD_TRUE;

    ae_sns_dft->init_exposure = g_init_exposure[vi_pipe] ? g_init_exposure[vi_pipe] : INIT_EXP_DEFAULT_WDR;
    if (g_fswdr_mode[vi_pipe] == OT_ISP_FSWDR_LONG_FRAME_MODE) {
        ae_sns_dft->ae_compensation = AE_COMENSATION_WDR_LONG_FRM; /* ae_compensation 56 */
        ae_sns_dft->ae_exp_mode = OT_ISP_AE_EXP_HIGHLIGHT_PRIOR;
    } else {
        ae_sns_dft->ae_compensation = AE_COMENSATION_WDR_NORM_FRM; /* ae_compensation 32 */
        ae_sns_dft->ae_exp_mode = OT_ISP_AE_EXP_LOWLIGHT_PRIOR;
        ae_sns_dft->man_ratio_enable = TD_TRUE;
        ae_sns_dft->arr_ratio[0] = AE_ARR_RATIO_0_WDR;
        ae_sns_dft->arr_ratio[1] = AE_ARR_RATIO_1_WDR;
        ae_sns_dft->arr_ratio[2] = AE_ARR_RATIO_2_WDR; /* array index 2 */
    }
    ae_sns_dft->ae_route_ex_valid = g_ae_route_ex_valid[vi_pipe];
    (td_void)memcpy_s(&ae_sns_dft->ae_route_attr, sizeof(ot_isp_ae_route),
                      &g_init_ae_route[vi_pipe],  sizeof(ot_isp_ae_route));
    (td_void)memcpy_s(&ae_sns_dft->ae_route_attr_ex, sizeof(ot_isp_ae_route_ex),
                      &g_init_ae_route_ex[vi_pipe],  sizeof(ot_isp_ae_route_ex));
    (td_void)memcpy_s(&ae_sns_dft->ae_route_sf_attr, sizeof(ot_isp_ae_route),
                      &g_init_ae_route_sf[vi_pipe], sizeof(ot_isp_ae_route));
    (td_void)memcpy_s(&ae_sns_dft->ae_route_sf_attr_ex, sizeof(ot_isp_ae_route_ex),
                      &g_init_ae_route_sf_ex[vi_pipe],  sizeof(ot_isp_ae_route_ex));
    return;
}

static td_s32 cmos_get_ae_default(ot_vi_pipe vi_pipe, ot_isp_ae_sensor_default *ae_sns_dft)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_check_pointer_return(ae_sns_dft);
    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_return(sns_state);

    (td_void)memset_s(&ae_sns_dft->ae_route_attr, sizeof(ot_isp_ae_route), 0, sizeof(ot_isp_ae_route));

    cmos_get_ae_comm_default(vi_pipe, ae_sns_dft, sns_state);

    switch (sns_state->wdr_mode) {
        case OT_WDR_MODE_NONE:   /* linear mode */
            cmos_get_ae_linear_default(vi_pipe, ae_sns_dft, sns_state);
            break;

        case OT_WDR_MODE_2To1_LINE:
            cmos_get_ae_2to1_line_wdr_default(vi_pipe, ae_sns_dft, sns_state);
            break;
        default:
            cmos_get_ae_linear_default(vi_pipe, ae_sns_dft, sns_state);
            break;
    }

    return TD_SUCCESS;
}

static td_void cmos_config_vmax(ot_isp_sns_state *sns_state, td_u32 vmax)
{
    if (sns_state->wdr_mode == OT_WDR_MODE_NONE) {
        sns_state->regs_info[0].i2c_data[VMAX_L_IDX].data = low_8bits(vmax);
        sns_state->regs_info[0].i2c_data[VMAX_H_IDX].data = high_8bits(vmax);
    } else {
        sns_state->regs_info[0].i2c_data[WDR_VMAX_L_IDX].data = low_8bits(vmax);
        sns_state->regs_info[0].i2c_data[WDR_VMAX_H_IDX].data = high_8bits(vmax);
    }

    return;
}

/* the function of sensor set fps */
static td_bool cmos_2to1_line_wdr_vmax_limit(td_u32 *vmax, td_u32 full_line, td_u32 step, td_bool fps_up)
{
    if (fps_up) {
        if ((*vmax) + step < full_line) {
            (*vmax) = (full_line - step);
            return TD_FALSE;
        }
    } else {
        if ((*vmax) > full_line + step) {
            (*vmax) = (full_line + step);
            return TD_FALSE;
        }
    }
    return TD_TRUE;
}

#ifdef SENSOR_SLAVE_MODE
td_float cmos_fps_set_slave(ot_vi_pipe vi_pipe, td_float fps)
{
    td_u32 inck, new_inck;
    static td_u32 last_inck[OT_ISP_MAX_PIPE_NUM] = {0};
    ot_isp_sns_state *sns_state = TD_NULL;
    sns_state = template_sns_get_ctx(vi_pipe);

    inck = g_template_sns_mode_tbl[sns_state->img_mode].inck;
    new_inck = (td_u32)(inck / div_0_to_1_float(fps));
    if (last_inck[vi_pipe] == 0) {
        last_inck[vi_pipe] = new_inck;
    } else {
        if (new_inck > last_inck[vi_pipe] && (new_inck - last_inck[vi_pipe]) > INCK_ONCE_INCREASE_MAX) {
            new_inck = last_inck[vi_pipe] + INCK_ONCE_INCREASE_MAX;
            fps = (td_float)inck / (td_float)new_inck;
        } else if (new_inck < last_inck[vi_pipe] && (last_inck[vi_pipe] - new_inck) > INCK_ONCE_INCREASE_MAX) {
            new_inck = last_inck[vi_pipe] - INCK_ONCE_INCREASE_MAX;
            fps = (td_float)inck / (td_float)new_inck;
        }
        last_inck[vi_pipe] = new_inck;
    }
    g_template_sns_slave_sync[vi_pipe].vs_time = new_inck;
    sns_state->regs_info[0].slv_sync.slave_vs_time = g_template_sns_slave_sync[vi_pipe].vs_time;
    return fps;
}
#endif

/* the function of sensor set fps */
static td_void cmos_fps_set(ot_vi_pipe vi_pipe, td_float fps, ot_isp_ae_sensor_default *ae_sns_dft)
{
    td_u32 lines, lines_max, vmax;
    td_bool achieve_fps_flag;
    td_float max_fps, min_fps;
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_check_pointer_void_return(ae_sns_dft);
    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);

    lines = g_template_sns_mode_tbl[sns_state->img_mode].ver_lines;
    lines_max = g_template_sns_mode_tbl[sns_state->img_mode].max_ver_lines;
    max_fps = g_template_sns_mode_tbl[sns_state->img_mode].max_fps;
    min_fps = g_template_sns_mode_tbl[sns_state->img_mode].min_fps;

    if ((fps > max_fps) || (fps < min_fps)) {
        isp_err_trace("Not support Fps: %f\n", fps);
        return;
    }

    achieve_fps_flag = TD_TRUE;
    vmax = (td_u32)(lines * max_fps / div_0_to_1_float(fps));

    if (sns_state->wdr_mode == OT_WDR_MODE_2To1_LINE) {
        achieve_fps_flag = cmos_2to1_line_wdr_vmax_limit(&vmax, sns_state->fl[0],
            20, fps > ae_sns_dft->fps); /* step 20 */  /* NEED CHECK OR REPLACE */
        vmax = (vmax > lines_max) ? lines_max : vmax;
        cmos_config_vmax(sns_state, vmax);
        sns_state->fl_std = vmax;
        ae_sns_dft->lines_per500ms = (td_u32)(lines * max_fps / 2); /* div 2 */
        ae_sns_dft->max_int_time = sns_state->fl_std - FL_OFFSET_WDR;   /* NEED CHECK OR REPLACE */
    } else {
        vmax = (vmax > lines_max) ? lines_max : vmax;
        cmos_config_vmax(sns_state, vmax);
        sns_state->fl_std = vmax;
        ae_sns_dft->lines_per500ms = (td_u32)(lines * max_fps / 2); /* div 2 */
        ae_sns_dft->max_int_time = sns_state->fl_std - FL_OFFSET_LINEAR; /* NEED CHECK OR REPLACE */
    }
    ae_sns_dft->fps = lines * max_fps * 0x40 / vmax / 0x40;
    ae_sns_dft->fps = (achieve_fps_flag) ? fps : ae_sns_dft->fps;

#ifdef SENSOR_SLAVE_MODE
    ae_sns_dft->fps = cmos_fps_set_slave(vi_pipe, fps);
#endif

    ae_sns_dft->full_lines_std = sns_state->fl_std;
    sns_state->fl[0] = sns_state->fl_std;
    ae_sns_dft->full_lines = sns_state->fl[0];
    ae_sns_dft->hmax_times =
        (td_u32)((1000000000) / (sns_state->fl_std * div_0_to_1_float(fps))); /* 1000000000ns */

    return;
}

#ifdef SENSOR_SLAVE_MODE
static td_void cmos_slow_framerate_set_slave(ot_vi_pipe vi_pipe, td_u32 full_lines)
{
    td_u32 lines, lines_max;
    td_u32 vmax;
    td_u32 inck, new_inck;
    td_float fps, fps_max;
    static td_u32 last_inck[OT_ISP_MAX_PIPE_NUM] = {0};

    ot_isp_sns_state *sns_state = TD_NULL;
    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);

    lines = g_template_sns_mode_tbl[sns_state->img_mode].ver_lines;
    lines_max = g_template_sns_mode_tbl[sns_state->img_mode].max_ver_lines;
    inck = g_template_sns_mode_tbl[sns_state->img_mode].inck;
    fps_max = g_template_sns_mode_tbl[sns_state->img_mode].max_fps;

    vmax = full_lines;
    vmax = (vmax > lines_max) ? lines_max : vmax;
    fps = lines * fps_max / (td_float)full_lines;
    new_inck = (td_u32)(inck / div_0_to_1_float(fps));
    if (last_inck[vi_pipe] == 0) {
        last_inck[vi_pipe] = g_template_sns_slave_sync[vi_pipe].vs_time;
    }
    if (new_inck > last_inck[vi_pipe] && (new_inck - last_inck[vi_pipe]) > INCK_ONCE_INCREASE_MAX) {
        new_inck = last_inck[vi_pipe] + INCK_ONCE_INCREASE_MAX;
        fps = (td_float)inck / (td_float)new_inck;
        vmax = (td_u32)(lines * fps_max / div_0_to_1_float(fps));
    }
    last_inck[vi_pipe] = new_inck;
    sns_state->fl[0] = vmax;
    g_template_sns_slave_sync[vi_pipe].vs_time = new_inck;
    sns_state->regs_info[0].slv_sync.slave_vs_time = g_template_sns_slave_sync[vi_pipe].vs_time;
    return;
}
#endif

static td_void cmos_slow_framerate_set(ot_vi_pipe vi_pipe, td_u32 full_lines, ot_isp_ae_sensor_default *ae_sns_dft)
{
    td_u32 lines_max, vmax;
    td_bool achieve_fps;
    ot_isp_sns_state *sns_state = TD_NULL;
    sns_check_pointer_void_return(ae_sns_dft);
    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);

    lines_max = g_template_sns_mode_tbl[sns_state->img_mode].max_ver_lines;

    if (sns_state->wdr_mode == OT_WDR_MODE_2To1_LINE) {
        vmax = full_lines;
        achieve_fps = cmos_2to1_line_wdr_vmax_limit(&vmax, sns_state->fl[0],
            20, full_lines < sns_state->fl[0]); /* 20 */  /* NEED CHECK OR REPLACE */
        vmax = (vmax > lines_max) ? lines_max : vmax;
        sns_state->fl[0] = vmax;
    } else {
        vmax = full_lines;
        vmax = (vmax > lines_max) ? lines_max : vmax;
        sns_state->fl[0] = vmax;
    }

#ifdef SENSOR_SLAVE_MODE
    cmos_slow_framerate_set_slave(vi_pipe, full_lines);
#endif

    ot_unused(achieve_fps);
    switch (sns_state->wdr_mode) {
        case OT_WDR_MODE_NONE:
            sns_state->regs_info[0].i2c_data[VMAX_L_IDX].data = low_8bits(sns_state->fl[0]);
            sns_state->regs_info[0].i2c_data[VMAX_H_IDX].data = high_8bits(sns_state->fl[0]);
            ae_sns_dft->max_int_time = sns_state->fl[0] - FL_OFFSET_LINEAR; /* NEED CHECK OR REPLACE */
            break;
        case OT_WDR_MODE_2To1_LINE:
            sns_state->regs_info[0].i2c_data[WDR_VMAX_L_IDX].data = low_8bits(sns_state->fl[0]);
            sns_state->regs_info[0].i2c_data[WDR_VMAX_H_IDX].data = high_8bits(sns_state->fl[0]);
            ae_sns_dft->max_int_time = sns_state->fl[0] - FL_OFFSET_WDR; /* NEED CHECK OR REPLACE */
            break;
        default:
            break;
    }

    ae_sns_dft->full_lines = sns_state->fl[0];

    return;
}

static td_void cmos_inttime_update_linear(ot_vi_pipe vi_pipe, td_u32 int_time)
{
    ot_isp_sns_state *sns_state = TD_NULL;
    td_u32 value;

    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);

    value = int_time;

    if (g_quick_start_en[vi_pipe] == TD_TRUE) {
        template_sns_write_register(vi_pipe, TEMPLATE_SNS_EXPO_L_ADDR, low_8bits(value));
        template_sns_write_register(vi_pipe, TEMPLATE_SNS_EXPO_H_ADDR, high_8bits(value));
    } else {
        sns_state->regs_info[0].i2c_data[EXPO_L_IDX].data = low_8bits(value);
        sns_state->regs_info[0].i2c_data[EXPO_H_IDX].data = high_8bits(value);
    }
    return;
}

static td_void cmos_inttime_update_2to1_line_wdr(ot_vi_pipe vi_pipe, td_u32 int_time)
{
    ot_isp_sns_state *sns_state = TD_NULL;
    static td_bool is_first[OT_ISP_MAX_PIPE_NUM] = {[0 ...(OT_ISP_MAX_PIPE_NUM - 1)] = 1};

    static td_u32 short_int_time[OT_ISP_MAX_PIPE_NUM] = {0};
    static td_u32 long_int_time[OT_ISP_MAX_PIPE_NUM] = {0};

    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);

    if (is_first[vi_pipe]) { /* short exposure */
        sns_state->wdr_int_time[0] = int_time;
        short_int_time[vi_pipe] = int_time;
        is_first[vi_pipe] = TD_FALSE;
    } else { /* long exposure */
        sns_state->wdr_int_time[1] = int_time;
        long_int_time[vi_pipe] = int_time;

        sns_state->regs_info[0].i2c_data[EXPO_L_IDX].data = low_8bits(long_int_time[vi_pipe]);
        sns_state->regs_info[0].i2c_data[EXPO_H_IDX].data = high_8bits(long_int_time[vi_pipe]);

        sns_state->regs_info[0].i2c_data[WDR_SHORT_EXPO_L_IDX].data = low_8bits(short_int_time[vi_pipe]);
        sns_state->regs_info[0].i2c_data[WDR_SHORT_EXPO_H_IDX].data = high_8bits(short_int_time[vi_pipe]);
        is_first[vi_pipe] = TD_TRUE;
    }

    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static td_void cmos_inttime_update(ot_vi_pipe vi_pipe, td_u32 int_time)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);

    if (sns_state->wdr_mode == OT_WDR_MODE_2To1_LINE) {
        cmos_inttime_update_2to1_line_wdr(vi_pipe, int_time);
    } else {
        cmos_inttime_update_linear(vi_pipe, int_time);
    }

    return;
}

static td_void cmos_again_calc_table(ot_vi_pipe vi_pipe, td_u32 *again_lin, td_u32 *again_db)
{
    /* reference to again table */  /* NEED CHECK OR REPLACE */
    int i;

    sns_check_pointer_void_return(again_lin);
    sns_check_pointer_void_return(again_db);

    ot_unused(vi_pipe);

    if (*again_lin >= g_gain_table[GAIN_NODE_NUM - 1]) {
        *again_lin = g_gain_table[GAIN_NODE_NUM - 1];
        *again_db = GAIN_NODE_NUM - 1;
        return;
    }

    for (i = 1; i < GAIN_NODE_NUM; i++) {
        if (*again_lin < g_gain_table[i]) {
            *again_lin = g_gain_table[i - 1];
            *again_db = i - 1;
            break;
        }
    }
    return;
}

static td_void cmos_dgain_calc_table(ot_vi_pipe vi_pipe, td_u32 *dgain_lin, td_u32 *dgain_db)
{
    /* reference to dgain table */  /* NEED CHECK OR REPLACE */
    int i;

    sns_check_pointer_void_return(dgain_lin);
    sns_check_pointer_void_return(dgain_db);

    ot_unused(vi_pipe);

    if (*dgain_lin >= g_gain_table[GAIN_NODE_NUM - 1]) {
        *dgain_lin = g_gain_table[GAIN_NODE_NUM - 1];
        *dgain_db = GAIN_NODE_NUM - 1;
        return;
    }

    for (i = 1; i < GAIN_NODE_NUM; i++) {
        if (*dgain_lin < g_gain_table[i]) {
            *dgain_lin = g_gain_table[i - 1];
            *dgain_db = i - 1;
            break;
        }
    }
    return;
}

/* -----------------------------------------------
AGAIN val    --> 2 bytes AGAIN reg
|--------|  |---H----|---L----|
|BBBBAAAA|  |0000BBBB|AAAA0000|

DGAIN val             --> 3 bytes DGAIN reg
|--------|--------|  |---H----|---M----|---L----|
|00CCCCBB|BBBBBBAA|  |0000CCCC|BBBBBBBB|AA000000|
-------------------------------------------------*/
static td_u32 get_reg_value(td_u32 reg_value, reg_mode mode)  /* NEED CHECK OR REPLACE */
{
    td_u32 bytes_val = 0;
    switch (mode) {
        case AGAIN_REG_LOW:
            bytes_val = ((reg_value & AGAIN_REG_LOW_MASK) << AGAIN_REG_LOW_LSHIFT);
            break;
        case AGAIN_REG_HIGH:
            bytes_val = ((reg_value & AGAIN_REG_HIGH_MASK) >> AGAIN_REG_HIGH_RSHIFT);
            break;
        case DGAIN_REG_LOW:
            bytes_val = ((reg_value & DGAIN_REG_LOW_MASK) << DGAIN_REG_LOW_LSHIFT);
            break;
        case DGAIN_REG_MID:
            bytes_val = ((reg_value & DGAIN_REG_MID_MASK) >> DGAIN_REG_MID_RSHIFT);
            break;
        case DGAIN_REG_HIGH:
            bytes_val = ((reg_value & DGAIN_REG_HIGH_MASK) >> DGAIN_REG_HIGH_RSHIFT);
            break;
        default:
            break;
    }
    return bytes_val;
}

static td_void cmos_gains_regs_set(ot_vi_pipe vi_pipe, ot_isp_sns_state *sns_state,
                                   td_u32 again_reg, td_u32 dgain_reg)
{
    static td_bool first_gain[OT_ISP_MAX_PIPE_NUM] = { [0 ...(OT_ISP_MAX_PIPE_NUM - 1)] = 1 };

    if (sns_state->wdr_mode == OT_WDR_MODE_NONE) {
        if (g_quick_start_en[vi_pipe]) {
            template_sns_write_register(vi_pipe, TEMPLATE_SNS_AGAIN_L_ADDR, get_reg_value(again_reg, AGAIN_REG_LOW));
            template_sns_write_register(vi_pipe, TEMPLATE_SNS_AGAIN_H_ADDR, get_reg_value(again_reg, AGAIN_REG_HIGH));
            template_sns_write_register(vi_pipe, TEMPLATE_SNS_DGAIN_L_ADDR, get_reg_value(dgain_reg, DGAIN_REG_LOW));
            template_sns_write_register(vi_pipe, TEMPLATE_SNS_DGAIN_M_ADDR, get_reg_value(dgain_reg, DGAIN_REG_MID));
            template_sns_write_register(vi_pipe, TEMPLATE_SNS_DGAIN_H_ADDR, get_reg_value(dgain_reg, DGAIN_REG_HIGH));
        } else {
            sns_state->regs_info[0].i2c_data[AGAIN_L_IDX].data = get_reg_value(again_reg, AGAIN_REG_LOW);
            sns_state->regs_info[0].i2c_data[AGAIN_H_IDX].data = get_reg_value(again_reg, AGAIN_REG_HIGH);
            sns_state->regs_info[0].i2c_data[DGAIN_L_IDX].data = get_reg_value(dgain_reg, DGAIN_REG_LOW);
            sns_state->regs_info[0].i2c_data[DGAIN_M_IDX].data = get_reg_value(dgain_reg, DGAIN_REG_MID);
            sns_state->regs_info[0].i2c_data[DGAIN_H_IDX].data = get_reg_value(dgain_reg, DGAIN_REG_HIGH);
        }
    } else if (sns_state->wdr_mode == OT_WDR_MODE_2To1_LINE) {
        if (first_gain[vi_pipe] == TD_TRUE) { /* long frame */
            sns_state->regs_info[0].i2c_data[WDR_AGAIN_L_IDX].data = get_reg_value(again_reg, AGAIN_REG_LOW);
            sns_state->regs_info[0].i2c_data[WDR_AGAIN_H_IDX].data = get_reg_value(again_reg, AGAIN_REG_HIGH);

            sns_state->regs_info[0].i2c_data[WDR_DGAIN_L_IDX].data = get_reg_value(dgain_reg, DGAIN_REG_LOW);
            sns_state->regs_info[0].i2c_data[WDR_DGAIN_M_IDX].data = get_reg_value(dgain_reg, DGAIN_REG_MID);
            sns_state->regs_info[0].i2c_data[WDR_DGAIN_H_IDX].data = get_reg_value(dgain_reg, DGAIN_REG_HIGH);

            first_gain[vi_pipe] = TD_FALSE;
        } else {                              /* short frame */
            sns_state->regs_info[0].i2c_data[WDR_SHORT_AGAIN_L_IDX].data = get_reg_value(again_reg, AGAIN_REG_LOW);
            sns_state->regs_info[0].i2c_data[WDR_SHORT_AGAIN_H_IDX].data = get_reg_value(again_reg, AGAIN_REG_HIGH);

            sns_state->regs_info[0].i2c_data[WDR_SHORT_DGAIN_LL_IDX].data = get_reg_value(dgain_reg, DGAIN_REG_LOW);
            sns_state->regs_info[0].i2c_data[WDR_SHORT_DGAIN_L_IDX].data = get_reg_value(dgain_reg, DGAIN_REG_MID);
            sns_state->regs_info[0].i2c_data[WDR_SHORT_DGAIN_H_IDX].data = get_reg_value(dgain_reg, DGAIN_REG_HIGH);

            first_gain[vi_pipe] = TD_TRUE;
        }
    }

    return;
}

static td_void cmos_gains_update(ot_vi_pipe vi_pipe, td_u32 again, td_u32 dgain)
{
    ot_isp_sns_state *sns_state = TD_NULL;
    td_u32 again_reg, dgain_reg;

    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);

    again_reg = clip3(again, TEMPLATE_SNS_AGAIN_MIN, TEMPLATE_SNS_AGAIN_MAX);
    dgain_reg = clip3(dgain, TEMPLATE_SNS_DGAIN_MIN, TEMPLATE_SNS_DGAIN_MAX);

    cmos_gains_regs_set(vi_pipe, sns_state, again_reg, dgain_reg);

    return;
}

static td_void cmos_step_calculate(ot_vi_pipe vi_pipe, time_step *step)  /* NEED CHECK OR REPLACE */
{
    td_u32 i;
    ot_isp_sns_state *sns_state = TD_NULL;
    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);

    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; ++i) {
        step->inc[i] = 0;
        step->dec[i] = 0;
    }
    for (i = 0; i < 2; ++i) { /* frame number is 2 */
        step->inc[i] = (sns_state->fl[1] - TEMPLATE_SNS_STEP_OFFSET) >> 1; /* shift 1 */
        step->dec[i] = (sns_state->fl[1] - TEMPLATE_SNS_STEP_OFFSET) >> 1; /* shift 1 */
    }
    return;
}

static td_void cmos_get_inttime_max_2to1_line_wdr(ot_vi_pipe vi_pipe, td_u32 *ratio,
    ot_isp_ae_int_time_range *int_time, td_u32 *lf_max_int_time)
{
    td_u32 short_max0, short_max, short_time_min_limit;
    ot_isp_sns_state *sns_state = TD_NULL;
    time_step step = {0};
    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);
    short_time_min_limit = 2; /* short_time_min_limit 2 */

    cmos_step_calculate(vi_pipe, &step);  /* NEED CHECK OR REPLACE */
    if (g_fswdr_mode[vi_pipe] == OT_ISP_FSWDR_LONG_FRAME_MODE) {
        short_max0 = sns_state->fl[1] - TEMPLATE_SNS_MARGIN - step.inc[0] - sns_state->wdr_int_time[0];
        short_max = sns_state->fl[0] - TEMPLATE_SNS_MARGIN - step.inc[0];
        short_max = (short_max0 < short_max) ? short_max0 : short_max;
        int_time->int_time_max[0] = short_time_min_limit;
        int_time->int_time_min[0] = short_time_min_limit;
        int_time->int_time_max[1] = short_max;
        int_time->int_time_min[1] = short_time_min_limit;
    } else {
        short_max0 = ((sns_state->fl[1] - TEMPLATE_SNS_MARGIN - step.inc[0] - sns_state->wdr_int_time[0]) * 0x40) /
            div_0_to_1(ratio[0]);
        short_max = ((sns_state->fl[0] - TEMPLATE_SNS_MARGIN - step.inc[0]) * 0x40) / (ratio[0] + 0x40);
        short_max = (short_max0 < short_max) ? short_max0 : short_max;
        short_max = (short_max == 0) ? 1 : short_max;
        *lf_max_int_time = sns_state->fl[0] - TEMPLATE_SNS_MARGIN - step.inc[0];
        if (short_max >= short_time_min_limit) {
            int_time->int_time_max[0] = short_max;
            int_time->int_time_max[1] = (int_time->int_time_max[0] * ratio[0]) >> 6; /* shift 6 */
            int_time->int_time_min[0] = short_time_min_limit;
            int_time->int_time_min[1] = (int_time->int_time_min[0] * ratio[0]) >> 6; /* shift 6 */
        } else {
            short_max = short_time_min_limit;
            int_time->int_time_max[0] = short_max;
            int_time->int_time_max[1] = (int_time->int_time_max[0] * 0xFFF) >> 6; /* shift 6 */
            int_time->int_time_min[0] = int_time->int_time_max[0];
            int_time->int_time_min[1] = int_time->int_time_max[1];
        }
    }
    return;
}

static td_void cmos_get_inttime_max(ot_vi_pipe vi_pipe, td_u16 man_ratio_enable, td_u32 *ratio,
    ot_isp_ae_int_time_range *int_time, td_u32 *lf_max_int_time)
{
    ot_isp_sns_state *sns_state = TD_NULL;
    ot_unused(man_ratio_enable);
    sns_check_pointer_void_return(ratio);
    sns_check_pointer_void_return(int_time);
    sns_check_pointer_void_return(lf_max_int_time);
    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);

    switch (sns_state->wdr_mode) {
        case OT_WDR_MODE_2To1_LINE:
            cmos_get_inttime_max_2to1_line_wdr(vi_pipe, ratio, int_time, lf_max_int_time);
            break;
        default:
            break;
    }

    return;
}

/* Only used in LINE_WDR mode */
static td_void cmos_ae_fswdr_attr_set(ot_vi_pipe vi_pipe, ot_isp_ae_fswdr_attr *ae_fswdr_attr)
{
    sns_check_pointer_void_return(ae_fswdr_attr);

    g_fswdr_mode[vi_pipe] = ae_fswdr_attr->fswdr_mode;
    g_max_time_get_cnt[vi_pipe] = 0;

    return;
}

static td_void cmos_ae_quick_start_status_set(ot_vi_pipe vi_pipe, td_bool quick_start_en)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);

    g_quick_start_en[vi_pipe] = quick_start_en;
    sns_state->sync_init = TD_FALSE;
}

static td_s32 cmos_init_ae_exp_function(ot_isp_ae_sensor_exp_func *exp_func)
{
    sns_check_pointer_return(exp_func);

    (td_void)memset_s(exp_func, sizeof(ot_isp_ae_sensor_exp_func), 0, sizeof(ot_isp_ae_sensor_exp_func));

    exp_func->pfn_cmos_get_ae_default     = cmos_get_ae_default;
    exp_func->pfn_cmos_fps_set            = cmos_fps_set;
    exp_func->pfn_cmos_slow_framerate_set = cmos_slow_framerate_set;
    exp_func->pfn_cmos_inttime_update     = cmos_inttime_update;
    exp_func->pfn_cmos_gains_update       = cmos_gains_update;
    exp_func->pfn_cmos_again_calc_table   = cmos_again_calc_table;
    exp_func->pfn_cmos_dgain_calc_table   = cmos_dgain_calc_table;
    exp_func->pfn_cmos_get_inttime_max    = cmos_get_inttime_max;
    exp_func->pfn_cmos_ae_fswdr_attr_set  = cmos_ae_fswdr_attr_set;
    exp_func->pfn_cmos_ae_quick_start_status_set = cmos_ae_quick_start_status_set;

    return TD_SUCCESS;
}

static td_s32 cmos_get_awb_default(ot_vi_pipe vi_pipe, ot_isp_awb_sensor_default *awb_sns_dft)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_check_pointer_return(awb_sns_dft);
    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_return(sns_state);

    (td_void)memset_s(awb_sns_dft, sizeof(ot_isp_awb_sensor_default), 0, sizeof(ot_isp_awb_sensor_default));
    awb_sns_dft->wb_ref_temp = CALIBRATE_STATIC_TEMP; /* wb_ref_temp 4950 */

    awb_sns_dft->gain_offset[0] = CALIBRATE_STATIC_WB_R_GAIN;
    awb_sns_dft->gain_offset[1] = CALIBRATE_STATIC_WB_GR_GAIN;
    awb_sns_dft->gain_offset[2] = CALIBRATE_STATIC_WB_GB_GAIN; /* index 2 */
    awb_sns_dft->gain_offset[3] = CALIBRATE_STATIC_WB_B_GAIN; /* index 3 */

    awb_sns_dft->wb_para[0] = CALIBRATE_AWB_P1;
    awb_sns_dft->wb_para[1] = CALIBRATE_AWB_P2;
    awb_sns_dft->wb_para[2] = CALIBRATE_AWB_Q1; /* index 2 */
    awb_sns_dft->wb_para[3] = CALIBRATE_AWB_A1; /* index 3 */
    awb_sns_dft->wb_para[4] = CALIBRATE_AWB_B1; /* index 4 */
    awb_sns_dft->wb_para[5] = CALIBRATE_AWB_C1; /* index 5 */

    awb_sns_dft->golden_rgain = GOLDEN_RGAIN;
    awb_sns_dft->golden_bgain = GOLDEN_BGAIN;

    switch (sns_state->wdr_mode) {
        case OT_WDR_MODE_NONE:
            (td_void)memcpy_s(&awb_sns_dft->ccm, sizeof(ot_isp_awb_ccm), &g_awb_ccm, sizeof(ot_isp_awb_ccm));
            (td_void)memcpy_s(&awb_sns_dft->agc_tbl, sizeof(ot_isp_awb_agc_table),
                              &g_awb_agc_table, sizeof(ot_isp_awb_agc_table));
            (td_void)memcpy_s(&awb_sns_dft->sector, sizeof(ot_isp_color_sector),
                &g_color_sector, sizeof(ot_isp_color_sector));
            break;
        case OT_WDR_MODE_2To1_LINE:
            (td_void)memcpy_s(&awb_sns_dft->ccm, sizeof(ot_isp_awb_ccm), &g_awb_ccm_wdr, sizeof(ot_isp_awb_ccm));
            (td_void)memcpy_s(&awb_sns_dft->agc_tbl, sizeof(ot_isp_awb_agc_table),
                              &g_awb_agc_table_wdr, sizeof(ot_isp_awb_agc_table));
            (td_void)memcpy_s(&awb_sns_dft->sector, sizeof(ot_isp_color_sector),
                &g_color_sector_wdr, sizeof(ot_isp_color_sector));
            break;
        default:
            (td_void)memcpy_s(&awb_sns_dft->ccm, sizeof(ot_isp_awb_ccm), &g_awb_ccm, sizeof(ot_isp_awb_ccm));
            (td_void)memcpy_s(&awb_sns_dft->agc_tbl, sizeof(ot_isp_awb_agc_table),
                              &g_awb_agc_table, sizeof(ot_isp_awb_agc_table));
            (td_void)memcpy_s(&awb_sns_dft->sector, sizeof(ot_isp_color_sector),
                &g_color_sector, sizeof(ot_isp_color_sector));
            break;
    }

    awb_sns_dft->init_rgain = g_init_wb_gain[vi_pipe][0]; /* 0: Rgain */
    awb_sns_dft->init_ggain = g_init_wb_gain[vi_pipe][1]; /* 1: Ggain */
    awb_sns_dft->init_bgain = g_init_wb_gain[vi_pipe][2]; /* 2: Bgain */
    awb_sns_dft->sample_rgain = g_sample_r_gain[vi_pipe];
    awb_sns_dft->sample_bgain = g_sample_b_gain[vi_pipe];

    return TD_SUCCESS;
}

static td_s32 cmos_init_awb_exp_function(ot_isp_awb_sensor_exp_func *exp_func)
{
    sns_check_pointer_return(exp_func);

    (td_void)memset_s(exp_func, sizeof(ot_isp_awb_sensor_exp_func), 0, sizeof(ot_isp_awb_sensor_exp_func));

    exp_func->pfn_cmos_get_awb_default = cmos_get_awb_default;

    return TD_SUCCESS;
}

static ot_isp_cmos_dng_color_param g_dng_color_param = {{ 286, 256, 608 }, { 415, 256, 429 },
    { 2810, { 0x01AC, 0x8093, 0x8019, 0x8070, 0x01EA, 0x807A, 0x802A, 0x80F3, 0x021D }},
    { 4940, { 0x01D7, 0x8084, 0x8053, 0x8053, 0x01D9, 0x8086, 0x8010, 0x80B3, 0x01C3 }}};

static td_void cmos_get_isp_dng_default(const ot_isp_sns_state *sns_state, ot_isp_cmos_default *isp_def)
{
    (td_void)memcpy_s(&isp_def->dng_color_param, sizeof(ot_isp_cmos_dng_color_param), &g_dng_color_param,
                      sizeof(ot_isp_cmos_dng_color_param));

    switch (sns_state->img_mode) {
        case TEMPLATE_SNS_4M_30FPS_12BIT_LINEAR_MODE:
            isp_def->sns_mode.dng_raw_format.bits_per_sample = DNG_RAW_FORMAT_BIT_LINEAR; /* 12bit */
            isp_def->sns_mode.dng_raw_format.white_level = DNG_RAW_FORMAT_WHITE_LEVEL_LINEAR; /* max 4095 */
            break;

        case TEMPLATE_SNS_4M_30FPS_12BIT_2TO1_LINE_WDR_MODE:
            isp_def->sns_mode.dng_raw_format.bits_per_sample = DNG_RAW_FORMAT_BIT_WDR; /* 12bit */
            isp_def->sns_mode.dng_raw_format.white_level = DNG_RAW_FORMAT_WHITE_LEVEL_WDR; /* max 4095 */
            break;

        default:
            isp_def->sns_mode.dng_raw_format.bits_per_sample = DNG_RAW_FORMAT_BIT_LINEAR; /* 12bit */
            isp_def->sns_mode.dng_raw_format.white_level = DNG_RAW_FORMAT_WHITE_LEVEL_LINEAR; /* max 4095 */
            break;
    }

    isp_def->sns_mode.dng_raw_format.default_scale.default_scale_hor.denominator = 1;
    isp_def->sns_mode.dng_raw_format.default_scale.default_scale_hor.numerator = 1;
    isp_def->sns_mode.dng_raw_format.default_scale.default_scale_ver.denominator = 1;
    isp_def->sns_mode.dng_raw_format.default_scale.default_scale_ver.numerator = 1;
    isp_def->sns_mode.dng_raw_format.cfa_repeat_pattern_dim.repeat_pattern_dim_row = 2; /* pattern 2 */
    isp_def->sns_mode.dng_raw_format.cfa_repeat_pattern_dim.repeat_pattern_dim_col = 2; /* pattern 2 */
    isp_def->sns_mode.dng_raw_format.black_level_repeat_dim.repeat_row = 2; /* pattern 2 */
    isp_def->sns_mode.dng_raw_format.black_level_repeat_dim.repeat_col = 2; /* pattern 2 */
    isp_def->sns_mode.dng_raw_format.cfa_layout = OT_ISP_CFALAYOUT_TYPE_RECTANGULAR;
    isp_def->sns_mode.dng_raw_format.cfa_plane_color[0] = 0;
    isp_def->sns_mode.dng_raw_format.cfa_plane_color[1] = 1;
    isp_def->sns_mode.dng_raw_format.cfa_plane_color[2] = 2; /* index 2, cfa_plane_color 2 */
    isp_def->sns_mode.dng_raw_format.cfa_pattern[0] = 0;
    isp_def->sns_mode.dng_raw_format.cfa_pattern[1] = 1;
    isp_def->sns_mode.dng_raw_format.cfa_pattern[2] = 1; /* index 2, cfa_pattern 1 */
    isp_def->sns_mode.dng_raw_format.cfa_pattern[3] = 2; /* index 3, cfa_pattern 2 */
    isp_def->sns_mode.valid_dng_raw_format = TD_TRUE;

    return;
}

static void cmos_get_isp_linear_default(ot_isp_cmos_default *isp_def)
{
    isp_def->key.bit1_demosaic         = 1;
    isp_def->demosaic                  = &g_cmos_demosaic;
    isp_def->key.bit1_sharpen          = 1;
    isp_def->sharpen                   = &g_cmos_yuv_sharpen;
    isp_def->key.bit1_drc              = 1;
    isp_def->drc                       = &g_cmos_drc;
    isp_def->key.bit1_bayer_nr         = 1;
    isp_def->bayer_nr                  = &g_cmos_bayer_nr;
    isp_def->key.bit1_anti_false_color = 1;
    isp_def->anti_false_color          = &g_cmos_anti_false_color;
    isp_def->key.bit1_cac              = 1;
    isp_def->cac                       = &g_cmos_cac;
    isp_def->key.bit1_bshp             = 1;
    isp_def->bshp                      = &g_cmos_bayershp;
    isp_def->key.bit1_ldci             = 1;
    isp_def->ldci                      = &g_cmos_ldci;
    isp_def->key.bit1_gamma            = 1;
    isp_def->gamma                     = &g_cmos_gamma;
    isp_def->key.bit1_clut             = 1;
    isp_def->clut                      = &g_cmos_clut;
#ifdef CONFIG_OT_ISP_CR_SUPPORT
    isp_def->key.bit1_ge               = 1;
    isp_def->ge                        = &g_cmos_ge;
#endif
    isp_def->key.bit1_dehaze = 1;
    isp_def->dehaze = &g_cmos_dehaze;
    isp_def->key.bit1_ca = 1;
    isp_def->ca = &g_cmos_ca;
    (td_void)memcpy_s(&isp_def->noise_calibration, sizeof(ot_isp_noise_calibration),
                      &g_cmos_noise_calibration, sizeof(ot_isp_noise_calibration));
    return;
}

static void cmos_get_isp_wdr_default(ot_isp_cmos_default *isp_def)
{
    isp_def->key.bit1_dpc            = 1;
    isp_def->dpc                     = &g_cmos_dpc_wdr;
    isp_def->key.bit1_demosaic       = 1;
    isp_def->demosaic                = &g_cmos_demosaic_wdr;
    isp_def->key.bit1_sharpen        = 1;
    isp_def->sharpen                 = &g_cmos_yuv_sharpen_wdr;
    isp_def->key.bit1_drc            = 1;
    isp_def->drc                     = &g_cmos_drc_wdr;
    isp_def->key.bit1_gamma          = 1;
    isp_def->gamma                   = &g_cmos_gamma_wdr;
    isp_def->key.bit1_clut           = 1;
    isp_def->clut                    = &g_cmos_clut_wdr;
#ifdef CONFIG_OT_ISP_PREGAMMA_SUPPORT
    isp_def->key.bit1_pregamma       = 1;
    isp_def->pregamma                = &g_cmos_pregamma;
#endif
    isp_def->key.bit1_bayer_nr       = 1;
    isp_def->bayer_nr                = &g_cmos_bayer_nr_wdr;
#ifdef CONFIG_OT_ISP_CR_SUPPORT
    isp_def->key.bit1_ge             = 1;
    isp_def->ge                      = &g_cmos_ge_wdr;
#endif
    isp_def->key.bit1_anti_false_color = 1;
    isp_def->anti_false_color = &g_cmos_anti_false_color_wdr;
    isp_def->key.bit1_cac = 1;
    isp_def->cac = &g_cmos_cac_wdr;
    isp_def->key.bit1_bshp= 1;
    isp_def->bshp = &g_cmos_bayershp_wdr;
    isp_def->key.bit1_ldci = 1;
    isp_def->ldci = &g_cmos_ldci_wdr;
    isp_def->key.bit1_dehaze = 1;
    isp_def->dehaze = &g_cmos_dehaze_wdr;

    (td_void)memcpy_s(&isp_def->noise_calibration, sizeof(ot_isp_noise_calibration),
                      &g_cmos_noise_calibration, sizeof(ot_isp_noise_calibration));
    return;
}

static td_s32 cmos_get_isp_default(ot_vi_pipe vi_pipe, ot_isp_cmos_default *isp_def)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_check_pointer_return(isp_def);
    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_return(sns_state);

    (td_void)memset_s(isp_def, sizeof(ot_isp_cmos_default), 0, sizeof(ot_isp_cmos_default));
#ifdef CONFIG_OT_ISP_CA_SUPPORT
    isp_def->key.bit1_ca      = 1;
    isp_def->ca               = &g_cmos_ca;
#endif
    isp_def->key.bit1_dpc     = 1;
    isp_def->dpc              = &g_cmos_dpc;

    isp_def->key.bit1_wdr     = 1;
    isp_def->wdr              = &g_cmos_wdr;

    isp_def->key.bit1_lsc      = 0;
    isp_def->lsc               = &g_cmos_lsc;

    isp_def->key.bit1_acs      = 0;
    isp_def->acs               = &g_cmos_acs;

#ifdef CONFIG_OT_ISP_PREGAMMA_SUPPORT
    isp_def->key.bit1_pregamma = 0;
    isp_def->pregamma          = &g_cmos_pregamma;
#endif
    switch (sns_state->wdr_mode) {
        default:
        case OT_WDR_MODE_NONE:
            cmos_get_isp_linear_default(isp_def);
            break;

        case OT_WDR_MODE_2To1_FRAME:
        case OT_WDR_MODE_2To1_LINE:
            cmos_get_isp_wdr_default(isp_def);

            break;
    }

    isp_def->wdr_switch_attr.exp_ratio[0] = 0x40;

    if (sns_state->wdr_mode == OT_WDR_MODE_2To1_LINE) {
        isp_def->wdr_switch_attr.exp_ratio[0] = 0x400;
    }

    isp_def->sns_mode.sns_id = TEMPLATE_SNS_ID;
    isp_def->sns_mode.sns_mode = sns_state->img_mode;
    cmos_get_isp_dng_default(sns_state, isp_def);

    return TD_SUCCESS;
}

static td_s32 cmos_get_isp_black_level(ot_vi_pipe vi_pipe, ot_isp_cmos_black_level *black_level)
{
    td_s32  i;
    ot_isp_sns_state *sns_state = TD_NULL;
    const ot_isp_cmos_black_level *cmos_blc_def = TD_NULL;

    sns_check_pointer_return(black_level);
    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_return(sns_state);

    if (sns_state->wdr_mode == OT_WDR_MODE_NONE) {
        cmos_blc_def = &g_cmos_blc;
    } else {
        cmos_blc_def = &g_cmos_blc_wdr;
    }
    (td_void)memcpy_s(black_level, sizeof(ot_isp_cmos_black_level), cmos_blc_def, sizeof(ot_isp_cmos_black_level));

    /* Don't need to update black level when iso change */
    black_level->auto_attr.update = TD_FALSE;

    /* black level of linear mode */
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        black_level->auto_attr.black_level[0][i] = BLACK_LEVEL_DEFAULT; /* config 4 bayer channel black level here */
    }

    return TD_SUCCESS;
}

static td_s32 cmos_get_isp_blc_clamp_info(ot_vi_pipe vi_pipe, td_bool *blc_clamp_en)
{
    sns_check_pointer_return(blc_clamp_en);

    *blc_clamp_en = blc_clamp_info[vi_pipe];

    return TD_SUCCESS;
}

static td_void cmos_set_pixel_detect(ot_vi_pipe vi_pipe, td_bool enable)
{
    td_u32 full_lines_5fps;
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);

#ifdef SENSOR_SLAVE_MODE
    (void)(ot_mpi_isp_get_sns_slave_attr(vi_pipe, &g_template_sns_slave_sync[vi_pipe]));
#endif

    if (sns_state->wdr_mode == OT_WDR_MODE_2To1_LINE) {
        return;
    } else {
        if (sns_state->img_mode == TEMPLATE_SNS_4M_30FPS_12BIT_LINEAR_MODE) {
#ifdef SENSOR_SLAVE_MODE
            g_template_sns_slave_sync[vi_pipe].vs_time =
                (td_u32)((g_template_sns_mode_tbl[sns_state->img_mode].inck_per_vs) *
                g_template_sns_mode_tbl[sns_state->img_mode].max_fps / 20); /* divide 20 */
            full_lines_5fps = (TEMPLATE_SNS_VMAX_LINEAR * STANDARD_FPS) / 20; /* 30fps, 20fps */
#else
            full_lines_5fps = (TEMPLATE_SNS_VMAX_LINEAR * STANDARD_FPS) / 5; /* 30fps, 5fps */
#endif
        } else {
            return;
        }
    }

    if (enable) { /* setup for ISP pixel calibration mode */
        template_sns_write_register(vi_pipe, TEMPLATE_SNS_AGAIN_L_ADDR, 0x00);
        template_sns_write_register(vi_pipe, TEMPLATE_SNS_AGAIN_H_ADDR, 0x01);

        template_sns_write_register(vi_pipe, TEMPLATE_SNS_DGAIN_L_ADDR, 0x00);
        template_sns_write_register(vi_pipe, TEMPLATE_SNS_DGAIN_M_ADDR, 0x00);
        template_sns_write_register(vi_pipe, TEMPLATE_SNS_DGAIN_H_ADDR, 0x01);

        template_sns_write_register(vi_pipe, TEMPLATE_SNS_VMAX_L_ADDR, low_8bits(full_lines_5fps));
        template_sns_write_register(vi_pipe, TEMPLATE_SNS_VMAX_H_ADDR, high_8bits(full_lines_5fps));

        template_sns_write_register(vi_pipe, TEMPLATE_SNS_EXPO_L_ADDR, low_8bits(full_lines_5fps - FL_OFFSET_LINEAR));
        template_sns_write_register(vi_pipe, TEMPLATE_SNS_EXPO_H_ADDR, high_8bits(full_lines_5fps - FL_OFFSET_LINEAR));
    } else { /* setup for ISP 'normal mode' */
        sns_state->fl_std = (sns_state->fl_std > TEMPLATE_SNS_FULL_LINES_MAX_LINEAR) ?
            TEMPLATE_SNS_FULL_LINES_MAX_LINEAR : sns_state->fl_std;
#ifdef SENSOR_SLAVE_MODE
        g_template_sns_slave_sync[vi_pipe].vs_time = (td_u32)g_template_sns_mode_tbl[sns_state->img_mode].inck_per_vs;
#endif
        template_sns_write_register(vi_pipe, TEMPLATE_SNS_VMAX_L_ADDR, low_8bits(sns_state->fl_std));
        template_sns_write_register(vi_pipe, TEMPLATE_SNS_VMAX_H_ADDR, high_8bits(sns_state->fl_std));
        sns_state->sync_init = TD_FALSE;
    }
#ifdef SENSOR_SLAVE_MODE
    (void)(ot_mpi_isp_set_sns_slave_attr(vi_pipe, &g_template_sns_slave_sync[vi_pipe]));
#endif
    return;
}

static td_s32 cmos_set_wdr_mode(ot_vi_pipe vi_pipe, td_u8 mode)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_return(sns_state);

    sns_state->sync_init = TD_FALSE;

    switch (mode & 0x3F) {
        case OT_WDR_MODE_NONE:
            sns_state->wdr_mode = OT_WDR_MODE_NONE;
            printf("linear mode\n");
            break;

        case OT_WDR_MODE_2To1_LINE:
            sns_state->wdr_mode = OT_WDR_MODE_2To1_LINE;
            printf("vc wdr 2t1 mode\n");
            break;

        default:
            isp_err_trace("NOT support this mode!\n");
            return TD_FAILURE;
    }

    (td_void)memset_s(sns_state->wdr_int_time, sizeof(sns_state->wdr_int_time), 0, sizeof(sns_state->wdr_int_time));

    return TD_SUCCESS;
}

/*****************************************
config your REG_ID settings
------------------------------------------------------------------------------------------------------------------------
| reg index | update | delay_frame_num | interrupt_pos | dev_addr | reg_addr  | addr_byte_num | data   | data_byte_num |
| XXX_ID0   | TD_TRUE| 0               | 0             | I2C_ADDR | REG_ADDR0 | ADDR_BYTE     | val0   | DATA_BYTE     |
| XXX_ID1   | TD_TRUE| 0               | 0             | I2C_ADDR | REG_ADDR1 | ADDR_BYTE     | val1   | DATA_BYTE     |
| XXX_ID2   | TD_TRUE| 0               | 0             | I2C_ADDR | REG_ADDR2 | ADDR_BYTE     | val2   | DATA_BYTE     |
|...
------------------------------------------------------------------------------------------------------------------------
******************************************/
static td_void cmos_comm_sns_reg_info_init(ot_vi_pipe vi_pipe, ot_isp_sns_state *sns_state)
{
    td_u32 i;
    sns_state->regs_info[0].sns_type = OT_ISP_SNS_TYPE_I2C;
    sns_state->regs_info[0].com_bus.i2c_dev = g_bus_info[vi_pipe].i2c_dev;
    if (g_quick_start_en[vi_pipe] == TD_TRUE) {
        sns_state->regs_info[0].cfg2_valid_delay_max = 1; /* delay_max 1 */
    } else {
        sns_state->regs_info[0].cfg2_valid_delay_max = 2; /* delay_max 2 */
    }
    sns_state->regs_info[0].reg_num = REG_MAX_IDX;

    if (sns_state->wdr_mode == OT_WDR_MODE_2To1_LINE) {
        sns_state->regs_info[0].reg_num = WDR_REG_MAX_IDX;
        sns_state->regs_info[0].cfg2_valid_delay_max = 2; /* delay_max 2 */
    }

    for (i = 0; i < sns_state->regs_info[0].reg_num; i++) {
        sns_state->regs_info[0].i2c_data[i].update = TD_TRUE;
        sns_state->regs_info[0].i2c_data[i].dev_addr = TEMPLATE_SNS_I2C_ADDR;
        sns_state->regs_info[0].i2c_data[i].addr_byte_num = TEMPLATE_SNS_ADDR_BYTE;
        sns_state->regs_info[0].i2c_data[i].data_byte_num = TEMPLATE_SNS_DATA_BYTE;
    }

    /* Linear Mode Regs */
    sns_state->regs_info[0].i2c_data[EXPO_L_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[EXPO_L_IDX].reg_addr = TEMPLATE_SNS_EXPO_L_ADDR;
    sns_state->regs_info[0].i2c_data[EXPO_H_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[EXPO_H_IDX].reg_addr = TEMPLATE_SNS_EXPO_H_ADDR;

    sns_state->regs_info[0].i2c_data[AGAIN_L_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[AGAIN_L_IDX].reg_addr = TEMPLATE_SNS_AGAIN_L_ADDR;
    sns_state->regs_info[0].i2c_data[AGAIN_H_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[AGAIN_H_IDX].reg_addr = TEMPLATE_SNS_AGAIN_H_ADDR;

    sns_state->regs_info[0].i2c_data[DGAIN_L_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[DGAIN_L_IDX].reg_addr = TEMPLATE_SNS_DGAIN_L_ADDR;
    sns_state->regs_info[0].i2c_data[DGAIN_M_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[DGAIN_M_IDX].reg_addr = TEMPLATE_SNS_DGAIN_M_ADDR;
    sns_state->regs_info[0].i2c_data[DGAIN_H_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[DGAIN_H_IDX].reg_addr = TEMPLATE_SNS_DGAIN_H_ADDR;

    sns_state->regs_info[0].i2c_data[VMAX_L_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[VMAX_L_IDX].reg_addr = TEMPLATE_SNS_VMAX_L_ADDR;
    sns_state->regs_info[0].i2c_data[VMAX_H_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[VMAX_H_IDX].reg_addr = TEMPLATE_SNS_VMAX_H_ADDR;

#ifdef SENSOR_SLAVE_MODE
    sns_state->regs_info[0].slv_sync.update = TD_TRUE;
    sns_state->regs_info[0].slv_sync.delay_frame_num = 1; /* delay_frame 1 */
    sns_state->regs_info[0].slv_sync.slave_bind_dev = g_template_sns_slave_bind_dev[vi_pipe];
#endif

    return;
}

static td_void cmos_2to1_line_wdr_sns_reg_info_init(ot_vi_pipe vi_pipe, ot_isp_sns_state *sns_state)
{
    ot_unused(vi_pipe);
    sns_state->regs_info[0].i2c_data[WDR_EXPO_L_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[WDR_EXPO_L_IDX].reg_addr = TEMPLATE_SNS_EXPO_L_ADDR;
    sns_state->regs_info[0].i2c_data[WDR_EXPO_H_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[WDR_EXPO_H_IDX].reg_addr = TEMPLATE_SNS_EXPO_H_ADDR;

    sns_state->regs_info[0].i2c_data[WDR_SHORT_EXPO_L_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[WDR_SHORT_EXPO_L_IDX].reg_addr = TEMPLATE_SNS_SHORT_EXPO_L_ADDR;
    sns_state->regs_info[0].i2c_data[WDR_SHORT_EXPO_H_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[WDR_SHORT_EXPO_H_IDX].reg_addr = TEMPLATE_SNS_SHORT_EXPO_H_ADDR;

    sns_state->regs_info[0].i2c_data[WDR_SHORT_AGAIN_L_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[WDR_SHORT_AGAIN_L_IDX].reg_addr = TEMPLATE_SNS_SHORT_AGAIN_L_ADDR;
    sns_state->regs_info[0].i2c_data[WDR_SHORT_AGAIN_H_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[WDR_SHORT_AGAIN_H_IDX].reg_addr = TEMPLATE_SNS_SHORT_AGAIN_H_ADDR;

    sns_state->regs_info[0].i2c_data[WDR_SHORT_DGAIN_LL_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[WDR_SHORT_DGAIN_LL_IDX].reg_addr = TEMPLATE_SNS_SHORT_DGAIN_LL_ADDR;
    sns_state->regs_info[0].i2c_data[WDR_SHORT_DGAIN_L_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[WDR_SHORT_DGAIN_L_IDX].reg_addr = TEMPLATE_SNS_SHORT_DGAIN_L_ADDR;
    sns_state->regs_info[0].i2c_data[WDR_SHORT_DGAIN_H_IDX].delay_frame_num = 0;
    sns_state->regs_info[0].i2c_data[WDR_SHORT_DGAIN_H_IDX].reg_addr = TEMPLATE_SNS_SHORT_DGAIN_H_ADDR;
    return;
}

static td_void cmos_sns_reg_info_update(ot_vi_pipe vi_pipe, ot_isp_sns_state *sns_state)
{
    td_u32 i;
    ot_unused(vi_pipe);

    for (i = 0; i < sns_state->regs_info[0].reg_num; i++) {
        if (sns_state->regs_info[0].i2c_data[i].data ==
            sns_state->regs_info[1].i2c_data[i].data) {
            sns_state->regs_info[0].i2c_data[i].update = TD_FALSE;
        } else {
            sns_state->regs_info[0].i2c_data[i].update = TD_TRUE;
        }
    }

#ifdef SENSOR_SLAVE_MODE
    if (sns_state->regs_info[0].slv_sync.slave_vs_time == sns_state->regs_info[1].slv_sync.slave_vs_time) {
        sns_state->regs_info[0].slv_sync.update = TD_FALSE;
    } else {
        sns_state->regs_info[0].slv_sync.update = TD_TRUE;
    }
#endif
    return;
}

static td_s32 cmos_get_sns_regs_info(ot_vi_pipe vi_pipe, ot_isp_sns_regs_info *sns_regs_info)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_check_pointer_return(sns_regs_info);
    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_return(sns_state);

    if ((sns_state->sync_init == TD_FALSE) || (sns_regs_info->config == TD_FALSE)) {
        cmos_comm_sns_reg_info_init(vi_pipe, sns_state);
        if (sns_state->wdr_mode == OT_WDR_MODE_2To1_LINE) {
            /* DOL 2t1 Mode Regs */
            cmos_2to1_line_wdr_sns_reg_info_init(vi_pipe, sns_state);
        }
        sns_state->sync_init = TD_TRUE;
    } else {
        cmos_sns_reg_info_update(vi_pipe, sns_state);
    }

    sns_regs_info->config = TD_FALSE;
    (td_void)memcpy_s(sns_regs_info, sizeof(ot_isp_sns_regs_info),
                      &sns_state->regs_info[0], sizeof(ot_isp_sns_regs_info));
    (td_void)memcpy_s(&sns_state->regs_info[1], sizeof(ot_isp_sns_regs_info),
                      &sns_state->regs_info[0], sizeof(ot_isp_sns_regs_info));
    sns_state->fl[1] = sns_state->fl[0];

    return TD_SUCCESS;
}

static td_void cmos_config_image_mode_param(ot_vi_pipe vi_pipe, td_u8 sns_image_mode,
    ot_isp_sns_state *sns_state)
{
    ot_unused(vi_pipe);
    switch (sns_image_mode) {
        case TEMPLATE_SNS_4M_30FPS_12BIT_LINEAR_MODE:
            sns_state->fl_std = TEMPLATE_SNS_VMAX_LINEAR;
            break;
        case TEMPLATE_SNS_4M_30FPS_12BIT_2TO1_LINE_WDR_MODE:
            sns_state->fl_std = TEMPLATE_SNS_VMAX_2TO1_LINE_WDR;
            break;
        default:
            sns_state->fl_std = TEMPLATE_SNS_VMAX_LINEAR;
            break;
    }

    return;
}

static td_s32 cmos_set_image_mode(ot_vi_pipe vi_pipe, const ot_isp_cmos_sns_image_mode *sns_image_mode)
{
    td_u32 i;
    td_u8 image_mode;
    ot_isp_sns_state *sns_state = TD_NULL;
    sns_check_pointer_return(sns_image_mode);
    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_return(sns_state);

    image_mode = sns_state->img_mode;

    for (i = 0; i < TEMPLATE_SNS_MODE_MAX; i++) {
        if (sns_image_mode->fps <= g_template_sns_mode_tbl[i].max_fps &&
            sns_image_mode->width <= g_template_sns_mode_tbl[i].width &&
            sns_image_mode->height <= g_template_sns_mode_tbl[i].height &&
            sns_state->wdr_mode == g_template_sns_mode_tbl[i].wdr_mode) {
            image_mode = (template_sns_res_mode)i;
            break;
        }
    }

    if (i >= TEMPLATE_SNS_MODE_MAX) {
        template_sns_err_mode_print(sns_image_mode, sns_state);
        return TD_FAILURE;
    }

    cmos_config_image_mode_param(vi_pipe, image_mode, sns_state);

    if ((sns_state->init == TD_TRUE) && (image_mode == sns_state->img_mode)) {
        return OT_ISP_DO_NOT_NEED_SWITCH_IMAGEMODE; /* Don't need to switch image_mode */
    }

    sns_state->sync_init = TD_FALSE;
    sns_state->img_mode = image_mode;
    sns_state->fl[0] = sns_state->fl_std;
    sns_state->fl[1] = sns_state->fl[0];

    return TD_SUCCESS;
}

static td_void cmos_sns_global_init(ot_vi_pipe vi_pipe)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_state = template_sns_get_ctx(vi_pipe);
    sns_check_pointer_void_return(sns_state);

    sns_state->init      = TD_FALSE;
    sns_state->sync_init = TD_FALSE;
    sns_state->img_mode  = TEMPLATE_SNS_4M_30FPS_12BIT_LINEAR_MODE;
    sns_state->wdr_mode  = OT_WDR_MODE_NONE;
    sns_state->fl_std    = TEMPLATE_SNS_VMAX_LINEAR; /* standard 30fps vmax */
    sns_state->fl[0]     = TEMPLATE_SNS_VMAX_LINEAR; /* now active vmax */
    sns_state->fl[1]     = TEMPLATE_SNS_VMAX_LINEAR; /* now active vmax */

    (td_void)memset_s(&sns_state->regs_info[0], sizeof(ot_isp_sns_regs_info), 0, sizeof(ot_isp_sns_regs_info));
    (td_void)memset_s(&sns_state->regs_info[1], sizeof(ot_isp_sns_regs_info), 0, sizeof(ot_isp_sns_regs_info));

    return;
}

static td_s32 cmos_init_sensor_exp_function(ot_isp_sns_exp_func *sensor_exp_func)
{
    sns_check_pointer_return(sensor_exp_func);

    (td_void)memset_s(sensor_exp_func, sizeof(ot_isp_sns_exp_func), 0, sizeof(ot_isp_sns_exp_func));

    sensor_exp_func->pfn_cmos_sns_init            = template_sns_init;
    sensor_exp_func->pfn_cmos_sns_exit            = template_sns_exit;
    sensor_exp_func->pfn_cmos_sns_global_init     = cmos_sns_global_init;
    sensor_exp_func->pfn_cmos_set_image_mode      = cmos_set_image_mode;
    sensor_exp_func->pfn_cmos_set_wdr_mode        = cmos_set_wdr_mode;
    sensor_exp_func->pfn_cmos_get_isp_default     = cmos_get_isp_default;
    sensor_exp_func->pfn_cmos_get_isp_black_level = cmos_get_isp_black_level;
    sensor_exp_func->pfn_cmos_get_blc_clamp_info  = cmos_get_isp_blc_clamp_info;
    sensor_exp_func->pfn_cmos_set_pixel_detect    = cmos_set_pixel_detect;
    sensor_exp_func->pfn_cmos_get_sns_reg_info    = cmos_get_sns_regs_info;

    return TD_SUCCESS;
}

static td_s32 template_sns_set_bus_info(ot_vi_pipe vi_pipe, ot_isp_sns_commbus sns_bus_info)
{
    template_sns_check_pipe_return(vi_pipe);
    g_bus_info[vi_pipe].i2c_dev = sns_bus_info.i2c_dev;

    return TD_SUCCESS;
}

static td_void template_sns_blc_clamp(ot_vi_pipe vi_pipe, ot_isp_sns_blc_clamp blc_clamp)
{
    td_s32 ret = TD_SUCCESS;

    template_sns_set_blc_clamp_value(vi_pipe, blc_clamp.blc_clamp_en);

    if (blc_clamp.blc_clamp_en == TD_TRUE) {
        ret += template_sns_write_register(vi_pipe, 0x4001, 0xeb);  /* clamp on */
    } else {
        ret += template_sns_write_register(vi_pipe, 0x4001, 0xea);  /* clamp off */
    }

    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

static td_s32 sensor_ctx_init(ot_vi_pipe vi_pipe)
{
    ot_isp_sns_state *sns_state_ctx = TD_NULL;

    sns_state_ctx = template_sns_get_ctx(vi_pipe);
    if (sns_state_ctx == TD_NULL) {
        sns_state_ctx = (ot_isp_sns_state *)malloc(sizeof(ot_isp_sns_state));
        if (sns_state_ctx == TD_NULL) {
            isp_err_trace("Isp[%d] SnsCtx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(sns_state_ctx, sizeof(ot_isp_sns_state), 0, sizeof(ot_isp_sns_state));

    template_sns_set_ctx(vi_pipe, sns_state_ctx);

    return TD_SUCCESS;
}

static td_void sensor_ctx_exit(ot_vi_pipe vi_pipe)
{
    ot_isp_sns_state *sns_state_ctx = TD_NULL;

    sns_state_ctx = template_sns_get_ctx(vi_pipe);
    sns_free(sns_state_ctx);
    template_sns_reset_ctx(vi_pipe);

    return;
}

static td_s32 sensor_register_callback(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *ae_lib, ot_isp_3a_alg_lib *awb_lib)
{
    td_s32 ret;
    ot_isp_sns_register isp_register;
    ot_isp_ae_sensor_register ae_register;
    ot_isp_awb_sensor_register awb_register;
    ot_isp_sns_attr_info sns_attr_info;

    template_sns_check_pipe_return(vi_pipe);
    sns_check_pointer_return(ae_lib);
    sns_check_pointer_return(awb_lib);

    ret = sensor_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sns_attr_info.sns_id = TEMPLATE_SNS_ID;
    ret = cmos_init_sensor_exp_function(&isp_register.sns_exp);
    if (ret != TD_SUCCESS) {
        isp_err_trace("cmos init exp function failed!\n");
        return TD_FAILURE;
    }
    ret = ot_mpi_isp_sensor_reg_callback(vi_pipe, &sns_attr_info, &isp_register);
    if (ret != TD_SUCCESS) {
        isp_err_trace("sensor register callback function failed!\n");
        return ret;
    }

    ret = cmos_init_ae_exp_function(&ae_register.sns_exp);
    if (ret != TD_SUCCESS) {
        isp_err_trace("cmos init ae exp function failed!\n");
        return TD_FAILURE;
    }
    ret = ot_mpi_ae_sensor_reg_callback(vi_pipe, ae_lib, &sns_attr_info, &ae_register);
    if (ret != TD_SUCCESS) {
        isp_err_trace("sensor register callback function to ae lib failed!\n");
        return ret;
    }

    ret = cmos_init_awb_exp_function(&awb_register.sns_exp);
    if (ret != TD_SUCCESS) {
        isp_err_trace("cmos init awb exp function failed!\n");
        return TD_FAILURE;
    }
    ret = ot_mpi_awb_sensor_reg_callback(vi_pipe, awb_lib, &sns_attr_info, &awb_register);
    if (ret != TD_SUCCESS) {
        isp_err_trace("sensor register callback function to awb lib failed!\n");
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 sensor_unregister_callback(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *ae_lib, ot_isp_3a_alg_lib *awb_lib)
{
    td_s32 ret;

    template_sns_check_pipe_return(vi_pipe);
    sns_check_pointer_return(ae_lib);
    sns_check_pointer_return(awb_lib);

    ret = ot_mpi_isp_sensor_unreg_callback(vi_pipe, TEMPLATE_SNS_ID);
    if (ret != TD_SUCCESS) {
        isp_err_trace("sensor unregister callback function failed!\n");
        return ret;
    }

    ret = ot_mpi_ae_sensor_unreg_callback(vi_pipe, ae_lib, TEMPLATE_SNS_ID);
    if (ret != TD_SUCCESS) {
        isp_err_trace("sensor unregister callback function to ae lib failed!\n");
        return ret;
    }

    ret = ot_mpi_awb_sensor_unreg_callback(vi_pipe, awb_lib, TEMPLATE_SNS_ID);
    if (ret != TD_SUCCESS) {
        isp_err_trace("sensor unregister callback function to awb lib failed!\n");
        return ret;
    }

    sensor_ctx_exit(vi_pipe);
    return TD_SUCCESS;
}

static td_s32 sensor_set_init(ot_vi_pipe vi_pipe, ot_isp_init_attr *init_attr)
{
    template_sns_check_pipe_return(vi_pipe);
    sns_check_pointer_return(init_attr);

    g_init_exposure[vi_pipe]  = init_attr->exposure;
    g_init_int_time[vi_pipe] = init_attr->exp_time;
    g_init_again[vi_pipe] = init_attr->a_gain;
    g_init_dgain[vi_pipe] = init_attr->d_gain;
    g_init_isp_dgain[vi_pipe] = init_attr->ispd_gain;
    g_lines_per500ms[vi_pipe] = init_attr->lines_per500ms;
    g_init_wb_gain[vi_pipe][0] = init_attr->wb_r_gain; /* 0: rgain */
    g_init_wb_gain[vi_pipe][1] = init_attr->wb_g_gain; /* 1: ggain */
    g_init_wb_gain[vi_pipe][2] = init_attr->wb_b_gain; /* 2: bgain */
    g_sample_r_gain[vi_pipe] = init_attr->sample_r_gain;
    g_sample_b_gain[vi_pipe] = init_attr->sample_b_gain;
    g_quick_start_en[vi_pipe] = init_attr->quick_start_en;
    g_ae_route_ex_valid[vi_pipe] = init_attr->ae_route_ex_valid;
    g_ae_stat_pos[vi_pipe]       = init_attr->ae_stat_pos;

    (td_void)memcpy_s(&g_init_ae_route[vi_pipe], sizeof(ot_isp_ae_route),
                      &init_attr->ae_route, sizeof(ot_isp_ae_route));
    (td_void)memcpy_s(&g_init_ae_route_ex[vi_pipe], sizeof(ot_isp_ae_route_ex),
                      &init_attr->ae_route_ex, sizeof(ot_isp_ae_route_ex));
    (td_void)memcpy_s(&g_init_ae_route_sf[vi_pipe], sizeof(ot_isp_ae_route),
                      &init_attr->ae_route_sf, sizeof(ot_isp_ae_route));
    (td_void)memcpy_s(&g_init_ae_route_sf_ex[vi_pipe], sizeof(ot_isp_ae_route_ex),
                      &init_attr->ae_route_sf_ex, sizeof(ot_isp_ae_route_ex));
    return TD_SUCCESS;
}

ot_isp_sns_obj g_sns_template_sns_obj = {
    .pfn_register_callback     = sensor_register_callback,
    .pfn_un_register_callback  = sensor_unregister_callback,
    .pfn_standby               = template_sns_standby,
    .pfn_restart               = template_sns_restart,
    .pfn_mirror_flip           = TD_NULL,
    .pfn_set_blc_clamp         = template_sns_blc_clamp,
    .pfn_write_reg             = template_sns_write_register,
    .pfn_read_reg              = template_sns_read_register,
    .pfn_set_bus_info          = template_sns_set_bus_info,
    .pfn_set_init              = sensor_set_init
};
