#!/bin/bash

usage() {
  echo "usage: ./make-world.sh"
  echo "       for example"
  echo "       sudo ./make-world.sh"
}

dbg() {
  echo $1
}

RTFS_DIR=rootfs-bootstrap
RTFS_BAK_DIR=rootfs-bootstrap.bak

BT_PKGLIST=debian-6.0-bootstrap-list.txt

BT_DEB_DIR=deb-bootstrap
BT_DEB_STORE_DIR=deb-store-bootstrap

# Check all files exist.

# Clean up rootfs environment.
rm -rf $RTFS_BAK_DIR
[ -d $RTFS_DIR ] && mv -f $RTFS_DIR $RTFS_BAK_DIR
#rm -rf /var/cache/apt/archives/*

# Copy prebuilt deb for bootstrap rootfs
echo "cp -af $BT_DEB_DIR/*.deb $BT_DEB_STORE_DIR/"
cp -af $BT_DEB_DIR/*.deb $BT_DEB_STORE_DIR/

# Fetch bootstrap rootfs environment needed packages.
echo "fetch-deb.sh $BT_PKGLIST $BT_DEB_STORE_DIR"
./fetch-deb.sh $BT_PKGLIST $BT_DEB_STORE_DIR || exit -1

# Build bootstrap rootfs environment.
echo "build-rootfs.sh $BT_PKGLIST $BT_DEB_STORE_DIR $RTFS_DIR"
./build-rootfs.sh $BT_PKGLIST $BT_DEB_STORE_DIR $RTFS_DIR || exit -1

# Setup bootsrap rootfs environment
echo "tar -zvxf $BT_DEB_DIR/busybox-links.tar.gz -C $RTFS_DIR"
tar -zvxf $BT_DEB_DIR/busybox-links.tar.gz -C $RTFS_DIR

