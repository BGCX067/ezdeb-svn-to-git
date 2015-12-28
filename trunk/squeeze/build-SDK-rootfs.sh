#!/bin/bash

PACKAGE_LIST=$1

usage() {
  echo "usage: ./build-SDK-rootfs.sh <package list file>"
  echo "       for example"
  echo "       ./build-SDK-rootfs.sh deb-list.txt"
}

dbg() {
  echo $1
}

if [ "x$PACKAGE_LIST" = "x" ] ; then
  usage
  exit -1
fi

do_prepare_dirs() {
# setup command
  ln -sf busybox /bin/ash
  ln -sf busybox /bin/su
  ln -sf bash /bin/sh
  ln -sf gawk /usr/bin/awk
  ln -sf aclocal-1.11 /usr/bin/aclocal
  ln -sf automake-1.11 /usr/bin/automake
  ln -sf fakeroot-sysv /usr/bin/fakeroot
  ln -sf python2.6 /usr/bin/python
  mkdir -p /dev
  mkdir -p /proc
  mkdir -p /sys
  mkdir -p /tmp
  mkdir -p /var
  mkdir -p /var/log
  mkdir -p /var/lib
  mkdir -p /var/run
  rm -rf /var/mail
  rm -rf /etc/init.d/.legacy-bootordering
}

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
      CMD=""
      echo "it's a concated string"
      for PKG in $LINE
      do
        dbg "PKG=[$PKG]"
        dbg "CMD=[$CMD]"
        if [ $PKG = "&" ] ; then
          echo "first & symbol, skip it"
        else
          CMD=$CMD"$PKG\_*.deb "
          dbg "CMD2=[$CMD]"
        fi
      done
      dbg "dpkg -i $CMD"
      dpkg -i $CMD
    else
      dbg "dpkg -i $LINE\_*.deb"
      dpkg -i $LINE\_*.deb
    fi
  done < $PACKAGE_LIST
}

export TERM=linux-vt
do_prepare_dirs
do_build_rootfs

