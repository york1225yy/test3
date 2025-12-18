/*
 *
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef MEM_MAP_H
#define MEM_MAP_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

extern void *memmap(unsigned long phy_addr, unsigned long size);
extern int memunmap(void *addr_mapped);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

