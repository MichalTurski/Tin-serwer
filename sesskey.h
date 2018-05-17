#ifndef TIN_SESSKEY
#define TIN_SESSKEY

#include <openssl/evp.h>
#include <cstdlib>
#include "packet.h"

class KEY;

class Sesskey {
private:
    unsigned char key[16];
    EVP_CIPHER_CTX *ctx;
public:
	Sesskey();
	Sesskey(const KEY &key);
	~Sesskey();
	/* Both encode and decode messages assumes that encrypted message has IV in the end */
	int encrypt(unsigned char *dest, const unsigned char *src, size_t src_size) const;
	int decrypt(unsigned char *dest, const unsigned char *src, size_t src_size) const;
	const unsigned char *getKeyBuf() const;
};
#endif //TIN_SESSKEY
