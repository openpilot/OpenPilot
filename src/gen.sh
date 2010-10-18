#!/bin/sh

if [ `which moc-qt4 2> /dev/null` ]; then
	UIC_CMD=uic-qt4
	MOC_CMD=moc-qt4
else if [ `which moc-mac 2> /dev/null` ]; then
	UIC_CMD=uic-mac
	MOC_CMD=moc-mac
else if [ `which moc 2> /dev/null` ]; then
	UIC_CMD=uic
	MOC_CMD=moc
else
	echo Missing moc and/or uic
	exit
fi
fi
fi

$UIC_CMD AddShapeDialog.ui > AddShapeDialog.h
$MOC_CMD ../include/qdisplay/ImageView.hpp > ImageView.moc
$MOC_CMD ../include/qdisplay/Viewer.hpp > Viewer.moc
$MOC_CMD ../include/qdisplay/init.hpp > init.moc
