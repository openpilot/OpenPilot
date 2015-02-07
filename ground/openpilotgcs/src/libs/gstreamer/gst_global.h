/**
 * This file is part of gst_lib.
 *
 * gst_lib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gst_lib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**********************************************************************/
#ifndef GST_GLOBAL_H
#define GST_GLOBAL_H

/**********************************************************************/
#include <QtCore/qglobal.h>
#include <QString>

/**********************************************************************/
#if defined(GST_LIB_LIBRARY)
#  define GST_LIB_EXPORT Q_DECL_EXPORT
#else
#  define GST_LIB_EXPORT Q_DECL_IMPORT
#endif

namespace gst {
GST_LIB_EXPORT void init(int *argc, char **argv[]);
GST_LIB_EXPORT QString version();

GST_LIB_EXPORT QList<QString> pluginList();
GST_LIB_EXPORT QList<QString> elementList(QString pluginName);
}

/**********************************************************************/
#endif // GST_GLOBAL_H
