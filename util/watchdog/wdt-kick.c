#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCK_PATH "/var/run/watchdogd.sock"

int main(int argc, char *argv[])
{
	struct sockaddr_un addr;
	char buf[64];
	int fd, len;

	if (argc < 2) {
		fprintf(stderr, "usage: wdt-kick <name> [timeout_sec]\n");
		return 1;
	}

	if (argc >= 3)
		len = snprintf(buf, sizeof(buf), "%s %s", argv[1], argv[2]);
	else
		len = snprintf(buf, sizeof(buf), "%s", argv[1]);
	if (len >= (int)sizeof(buf))
		len = sizeof(buf) - 1;

	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0)
		return 1;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

	sendto(fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
	close(fd);
	return 0;
}
