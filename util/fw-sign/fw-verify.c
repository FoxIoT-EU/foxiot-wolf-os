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
		fprintf(stderr, "usage: fw-verify <firmware.itb> <public.key>\n");
		return 1;
	}

	const char *fw_path  = argv[1];
	const char *key_path = argv[2];

	/* Read public key (32 bytes) */
	uint8_t public_key[32];
	if (read_key(key_path, public_key, 32) != 0)
		return 1;

	/* Open signed firmware */
	FILE *fw = fopen(fw_path, "rb");
	if (!fw) { perror(fw_path); return 1; }

	fseek(fw, 0, SEEK_END);
	long len = ftell(fw);
	if (len < 0) { perror(fw_path); fclose(fw); return 1; }
	size_t file_size = (size_t)len;

	if (file_size < FW_TRAILER_SIZE) {
		fprintf(stderr, "fw-verify: FAILED - file too small\n");
		fclose(fw); return 1;
	}

	/* Read trailer (last 82 bytes) */
	uint8_t trailer[FW_TRAILER_SIZE];
	fseek(fw, -(long)FW_TRAILER_SIZE, SEEK_END);
	if (fread(trailer, 1, FW_TRAILER_SIZE, fw) != FW_TRAILER_SIZE) {
		fprintf(stderr, "fw-verify: read error\n");
		fclose(fw); return 1;
	}

	/* Check magic */
	if (memcmp(trailer + FW_TRAILER_SIZE - FW_MAGIC_SIZE,
	           FW_TRAILER_MAGIC, FW_MAGIC_SIZE) != 0) {
		fprintf(stderr, "fw-verify: FAILED - "
		        "not a signed firmware (no WSGN magic)\n");
		fclose(fw); return 1;
	}

	uint8_t  version   = trailer[0];
	uint8_t *key_id    = trailer + 2;
	uint32_t timestamp = get_be32(trailer + 6);
	uint32_t fw_size   = get_be32(trailer + 10);
	uint8_t *signature = trailer + FW_TRAILER_PREFIX_SIZE;

	if (version != FW_TRAILER_VERSION) {
		fprintf(stderr, "fw-verify: FAILED - "
		        "unknown trailer version %u\n", version);
		fclose(fw); return 1;
	}

	if ((size_t)fw_size + FW_TRAILER_SIZE != file_size) {
		fprintf(stderr, "fw-verify: FAILED - size mismatch "
		        "(header says %u, file is %zu)\n", fw_size, file_size);
		fclose(fw); return 1;
	}

	if (memcmp(key_id, public_key, 4) != 0) {
		fprintf(stderr, "fw-verify: FAILED - key_id mismatch "
		        "(firmware=%02x%02x%02x%02x, "
		        "device=%02x%02x%02x%02x)\n",
		        key_id[0], key_id[1], key_id[2], key_id[3],
		        public_key[0], public_key[1], public_key[2],
		        public_key[3]);
		fclose(fw); return 1;
	}

	/* Stream firmware (fw_size bytes only) through SHA-512 */
	crypto_sha512_ctx ctx;
	crypto_sha512_init(&ctx);

	fseek(fw, 0, SEEK_SET);
	uint8_t block[BLOCK_SIZE];
	size_t remaining = fw_size;
	while (remaining > 0) {
		size_t chunk = remaining < BLOCK_SIZE ? remaining : BLOCK_SIZE;
		size_t n = fread(block, 1, chunk, fw);
		if (n != chunk) {
			fprintf(stderr, "fw-verify: read error\n");
			fclose(fw); return 1;
		}
		crypto_sha512_update(&ctx, block, n);
		remaining -= n;
	}
	if (ferror(fw)) {
		fprintf(stderr, "fw-verify: read error\n");
		fclose(fw); return 1;
	}
	fclose(fw);

	/* Hash trailer prefix */
	crypto_sha512_update(&ctx, trailer, FW_TRAILER_PREFIX_SIZE);

	uint8_t hash[64];
	crypto_sha512_final(&ctx, hash);

	/* Verify with Ed25519ph */
	if (crypto_ed25519_ph_check(signature, public_key, hash) != 0) {
		fprintf(stderr, "fw-verify: FAILED - signature mismatch\n");
		return 1;
	}

	time_t ts = (time_t)timestamp;
	struct tm *tm = gmtime(&ts);
	char timestr[20];
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm);

	printf("fw-verify: valid (key_id=%02x%02x%02x%02x, "
	       "signed=%s, size=%u)\n",
	       key_id[0], key_id[1], key_id[2], key_id[3],
	       timestr, fw_size);
	return 0;
}
