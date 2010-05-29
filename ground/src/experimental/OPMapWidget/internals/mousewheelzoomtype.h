#ifndef MOUSEWHEELZOOMTYPE_H
#define MOUSEWHEELZOOMTYPE_H
#include <QObject>
#include <QStringList>
#include <QMetaType>
struct MouseWheelZoomType
{
    enum Types
    {
        /// <summary>
        /// zooms map to current mouse position and makes it map center
        /// </summary>
        MousePositionAndCenter,

        /// <summary>
        /// zooms to current mouse position, but doesn't make it map center,
        /// google/bing style ;}
        /// </summary>
        MousePositionWithoutCenter,

        /// <summary>
        /// zooms map to current view center
        /// </summary>
        ViewCenter,
    };
    static QStringList TypesStrList(){return strList;}
    static Types TypeByStr(QString const& value){return (Types)MouseWheelZoomType::strList.indexOf(value);}
private:
    static QStringList strList;
};
Q_DECLARE_METATYPE(MouseWheelZoomType::Types)
#endif // MOUSEWHEELZOOMTYPE_H

