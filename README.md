Plugin module for MariaDB/MySQL that provides some HMAC related functionality.

# Installation Instructions

Run:

make
make install

For package managers you can pass DESTDIR for the latter, eg:

make install DESTDIR=/var/tmp/portage/...

# Usage instructions

Once installed, you can activate a specific function by doing for example:

CREATE FUNCTION pbkdf2\_hmac RETURNS STRING SONAME 'mypbkdf2.so';

# Available Modules/Functions
## mypbkdf2.so

Functions we use for implementing HMAC auth in programs that doesn't support
HMAC but does have MySQL authentication connectors.

### get\_salt(bytes[I]) [S]

Simple function to generate random bytes, which presumably will be used for
SALT when running passwords through PBKDF2.  This is slightly misnamed, but
we're not going to fix that.

### pbkdf2\_hmac(hash[S], password[S], salt[S], iters[I]) [S]

hash is the name of the hash function to use, as per OpenSSL's EVP\_get\_digestbyname.

password is the plaintext password to be hashed.

salt is the salt to use.

iters is the number of iterations to run (>=1).

This will return NULL on error (and set appropriate error message).

On success it will return a byte sequence.

## pipesignal.so

This is in order to open a file and write a single character buffer to it, and
close it.  This should be used carefully as it can have very serious
performance implications.

### pipesignal(path[S], string[S])

Both buffers are limited to 256 characters by default.

Will open(path) and then write(string) to it before closing.

IMPORTANT:  Any file that's writable by the MySQL/MariaDB process can be
overwritten using this, including stuff in datadir.  Do NOT deploy on servers
where users can't be trusted.

## uls\_inet6

Functions that we find lacking in MariaDB which can be used to manipulate INET6
addresses.  Not particularly efficiently since MariaDB essentially converts
them to a string before passing them to us, and then for most calculations we
want a binary format, and then convert back to string before handing back to
MariaDB.  Still, better that the alternatives.

*IMPORTANT*: MariaDB and UDFs combined with non-basic types (like inet6) is a
mess.  Since the function is accepting a string it will cast from INET6 to
STRING, so this is inefficient due to back and forth conversions, but more
importantly, we can't specify the character set.  MySQL has some stuff for
that, but it doesn't have an INET6 data type.  For this reason, use:

```
CAST(uls_inet6_*(prefix, length) AS CHAR)
```

Which will fix the type, and result in the correct result.

The real fix here is twofold:

1. For MySQL we need to integrate the string typing.
1. For MariaDB we should rather use the PLUGIN architecture rather than an UDF.

### uls\_inet6\_network\_address(address[S], prefixlen[I]) [S]

This will calculate the network address of an ip/prefix pair, and return the
base address.  As an example:

```
uls\_inet6\_network\_address("::ffff:192.168.255.0", 120) => "::ffff:192.168.240.0"
```

Which also shows that (with some foresight) these functions can also be used to
manipulate IPv4 addresses.

### uls\_inet6\_last\_address(address[S], prefixlen[I]) [S]

This will calculate what was traditionally known as the broadcast address, and
can be used for IPv4 to still calculate that.  More correctly the aim here is
to enable things like:

```
SELECT stuff FROM table WHERE 'dead:beaf::' <= inet6_column && inet6_column <= uls_inet6_last_address('dead:beaf::', 32);
```
