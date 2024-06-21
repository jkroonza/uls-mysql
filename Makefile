TARGET_MODS = mypbkdf2 pipesignal uls_inet6

LIBS_mypbkdf2 = crypto

MODDIR=$(shell mysqld --help --verbose 2>/dev/null | awk '$$1=="plugin-dir" { print $$2 }')

include Makefile.inc
