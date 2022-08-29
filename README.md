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

### get\_salt(bytes[I])

Simple function to generate random bytes, which presumably will be used for
SALT when running passwords through PBKDF2.  This is slightly misnamed, but
we're not going to fix that.

### pbkdf2\_hmac(hash[S], password[S], salt[S], iters[I])

hash is the name of the hash function to use, as per OpenSSL's EVP\_get\_digestbyname.

password is the plaintext password to be hashed.

salt is the salt to use.

iters is the number of iterations to run (>=1).

This will return NULL on error (and set appropriate error message).

On success it will return a byte sequence.
