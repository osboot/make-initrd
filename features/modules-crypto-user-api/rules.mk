CRYPTO_USER_API_PATTERN_SET =
CRYPTO_USER_API_PATTERN_SET += depends:^(.*,)?af_alg(,.*)?$

MODULES_PATTERN_SETS += CRYPTO_USER_API_PATTERN_SET

$(call require,depmod-image)

modules-crypto-user-api: create

depmod-image: modules-crypto-user-api
