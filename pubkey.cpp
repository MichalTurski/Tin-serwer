#include <openssl/pem.h>
#include <stdexcept>
#include "pubkey.h"
#include "utils.h"

Pubkey::Pubkey(const char* file_name) {
	FILE *fp = fopen(file_name,"rb");
	if(fp == NULL) {
		log(1, "Unable to open public key file\n");
		throw std::runtime_error("Unable to open pubkey file");
	}
	
	key = RSA_new();
	if (key == NULL) {
		log(2, "Unable to allocate RSA key\n"); 
		fclose(fp);
		throw std::runtime_error("Unable to allocate RSA key");
	}
	
	key = PEM_read_RSA_PUBKEY(fp, &key, NULL, NULL);
	if (key == NULL) {
		log(1, "Unable to read public key from file %s\n", file_name);
		fclose(fp);
		throw std::runtime_error("Unable to read public key");
	}

	fclose(fp);
}
Pubkey::~Pubkey() {
	RSA_free(key);
}
/*data size have to be big enough to keep decrypted message. */
int Pubkey::decrypt (const unsigned char *encrypted, size_t encrypted_len, unsigned char *data) const {
	int result;
	result = RSA_public_decrypt(encrypted_len, encrypted, data, key, RSA_PKCS1_PADDING);
	return result;
}
int Pubkey::encrypt(const unsigned char *data, size_t data_len, unsigned char *encrypted) const {
	int result;
	result = RSA_public_encrypt(data_len, data, encrypted, key, RSA_PKCS1_PADDING);
	//result = RSA_public_encrypt(data_len, data, encrypted, key, RSA_PKCS1_PADDING);
	return result;
}

bool Pubkey::verify_resp(const unsigned char *resp, size_t resp_size, const unsigned char *chal, size_t chal_size) const{
	unsigned char *buf;
	buf = new unsigned char[chal_size ];
	if (decrypt(resp, resp_size, buf) < 0) {
	    delete(buf);
		return false;
	}
	for (int i = 0; i < chal_size; i++) {
		if (buf[i] != chal [i]) {
			delete[] buf;
			return false;
		}
	}
	delete[] buf;
	return true;
}
