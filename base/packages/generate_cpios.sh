#!/bin/bash

for i in list/* ; do
	f=`basename $i`
	echo generating "cpio/$f.cpio"
	../bin/gen_init_fs -I"list/$f" -c "list/$f/$f.list" >"cpio/$f.cpio"
done

