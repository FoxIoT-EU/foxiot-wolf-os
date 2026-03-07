#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/watchdog.h>
#include <unistd.h>

#define SOCK_PATH	"/var/run/watchdogd.sock"
#define MAX_CLIENTS	8
#define DEFAULT_TIMEOUT	120
#define NAME_LEN	32
#define LOG_MAX_SIZE	(250 * 1024)

struct client {
	char name[NAME_LEN];
	int timeout_sec;
	struct timespec last_kick;
};

static int wdt_fd = -1;
static int sock_fd = -1;
static struct client clients[MAX_CLIENTS];
static int num_clients;
static FILE *log_fp;
static char log_path[128];

static void log_rotate(void)
{
	struct stat st;
	char old_path[140];

	if (!log_fp || !log_path[0])
		return;
	if (fstat(fileno(log_fp), &st) < 0 || st.st_size < LOG_MAX_SIZE)
		return;

	snprintf(old_path, sizeof(old_path), "%s.old", log_path);
	fclose(log_fp);
	rename(log_path, old_path);
	log_fp = fopen(log_path, "a");
}

static void wdt_log(const char *fmt, ...)
{
	FILE *out;
	va_list ap;
	time_t t;
	struct tm tm;
	char ts[20];

	log_rotate();

	out = log_fp ? log_fp : stderr;
	t = time(NULL);
	localtime_r(&t, &tm);
	strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm);
	fprintf(out, "[%s] ", ts);

	va_start(ap, fmt);
	vfprintf(out, fmt, ap);
	va_end(ap);

	fflush(out);
	if (out != stderr)
		fsync(fileno(out));
}

static void set_log_file(const char *path)
{
	if (log_fp) {
		fclose(log_fp);
		log_fp = NULL;
	}

	strncpy(log_path, path, sizeof(log_path) - 1);
	log_path[sizeof(log_path) - 1] = '\0';

	log_fp = fopen(log_path, "a");
	if (!log_fp) {
		log_path[0] = '\0';
		fprintf(stderr, "failed to open log file '%s'\n", path);
		return;
	}
	wdt_log("logging started\n");
}

static void shutdown_handler(int sig)
{
	(void)sig;
	if (sock_fd >= 0) {
		close(sock_fd);
		unlink(SOCK_PATH);
	}
	/* no 'V' — let HW WDT reset the system */
	_exit(0);
}

static void now(struct timespec *ts)
{
	clock_gettime(CLOCK_MONOTONIC, ts);
}

static int elapsed_sec(struct timespec *since)
{
	struct timespec ts;
	now(&ts);
	return (int)(ts.tv_sec - since->tv_sec);
}

static struct client *find_client(const char *name)
{
	int i;
	for (i = 0; i < num_clients; i++) {
		if (strcmp(clients[i].name, name) == 0)
			return &clients[i];
	}
	return NULL;
}

static struct client *register_client(const char *name, int timeout_sec)
{
	struct client *c;

	if (num_clients >= MAX_CLIENTS) {
		wdt_log("client table full, rejecting '%s'\n", name);
		return NULL;
	}

	c = &clients[num_clients++];
	strncpy(c->name, name, NAME_LEN - 1);
	c->name[NAME_LEN - 1] = '\0';
	c->timeout_sec = timeout_sec;
	now(&c->last_kick);

	wdt_log("registered client '%s' timeout=%ds\n",
		c->name, c->timeout_sec);
	return c;
}

static void handle_message(char *buf, int len)
{
	char name[NAME_LEN];
	int timeout_sec = DEFAULT_TIMEOUT;
	struct client *c;

	if (len <= 0)
		return;
	buf[len] = '\0';

	/* trim trailing newline */
	if (buf[len - 1] == '\n')
		buf[len - 1] = '\0';

	/* "log <path>" — set log file */
	if (strncmp(buf, "log ", 4) == 0) {
		set_log_file(buf + 4);
		return;
	}

	/* parse: "<name>" or "<name> <timeout>" */
	if (sscanf(buf, "%31s %d", name, &timeout_sec) < 1)
		return;
	if (timeout_sec < 1)
		timeout_sec = DEFAULT_TIMEOUT;

	c = find_client(name);
	if (c) {
		now(&c->last_kick);
	} else {
		register_client(name, timeout_sec);
	}
}

static int check_clients(void)
{
	int i;
	for (i = 0; i < num_clients; i++) {
		if (elapsed_sec(&clients[i].last_kick) > clients[i].timeout_sec) {
			wdt_log("client '%s' expired — stopping WDT kicks\n",
				clients[i].name);
			return -1;
		}
	}
	return 0;
}

static int setup_socket(void)
{
	struct sockaddr_un addr;
	int fd;

	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0) {
		wdt_log("socket() failed\n");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

	unlink(SOCK_PATH);
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		wdt_log("bind(%s) failed\n", SOCK_PATH);
		close(fd);
		return -1;
	}

	return fd;
}

int main(void)
{
	int timeout = 32;
	struct pollfd pfd;
	char buf[64];
	int expired = 0;

	signal(SIGTERM, shutdown_handler);
	signal(SIGINT, shutdown_handler);

	wdt_fd = open("/dev/watchdog", O_RDWR);
	if (wdt_fd < 0) {
		wdt_log("/dev/watchdog open failed\n");
		exit(1);
	}

	if (ioctl(wdt_fd, WDIOC_SETTIMEOUT, &timeout) < 0)
		wdt_log("failed to set watchdog timeout\n");

	if (ioctl(wdt_fd, WDIOC_GETTIMEOUT, &timeout) < 0)
		wdt_log("failed to get watchdog timeout\n");
	else
		wdt_log("watchdog timeout is %d seconds\n", timeout);

	sock_fd = setup_socket();
	if (sock_fd < 0) {
		wdt_log("running without socket\n");
	}

	pfd.fd = sock_fd;
	pfd.events = POLLIN;

	while (1) {
		if (sock_fd >= 0 && poll(&pfd, 1, 500) > 0 &&
		    (pfd.revents & POLLIN)) {
			int n;
			while ((n = recvfrom(sock_fd, buf, sizeof(buf) - 1,
					     MSG_DONTWAIT, NULL, NULL)) > 0)
				handle_message(buf, n);
		}

		if (!expired && check_clients() < 0)
			expired = 1;

		if (!expired) {
			if (ioctl(wdt_fd, WDIOC_KEEPALIVE, 0) < 0)
				wdt_log("failed to kick watchdog\n");
		}

		if (sock_fd < 0)
			usleep(500000);
	}

	return 0;
}
