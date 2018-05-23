//
// Created by michal on 10.05.18.
//

#include <openssl/pem.h>
#include <stdexcept>
#include "privkey.h"
#include "utils.h"

Privkey::Privkey(const char* file_name) {
    FILE *fp = fopen(file_name,"rb");
    if(fp == NULL) {
        log(1, "CRITICAL: Unable to open private key file\n");
        throw std::runtime_error("Unable to open privkey file");
    }

    key = RSA_new();
    if (key == NULL) {
        log(1, "CRITICAL: Unable to allocate RSA key\n");
        fclose(fp);
        throw std::runtime_error("Unable to allocate RSA key");
    }

    //key = PEM_read_RSA_PRIVKEY(fp, &key, NULL, NULL);
    key = PEM_read_RSAPrivateKey(fp, &key, NULL, NULL);
    if (key == NULL) {
        log(1, "CRITICAL: Unable to read private key from file %s\n", file_name);
        fclose(fp);
        throw std::runtime_error("Unable to read private key");
    }

    fclose(fp);
}
Privkey::Privkey() {
    key = nullptr;
}
Privkey::~Privkey() {
    RSA_free(key);
}
/*data size have to be big enough to keep decrypted message. */
int Privkey::decrypt (const unsigned char *encrypted, size_t encrypted_len, unsigned char *data) const {
    int result;
    result = RSA_private_decrypt(encrypted_len, encrypted, data, key, RSA_PKCS1_OAEP_PADDING);
    return result;
}
int Privkey::encrypt(const unsigned char *data, size_t data_len, unsigned char *encrypted) const {
    int result;
    result = RSA_private_encrypt(data_len, data, encrypted, key, RSA_PKCS1_OAEP_PADDING);
    return result;
}
int Privkey::sign(const unsigned char *data, size_t data_len, unsigned char *sign, unsigned int *sign_len) const {
    SHA_CTX sha_ctx = { 0 };
    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1_Init(&sha_ctx);
    SHA1_Update(&sha_ctx, data, data_len);
    SHA1_Final(digest, &sha_ctx);
    return RSA_sign(NID_sha1, digest, SHA_DIGEST_LENGTH, sign, sign_len, key);
}
