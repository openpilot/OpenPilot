#ifndef MOUSEWHEELZOOMTYPE_H
#define MOUSEWHEELZOOMTYPE_H

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
};

#endif // MOUSEWHEELZOOMTYPE_H
