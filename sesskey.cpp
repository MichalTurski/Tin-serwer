#include <cstring>
#include "sesskey.h"
#include "utils.h"
#include "RNG.h"

extern RNG rng;

Sesskey::Sesskey() {
	if (rng.generate(key, (size_t) 16) == -1) {
		log(1, "Unable to create sesskey\n");
	}
	if (!(ctx = EVP_CIPHER_CTX_new()))
	    log(1, "Unable to create OPENSSL context.\n");
}

Sesskey::~Sesskey() {
	EVP_CIPHER_CTX_free(ctx);
}

// This function is called only for messages that needs encryption
int Sesskey::encrypt(unsigned char *dest, const unsigned char *src, size_t src_size) const{
	int len, ciphertext_len;
	unsigned char iv[16];

	if (rng.generate(iv, (size_t) 16) == -1) {
		log(1, "Unable to create IV\n");
		return -1;
	}

	if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv)) {
		log(1, "Unable to init EVP encryption.\n");
		return -1;
	}
	if (1 != EVP_EncryptUpdate(ctx, dest, &len, src, src_size)){
		log(1, "Unable to encrypt.\n");
		return -1;
	}
	ciphertext_len = len;
	if(1 != EVP_EncryptFinal_ex(ctx, dest + len, &len)) {
		log(1, "Unable to finalize EVP encryption.\n");
		return -1;
	}
	ciphertext_len += len;
	memcpy(dest + ciphertext_len, iv, 16);
	ciphertext_len += 16;

	return ciphertext_len;
}

// This function is called only for messages that needs decryption
int Sesskey::decrypt(unsigned char *dest, const unsigned char *src, size_t src_size) const{
	const unsigned char *iv;
	unsigned char plain_size;
	int len;
	int msg_buf_size;

	msg_buf_size = src_size - 16;// -16 for iv

	iv = &src[(plain_size / 16 + 1) * 16];
	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv)) {
		log(1, "Unable to init EVP decryption.\n");
		return -1;
	}

  	if (1 != EVP_DecryptUpdate(ctx, dest, &len, src, msg_buf_size)) {
		log(1, "Unable to decrypt.\n");
		return -1;
	}

  	if (1 != EVP_DecryptFinal_ex(ctx, dest + len, &len)) {
		log(1, "Unable to finalize EVP encryption.\n");
		return -1;
	}

  	return 0;
}
const unsigned char* Sesskey::getKeyBuf() const {
    return key;
}
Sesskey::Sesskey(const KEY &keyPck) {
	memcpy(key, keyPck.getKeyBuf(), 16);
}
