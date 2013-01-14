#include <QDebug>
#include <gst/gst.h>

#include "gst_global.h"

#include "gst-plugins-bad/sys/winscreencap/gstwinscreencap.h"

static bool initialized = false;

// Not thread safe. Does it need to be?
void gst::init(int *argc, char **argv[])
{
	if (initialized) return;
	initialized = true;

	qDebug() << "gstreamer - initializing";
	gst_init(argc, argv);

	qDebug() << QString("gstreamer - version %0").arg(gst_version_string());

	qDebug() << "gstreamer - registering plugins";
	if (!register_plugin2()) {
		qDebug() << "gstreamer - failed to register plugin";
	}
}

QString gst::version(void)
{
	return QString(gst_version_string());
}
