#include <uuid/uuid.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <gcrypt.h>
#include <assert.h>
#include "auth.h"
/*libgcrypt11-dev*/
/*gcc -o auth auth.c -luuid -lgcrypt*/
static int auth_encrypt(char * secret, char * out, int outlen, char * in, int inlen)
{
    gcry_error_t e;
    gcry_cipher_hd_t hd;
    int algo = GCRY_CIPHER_ARCFOUR;
    int mode = GCRY_CIPHER_MODE_STREAM;
    int flags = 0;
    e = gcry_cipher_open(&hd, algo, mode, flags);
    if (e)
        return GCRYPT_ERR_OPEN;

    e = gcry_cipher_setkey(hd, secret, strlen(secret));
    if (e)
        return GCRYPT_ERR_KEY;

    e = gcry_cipher_encrypt(hd, out, outlen, in, inlen); 
    if (e)
        return GCRYPT_ERR_ENCRYPT;

    gcry_cipher_close(hd);

    return 0;
}

static int auth_decrypt(char * secret, char * out, int outlen, char * in, int inlen)
{
    gcry_error_t e;
    gcry_cipher_hd_t hd;
    int algo = GCRY_CIPHER_ARCFOUR;
    int mode = GCRY_CIPHER_MODE_STREAM;
    int flags = 0;
    e = gcry_cipher_open(&hd, algo, mode, flags);
    if (e) 
        return GCRYPT_ERR_OPEN;

    e = gcry_cipher_setkey(hd, secret, strlen(secret));
    if (e) 
        return GCRYPT_ERR_KEY;

    e = gcry_cipher_decrypt(hd, out, outlen, in, inlen);
    if (e) 
        return GCRYPT_ERR_DECRYPT;

    gcry_cipher_close(hd);

    return 0;
}

int auth_init(void)
{
    if (!gcry_check_version(GCRYPT_VERSION))
        return GCRYPT_ERR_INIT;

    /* Disable secure memory.  */
    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);

    /* Tell Libgcrypt that initialization has completed. */
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

    return 0;
}

#define UUID_STR_LEN 40
/* generate uuid string */
static char * util_uuid(char * in, int in_len)
{
    if (in && in_len < UUID_STR_LEN)
        return NULL;
    if (in == NULL) {
        in = malloc(UUID_STR_LEN);
        if (in == NULL)
            return NULL;
        bzero(in, UUID_STR_LEN);
    }
    uuid_t u;
    uuid_generate(u);
    uuid_unparse(u, in);

    return in;
}
/* use uuid as random secret key */
char * auth_secret(void)
{
    return util_uuid(NULL, 0);
}

/* use uuid as a random string for challenge request */
int auth_prepare(AuthConfig * ac)
{
    if (ac == NULL)
        return -1;
    if (ac->challenge)
        free(ac->challenge);
    ac->challenge = util_uuid(NULL, 0);
    if (ac->challenge == NULL)
        return -1;
    return 0;
}

/*
** When return, data contains encrypted data
*/
int auth_answer(AuthConfig * ac, void * data, int data_len)
{
    if (ac == NULL || data == NULL || data_len <= 0)
        return -1;
    if (ac->secret && auth_encrypt(ac->secret, data, data_len, NULL, 0) < 0) {
        return -1;
    }
    return 0;
}

/*
** Return: -1, error
**          0, verification failed
**          1, verification succeeded
**
** When return, data contains decrypted data
*/
int lauth_verify(AuthConfig * ac, void * data, int data_len)
{
    if (ac == NULL || ac->challenge == NULL || data == NULL || data_len <= 0)
        return -1;
    if (ac->secret && auth_decrypt(ac->secret, data, data_len, NULL, 0) < 0) 
	{
        return -1; 
    }
    if (strcasecmp(ac->challenge, (char *)data) == 0)
        return 1;
    return 0;
}

int auth_verify(AuthConfig * ac, void * data, int data_len)
{
    if (ac == NULL || ac->challenge == NULL || data == NULL || data_len <= 0)
        return -1;
    if (ac->secret && auth_decrypt(ac->secret, data, data_len, NULL, 0) < 0) 
	{
        return -1; 
    }

    return 0;
}

/* free AuthConfig struct */
void auth_free(AuthConfig * ac)
{
    if (ac == NULL)
        return;
    if (ac->secret) {
        free(ac->secret);
        ac->secret = NULL;
    }
    if (ac->challenge) {
        free(ac->challenge);
        ac->challenge = NULL;
    }
    return;
}

int main()
{
	AuthConfig input;
	char *data = strdup("hello encrpt world!");
	input.secret = auth_secret();
	printf("secret key(%d): %s\n", strlen(input.secret), input.secret);
	int ret = auth_init();
	assert(ret == 0);

	/*ret = auth_prepare(&input);
	assert(ret == 0);*/
	printf("before encrpt data:%s\n", data);

	ret = auth_answer(&input, data, strlen(data));
	assert(ret == 0);

	printf(" after encrpt data:%s\n", data);

	ret = auth_verify(&input, data, strlen(data));
	assert(ret == 0);

	printf(" after decrpt data:%s\n", data);
	
	return 0;
}
