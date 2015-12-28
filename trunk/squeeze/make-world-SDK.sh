#!/bin/bash

usage() {
  echo "usage: ./make-world.sh"
  echo "       for example"
  echo "       sudo ./make-world.sh"
}

dbg() {
  echo $1
}

RTFS_DIR=rootfs-SDK
RTFS_BAK_DIR=rootfs-SDK.bak

SDK_PKGLIST=debian-6.0-SDK-list.txt

SDK_DEB_DIR=deb-SDK
SDK_DEB_STORE_DIR=deb-store-SDK

# Check all files exist.

# Clean up rootfs environment.
rm -rf $RTFS_BAK_DIR
[ -d $RTFS_DIR ] && mv -f $RTFS_DIR $RTFS_BAK_DIR
#rm -rf /var/cache/apt/archives/*

# Copy prebuilt deb for SDK rootfs
echo "cp -af $SDK_DEB_DIR/*.deb $SDK_DEB_STORE_DIR/"
cp -af $SDK_DEB_DIR/*.deb $SDK_DEB_STORE_DIR/

# Fetch SDK rootfs environment needed packages.
echo "fetch-deb.sh $SDK_PKGLIST $SDK_DEB_STORE_DIR"
./fetch-deb.sh $SDK_PKGLIST $SDK_DEB_STORE_DIR || exit -1

# Build SDK rootfs environment.
echo "build-rootfs.sh $SDK_PKGLIST $SDK_DEB_STORE_DIR $RTFS_DIR"
./build-rootfs.sh $SDK_PKGLIST $SDK_DEB_STORE_DIR $RTFS_DIR || exit -1

# Setup SDK rootfs environment
echo "tar -zvxf $SDK_DEB_DIR/busybox-links.tar.gz -C $RTFS_DIR"
tar -zvxf $SDK_DEB_DIR/busybox-links.tar.gz -C $RTFS_DIR

ln -sf bash $RTFS_DIR/bin/sh
ln -sf gawk $RTFS_DIR/usr/bin/awk
ln -sf aclocal-1.11 $RTFS_DIR/usr/bin/aclocal
ln -sf automake-1.11 $RTFS_DIR/usr/bin/automake
ln -sf fakeroot-sysv $RTFS_DIR/usr/bin/fakeroot
ln -sf python2.6 $RTFS_DIR/usr/bin/python

mkdir -p $RTFS_DIR/dev
mkdir -p $RTFS_DIR/proc
mkdir -p $RTFS_DIR/sys
mkdir -p $RTFS_DIR/tmp
mkdir -p $RTFS_DIR/var
mkdir -p $RTFS_DIR/var/log
mkdir -p $RTFS_DIR/var/lib
mkdir -p $RTFS_DIR/var/run

rm -rf $RTFS_DIR/var/mail
rm -rf $RTFS_DIR/etc/init.d/.legacy-bootordering

cp -af ./files/* $RTFS_DIR/
cp -af $SDK_DEB_STORE_DIR $RTFS_DIR/

chroot $RTFS_DIR /bin/ash
