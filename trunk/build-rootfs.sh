#!/bin/bash

PACKAGE_LIST=$1
ROOTFS_DIR=$2

usage() {
  echo "usage: ./build-rootfs.sh <package list file> <rootfs directory>"
  echo "       for example"
  echo "       ./build-rootfs.sh deb-list.txt rootfs"
}

dbg() {
  echo $1
}

if [ "x$PACKAGE_LIST" = "x" ] ; then
  usage
  exit -1
fi

if [ "x$ROOTFS_DIR" = "x" ] ; then
  usage
  exit -1
fi

do_build_rootfs() {
  while read LINE
  do
    echo $LINE
    if [ "x$LINE" = "x" ] ; then
      echo "LINE is empty."
      continue;
    fi
    FIRST_CHAR=${LINE:0:1}
    if [ $FIRST_CHAR = "#" ] ; then
      echo "it's a comment"
    elif [ $FIRST_CHAR = "&" ] ; then
      echo "it's a concated string"
      for PKG in $LINE
      do
        if [ $PKG = "&" ] ; then
          echo "first & symbol, skip it"
        else
          dbg "dpkg -x $PKG\_*.deb $ROOTFS_DIR"
          dpkg -x $PKG\_*.deb $ROOTFS_DIR
        fi
      done
    else
      dbg "dpkg -x $LINE\_*.deb $ROOTFS_DIR"
      dpkg -x $LINE\_*.deb $ROOTFS_DIR
    fi
  done < $PACKAGE_LIST
}

do_build_rootfs

