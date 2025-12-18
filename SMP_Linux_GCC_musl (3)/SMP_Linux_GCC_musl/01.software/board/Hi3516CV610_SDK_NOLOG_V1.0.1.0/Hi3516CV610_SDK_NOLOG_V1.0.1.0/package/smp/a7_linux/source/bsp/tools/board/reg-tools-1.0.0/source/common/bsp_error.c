/*
 *
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include "bsp_config.h"
#include "bsp_dbg.h"
#include "bsp_error.h"

#ifdef LOGQUEUE
#include "bsp_message.h"
#endif

#define STRFMT_ERRCODE "%#010lX"

#ifdef DEBUG
void err_exit(const char *msg, int err_code)
{
	if (msg == NULL) {
		printf("msg is null!\n");
		return;
	}

	write_log_error("%s exit:"STRFMT_ERRCODE".{%s:%d}\n", msg, err_code, __FILE__, __LINE__);
	printf("[END]\n");
	exit(err_code);
}
#else
void err_exit(const char *msg, int err_code)
{
	if (msg == NULL) {
		printf("msg is null!\n");
		return;
	}

	printf("[END]\n");
	(exit(err_code));
}
#endif
