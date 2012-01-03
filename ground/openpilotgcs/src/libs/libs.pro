TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS   = \
    qscispinbox\
    qtconcurrent \
    aggregation \
    extensionsystem \
    utils \
    opmapcontrol \
    qwt \
    qextserialport \
    libqxt

!android-g++{
    SUBDIRS += glc_lib \
    sdlgamepad
}
SUBDIRS +=
