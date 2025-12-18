1. 编译方法
    默认使用mbedtls的原生软件方案，请执行 make -j 或者 make all -j;
    如果用户希望使用硬化MbedTLS方案，请按照以下步骤执行：
        1) 在当前目录下执行make harden。
        2) 进入mbedtls-3.1.0/harden/build/vision/out(或mbedtls-3.6.0/harden/build/vision/out)目录，
        MbedTLS硬化方案编译出的动态库和静态库就生成在此目录下。
        若纯软或硬化组件完成编译后需使用另一替代方案，请先在当前目录下执行 make clean 清除中间生成的二进制缓存。

2. 功能设置
    1) 用户可以在`mbedtls-3.1.0/harden/platform/vision/mbedtls_platform_user_config.h` 中手动配置不希望打开的硬化功能，
    默认支持的功能都会打开。

3. 使用方法
    1)若使用out目录下编译得到的硬化库，则需要加载ot_mpi_cipher.ko、ot_mpi_km.ko、ot_mpi_otp.ko，
    且需要链接cipher、km、otp三个模块的动态库或静态库。

    2)Hi3516CV610要使用mbedtls硬化功能，需要在security_subsys目录下的的crypto_features.h中开启宏CONFIG_SYMC_HARDEN_INTERFACE_SUPPORT
    和CONFIG_HASH_HARDEN_INTERFACE_SUPPORT。由于会调用ecb模式的加解密接口，还需要打开宏CONFIG_SYMC_MODE_ECB_SUPPORT。
    之后重新编译cipher和km模块的库文件，并重新编译和加载cipher和km模块的ko。

    3)此外，需要用户额外链接mbedtls_harden_adapt动态库或静态库，并引入mbedtls_harden_adapt.h头文件，在调用调用MbedTLS硬化接口前，
    先调用`int mbedtls_adapt_register_func(void)`函数，完成cipher的硬化函数注册，使用后需要调用void mbedtls_adapt_unregister_func(void)。

4. 注意事项
    1) Hi3519DV500使用mbedtls版本为mbedtls-3.1.0，Hi3516CV610使用mbedtls版本为mbedtls-3.6.0

    2) 由于SDK包无法重编hi层的库文件，重编后需要额外链接ot层的cipher和km模块库文件。

    3) 对于ECC密钥生成和ECDSA签名函数，如果用户传了随机数函数f_rng，接口将调用mbedtls原生的软件算法，
    如果传的是NULL (mbedTLS原生接口不允许传NULL，但MbedTLS硬化方案允许)，将使用硬化加速算法。
    这样设计的原因是：MbedTLS在自己的测试函数中传入了伪随机函数进行结果比对，如果使用硬件自身的随机数函数将无法获得正确的结果。
    这两个函数的声明如下：

    int mbedtls_ecp_gen_keypair( mbedtls_ecp_group *grp,mbedtls_mpi *d, mbedtls_ecp_point *Q, int (*f_rng)(void *,
        unsigned char *, size_t),void *p_rng );

    int mbedtls_ecdsa_sign( mbedtls_ecp_group *grp, mbedtls_mpi *r, mbedtls_mpi *s, const mbedtls_mpi *d,
        const unsigned char *buf, size_t blen, int (*f_rng)(void *, unsigned char *, size_t), void *p_rng );

    4) 对Hi3519DV500，AES算法、AES算法GCM模式、AES算法CCM模式，要求用户的传入数据buffer地址为使用mmz接口申请得到的虚拟地址。

    5) 对Hi3519DV500，AES算法、CMAC算法和大数运算，因性能原因默认不开放，如需使用请至harden/platform/vision/mbedtls_platform_user_config.h文件中
    打开相关宏：MBEDTLS_AES_ALT、MBEDTLS_CMAC_ALT、MBEDTLS_BIGNUM_EXP_MOD_USE_HARDWARE、MBEDTLS_BIGNUM_MOD_USE_HARDWARE。

    6) 对Hi3516CV610, AES算法、AES算法GCM模式、AES算法CCM模式和CMAC算法要使用硬件加速，需要在
    harden/platform/vision/mbedtls_platform_user_config.h文件中打开宏：MBEDTLS_AES_ALT。
    
    7) 对Hi3516CV610，开启宏CONFIG_HASH_HARDEN_INTERFACE_SUPPORT后，mbedtls硬化方案会占用一个物理通道，
    此时调用hash算法不支持占用两个及以上的长期通道，如hmac clone。

    8) Hi3516CV610硬件不支持hash算法SHA-384、SHA-512，支持SHA-224，SHA-256, 不支持曲线RFC 8032、RFC 7748 Curve25519。

    9) 对于摘要算法，建议在数据量大于10K的情况下使用硬化方案。

5. 硬化算法与接口清单
    5.1 AES算法
    硬化支持的模式包括CBC、OFB、CFB、CTR模式，支持的密钥长度为 128bits 和 256bits，涉及的接口包括：
    1) mbedtls_aes_init
    2) mbedtls_aes_deinit
    3) mbedtls_aes_setkey_enc
    4) mbedtls_aes_setkey_dec
    5) mbedtls_aes_crypt_cbc
    6) mbedtls_aes_crypt_cfb128
    7) mbedtls_aes_crypt_cfb8
    8) mbedtls_aes_crypt_ofb
    9) mbedtls_aes_crypt_ctr

    5.2 摘要算法
    硬化支持的摘要算法包括SHA-256、SHA-384、SHA-512，涉及的接口包括：
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

    5.3 认证算法
    硬化支持的认证算法包括CMAC、HMAC-SHA-256、HMAC-SHA-384、HMAC-SHA-512，涉及的接口包括：
    1) mbedtls_md_setup
    2) mbedtls_md_hmac_starts
    3) mbedtls_md_hmac_update
    4) mbedtls_md_hmac_finish
    5) mbedtls_md_hmac_reset
    6) mbedtls_md_hmac
    7) mbedtls_cipher_cmac_starts
    8) mbedtls_cipher_cmac_update
    9) mbedtls_cipher_cmac_finish

    5.4 密钥派生算法
    硬化支持的密钥派生算法包括PBKDF2-SHA256、PBKDF2-SHA256、PBKDF2-SHA512，涉及的接口包括：
    1) mbedtls_pkcs5_pbkdf2_hmac

    5.5 大数运算
    硬化支持的大数运算接口包括取模运算和模幂运算，涉及的接口包括：
    1) mbedtls_mpi_mod_mpi
    2) mbedtls_mpi_exp_mod

    5.6 椭圆曲线算法
    硬化支持使用椭圆曲线算法进行签名、验签、密钥生成、密钥协商，支持的曲线包括RFC 5639 BrainpoolP256/384/512、
    RFC 8032、RFC 7748 Curve25519，涉及的接口包括：
    1) mbedtls_ecdh_compute_shared
    2) mbedtls_ecdsa_sign
    3) mbedtls_ecdsa_verify
    4) mbedtls_ecp_gen_keypair_base

    5.7 硬件随机数源
    硬化支持使用硬件真随机数作为随机数源，，涉及的接口包括：
    1) mbedtls_hmac_drbg_seed
    2) mbedtls_hmac_drbg_reseed
    3) mbedtls_hmac_drbg_random_with_add
    4) mbedtls_entropy_add_source
    5) mbedtls_ctr_drbg_seed
    6) mbedtls_ctr_drbg_random