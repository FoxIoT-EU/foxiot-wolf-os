#ifndef FW_COMMON_H
#define FW_COMMON_H

#include <stdint.h>
#include <stdio.h>

#define FW_TRAILER_VERSION  0x01
#define FW_TRAILER_MAGIC    "WSGN"
#define FW_MAGIC_SIZE       4
#define FW_SIG_SIZE         64
#define FW_TRAILER_SIZE     82  /* 1+1+4+4+4+64+4 */

/*
 * Trailer layout (appended to firmware):
 *
 * offset  size  field
 *   0      1    version
 *   1      1    flags (reserved, 0x00)
 *   2      4    key_id (first 4 bytes of public key)
 *   6      4    timestamp (uint32 big-endian, unix epoch)
 *  10      4    fw_size (uint32 big-endian, original firmware size)
 *  14     64    signature (Ed25519, over firmware || trailer[0..13])
 *  78      4    magic ("WSGN")
 *
 * Signature covers: SHA-512(firmware_data + trailer bytes 0..13),
 * signed with Ed25519ph.
 */

#define FW_TRAILER_PREFIX_SIZE  14  /* bytes before signature */

static inline void put_be32(uint8_t *buf, uint32_t val)
{
	buf[0] = (val >> 24) & 0xff;
	buf[1] = (val >> 16) & 0xff;
	buf[2] = (val >>  8) & 0xff;
	buf[3] =  val        & 0xff;
}

static inline uint32_t get_be32(const uint8_t *buf)
{
	return ((uint32_t)buf[0] << 24) |
	       ((uint32_t)buf[1] << 16) |
	       ((uint32_t)buf[2] <<  8) |
	        (uint32_t)buf[3];
}

static int read_key(const char *path, uint8_t *buf, size_t expected)
{
	FILE *f = fopen(path, "rb");
	if (!f) { perror(path); return -1; }

	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (len != (long)expected) {
		fprintf(stderr, "%s: expected %zu bytes, got %ld\n",
		        path, expected, len);
		fclose(f);
		return -1;
	}
	if (fread(buf, 1, expected, f) != expected) {
		perror(path);
		fclose(f);
		return -1;
	}
	fclose(f);
	return 0;
}

#endif /* FW_COMMON_H */
