/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include <types.h>
#include <platform.h>
#include <lib.h>

#define TIMER_DIVIDER_MS  (TIMER_FEQ / TIMER_DIV / 1000)

unsigned long timer_get_divider()
{
	return TIMER_DIVIDER_MS;
}
