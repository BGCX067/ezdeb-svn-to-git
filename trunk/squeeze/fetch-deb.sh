#!/bin/bash

PACKAGE_LIST=$1
STORE_DIR=$2

usage() {
  echo "usage: ./fetch-deb.sh <package list file> <target storage directory>"
  echo "       for example"
  echo "       sudo ./fetch-deb.sh deb-list.txt deb-store"
}

dbg() {
  echo $1
}

if [ "x$PACKAGE_LIST" = "x" -o "x$STORE_DIR" = "x" ] ; then
  usage
  exit -1
fi

# prepare store directory
mkdir -p $STORE_DIR

do_fetch_deb() {
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
          if [ `find $STORE_DIR/ -name ${PKG}_*.deb` ] ; then
            echo "${PKG} exists, skip downloading it"
          else
            dbg "apt-get -d -y --reinstall install $PKG"
            apt-get -d -y --reinstall install $PKG
            cp -af /var/cache/apt/archives/${PKG}_*.deb $STORE_DIR/
          fi
        fi
      done
    else
      if [ `find $STORE_DIR/ -name ${LINE}_*.deb` ] ; then
        echo "${LINE} exists, skip downloading it"
      else
        dbg "apt-get -d -y --reinstall install $LINE"
        apt-get -d -y --reinstall install $LINE
        cp -af /var/cache/apt/archives/${LINE}_*.deb $STORE_DIR
      fi
    fi
  done < $PACKAGE_LIST
}

do_fetch_deb

