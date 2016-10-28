#ifndef _INCLUDE_UTIL_LYAUTH_H_
#define _INCLUDE_UTIL_LYAUTH_H_

/*
** encryption/decryption with libgcrypt
*/
#define GCRYPT_ERR_INIT  -1
#define GCRYPT_ERR_OPEN  -2
#define GCRYPT_ERR_KEY   -3
#define GCRYPT_ERR_ENCRYPT   -4
#define GCRYPT_ERR_DECRYPT   -5

int auth_init(void);

/* authtication struct */
typedef struct AuthConfig_t {
    char * secret;
    char * challenge;
} AuthConfig;

/* prepare/verifiy authentication */
/*int auth_prepare(AuthConfig * ac);
int auth_verify(AuthConfig * ac, void * data, int data_len);
int auth_answer(AuthConfig * ac, void * data, int data_len);
void auth_free(AuthConfig * ac);*/

#endif
