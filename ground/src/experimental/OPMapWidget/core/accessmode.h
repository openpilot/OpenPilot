#ifndef ACCESSMODE_H
#define ACCESSMODE_H

struct AccessMode
{

public:
    enum Types
    {
        /// <summary>
        /// access only server
        /// </summary>
        ServerOnly,

        /// <summary>
        /// access first server and caches localy
        /// </summary>
        ServerAndCache,

        /// <summary>
        /// access only cache
        /// </summary>
        CacheOnly,
    };

};
#endif // ACCESSMODE_H
