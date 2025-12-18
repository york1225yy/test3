/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SYS_CTRL_H
#define SYS_CTRL_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
    VI_ONLINE_VIDEO_NORM_VPSS_ONLINE = 0,   /* OT_VI_ONLINE_VPSS_ONLINE */
    VI_ONLINE_VIDEO_NORM_VPSS_OFFLINE,      /* OT_VI_ONLINE_VPSS_OFFLINE */
    VI_OFFLINE_VIDEO_NORM_VPSS_ONLINE,      /* OT_VI_OFFLINE_VPSS_ONLINE */
    VI_OFFLINE_VIDEO_NORM_VPSS_OFFLINE,     /* OT_VI_OFFLINE_VPSS_OFFLINE */
    VI_VPSS_ONLINE_BUTT
} vi_vpss_online_mode;

#define VI_VPSS_ONLINE_MODE_DEF VI_OFFLINE_VIDEO_NORM_VPSS_OFFLINE

void sys_ctl(int online_flag);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* SYS_CTRL_H */
