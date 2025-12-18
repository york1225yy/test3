/*
 *
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/mman.h>
#include <ctype.h>
#include <math.h>

#include "bsp_type.h"
#include "memmap.h"
#include "strfunc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define OPEN_FILE "/dev/mem"
#define DDR_STAT_SIG SIGUSR1

#define DDRC_PLL_CTRL 0x11151010

#define PAGE_SIZE_MASK 0xfffff000

#define TIMER_INTERVAL 1000

#define DDRC_BASE_ADDR 0x11148000

#define DDRC_MAP_LENGTH 0x20000

#define DMC_NUM	1
#define DMC0 0

#define DDRC0_ADDR 0x0

#define DDRC_TEST_EN 0x010 /* enable traffic statistics collection. */
#define DDRC_TEST7 0x26C /* configure the statistics collection mode and time. */
#define DDRC_TEST8 0x380 /* read the write traffic statistics. */
#define DDRC_TEST9 0x384 /* read the read traffic statistics. */
#define DDRC_TEST10 0x388 /* read write Command Statistics */
#define DDRC_TEST11 0x38C /* read read Command Statistics */

#define MHZ_HZ_RATIO 1000000
#define DDRC_PERF_MODE_ONE_TIME	(1 << 28)
#define PREF_RATIO 16
#define PERF_PRF_MASK 			0x0fffffff

#define MAX_INTERVAL 3000
#define DEFAULT_FREQ_MHZ 530
#define PERCENTAGE 100

#define STATUS_PRINT_HELP 2

static unsigned int dmc_bit_width = 64;

struct bspddrs_args {
	unsigned dmc_sel[DMC_NUM];
	unsigned int timer_interval;
	unsigned int ddrc_freq;
	unsigned int table_freq;
};

static struct bspddrs_args statistic_args;

static int mem_fd;
static void* ddrc_pll_ctrl_map;
static void *ddrc_base_addr;

static timer_t statistic_timer;

static size_t ddrc_addr_offset[DMC_NUM] = {
	DDRC0_ADDR,
};
static void *ddrc_addr[DMC_NUM];

static int ddrc_remap(void);
static void ddrc_unmap(void);
static void print_usage(void);

static unsigned int ddrc_reg_read(int dmc_num, size_t offset)
{
	void *addr = ddrc_addr[dmc_num];
	return *(volatile unsigned int *)((uintptr_t)addr + offset);
}

static unsigned int ddrc_freq_read(void* pll_ctrl_addr)
{
	return *(volatile unsigned int *)(uintptr_t)(pll_ctrl_addr);
}

static void ddrc_reg_write(int dmc_num, size_t offset, unsigned int val)
{
	void *addr = ddrc_addr[dmc_num];
	*(volatile unsigned int *)((uintptr_t)addr + offset) = val;
}

static void init_ddr_bandwidth_statistic(int dmc_num, unsigned int perf_prd)
{
	unsigned int reg_value;
	reg_value = (perf_prd & 0x0fffffff) | DDRC_PERF_MODE_ONE_TIME;
	ddrc_reg_write(dmc_num, DDRC_TEST7, reg_value);
}

static inline void enable_ddr_bandwidth_statistic(int dmc_num)
{
	ddrc_reg_write(dmc_num, DDRC_TEST_EN, 0x1);
}

static inline void disable_ddr_bandwidth_statistic(int dmc_num)
{
	ddrc_reg_write(dmc_num, DDRC_TEST_EN, 0x0);
}

static unsigned int get_bit_width(int dmc_num)
{
	unsigned int value = 0;

	value = ddrc_reg_read(dmc_num, 0x050);
	value = (value >> 4) & 0x3; /* 4:Move Length */

	return value;
}

static void get_ddr_frequency(void)
{
	unsigned int value;
	unsigned int i, j;

	value = ddrc_freq_read(ddrc_pll_ctrl_map);
	i = (value >> 17) & 0x7;

	j = (value >> 20) & 0xff;

	statistic_args.ddrc_freq = j * 6 + i; /* 6:multiple of the calculation frequency */
	printf("table_freq = %u MHz !\n", statistic_args.ddrc_freq);
}

static void get_ddr_utilization(int dmc_num)
{
	unsigned int ddr_read;
	unsigned int ddr_write;
	unsigned int ddr_read_count;
	unsigned int ddr_write_count;
	unsigned int tmp1;
	double tmp2;
	double rate;

	tmp1 = ddrc_reg_read(dmc_num, DDRC_TEST7);
	ddr_read = ddrc_reg_read(dmc_num, DDRC_TEST8);
	ddr_write = ddrc_reg_read(dmc_num, DDRC_TEST9);
	ddr_read_count = ddrc_reg_read(dmc_num, DDRC_TEST11);
	ddr_write_count = ddrc_reg_read(dmc_num, DDRC_TEST10);
	tmp1 = tmp1 & PERF_PRF_MASK;
	tmp2 = ddr_read + ddr_write;
	rate = (tmp2 / tmp1) * PERCENTAGE / PREF_RATIO;

	if ((get_bit_width(dmc_num) == 1) && (dmc_bit_width == 128)) /* 128:DMC Data bit width */
		rate = rate * 2; /* 2:multiples */

	printf("ddrc[%d][%0.2f%%], rcount[%u], wcount[%u]\n", dmc_num, rate, ddr_read_count, ddr_write_count);
}

static void ddr_statistic(int n)
{
	int i;
	(void)n;
	for (i = 0; i < DMC_NUM; i++) {
		if (statistic_args.dmc_sel[i]) {
			get_ddr_utilization(i);
			enable_ddr_bandwidth_statistic(i);
		}
	}
	printf("\n");
	if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
		printf("failed to set up buffer for stdout file\n");

	return;
}

static void ddr_ctrl_c(int n)
{
	int i;
	/* FALSE ddr bandwidth statistic */
	(void)n;
	for (i = 0; i < DMC_NUM; i++) {
		if (statistic_args.dmc_sel[i])
			disable_ddr_bandwidth_statistic(i);
	}
	timer_delete(statistic_timer);
	ddrc_unmap();
	close(mem_fd);
	mem_fd = -1;
	_exit(0);
}

static int ddrc_remap(void)
{
	unsigned int phy_addr_in_page = DDRC_BASE_ADDR & PAGE_SIZE_MASK;
	int i;

	ddrc_base_addr = (unsigned char *)mmap(NULL, DDRC_MAP_LENGTH, PROT_READ | PROT_WRITE,
				MAP_SHARED, mem_fd, phy_addr_in_page);
	if (ddrc_base_addr == MAP_FAILED) {
		printf("ddr statistic mmap failed.\n");
		return -1;
	}

	ddrc_pll_ctrl_map = ddrc_base_addr + (DDRC_PLL_CTRL - DDRC_BASE_ADDR);

	for (i = 0; i < DMC_NUM; i++) {
		if (statistic_args.dmc_sel[i])
			ddrc_addr[i] = ddrc_base_addr + ddrc_addr_offset[i];
		else
			ddrc_addr[i] = NULL;
	}

	return 0;
}

static void ddrc_unmap(void)
{
	munmap(ddrc_base_addr, DDRC_MAP_LENGTH);
	ddrc_base_addr = NULL;
	return;
}

static void print_usage(void)
{
	printf("NAME\n");
	printf("  ddrs - ddr statistic\n\n");
	printf("DESCRIPTION\n");
	printf("  Statistic percentage of occupation of ddr.\n\n");
	printf("  -d, --ddrc\n");
	printf("      Statistics of multiple controllers are supported.\n");
	printf("  -i, --interval\n");
	printf("      the range is 1~3000 milliseconds, 1000 milliseconds as default.\n");
	printf("  -h, --help\n");
	printf("      display this help and exit\n");
	printf("  eg:\n");
	printf("      $ bspddrs -d 0 -i 1000\n");
	printf("      or\n");
	printf("      $ bspddrs\n");
	return;
}

static int check_digit(const char *str)
{
	size_t k, len;

	len = strlen(str);
	for (k = 0; k < len; k++) {
		if (isdigit(str[k]))
			continue;
		return -1;
	}
	return 0;
}

static int opt_match(const char *short_opt, const char *long_opt, const char *str)
{
	if (strcmp(short_opt, str) == 0)
		return 0;
	if (strcmp(long_opt, str) == 0)
		return 0;
	return -1;
}

static int str2int(const char *str, int *ret_val)
{
	if (check_digit(str) != 0)
		return -1;
	*ret_val = atoi(str);
	return 0;
}

static int set_args_dmc_sel(char *argv[], int cnt, struct bspddrs_args *args)
{
	int i;
	if (cnt > DMC_NUM)
		return -1;
	for (i = 0; i < DMC_NUM; i++)
		args->dmc_sel[i] = FALSE; /* reset dmc selection */
	for (i = 0; i < cnt; i++) {
		int val = 0;
		int ret = str2int(argv[i], &val);
		if (ret != 0 || val < 0 || val >= DMC_NUM)
			return -1;
		args->dmc_sel[val] = TRUE;
	}
	return 0;
}

static int parse_args(int argc, char *argv[], struct bspddrs_args *args)
{
	int i = 0;
	int val = 0;
	int ret;
	int cnt = 0;

	if ((argc == 1) && opt_match("-h", "--help", argv[i]) == 0) {
		print_usage();
		return STATUS_PRINT_HELP;
	}

	while (i < argc) {
		if (opt_match("-d", "--ddrc", argv[i]) == 0) {
			/* get the number of -ddrc arguments */
			i++;
			while ((i + cnt) < argc && argv[i + cnt][0] != '-')
				cnt++;
			if (cnt == 0 || set_args_dmc_sel(argv + i, cnt, args) != 0)
				goto error;
			i += cnt;
		} else if (opt_match("-i", "--interval", argv[i]) == 0) {
			if (++i >= argc)
				goto error;
			ret = str2int(argv[i++], &val);
			if (ret != 0 || val <= 0 || val > MAX_INTERVAL)
				goto error;
			args->timer_interval = (unsigned int)val;
		} else {
			goto error;
		}
	}

	return 0;
error:
	print_usage();
	return -1;
}

static int install_signal(struct sigaction *sa, void (*handle)(int), int sig)
{
	sa->sa_flags = SA_RESTART;
	sa->sa_handler = handle;

	sigemptyset(&sa->sa_mask);

	if (sigaction(sig, sa, NULL))
		return -1;
	return 0;
}

static int setup_timer(struct sigevent *timer_event, struct itimerspec *tspec,
					   unsigned int interval, int sig)
{
	timer_event->sigev_notify = SIGEV_SIGNAL;
	timer_event->sigev_signo = sig;
	timer_event->sigev_value.sival_ptr = (void *)&statistic_timer;

	if (timer_create(CLOCK_REALTIME, timer_event, &statistic_timer) < 0) {
		printf("ddr statistic timer create failed.\n");
		return -1;
	}

	/* if interval is less than 1s, the measurement is triggered at the frequency of 1s,
		Otherwise, the measurement is triggered at the preset period. */
	if (interval < TIMER_INTERVAL) {
		tspec->it_value.tv_sec = 1;
		tspec->it_value.tv_nsec = 0;
		tspec->it_interval.tv_sec = 1;
		tspec->it_interval.tv_nsec = 0;
	} else {
		tspec->it_value.tv_sec = interval / TIMER_INTERVAL;
		tspec->it_value.tv_nsec = (interval % TIMER_INTERVAL) * MHZ_HZ_RATIO; // ms to ns
		tspec->it_interval.tv_sec = interval / TIMER_INTERVAL;
		tspec->it_interval.tv_nsec = (interval % TIMER_INTERVAL) * MHZ_HZ_RATIO; // ms to ns
	}

	if (timer_settime(statistic_timer, TIMER_ABSTIME, tspec, NULL) < 0) {
		printf("ddr statistic set timer failed.\n");
		timer_delete(statistic_timer);
		return -1;
	}
	return 0;
}

static void ddr_statistic_start(void)
{
	int i;
	unsigned int perf_prd;
	unsigned int ddrc_freq_hz = statistic_args.ddrc_freq * MHZ_HZ_RATIO;
	perf_prd = ddrc_freq_hz / (PREF_RATIO * TIMER_INTERVAL) * statistic_args.timer_interval;

	for (i = 0; i < DMC_NUM; i++) {
		if (statistic_args.dmc_sel[i]) {
			init_ddr_bandwidth_statistic(i, perf_prd);
			enable_ddr_bandwidth_statistic(i);
		}
	}
}

int bspddrs(int argc, char *argv[])
{
	int ret;
	struct itimerspec tspec;
	struct sigevent timer_event;
	struct sigaction sa_statc;
	struct sigaction sa_ctrl_c;

	statistic_args.dmc_sel[DMC0] = TRUE;
	statistic_args.timer_interval = TIMER_INTERVAL;

	ret = parse_args(argc - 1, &argv[1], &statistic_args);
	if (ret == STATUS_PRINT_HELP)
		return 0;
	else if (ret != 0)
		return -1;

	mem_fd = open(OPEN_FILE, O_RDWR | O_SYNC);
	if (mem_fd == -1) {
		perror("ddr statistic open failed:");
		goto open_err;
	}

	if (ddrc_remap())
		goto ddrc_remap_err;

	get_ddr_frequency();

	if (install_signal(&sa_statc, ddr_statistic, DDR_STAT_SIG)) {
		printf("ddr statistic install signal failed.\n");
		goto start_statistic_err;
	}

	ddr_statistic_start();

	if (setup_timer(&timer_event, &tspec, statistic_args.timer_interval, DDR_STAT_SIG))
		goto start_statistic_err;

	printf("===== ddr statistic =====\n");

	if (install_signal(&sa_ctrl_c, ddr_ctrl_c, SIGINT)) {
		printf("ddr statistic catch SIGINT signal failed.\n");
		goto install_sigint_err;
	}

	while (1)
		sleep(1);
	return 0;

install_sigint_err:
	timer_delete(statistic_timer);
start_statistic_err:
	ddrc_unmap();
ddrc_remap_err:
	close(mem_fd);
	mem_fd = -1;
open_err:
	return -1;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
