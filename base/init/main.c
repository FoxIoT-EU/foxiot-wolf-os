#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <signal.h>
#include <pwd.h>
#include <sys/klog.h>
#include <signal.h>
#include <dirent.h>

#ifndef SYSLOG_ACTION_CONSOLE_OFF
#define SYSLOG_ACTION_CONSOLE_OFF 6
#endif

char ttyinit[256];
volatile pid_t pid;
volatile int idle;

static void parse_tty();
static void handler(int signal);
static void boot_next();
static void delete_contents(int level, const char *directory, dev_t rootdev);
static void switch_root(char *root);
static int of_get_u32(char *path, uint32_t *val);

int main()
{
	int status, fd;
	struct passwd *passwd;
	struct stat s;

	klogctl(SYSLOG_ACTION_CONSOLE_OFF, NULL, 0);
	if (write(1, "", 0) < 0) {
		fd = open("/dev/ttyinit", O_RDWR);
		if (fd >= 0) {
			dup2(fd, 0);
			dup2(fd, 1);
			dup2(fd, 2);
			if (fd > 2)
				close(fd);
		}
	}

	printf("\n");
	printf("Simple init for linux\n");
	printf("2018, Rebane, rebane@alkohol.ee\n\n");

	signal(SIGCHLD, SIG_IGN);
	if (mount("none", "/dev", "devtmpfs", 0, "") == 0)
		printf("/dev mounted\n");
	else
		fprintf(stderr, "failed to mount /dev\n");

	if (!stat("/rootfs", &s)) {
		printf("booting next stage init...\n");
		boot_next();
	}

	if (mount("none", "/proc", "proc", 0, "") == 0)
		printf("/proc mounted\n");
	else
		fprintf(stderr, "failed to mount /proc\n");

	parse_tty();
	printf("console: %s\n", ttyinit);
	printf("starting /etc/rc.init\n");
	pid = -1;
	idle = 0;
	signal(SIGUSR1, handler);
	system("/etc/rc.init");
	if (!stat("/etc/nologin", &s)) {
		printf("not invoking shell\n");
		idle = 1;
	}
	fflush(stdout);
	while(1) {
		if (idle) {
			close(0);
			close(1);
			close(2);
			while(1)
				sleep(1);
		}
		pid = fork();
		if (pid == 0) {
			setsid();
			fd = open("/dev/ttyinit", O_RDWR);
			if (fd >= 0) {
				dup2(fd, 0);
				dup2(fd, 1);
				dup2(fd, 2);
				if (fd > 2)
					close(fd);
			} else {
				fd = open(ttyinit, O_RDWR);
				if (fd >= 0) {
					dup2(fd, 0);
					dup2(fd, 1);
					dup2(fd, 2);
					if (fd > 2)
						close(fd);
				}
			}
			passwd = getpwuid(0);
			if ((passwd != NULL) && (passwd->pw_dir != NULL)) {
				chdir(passwd->pw_dir);
				setenv("HOME", passwd->pw_dir, 1);
			}
			signal(SIGCHLD, SIG_IGN);
			if (!stat("/etc/pwlogin", &s)) {
				printf("execl /bin/login\n");
				execl("/bin/login", "login", NULL);
				fprintf(stderr, "failed to exec /bin/login\n");
			} else {
				printf("execl /bin/sh\n");
				execl("/bin/sh", "sh", "-l", NULL);
				fprintf(stderr, "failed to exec /bin/sh\n");
			}
			sleep(5);
			exit(1);
		} else if(pid < 0) {
			fprintf(stderr, "failed to fork\n");
			sleep(5);
		} else {
			waitpid(pid, &status, 0);
			usleep(500000);
		}
	}
}

static void boot_next()
{
	uint32_t val, fs_start, fs_end;
	uint8_t btssel;
	int dev_umounted = 0;

	if (mount("none", "/sys", "sysfs", 0, "") != 0) {
		fprintf(stderr, "failed to mount /sys\n");
		return;
	}

	if (!of_get_u32("/sys/firmware/devicetree/base/chosen/foxiot,pwron", &val)) {
		umount("/sys");
		fprintf(stderr, "no boot source information\n");
		return;
	}

	btssel = val & 0x03;
	printf("boot source: %u\n", (unsigned int)btssel);
	of_get_u32("/sys/firmware/devicetree/base/chosen/foxiot,filesystem-start", &fs_start);
	of_get_u32("/sys/firmware/devicetree/base/chosen/foxiot,filesystem-end", &fs_end);
	umount("/sys");
	printf("filesystem-start: %08X\n", (unsigned int)fs_start);
	printf("filesystem-end: %08X\n", (unsigned int)fs_end);

	if (btssel == 3) {
		printf("mounting root filesystem...\n");

		if (mount("/dev/mtdblock3", "/rootfs", "jffs2", MS_RDONLY, "") != 0) {
			fprintf(stderr, "failed to mount root filesystem\n");
			return;
		}

		if (umount("/dev"))
			fprintf(stderr, "failed to umount /dev\n");
		else
			dev_umounted = 1;

	} else {
		printf("unsupported boot source\n");
		return;
	}

	/* OVERLAYFS */
	if ((!mkdir("/overlayfs/lower", 0755))
			&& (!mkdir("/overlayfs/upper", 0755))
			&& (!mkdir("/overlayfs/work", 0755))
			&& (!mount("/rootfs", "/overlayfs/lower", NULL, MS_MOVE, NULL))
			&& (!mount("none", "/rootfs", "overlay", 0, "lowerdir=/overlayfs/lower,upperdir=/overlayfs/upper,workdir=/overlayfs/work"))) {
		printf("overlayfs installed...\n");
	}
	/* OVERLAYFS */

	switch_root("/rootfs");

	if (dev_umounted)
		mount("none", "/dev", "devtmpfs", 0, "");

	fprintf(stderr, "failed to switch root\n");
}

static void delete_contents(int level, const char *directory, dev_t rootdev)
{
	struct dirent *d;
	struct stat st;
	char *newdir;
	DIR *dir;

	if (lstat(directory, &st) || st.st_dev != rootdev)
		return;

	if (S_ISDIR(st.st_mode)) {
		dir = opendir(directory);
		if (dir) {
			while ((d = readdir(dir))) {
				if (!strcmp(d->d_name, "."))
					continue;
				if (!strcmp(d->d_name, ".."))
					continue;
				if ((level == 0) && !strcmp(d->d_name, "overlayfs"))
					continue;

				asprintf(&newdir, "%s/%s", directory, d->d_name);
				delete_contents(level + 1, newdir, rootdev);
				free(newdir);
			}
			closedir(dir);
			rmdir(directory);
		}
	} else {
		unlink(directory);
	}
}

static void switch_root(char *root)
{
	struct stat st;

	chdir(root);
	stat("/", &st);
	delete_contents(0, "/", st.st_dev);
	mount(".", "/", NULL, MS_MOVE, NULL);
	chroot(".");
	chdir("/");
	execl("/init", "init", NULL);
}

static int of_get_u32(char *path, uint32_t *val)
{
	uint8_t buffer[4];
	int f;

	f = open(path, O_RDONLY);
	if (f < 0)
		return(0);
	if (read(f, buffer, 4) != 4) {
		close(f);
		return(0);
	}
	close(f);
	*val = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) | buffer[3];
	return(1);
}

static void parse_tty()
{
	int fd, i;
	char buffer[256], *s;

	ttyinit[0] = 0;
	memset(buffer, 0, 256);

	fd = open("/proc/cmdline", O_RDONLY);
	if (fd < 0)
		return;

	if (read(fd, buffer, 255) < 0) {
		close(fd);
		return;
	}

	close(fd);
	s = strstr(buffer, "console=");
	if (s == NULL)
		return;
	s += 8;
	memcpy(ttyinit, "/dev/", 5);
	for (i = 0; i < 250; i++) {
		if(s[i] == 0)break;
		if(s[i] == ',')break;
		if(s[i] == ' ')break;
		if(s[i] == '\t')break;
		if(s[i] == '\r')break;
		if(s[i] == '\n')break;
		ttyinit[5 + i] = s[i];
	}
	ttyinit[5 + i] = 0;
}

static void handler(int signal)
{
	if (!idle) {
		idle = 1;
		if (pid > 1)
			kill(pid, SIGKILL);
	}
}

