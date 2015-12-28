#!/bin/sh

usage() {
  echo "usage: ./prepare-base-files.sh"
}

dbg() {
  echo $1
}

mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t tmpfs udev /dev

mknod /dev/console -m 0600 c 5 1
mknod /dev/null -m 0666 c 1 3
mknod /dev/zero -m 0666 c 1 5
mknod /dev/tty -m 0666 c 5 0
mknod /dev/tty0 -m 0660 c 4 0
mknod /dev/tty1 -m 0660 c 4 1
mknod /dev/random -m 0666 c 1 8
mknod /dev/urandom -m 0666 c 1 9

mkdir /dev/pts -m 0755
mkdir /dev/shm -m 0755

