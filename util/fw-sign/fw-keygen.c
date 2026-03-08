#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include "monocypher.h"
#include "monocypher-ed25519.h"

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: fw-keygen <output-dir>\n");
		return 1;
	}

	const char *dir = argv[1];
	uint8_t seed[32], secret_key[64], public_key[32];
	int err = 0;

	FILE *rng = fopen("/dev/urandom", "rb");
	if (!rng) {
		perror("/dev/urandom");
		return 1;
	}
	if (fread(seed, 1, 32, rng) != 32) {
		fprintf(stderr, "failed to read random seed\n");
		fclose(rng);
		return 1;
	}
	fclose(rng);

	crypto_ed25519_key_pair(secret_key, public_key, seed);

	if (mkdir(dir, 0700) != 0 && errno != EEXIST) {
		perror(dir);
		err = 1;
		goto out;
	}

	char path[4096];

	snprintf(path, sizeof(path), "%s/secret.key", dir);
	FILE *f = fopen(path, "wb");
	if (!f) { perror(path); err = 1; goto out; }
	if (fwrite(secret_key, 1, 64, f) != 64) {
		perror("write secret.key");
		err = 1;
	}
	fclose(f);
	chmod(path, 0600);
	if (err) goto out;

	snprintf(path, sizeof(path), "%s/public.key", dir);
	f = fopen(path, "wb");
	if (!f) { perror(path); err = 1; goto out; }
	if (fwrite(public_key, 1, 32, f) != 32) {
		perror("write public.key");
		err = 1;
	}
	fclose(f);
	if (err) goto out;

	printf("keypair generated in %s/\n", dir);
	printf("key_id: %02x%02x%02x%02x\n",
	       public_key[0], public_key[1], public_key[2], public_key[3]);

out:
	crypto_wipe(seed, sizeof(seed));
	crypto_wipe(secret_key, sizeof(secret_key));
	return err;
}
