// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include "wireguard.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
		
	if (argc < 3) {
		printf("Usage: %s <command> <interface>\n", argv[0]);
		printf("\t add: add interface\n");
		printf("\t del: delete interface\n");
		exit(1);
	}
	
	wg_device new_device;
	strncpy(new_device.name, argv[2], IFNAMSIZ);

	if (!strcmp("add", argv[1])) {
		if (wg_add_device(new_device.name) < 0) {
			perror("Unable to add device");
			exit(1);
		}
	} else if (!strcmp("del", argv[1])){
		if (wg_del_device(new_device.name) < 0) {
			perror("Unable to delete device");
			exit(1);
		}
	} else {
		fprintf(stderr, "Usage: %s <command> <interface>\n", argv[0]);
	}

	return 0;
}

