#!/bin/sh

TMP_DIR=../nemo-qml-plugin-folderlistmodel-regression-test-build

FOLDERLISTMODEL_DIR=`dirname $0`

isAbsolutePath=`echo $FOLDERLISTMODEL_DIR |  grep "^/"`

if [ "$isAbsolutePath" = "" ]
then
   FOLDERLISTMODEL_DIR="`pwd`/$FOLDERLISTMODEL_DIR"
fi

mkdir -p $TMP_DIR
cd    $TMP_DIR

qmake $CONFIG $FOLDERLISTMODEL_DIR/test/regression/regression_folderlilstmodel.pro

make clean && make check
