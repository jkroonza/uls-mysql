#include <mysql/mysql.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

bool uls_inet6_network_address_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
{
	if (args->arg_count != 2) {
		snprintf(message, MYSQL_ERRMSG_SIZE, "%s: Takes two arguments, an INET6 address and a mask.", __FUNCTION__);
		return 1;
	}

	args->arg_type[0] = STRING_RESULT;

	if (args->arg_type[1] != INT_RESULT) {
		snprintf(message, MYSQL_ERRMSG_SIZE, "%s: Second argument is not an integer.", __FUNCTION__);
		return 1;
	}

	args->maybe_null[0] = 0;
	args->maybe_null[1] = 0;

	initid->max_length = 40; /* 128 / 4 (nibbles) + 128 / 16 - 1 (colons) + 1 (trailing \0, to be safe) */
	initid->maybe_null = 1; /* in case of invalid input */

	return 0;
}

static
void in6_clear_host_bits(struct in6_addr *in6, int prefix_len)
{
	int bits_to_clear = 128 - prefix_len;
	int clear_index = prefix_len >> 5; /* we deal with 32 bits at a time */

	while (bits_to_clear >= 32) {
		in6->s6_addr32[clear_index--] = 0;
		bits_to_clear -= 32;
	}

	uint32_t last_mask = (~0U) << bits_to_clear;
	in6->s6_addr32[clear_index] &= htonl(last_mask);
}

char* uls_inet6_network_address(UDF_INIT * initid __attribute__((unused)), UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error __attribute__((unused)))
{
	struct in6_addr in6;
	long long prefix_len = *(long long*)args->args[1];
	*is_null = 1;

	if (prefix_len < 0 || prefix_len > 128)
		return NULL;

	if (inet_pton(AF_INET6, args->args[0], &in6) != 1)
		return NULL;

	in6_clear_host_bits(&in6, prefix_len);

	if (!inet_ntop(AF_INET6, &in6, result, *length))
		return NULL;

	*length = strlen(result);
	*is_null = 0;
	return result;
}
