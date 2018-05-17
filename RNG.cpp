#include <cstdlib>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/rand.h>

#include "RNG.h"
#include "utils.h"

RNG::RNG() {
	ERR_load_crypto_strings ();
	OpenSSL_add_all_algorithms ();
	OPENSSL_config (NULL);
}
RNG::~RNG() {
	ERR_free_strings ();
	RAND_cleanup ();
	EVP_cleanup ();
	CONF_modules_free ();
	ERR_remove_state (0);
}

int RNG::generate(unsigned char *buf,size_t size){
	int rc = RAND_bytes(buf, size);
	if(rc != 1) {
		log(1, "Random genration failed\n");
		return -1;
	}
	return 0;
}
