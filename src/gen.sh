#!/bin/sh

uic-qt4 AddShapeDialog.ui > AddShapeDialog.h
moc-qt4 ../include/qdisplay/ImageView.hpp > ImageView.moc
moc-qt4 ../include/qdisplay/Viewer.hpp > Viewer.moc
