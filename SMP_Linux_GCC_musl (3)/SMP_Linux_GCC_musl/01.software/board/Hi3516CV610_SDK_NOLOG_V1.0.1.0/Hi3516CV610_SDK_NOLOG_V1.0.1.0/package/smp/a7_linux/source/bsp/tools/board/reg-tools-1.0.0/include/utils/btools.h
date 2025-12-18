/*
 *
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef BTOOLS_H
#define BTOOLS_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int bspmd_l(int argc, char *argv[]);

int bspmd(int argc, char *argv[]);

int bspmm(int argc, char *argv[]);

int bspddrs(int argc, char *argv[]);

int i2c_read(int argc, char *argv[]);

int i2c_write(int argc, char *argv[]);

int ssp_read(int argc, char *argv[]);

int ssp_write(int argc, char *argv[]);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* BTOOLS_H */
