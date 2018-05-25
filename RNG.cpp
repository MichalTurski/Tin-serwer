#include <cstdlib>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/rand.h>

#include "RNG.h"
#include "log.h"

int RNG::generate(unsigned char *buf,size_t size){
	int rc = RAND_bytes(buf, size);
	if(rc != 1) {
		log(3, "Random generating failed\n");
		return -1;
	}
	return 0;
}
