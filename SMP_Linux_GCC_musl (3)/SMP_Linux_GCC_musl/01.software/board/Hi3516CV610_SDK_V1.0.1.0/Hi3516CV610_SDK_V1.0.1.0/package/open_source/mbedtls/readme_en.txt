1. Compilation
    By default, the mbedtls native software solution is used. Run the make -j or make all -j command.
    If you want to use the hardened MbedTLS scheme, perform the following steps:
    1) Run the make command in the current directory.
    2) Go to the mbedtls-3.1.0/harden/build/vision/out directory. (or mbedtls-3.6.0/harden/build/vision/out)
    The dynamic libraries and static libraries compiled by the MbedTLS solution are generated in this directory.
    If another alternative solution is required after software or hardware components are compiled, run the make clean
    command in the current directory to clear the generated binary cache.


2. Function settings
    You can manually configure hardened features in `mbedtls-3.1.0/harden/platform/vision/mbedtls_Platform_user_config.h`
    that you do not want to turn on. By default, all supported functions are enabled.

3. How to use

    1) Before using the hardened library, ot_mpi_cipher.ko, ot_mpi_km.ko, and ot_mpi_otp.ko need to be loaded, and the
    dynamic or static libraries of the cipher, km, and otp modules need to be linked.

    2) To use the mbedtls harden function for Hi3516CV610, enable the macro CONFIG_SYMC_HARDEN_INTERFACE_SUPPORT 
    and CONFIG_HASH_HARDEN_INTERFACE_SUPPORT in crypto_features.h in security_subsys. The macro CONFIG_SYMC_MODE_ECB_SUPPORT 
    needs to be enabled because the encryption and decryption APIs in ECB mode are called. Recompile the library files 
    of the cipher and KM modules, and recompile and load the .ko files of the cipher and KM modules.

    3) In addition, you need to link the mbedtls_harden_adapt dynamic library or static library, and the mbedtls_harden_adapt.h 
    header file should be included. Before calling the MbedTLS hardened interface, the `int mbedtls_adapt_register_func(void)` 
    function should be called to make sure the cipher function is registered. And call 'void mbedtls_adapt_unregister_func(void).'
    after using.

4. Precautions
    1) For Hi3519DV500, the mbedtls version is mbedtls-3.1.0. For Hi3516CV610, the mbedtls version is mbedtls-3.6.0.

    2) The library files of the hi layer cannot be recompiled in the SDK package. After the recompilation, 
    the cipher and KM module library files of the ot layer need to be linked.

    3) For the ECC key generation function and ECDSA signature function, if the user transfers the random number function
    f_rng, the interface invokes the native software algorithm of mbedtls, If it's empty (The mbedTLS native interface
    does not allow empty transfer, but the MbedTLS harden solution allows this transfer.) , the hardened acceleration
    algorithm will be used. The reason for this design is that MbedTLS transfers a pseudo-random function to its test
    function for result comparison. If the random number function of the hardware is used, the correct result cannot be
    obtained. The two functions are declared as follows:

    int mbedtls_ecp_gen_key (mbedtls_ecp_group *grp,mbedtls_mpi *d, mbedtls_ecp_point *Q, int (*f_rng)(invalid *,
    unsigned character *, size_t), invalid *p_rng);

    int mbedtls_ecdsa_sign(mbedtls_ecp_group *grp, mbedtls_mpi *r, mbedtls_mpi *s, const mbedtls_mpi *d,
    constant unsigned character *Buf, size_t blen,int(*f_rng) (empty *, unsigned character *, size _t) , empty *p_rng);

    4) For Hi3519DV500, the hardened MbedTLS library of the AES algorithm, AES algorithm GCM mode, and AES algorithm CCM mode,
    the address of the incoming data buffer must be the virtual address obtained through the MMZ interface.

    5) For Hi3519DV500, the AES algorithm, CMAC algorithm, and large number calculation are disabled by default due to 
    performance reasons. If you need to use the AES algorithm, CMAC algorithm, and large number calculation, see the
    harden/platform/vision/mbedtls_platform_user_config.h file. Enable related macros: MBEDTLS_AES_USE_HARDWARE,
    MBEDTLS_AES_CMAC_USE_HARDWARE, MBEDTLS_BIGNUM_MOD_USE_HARDWARE, MBEDTLS_BIGNUM_EXP_MOD_USE_HARDWARE.

    6) For Hi3516CV610, If you need to use the AES algorithm, AES algorithm GCM mode, AES algorithm CCM mode
    and CMAC algorithm, see the harden/platform/vision/mbedtls_platform_user_config.h file. Enable related macros:
    MBEDTLS_AES_ALT.

    7) For Hi3516CV610, after the macro CONFIG_HASH_HARDEN_INTERFACE_SUPPORT is enabled, the mbedtls hardening solution 
    occupies a physical channel. In this case, two or more long-term channels cannot be occupied when the hash algorithm 
    is invoked, for example, hmac clone.

    8) The Hi3516CV610 hardware does not support the hash algorithms SHA-384 and SHA-512, but supports SHA-224, SHA-256
    and does not support the curve RFC 8032 and RFC 7748 Curve25519.

    9) For the digest algorithm, you are advised to use the hardening solution when the data volume is greater than 10 KB.

5. Hardening algorithm and interface list
    5.1 AES Algorithm
    The supported modes include CBC, OFB, CFB, and CTR. The supported key lengths are 128 bits and 256 bits. The involved
    APIs are as follows:
    1) mbedtls_aes_init
    2) mbedtls_aes_deinit
    3) mbedtls_aes_setkey_enc
    4) mbedtls_aes_setkey_dec
    5) mbedtls_aes_crypt_cbc
    6) mbedtls_aes_crypt_cfb128
    7) mbedtls_aes_crypt_cfb8
    8) mbedtls_aes_crypt_ofb
    9) mbedtls_aes_crypt_ctr
    
    5.2 Digest Algorithm
    The following digest algorithms are supported: SHA-256, SHA-384, and SHA-512. The involved APIs are as follows:
    1) mbedtls_sha256_init
    2) mbedtls_sha256_free
    3) mbedtls_sha256_clone
    4) mbedtls_sha256_starts
    5) mbedtls_sha256_update
    6) mbedtls_sha256_finish
    7) mbedtls_sha256
    1) mbedtls_sha512_init
    2) mbedtls_sha512_free
    3) mbedtls_sha512_clone
    4) mbedtls_sha512_starts
    5) mbedtls_sha512_update
    6) mbedtls_sha512_finish
    7) mbedtls_sha512
    
    5.3 Authentication Algorithm
    The following authentication algorithms are supported: CMAC, HMAC-SHA-256, HMAC-SHA-384, and HMAC-SHA-512. The APIs
    involved are as follows:
    1) mbedtls_md_setup
    2) mbedtls_md_hmac_starts
    3) mbedtls_md_hmac_update
    4) mbedtls_md_hmac_finish
    5) mbedtls_md_hmac_reset
    6) mbedtls_md_hmac
    7) mbedtls_cipher_cmac_starts
    8) mbedtls_cipher_cmac_update
    9) mbedtls_cipher_cmac_finish
    
    5.4 Key Derivation Algorithms
    The following key derivation algorithms are supported: PBKDF2-SHA256, PBKDF2-SHA256, and PBKDF2-SHA512. The APIs
    involved are as follows:
    1) mbedtls_pkcs5_pbkdf2_hmac
    
    5.5 Large number operation
    The following interfaces are supported for large number operations: modulo operation and modular exponentiation
    operation. The APIs involved are as follows:
    1) mbedtls_mpi_mod_mpi
    2) mbedtls_mpi_exp_mod
    
    5.6 Elliptic Curve Algorithm
    Signature, signature verification, key generation, and key negotiation are supported using the elliptic curve
    algorithm. The supported curves include RFC 5639 BrainpoolP256/384/512, RFC 8032 and RFC 7748 Curve25519. The
    following APIs are involved:
    1) mbedtls_ecdh_compute_shared
    2) mbedtls_ecdsa_sign
    3) mbedtls_ecdsa_verify
    4) mbedtls_ecp_gen_keypair_base
    
    5.7 Hardware Random Number Source
    Hardware true random numbers can be used as random number sources. The following APIs are involved:
    1) mbedtls_hmac_drbg_seed
    2) mbedtls_hmac_drbg_reseed
    3) mbedtls_hmac_drbg_random_with_add
    4) mbedtls_entropy_add_source
    5) mbedtls_ctr_drbg_seed
    6) mbedtls_ctr_drbg_random