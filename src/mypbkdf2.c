#include <mysql/mysql.h>
#include <memory.h>
#include <stdarg.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define PBKDF2_MAX_OUTPUT_SIZE 64 /* SHA512 .. SHA3 is 1600 bits, but we're not aiming at that yet */

static int pbkdf2_load_uint(const char* bytes, long len)
{
	if (!bytes || len > 16)
		return -1;
	while (len > (long)sizeof(int)) {
		if (bytes[--len])
			return -1;
	}

	int res = 0;
	memcpy(&res, bytes, len);
	if (res < 1)
		return -1;
	return res;
}

bool pbkdf2_hmac_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
{
	if (args->arg_count != 4) {
		snprintf(message,  MYSQL_ERRMSG_SIZE, "pbkdf2: wrong number of arguments, must be (hash[S], password[S], salt[S], iters[I])");
		return 1;
	}

	args->arg_type[0] = STRING_RESULT;
	args->arg_type[1] = STRING_RESULT;
	args->arg_type[2] = STRING_RESULT;
	args->arg_type[3] = INT_RESULT;

	args->maybe_null[0] = 0;
	args->maybe_null[1] = 0;
	args->maybe_null[2] = 0;
	args->maybe_null[3] = 0;

	args->lengths[0] = 10;
	args->lengths[3] = sizeof(int);

	initid->max_length = PBKDF2_MAX_OUTPUT_SIZE;
	initid->maybe_null = 0;

	OpenSSL_add_all_digests();

	return 0;
}

unsigned char* pbkdf2_hmac(UDF_INIT * initid __attribute__((unused)), UDF_ARGS *args, unsigned char *result, unsigned long *length, char *is_null, char *error)
{
	/* assume error */
	*is_null = 1;
	*error = 1;

	/* get number of iterations */
	int iter = pbkdf2_load_uint(args->args[3], args->lengths[3]);
	if (iter < 0)
		return NULL;

	char hashfunc[16];
	snprintf(hashfunc, sizeof(hashfunc), "%*s", (int)args->lengths[0], args->args[0]);
	const EVP_MD* md = EVP_get_digestbyname(hashfunc);
	if (!md)
		return NULL;

	if (EVP_MD_size(md) > PBKDF2_MAX_OUTPUT_SIZE)
		return NULL;

	*length = EVP_MD_size(md);
	int r = PKCS5_PBKDF2_HMAC(
			args->args[1], args->lengths[1], /* password */
			(unsigned char*)args->args[2], args->lengths[2], /* salt */
			iter, md, *length, result);

	if (r == 0)
		return NULL;

	*is_null = 0;
	*error = 0;

	return result;
}

bool get_salt_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
{
	if (args->arg_count != 1) {
		snprintf(message, MYSQL_ERRMSG_SIZE, "get_salt: takes exactly one argument, the size in bytes of the salt");
		return 1;
	}

	args->arg_type[0] = INT_RESULT;
	args->maybe_null[0] = 0;
	args->lengths[0] = sizeof(int);

	initid->max_length = 65536;
	initid->maybe_null = 0;

	return 0;
}

unsigned char* get_salt(UDF_INIT * initid __attribute__((unused)), UDF_ARGS *args, unsigned char *result, unsigned long *length, char *is_null, char *error)
{
	/* assume error */
	*is_null = 1;
	*error = 1;

	int bytes = pbkdf2_load_uint(args->args[0], args->lengths[0]);
	if (bytes < 1)
		return NULL;

	if (bytes > 65536)
		return NULL;

	if (bytes > 255) {
		initid->ptr = malloc(bytes);
		if (!initid->ptr)
			return NULL;
		result = (unsigned char*)initid->ptr;
	}

	*length = bytes;
	if (RAND_bytes(result, bytes) != 1)
		return NULL;

	*is_null = 0;
	*error = 0;

	return result;
}

void get_salt_deinit(UDF_INIT * initid)
{
	if (initid->ptr)
		free(initid->ptr);
}
