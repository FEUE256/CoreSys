#!/usr/bin/env bash
set -e

mkdir -p root/

cp init root/
cp ../../src/lua/lua root/
cp ../../bin/Release/utils/busybox root/

chmod +x root/init root/lua

cd root/
find . -type f | cpio -H newc -o > ../init.cpio
cd ..

gzip -f init.cpio
mv init.cpio.gz gz.gz
