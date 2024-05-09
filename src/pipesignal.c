#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef PIPESIGNAL_SAFEBASE
#warning You really aught to pass EXTRA_CFLAGS='-DPIPESIGNAL_SAFEBASE=\"/some/path/\"'
#endif

bool pipesignal_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
{
	if (args->arg_count != 2) {
		snprintf(message,  MYSQL_ERRMSG_SIZE, "pipesignal: Wrong number of arguments, takes exactly two, path and string to write.");
		return 1;
	}

	args->arg_type[0] = STRING_RESULT;
	args->arg_type[1] = STRING_RESULT;

	args->maybe_null[0] = 0;
	args->maybe_null[1] = 0;

	args->lengths[0] = 256;
	args->lengths[1] = 256;

	initid->max_length = 1;
	initid->maybe_null = 0;

	return 0;
}

long long pipesignal(UDF_INIT * initid __attribute__((unused)), UDF_ARGS *args, char *is_null, char *error)
{
	/* assume error */
	*is_null = 1;
	*error = 1;

#ifdef PIPESIGNAL_SAFEBASE
	if (strncmp(PIPESIGNAL_SAFEBASE, args->args[0], strlen(PIPESIGNAL_SAFEBASE)))
		return 0;
#endif

	int fd = open(args->args[0], O_WRONLY | O_CLOEXEC | O_TRUNC | O_CREAT, 0666);
	if (fd < 0)
		return 0;

	write(fd, args->args[1], args->lengths[1]);
	close(fd);

	*is_null = 0;
	*error = 0;

	return 1;
}
