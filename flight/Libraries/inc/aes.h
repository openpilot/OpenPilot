
#ifndef _AES_H_
#define _AES_H_

#define N_ROW			4
#define N_COL			4
#define N_BLOCK			(N_ROW * N_COL)

void aes_encrypt_cbc_128(void *data, void *key, void *chain_block);
void aes_decrypt_cbc_128(void *data, void *key, void *chain_block);
void aes_decrypt_key_128_create(void *enc_key, void *dec_key);

void aes_encrypt_cbc_256(void *data, void *key, void *chain_block);
void aes_decrypt_cbc_256(void *data, void *key, void *chain_block);
void aes_decrypt_key_256_create(void *enc_key, void *dec_key);

#endif
