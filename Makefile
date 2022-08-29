TARGET_MODS = mypbkdf2

MODS_mypbkdf2 = mypbkdf2
LIBS_mypbkdf2 = crypto

VARIANT=mariadb
MODDIR=$(shell $(VARIANT)_config --plugindir)

include Makefile.inc
