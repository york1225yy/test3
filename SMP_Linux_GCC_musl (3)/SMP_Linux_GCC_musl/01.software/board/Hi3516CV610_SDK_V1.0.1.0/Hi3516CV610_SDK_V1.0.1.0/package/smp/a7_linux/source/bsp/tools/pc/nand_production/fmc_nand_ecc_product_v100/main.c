/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <securec.h>
#include "nandc_ecc.h"
#include "oob_config.h"

#define REVERSE_DATA_LENTH         32
#define MAX_ECC_THREADS 64
#define DECIMAL 10

struct parm {
	FILE *infile;
	FILE *outfile;
	struct oob_info *info;
	struct oobuse_info *oobuseinfo;
	char *infilepath;
	char *outfilepath;

	unsigned int infilesize;
	unsigned int pagetype;
	unsigned int ecctype;
	unsigned int yaffs;
	unsigned int random;
	unsigned int pagenum;
	unsigned int savepin;

	unsigned int page_ecc_reverse_sector;
	unsigned int block_reverse_sector;
	unsigned int page0_reverse_flag;
	unsigned int page1_reverse_flag;
	/* oobsize: the actual size of the OOB area of the device
	 * oobuse: the size of the OOB zone actually used by logic
	 * info->oobsize: the oobsize,nandcv610 reserved for software use
	 * in the original image is 32 bytes, and if it is a yaffs image,
	 * it needs to be aligned with the pagesize+info->oobsize.
	 */
	unsigned int oobsize;
	unsigned int oobuse;
	unsigned int pagesize;
	unsigned int pagecount;
	unsigned int page_no_total_block;
	unsigned int page_no_incl_data;
	unsigned int page_no_fill_null;
	unsigned char *wbuf;
	unsigned char *rbuf;

	/* for nandcv610/nandcv620/fmcv100 province pin mode */
	struct reverse_pagesize_ecctype_sector *reverse_page_ecc_sector;
	struct reverse_blocksize_sector *reverse_block_sector;
};

typedef struct parm user_parm;

typedef struct ecc_thread {
	user_parm *data;
	unsigned int page_idx;
	unsigned int page_cnt;
	pthread_t thread;
	int id;
	int retval;
} ecc_thread_t;

static int reverse_buf(unsigned char *buf, unsigned int buflen)
{
	unsigned int i;
	for (i = 0; i < buflen; i++)
		buf[i] = ~buf[i];

	return 0;
}

static void help_info_print(char * const *argv)
{
	printf("Usage:\n%s\tinputfile\toutputfile\t"
	       "pagetype\tecctype\toobsize\tyaffs\trandomizer\tpagenum/block\t"
	       "save_pin:\n"
	       "For example:\t\t |\t\t |\t\t |\t\t |\t |\t |\t |\t\t |\t\t |\n"
	       "%s\tin_2k4b.yaffs\tout_2k4b.yaffs\t"
	       " 0\t\t 1\t 64\t 1\t 0\t\t 64\t\t 0\n"
	       "Page type Page size:\n"
	       "Input file:\n"
	       "Output file:\n"
	       "Pagetype:\n"
	       "0        2KB\n"
	       "1        4KB\n"
	       "ECC type ECC size:\n"
	       "1        4bit/512B\n"
	       "2        16bit/1K\n"
	       "3        24bit/1K\n"
	       "Chip OOB size:\n"
	       "yaffs2 image format:\n"
	       "0	 NO\n"
	       "1	 YES\n"
	       "Randomizer:\n"
	       "0        randomizer_disabled\n"
	       "1        randomizer_enabled\n"
	       "Pages_per_block:\n"
	       "64       64pages/block\n"
	       "128      128pages/block\n"
	       "Save Pin Mode:\n"
	       "0	 disable\n"
	       "1	 enable\n", argv[0], argv[0]);
}

static void get_input_data(user_parm *data, char **argv)
{
	/* argv[n]: input parameter of user */
	data->infilepath = argv[1]; /* source file path */
	data->outfilepath = argv[2]; /* 2: Second command line argument,destination file path */
	data->pagetype = (unsigned int)strtol(argv[3], NULL, DECIMAL); /* 3: page size type */
	data->ecctype = (unsigned int)strtol(argv[4], NULL, DECIMAL); /* 4: ecc level */
	data->oobsize = (unsigned int)strtol(argv[5], NULL, DECIMAL); /* 5: nand controler
							best oob size for chip OOB area */
	data->yaffs = (unsigned int)strtol(argv[6], NULL, DECIMAL); /* 6: yaffs mirror or not */
	data->random = (unsigned int)strtol(argv[7], NULL, DECIMAL); /* 7: user data ramdom to nand or not */
	data->pagenum = (unsigned int)strtol(argv[8], NULL, DECIMAL); /* 8: nand page number of one block */
	data->savepin = (unsigned int)strtol(argv[9], NULL, DECIMAL); /* 9: save pin mode,
										only for nandcv620/fmcv100 */

	printf("pagetype=%x,ecctype=%x,oobsize=%x,yaffs=%x,randomizer=%x,pagenum=%x,savepin=%x,infilepath=%s,outfilepath=%s\n",
	       data->pagetype, data->ecctype, data->oobsize, data->yaffs, data->random, data->pagenum,
	       data->savepin,
	       data->infilepath, data->outfilepath);
}

static int get_sector_number(user_parm *data)
{
	if (data->savepin) {
		data->reverse_page_ecc_sector = get_pagesize_ecctype_reverse_sector(data->pagetype, data->ecctype);
		/* for nandc v610/v620 and fmcv100 province pin mode, info->oobsize = 32 */
		if (data->reverse_page_ecc_sector == NULL) {
			fprintf(stderr, "get reverse sector failed.\n");
			return -EINVAL;
		}
		data->page_ecc_reverse_sector = data->reverse_page_ecc_sector->reverse_sec;
		data->reverse_block_sector = get_blocksize_reverse_sector(data->pagenum);
		/* for nandc v610/v620 and fmcv100 province pin mode, info->oobsize = 32 */
		if (data->reverse_block_sector == NULL) {
			fprintf(stderr, "get block sector failed.\n");
			return -EINVAL;
		}
		data->block_reverse_sector = data->reverse_block_sector->reverse_sec;
	}
	if (data->oobsize < data->info->oobsize) {
		fprintf(stderr, "Chip OOB size too small.\n");
		return -EINVAL;
	}

	data->pagesize = get_pagesize(data->pagetype);
	printf("pagesize=%x\n", data->pagesize);
	return 0;
}

static int data_init(user_parm *data, int argc, char **argv)
{
	int ret = 0;

	if (argc != 10) { /* fixed number of user input parameter is 10 */
		help_info_print(argv);
		return -EINVAL;
	}
	get_input_data(data, argv);

	data->page0_reverse_flag = 0;
	data->page1_reverse_flag = 0;

	data->info = get_oob_info(data->pagetype, data->ecctype);
	/* for nandc v610/v620/ and fmcv100, info->oobsize = 32 */
	if (data->info == NULL) {
		fprintf(stderr, "Not support this parameter page: %x ecc: %x\n",
			data->pagetype, data->ecctype);
		return -EINVAL;
	}
	/* For Nandcv610/nandcv620/fmcv100 province pin mode, get sector numbers that require data to be inverted */
	ret = get_sector_number(data);
	if (ret != 0)
		return ret;

	data->infile = fopen(data->infilepath, "rb");
	if (data->infile == NULL) {
		fprintf(stderr, "Could not open input file %s\n", data->infilepath);
		return errno;
	}
	return ret;
}

static int get_input_file_pagenum(user_parm *data)
{
	int ret;
	ret = fseek(data->infile, 0, SEEK_END);
	if (ret != 0) {
		fprintf(stderr, "seek end fail!");
		return -EINVAL;
	}
	data->infilesize = (unsigned int)ftell(data->infile);
	ret = fseek(data->infile, 0, SEEK_SET);
	if (ret != 0) {
		fprintf(stderr, "seek set fail!");
		return -EINVAL;
	}
	printf("infilesize=0x%x\n", data->infilesize);
	if (data->yaffs) {
		if ((data->infilesize == 0) || ((data->infilesize % (data->pagesize + data->info->oobsize)) != 0)) {
			fprintf(stderr, "Input file length is not page + oob aligned."
				"infilesize=%x, pagesize=%x oobsize=%x\n",
				data->infilesize, data->pagesize, data->info->oobsize);
			return -EINVAL;
		}
		data->pagecount = data->infilesize / (data->pagesize + data->info->oobsize);
	} else {
		data->pagecount = (data->infilesize + data->pagesize - 1) / data->pagesize;
	}
	data->page_no_incl_data = data->pagecount;

	return 0;
}

static int set_oob_data(unsigned char *pagebuf, size_t buflen, user_parm *data, unsigned int page_idx)
{
	int ret;
	size_t sz = data->pagesize;

	memset_s(pagebuf, buflen, 0xFF, buflen);
	if (data->yaffs == 0) {
		size_t ofst = page_idx * data->pagesize;
		size_t end = ofst + data->pagesize;

		if (ofst >= data->infilesize)
			sz = 0;
		else if (ofst + data->pagesize >= data->infilesize)
			sz = data->infilesize - ofst;

		ret = memcpy_s(pagebuf, data->pagesize + data->info->oobsize, data->rbuf + ofst, sz);
		if (ret != EOK) {
			printf("%s: %d:memcpy_s failed\n", __FILE__, __LINE__);
			return 0;
		}
		ret = sz;

		(void)memset_s(pagebuf + data->pagesize, data->info->oobsize, 0xff, data->info->oobsize);
		/* for nandcv610/nandcv620 only, empty page flag. */
		if (data->ecctype != ET_ECC_16BIT1K) {
			*(pagebuf + data->pagesize +
				OOB_LEN_NORMAL - EB_BYTE1_OFFSET) = 0;
			*(pagebuf + data->pagesize +
				OOB_LEN_NORMAL - EB_BYTE0_OFFSET) = 0;
		} else {
			if (data->pagesize == _2K) {
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_2KPAGE - EB_BYTE1_OFFSET) = 0;
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_2KPAGE - EB_BYTE0_OFFSET) = 0;
			} else if (data->pagesize == _4K) {
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_4KPAGE - EB_BYTE1_OFFSET) = 0;
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_4KPAGE - EB_BYTE0_OFFSET) = 0;
			}
		}
	} else {
		sz += data->info->oobsize;
		size_t ofst = page_idx * sz;
		ret = memcpy_s(pagebuf, sz, data->rbuf + ofst, sz);
		if (ret != EOK) {
			printf("%s: %d:memcpy_s failed\n", __FILE__, __LINE__);
			return 0;
		}
		ret = sz;

		/* for nandcv610/nandcv620 only, empty page flag */
		*(pagebuf + data->pagesize + OOB_LEN_NORMAL - EB_BYTE1_OFFSET) = 0;
		*(pagebuf + data->pagesize + OOB_LEN_NORMAL - EB_BYTE0_OFFSET) = 0;
	}

	return ret;
}

static int data_random_op(unsigned char *pagebuf, user_parm *data, unsigned int page_idx)
{
	/* randomzer enable. */
	/* 1. First you need to get the OOB size that the logic actually uses
	* 2. Second, the data is randomizer according to the OOB size actually used by logic.
	*/
	data->oobuseinfo = get_oobuse_info(data->pagetype, data->ecctype);
	if (data->oobsize < data->oobuseinfo->oobuse) {
		fprintf(stderr, "CHIP OOBSIZE too small.\n");
		return -EINVAL;
	}
	if (data->random)
		page_random_gen(pagebuf, data->pagetype, data->ecctype, page_idx % data->pagenum,
				data->oobuseinfo->oobuse);

	return 0;
}

static void save_pin_mode_op(unsigned char *wbuf, user_parm *data)
{
	/* For nandcv610/nandcv620 province pin mode, Take the reverse for pag0
	* (pagesize and ECC combination) and Page1 (blocksize) sector data
	*/
	/* First, reverse the corresponding sector data of the page0. */
	unsigned char *reverse_sector;
	unsigned char *page = wbuf;

	/* page 0 */
	reverse_sector = page + REVERSE_DATA_LENTH * data->page_ecc_reverse_sector;
	reverse_buf(reverse_sector, REVERSE_DATA_LENTH);

	/* page 1 */
	page += (data->pagesize + data->oobsize);
	reverse_sector = page + REVERSE_DATA_LENTH * data->block_reverse_sector;
	reverse_buf(reverse_sector, REVERSE_DATA_LENTH);

	data->savepin = 0;
}

static int fill_null_page(unsigned char *pagebuf, size_t buflen, user_parm *data)
{
	int ret = 0;

	data->page_no_total_block = data->page_no_incl_data & ~(data->pagenum - 1);
	data->page_no_total_block += ((data->page_no_incl_data & (data->pagenum - 1)) != 0) ? data->pagenum : 0;
	data->page_no_fill_null = data->page_no_total_block - data->page_no_incl_data;

	printf("Total block page number:%x\n", data->page_no_total_block);
	printf("Include data page number:%x\n", data->page_no_incl_data);
	printf("Need fill NULL page number:%x\n", data->page_no_fill_null);

	while (data->page_no_fill_null--) {
		size_t len = 0;
		memset_s(pagebuf, buflen, 0xff, buflen);

		if (data->yaffs == 0)
			memset_s(pagebuf + data->pagesize, data->info->oobsize, 0xff, data->info->oobsize);

		if (data->ecctype != ET_ECC_16BIT1K) {
			*(pagebuf + data->pagesize +
				OOB_LEN_NORMAL - EB_BYTE1_OFFSET) = 0;
			*(pagebuf + data->pagesize + OOB_LEN_NORMAL -
				EB_BYTE0_OFFSET) = 0;
		} else {
			if (data->pagesize == _2K) {
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_2KPAGE - EB_BYTE1_OFFSET) = 0;
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_2KPAGE - EB_BYTE0_OFFSET) = 0;
			} else if (data->pagesize == _4K) {
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_4KPAGE - EB_BYTE1_OFFSET) = 0;
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_4KPAGE - EB_BYTE0_OFFSET) = 0;
			}
		}

		ret = page_ecc_gen(pagebuf, data->pagetype, data->ecctype);
		if (ret < 0) {
			fprintf(stderr, "page_ecc_gen error.\n");
			return ret;
		}

		len = fwrite(pagebuf, 1, (size_t)(data->pagesize + data->oobsize), data->outfile);
		if (len != (size_t)(data->pagesize + data->oobsize)) {
			fprintf(stderr, "Could not fill NULL page in output file %s\n", data->outfilepath);
			ret = errno;
			return ret;
		}
	}

	return ret;
}

static void *__thread_fill_ecc_oob(void *arg)
{
	ecc_thread_t *thread = (ecc_thread_t *)arg;
	user_parm *data = thread->data;
	unsigned int index = thread->page_idx;

	thread->retval = -1;

	while (index < thread->page_idx + thread->page_cnt) {
		unsigned char *pagebuf;
		size_t ofst;
		int ret;

		ofst = (data->pagesize + data->oobsize) * index;
		pagebuf = data->wbuf + ofst;

		ret = set_oob_data(pagebuf, data->pagesize + data->oobsize, data, index);
		if (ret == 0)
			break;

		if (ret < 0) {
			fprintf(stderr, "Could not read input file %s\n", data->infilepath);
			ret = errno;
			return NULL;
		}

		ret = page_ecc_gen(pagebuf, data->pagetype, data->ecctype);
		if (ret < 0) {
			fprintf(stderr, "page_ecc_gen error.\n");
			return NULL;
		}
		ret = data_random_op(pagebuf, data, index);
		if (ret)
			return NULL;

		index++;
	}

	thread->retval = 0;

	return NULL;
}

static int __fill_ecc_oob_to_page(unsigned char *pagebuf, int buflen, user_parm *data, size_t pg_cnt)
{
	int cpu_nums = get_nprocs();
	int thd_nums = cpu_nums > MAX_ECC_THREADS ? MAX_ECC_THREADS : cpu_nums;
	unsigned int cnt_pre_thd = pg_cnt / thd_nums;
	ecc_thread_t *threads;
	int ret = 0;
	int i;

	data->wbuf = (unsigned char *)malloc((data->pagesize + data->oobsize) * pg_cnt);
	if (data->wbuf == NULL)
		return -1;

	threads = (ecc_thread_t*)malloc(thd_nums * sizeof(ecc_thread_t));
	if (threads == NULL) {
		free(data->wbuf);
		return -1;
	}

	for (i = 0; i < thd_nums; i++) {
		threads[i].data = data;
		threads[i].page_idx = i * cnt_pre_thd;
		if (i != thd_nums - 1)
			threads[i].page_cnt = cnt_pre_thd;
		else
			threads[i].page_cnt = pg_cnt - threads[i].page_idx;
		threads[i].id = i;
		ret = pthread_create(&(threads[i].thread), NULL, __thread_fill_ecc_oob, &(threads[i]));
		if (ret == 0)
			continue;
		printf("Failed to creat pthread!\n");
		goto out;
	}

	for (i = 0; i < thd_nums; i++)
		pthread_join(threads[i].thread, NULL);

	for (i = 0; i < thd_nums; i++) {
		ret = threads[i].retval;
		if (ret != 0)
			goto out;
	}

	if (data->savepin != 0)
		save_pin_mode_op(data->wbuf, data);

	ret = fwrite(data->wbuf, 1, (data->pagesize + data->oobsize) * pg_cnt, data->outfile);
	if (ret != (data->pagesize + data->oobsize) * pg_cnt) {
		fprintf(stderr, "Could not write output file %s\n", data->outfilepath);
		ret = errno;
	} else {
		ret = 0;
	}
out:
	free(threads);
	free(data->wbuf);
	return ret;
}

# define PAGE_CNT_ONCE	0x4000

static int fill_ecc_oob_to_page(user_parm *data)
{
	size_t pg_len = data->yaffs ? (data->pagesize + data->info->oobsize) : data->pagesize;
	size_t data_size = PAGE_CNT_ONCE * pg_len;
	size_t loop = (data->infilesize + data_size - 1) / data_size;
	unsigned char pagebuf[MAX_PAGE_SIZE + MAX_OOB_SIZE];
	int ret;
	int i;

	printf("pagecount=%x\n", data->pagecount);
	printf("outfilepath=%s\n", data->outfilepath);

	data->rbuf = malloc(data_size);
	if (data->rbuf == NULL)
		return -1;

	for (i = 0; i < loop; i++) {
		size_t r_remain = data->infilesize - (i * data_size);
		size_t r_size = r_remain < data_size ? r_remain : data_size;
		size_t pg_cnt = (r_size + pg_len - 1) / pg_len;

		size_t read_len = fread(data->rbuf, 1, r_size, data->infile);
		if (read_len != r_size) {
			ret = -1;
			goto exit;
		}

		ret = __fill_ecc_oob_to_page(pagebuf, (MAX_PAGE_SIZE + MAX_OOB_SIZE), data, pg_cnt);
		if (ret)
			goto exit;
	}
exit:
	free(data->rbuf);
	return ret;
}

int main(int argc, char **argv)
{
	unsigned char *pagebuf = NULL;
	int ret;
	user_parm *data = NULL;

	data = malloc(sizeof(user_parm));
	if (data == NULL)
		return -ENOMEM;

	pagebuf = (unsigned char *)malloc(sizeof(unsigned char) * (MAX_PAGE_SIZE + MAX_OOB_SIZE));
	if (pagebuf == NULL) {
		ret = -ENOMEM;
		goto fail_free_data;
	}

	memset_s(data, sizeof(user_parm), 0, sizeof(user_parm));

	ret = data_init(data, argc, argv);
	if (ret)
		goto fail_close_file;

	ret = get_input_file_pagenum(data);
	if (ret)
		goto fail_close_file;

	data->outfile = fopen(data->outfilepath, "wb");
	if (data->outfile == NULL) {
		fprintf(stderr, "Could not open output file %s\n", data->outfilepath);
		ret = errno;
		goto fail_close_file;
	}

	ret = fill_ecc_oob_to_page(data);
	if (ret)
		goto fail_close_file;

	ret = fill_null_page(pagebuf, (MAX_PAGE_SIZE + MAX_OOB_SIZE), data);
	if (ret)
		goto fail_close_file;

	ret = 0;
	if ((data->savepin != 0) && ((data->page0_reverse_flag == 0) || (data->page1_reverse_flag == 0))) {
		printf("savepin mode: reverse data failed.\n");
		ret = -1;
	}

fail_close_file:
	if (data->infile)
		fclose(data->infile);
	if (data->outfile)
		fclose(data->outfile);
	free(pagebuf);
fail_free_data:
	free(data);
	return ret;
}
