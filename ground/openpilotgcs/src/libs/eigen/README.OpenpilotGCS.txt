To use Eigen in a GCS plugin, just add a relative INCLUDEPATH
directive to that plugin's project file. ex:
INCLUDPATH += ../../libs/eigen

Eigen is a header-only template library.  It is included because noone
aught to be reinventing their own linear algebra library.

Version contained here is latest stable 3.2.0 release from
http://bitbucket.org/eigen/eigen/get/3.2.0.tar.bz2

