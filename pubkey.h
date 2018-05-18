#ifndef PUBKEY_H
#define PUBKEY_H

#include <openssl/rsa.h>

class Pubkey{
private:
	RSA *key;

public:
	int decrypt(const unsigned char *encrypted, size_t encrypted_len, unsigned char *data) const;
	int encrypt(const unsigned char *data, size_t data_len, unsigned char *encrypted) const;
	Pubkey(const char* file_name);
	~Pubkey();
	bool verify_resp(const unsigned char *resp, size_t resp_size, const unsigned char *chall, size_t chal_size) const;
};
#endif //PUBKEY_H
