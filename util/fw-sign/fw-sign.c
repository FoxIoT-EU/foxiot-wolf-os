#include <stdio.h>
#include <string.h>
#include <time.h>
#include "monocypher.h"
#include "monocypher-ed25519.h"
#include "fw-common.h"

#define BLOCK_SIZE 4096

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: fw-sign <firmware.itb> <secret.key>\n");
		return 1;
	}

	const char *fw_path  = argv[1];
	const char *key_path = argv[2];
	int ret = 1;

	/* Read secret key (64 bytes: seed || public_key) */
	uint8_t secret_key[64];
	if (read_key(key_path, secret_key, 64) != 0)
		return 1;
	uint8_t *public_key = secret_key + 32;

	/* Open firmware */
	FILE *fw = fopen(fw_path, "rb");
	if (!fw) { perror(fw_path); goto out; }

	fseek(fw, 0, SEEK_END);
	long len = ftell(fw);
	if (len < 0) { perror(fw_path); fclose(fw); goto out; }
	size_t fw_size = (size_t)len;

	if (fw_size > UINT32_MAX) {
		fprintf(stderr, "firmware too large\n");
		fclose(fw); goto out;
	}

	/* Refuse to sign twice */
	if (fw_size >= FW_MAGIC_SIZE) {
		uint8_t tail[FW_MAGIC_SIZE];
		fseek(fw, -(long)FW_MAGIC_SIZE, SEEK_END);
		if (fread(tail, 1, FW_MAGIC_SIZE, fw) == FW_MAGIC_SIZE &&
		    memcmp(tail, FW_TRAILER_MAGIC, FW_MAGIC_SIZE) == 0) {
			fprintf(stderr, "firmware is already signed\n");
			fclose(fw); goto out;
		}
	}

	/* Build trailer prefix */
	uint8_t prefix[FW_TRAILER_PREFIX_SIZE];
	prefix[0] = FW_TRAILER_VERSION;
	prefix[1] = 0x00;
	memcpy(prefix + 2, public_key, 4);
	put_be32(prefix + 6, (uint32_t)time(NULL));
	put_be32(prefix + 10, (uint32_t)fw_size);

	/* Stream firmware through SHA-512 */
	crypto_sha512_ctx ctx;
	crypto_sha512_init(&ctx);

	fseek(fw, 0, SEEK_SET);
	uint8_t block[BLOCK_SIZE];
	size_t n;
	while ((n = fread(block, 1, BLOCK_SIZE, fw)) > 0)
		crypto_sha512_update(&ctx, block, n);
	if (ferror(fw)) {
		fprintf(stderr, "read error: %s\n", fw_path);
		fclose(fw); goto out;
	}
	fclose(fw);

	crypto_sha512_update(&ctx, prefix, FW_TRAILER_PREFIX_SIZE);

	uint8_t hash[64];
	crypto_sha512_final(&ctx, hash);

	/* Sign hash with Ed25519ph */
	uint8_t signature[FW_SIG_SIZE];
	crypto_ed25519_ph_sign(signature, secret_key, hash);

	/* Append trailer to file */
	FILE *f = fopen(fw_path, "ab");
	if (!f) { perror(fw_path); goto out; }
	if (fwrite(prefix, 1, FW_TRAILER_PREFIX_SIZE, f) != FW_TRAILER_PREFIX_SIZE ||
	    fwrite(signature, 1, FW_SIG_SIZE, f)         != FW_SIG_SIZE ||
	    fwrite(FW_TRAILER_MAGIC, 1, FW_MAGIC_SIZE, f) != FW_MAGIC_SIZE) {
		perror("write trailer");
		fclose(f); goto out;
	}
	fclose(f);

	printf("signed: %s (%zu bytes + %d byte trailer)\n",
	       fw_path, fw_size, FW_TRAILER_SIZE);
	printf("key_id: %02x%02x%02x%02x\n",
	       public_key[0], public_key[1], public_key[2], public_key[3]);
	ret = 0;

out:
	crypto_wipe(hash, sizeof(hash));
	crypto_wipe(block, sizeof(block));
	crypto_wipe(prefix, sizeof(prefix));
	crypto_wipe(secret_key, sizeof(secret_key));
	return ret;
}
