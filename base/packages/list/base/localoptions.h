/* wolf-os dropbear 2025.89 local options — CRA-aligned minimal build
 *
 * Only overrides from upstream default_options.h are listed here.
 * Everything not set remains at the upstream default.
 */
#ifndef WOLFOS_LOCALOPTIONS_H
#define WOLFOS_LOCALOPTIONS_H

/* --- Extra MAC: keep SHA-512 HMAC alongside SHA-256 as a stronger fallback --- */
#define DROPBEAR_SHA2_512_HMAC 1

/* --- Disable SFTP subsystem ---
 * Dropbear's SFTP support just invokes an external sftp-server binary at
 * /usr/libexec/sftp-server, which wolf-os does not ship. Compiling it in
 * would be dead code.
 */
#define DROPBEAR_SFTPSERVER 0

/* --- Disable U2F / FIDO security-key auth ---
 * Hardware security keys (sk-ecdsa / sk-ed25519) don't apply to a
 * headless embedded box.
 */
#define DROPBEAR_SK_KEYS 0

/* --- Disable server-side agent forwarding ---
 * Accepting -A from incoming clients would expose the laptop's ssh-agent
 * to processes running on the wolf device. Jump-host workflows (ssh -J)
 * do not need this — they use TCP forwarding, which stays enabled.
 * Client-side agent forwarding (wolf dbclient -> another server) is left
 * at the upstream default (enabled).
 */
#define DROPBEAR_SVR_AGENTFWD 0

/* --- No MOTD ---
 * Embedded device, no /etc/motd shipped.
 */
#define DO_MOTD 0

/* --- Post-quantum key exchange ---
 * sntrup761 (~9 KB on 32-bit ARM) is a worthwhile hedge against
 * "harvest now, decrypt later" attacks. mlkem768 (~34 KB) is too large
 * for our footprint budget on armv5.
 */
#define DROPBEAR_SNTRUP761 1
#define DROPBEAR_MLKEM768  0

/* --- Explicit documentation of CRA policy ---
 * These are already the upstream defaults in 2025.89, but listed here
 * as explicit policy so future maintainers can see the intent without
 * cross-referencing default_options.h.
 */
#define DROPBEAR_3DES 0
#define DROPBEAR_ENABLE_CBC_MODE 0
#define DROPBEAR_SHA1_HMAC 0
#define DROPBEAR_SHA1_96_HMAC 0
#define DROPBEAR_RSA_SHA1 0
#define DROPBEAR_DSS 0
#define DROPBEAR_DH_GROUP14_SHA1 0
#define DROPBEAR_DH_GROUP1 0
#define DROPBEAR_DH_GROUP16 0
#define DROPBEAR_X11FWD 0
#define DO_HOST_LOOKUP 0

#endif /* WOLFOS_LOCALOPTIONS_H */
