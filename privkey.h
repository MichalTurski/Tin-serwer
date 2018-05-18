#ifndef PRIVKEY_H
#define PRIVKEY_H

#include <openssl/rsa.h>

class Privkey{
private:
	RSA *key;

public:
	int decrypt(const unsigned char *encrypted, size_t encrypted_len, unsigned char *data) const;
	int encrypt(const unsigned char *data, size_t data_len, unsigned char *encrypted) const;
	Privkey(const char* file_name);
	Privkey();
	~Privkey();
};
#endif //PRIVKEY_H
