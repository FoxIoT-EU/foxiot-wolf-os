CROSS    = arm-linux-gnueabi-
HOST_CC  = gcc
CROSS_CC = $(CROSS)gcc
CFLAGS   = -Wall -Os

MONO_DIR = Monocypher/src
MONO_INC = -I$(MONO_DIR) -I$(MONO_DIR)/optional
MONO_SRC = $(MONO_DIR)/monocypher.c $(MONO_DIR)/optional/monocypher-ed25519.c

.PHONY: all host target clean

all: host target

host: fw-sign fw-keygen

target: fw-verify

fw-sign: fw-sign.c fw-common.h $(MONO_SRC)
	$(HOST_CC) $(CFLAGS) $(MONO_INC) -o $@ fw-sign.c $(MONO_SRC)

fw-keygen: fw-keygen.c $(MONO_SRC)
	$(HOST_CC) $(CFLAGS) $(MONO_INC) -o $@ fw-keygen.c $(MONO_SRC)

fw-verify: fw-verify.c fw-common.h $(MONO_SRC)
	$(CROSS_CC) $(CFLAGS) $(MONO_INC) -o $@ fw-verify.c $(MONO_SRC)
	$(CROSS)strip $@

clean:
	rm -f fw-sign fw-keygen fw-verify
