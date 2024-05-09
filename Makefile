TARGET_MODS = mypbkdf2 pipesignal

MODS_mypbkdf2 = mypbkdf2
LIBS_mypbkdf2 = crypto

MODS_pipesignal = pipesignal

MODDIR=$(shell mysqld --help --verbose 2>/dev/null | awk '$$1=="plugin-dir" { print $$2 }')

include Makefile.inc
