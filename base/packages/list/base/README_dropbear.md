# Rebuilding dropbearmulti

Dropbear 2025.89, MULTI build, cross-compiled for `arm-linux-gnueabi`.

```sh
git clone https://github.com/mkj/dropbear.git dropbear-2025.89
cd dropbear-2025.89
git checkout DROPBEAR_2025.89

# drop the wolf-os feature overrides in place (next to configure)
cp /path/to/base/localoptions.h .

CFLAGS="-Os" ./configure \
    --host=arm-linux-gnueabi \
    --disable-zlib \
    --disable-lastlog \
    --disable-utmp --disable-utmpx \
    --disable-wtmp --disable-wtmpx \
    --disable-loginfunc \
    --disable-pututline --disable-pututxline

make MULTI=1 PROGRAMS="dropbear dbclient dropbearkey dropbearconvert scp" -j$(nproc)
arm-linux-gnueabi-strip dropbearmulti

cp dropbearmulti /path/to/base/root/bin/dropbearmulti
```

`localoptions.h` lives next to this README and is the authoritative file for feature selection — override anything in it to change compiled-in features.
