TARGET_MODS = mypbkdf2

MODS_mypbkdf2 = mypbkdf2
LIBS_mypbkdf2 = crypto

MODDIR=$(shell mysql_config --plugindir)

include Makefile.inc
