/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description:
 * Author: pqtool team
 * Create: 2023/6/1
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <limits.h>

#include "ot_pq_bin.h"
#include "ss_mpi_isp.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static int sample_bin_export(PQ_BIN_MODULE_S *binParam)
{
    unsigned int tempLen;
    unsigned int totalLen, ispDataLen, nrxDataLen;
    unsigned char *binBuffer = NULL;
    int ret;

    ispDataLen = OT_PQ_GetISPDataTotalLen();
    nrxDataLen = OT_PQ_GetStructParamLen(binParam);
    totalLen = nrxDataLen + ispDataLen;
    binBuffer = (unsigned char *)malloc(totalLen);
    if (binBuffer == NULL) {
        printf("malloc error!\n");
        ret = -1;
        goto EXIT;
    }
    memset_s(binBuffer, totalLen, 0, totalLen);

    FILE *fp = fopen("./sample.bin", "wb+");
    if (fp == NULL) {
        ret = -1;
        goto EXIT;
    }
    ret = OT_PQ_BIN_ExportBinData(binParam, binBuffer, totalLen);
    if (ret != 0) {
        printf("OT_PQ_BIN_ExportBinData error(%#x)!\n", ret);
        goto EXIT;
    }
    tempLen = fwrite(binBuffer, 1, totalLen, fp);
    if (tempLen != totalLen) {
        printf("write file error!\n");
        ret = -1;
        goto EXIT;
    }
    printf("OT_PQ_BIN_ParseBinData success!\n");

EXIT:
    if (binBuffer) {
        free(binBuffer);
        binBuffer = NULL;
    }
    if (fp != NULL) {
        fclose(fp);
    }

    return ret;
}

static int sample_bin_import(PQ_BIN_MODULE_S *binParam)
{
    unsigned int tempLen;
    unsigned int size;
    unsigned char *binBuffer = NULL;
    int ret;
    FILE *fp = fopen("./sample.bin", "r");
    if (fp == NULL) {
        printf("fopen error.\n");
        ret = -1;
        goto EXIT;
    }
    ret = fseek(fp, 0, SEEK_END);
    if (ret == -1) {
        printf("fseek end fail \n");
        goto EXIT;
    }
    size = ftell(fp);
    ret = fseek(fp, 0, SEEK_SET);
    if (ret == -1) {
        printf("fseek set fail \n");
        goto EXIT;
    }

    binBuffer = (unsigned char *)malloc(size);
    if (binBuffer == NULL) {
        printf("malloc erro\n");
        ret = -1;
        goto EXIT;
    }
    tempLen = fread(binBuffer, sizeof(unsigned char), size, fp);
    if (tempLen == 0) {
        printf("read erro\n");
        ret = -1;
        goto EXIT;
    }
    ret = OT_PQ_BIN_ImportBinData(binParam, binBuffer, size);
    if (ret != 0) {
        printf("OT_PQ_BIN_ImportBinData error(%#x)\n", ret);
    } else {
        printf("OT_PQ_BIN_ParseBinData success!\n");
    }

EXIT:
    if (binBuffer != NULL) {
        free(binBuffer);
        binBuffer = NULL;
    }
    if (fp != NULL) {
        fclose(fp);
    }

    return ret;
}

static const int ARGNUM = 2;
#if (defined(__LITEOS__))
int pq_libbin_main(int argc, char **argv)
#else
int main(int argc, char *argv[])
#endif
{
    int ret;
    if (argc < ARGNUM) {
        printf("\nPlease select the index.  \n");
        printf("\t 0)OT_PQ_BIN_ExportBinData\n");
        printf("\t 1)OT_PQ_BIN_ImportBinData\n");
        return 0;
    }

    PQ_BIN_MODULE_S binParam;
    memset_s(&binParam, sizeof(binParam), 0, sizeof(binParam));
    binParam.stISP.enable = 1;
    binParam.st3DNR.enable = 1;
    binParam.st3DNR.viPipe = 0;

    char *endPtr = TD_NULL;
    unsigned int tmp =
        (unsigned int) strtol(argv[1], &endPtr, 10); /* base 10, argv[1] has been check between [0, 1] */
    if ((endPtr == argv[1]) || (*endPtr) != '\0') {
        printf("data error \n");
        return -1;
    }
    switch (tmp) {
        case 0:
            ret = sample_bin_export(&binParam);
            break;
        case 1:
            ret = sample_bin_import(&binParam);
            break;
        default:
            printf("command err,exit sample!\n");
            ret = -1;
            break;
    }

    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
