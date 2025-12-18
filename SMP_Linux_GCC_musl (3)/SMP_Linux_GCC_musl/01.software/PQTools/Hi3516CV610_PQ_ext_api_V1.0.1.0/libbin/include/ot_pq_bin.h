/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description:
 * Author: pqtool team
 * Create: 2024/3/15
 */

#ifndef __OT_PQ_BIN_H__
#define __OT_PQ_BIN_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define OT_BIN_NULL_POINT 0xCB000001
#define OT_BIN_REG_ATTR_ERR 0xCB000002
#define OT_BIN_MALLOC_ERR 0xCB000003
#define OT_BIN_CHIP_ERR 0xCB000004
#define OT_BIN_CRC_ERR 0xCB000005
#define OT_BIN_SIZE_ERR 0xCB000006
#define OT_BIN_LEBLE_ERR 0xCB000007
#define OT_BIN_DATA_ERR 0xCB000008
#define OT_BIN_GET_MPI_FAILED 0xCB000101
#define OT_BIN_SET_MPI_FAILED 0xCB000102
#define OT_BIN_SECURITY_SOLUTION_FAILED 0xCB00000A

typedef struct OtPqBinIspS {
    int enable;
} PQ_BIN_ISP_S;

typedef struct OtPqBinNrxS {
    int enable;
    int viPipe;
    int vpssGrp;                        /* not support for Hi3516CV610 */
} PQ_BIN_NRX_S;

typedef struct OtPqBinIspEvoS {
    int enable;
    int viPipe;
} PQ_BIN_ISP_EVO_S;                     /* not support for Hi3516CV610 */

typedef struct OtPqBinModuleS {
    PQ_BIN_ISP_S stISP;
    PQ_BIN_NRX_S st3DNR;
    PQ_BIN_ISP_EVO_S stIspEvo;          /* not support for Hi3516CV610 */
} PQ_BIN_MODULE_S;

/*****************************************************************************
 *   Prototype    : OT_PQ_GetISPDataTotalLen
 *   Description: : get length of isp module data
 *   Input        : void
 *   Output       : void
 *   Return Value : the length of isp module data
 *****************************************************************************/
unsigned int OT_PQ_GetISPDataTotalLen(void);

/*****************************************************************************
 *   Prototype    : OT_PQ_GetStructParamLen
 *   Description: : get length of isp module data
 *   Input        : binParam: set isp and nrx params
 *   Output       : void
 *   Return Value : the length of nrx params
 *****************************************************************************/
unsigned int OT_PQ_GetStructParamLen(PQ_BIN_MODULE_S *binParam);

/*****************************************************************************
 *   Prototype    : OT_PQ_BIN_ExportBinData
 *   Description: : get bin data from binBuf
 *   Input        : binParam:set isp and nrx params
 *                  binBuf:save bin data
 *                  binBufLen:length of bin data
 *   Output       : void
 *   Return Value : 0: Success;
 *                  0xCB000001:input para is null
 *                  0xCB000003:malloc error
 *                  0xCB000004:chipid not match
 *                  0xCB000005:crc error
 *                  0xCB000008:data error
 *                  0xCB00000A:security solution failed
 *****************************************************************************/
int OT_PQ_BIN_ExportBinData(PQ_BIN_MODULE_S *binParam, unsigned char *binBuf, unsigned int binBufLen);

/*****************************************************************************
 *   Prototype    : OT_PQ_BIN_ImportBinData
 *   Description: : set bin data from binBuf
 *   Input        : binParam:set isp and nrx params
 *                  binBuf:save bin data
 *                  binBufLen:length of bin data
 *   Output       : void
 *   Return Value : 0: Success;
 *                  0xCB000001:input para is null
 *                  0xCB000003:malloc error
 *                  0xCB000008:data erro
 *                  0xCB00000A:security solution failed
 *****************************************************************************/
int OT_PQ_BIN_ImportBinData(PQ_BIN_MODULE_S *binParam, unsigned char *binBuf, unsigned int binBufLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* End of #ifndef __OT_PQ_BIN_H__ */
