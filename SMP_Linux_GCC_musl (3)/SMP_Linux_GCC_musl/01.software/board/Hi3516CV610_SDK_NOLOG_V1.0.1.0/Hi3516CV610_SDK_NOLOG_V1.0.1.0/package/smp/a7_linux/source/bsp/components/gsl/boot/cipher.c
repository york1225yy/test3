/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include "types.h"
#include "platform.h"
#include "lib.h"
#include "../drivers/otp/otp.h"
#include "share_drivers.h"
#include "flash_map.h"
#include "common.h"
#include "securecutil.h"
#include "err_print.h"
#include "soc_errno.h"
#include "soc_drv_common.h"
#include "checkup.h"

td_u8 hash_verify_buf[SHA_256_LEN];

#define KEYSALT_LEN 28
#define KEY_MATERIAL 16
#define SYM_KEY_TYPE_OFF 24
#define IV_LEN 16

int decrypt_data(u32 type, para_enc_info *enc_info, u8 *code_dest, u8 *code_src,
		 u32 code_len, u32 sym_key_type)
{
	volatile td_s32 ret = EXT_SEC_FAILURE;
	unsigned int keyslot_handle;

	ext_km_rootkey_type rootkey_type = 0;

	unsigned char keysalt[KEYSALT_LEN];

	ret = memset_ss(keysalt, KEYSALT_LEN, 0, KEYSALT_LEN, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		return EXT_SEC_FAILURE;
	}

	ret = memcpy_ss(keysalt, KEY_MATERIAL, enc_info->key_material, KEY_MATERIAL, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		return EXT_SEC_FAILURE;
	}

	*(td_u32 *)(keysalt + SYM_KEY_TYPE_OFF) = sym_key_type;

	if (type == REE) {
		rootkey_type = EXT_KM_ROOTKEY_TYPE_ERK_REE;
	} else if (type == TEE) {
		rootkey_type = EXT_KM_ROOTKEY_TYPE_ERK_TEE;
	} else {
		return EXT_SEC_FAILURE;
	}

	// _sal_len
	ret = ext_drv_cipher_create_keyslot_by_set_effective_key(
		&keyslot_handle, rootkey_type, keysalt, KEYSALT_LEN);
	if (ret != EXT_SEC_SUCCESS) {
		return EXT_SEC_FAILURE;
	}

	ret = ext_drv_cipher_symc_decrypt(EXT_SYMC_ALG_AES, keyslot_handle,
					  enc_info->iv, IV_LEN, (uint64_t)(uint32_t)code_src,
					  (uint64_t)(uint32_t)code_dest, code_len);
	if (ret != EXT_SEC_SUCCESS) {
		return EXT_SEC_FAILURE;
	}
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"decrypt_data ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

td_s32 calc_hash(td_u32 src_addr, td_u32 src_len, td_u8 *data_sha,
		 td_u32 data_sha_len, td_u32 check_word)
{
	volatile td_s32 ret = EXT_SEC_FAILURE;
	ret = ext_drv_cipher_sha256((td_u8 *)(uintptr_t)src_addr, src_len,
				    data_sha, data_sha_len);
	if (ret != EXT_SEC_SUCCESS) {
		return EXT_SEC_FAILURE;
	}
	return ret;
}

static td_s32 store_to_hash_buf(td_u8 *hash_verify_buf, td_u8 *data_hash)
{
	td_s32 ret = EXT_SEC_FAILURE;

	ret = memcpy_ss(hash_verify_buf, SHA_256_LEN, data_hash, SHA_256_LEN,
			NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		return EXT_SEC_FAILURE;
	}

	return EXT_SEC_SUCCESS;
}

td_s32 verify_signature(const ext_data *data,
			const ext_pke_rsa_pub_key *pub_key,
			const ext_pke_data *sign, td_u32 check_word)
{
	td_s32 ret = EXT_SEC_FAILURE;
	td_u8 data_hash[SHA_256_LEN];

	ext_pke_data input_hash;
	/* Initialise hash arrays and structs */
	ret = memset_s(data_hash, SHA_256_LEN, 0x5a, SHA_256_LEN);
	if (ret != EOK) {
		return ret;
	}
	(void)calc_hash((uintptr_t)data->data, data->length, data_hash,
			SHA_256_LEN, 0);
	ret = store_to_hash_buf(hash_verify_buf, data_hash);
	if (ret != EXT_SEC_SUCCESS) {
		return EXT_SEC_FAILURE;
	}

	input_hash.data = hash_verify_buf;
	input_hash.length = SHA_256_LEN;

	ret = EXT_SEC_FAILURE;

	ret = ext_drv_cipher_pke_rsa_verify(pub_key,
					    EXT_PKE_RSA_SCHEME_PKCS1_V21,
					    EXT_PKE_HASH_TYPE_SHA256,
					    &input_hash, sign);
	return ret;
}


void lock_ree_bootkey(void)
{
	reg_set(RKP_LOCK, RKP_REE_LOCK);
	reg_set(RKP_ONEWAY, LOCK_REE_KEY);
	reg_set(RKP_LOCK, RKP_UNLOCK);
}


void lock_tee_bootkey(void)
{
	reg_set(RKP_LOCK, RKP_TEE_LOCK);
	reg_set(RKP_ONEWAY, LOCK_TEE_REE_KEY);
	reg_set(RKP_LOCK, RKP_UNLOCK);
}


int dma_copy(uintptr_t dest, u32 count, uintptr_t src)
{
	return ext_drv_cipher_dma_copy(src, dest, count);
}
