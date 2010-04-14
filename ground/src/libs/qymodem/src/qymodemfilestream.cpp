/**
 ******************************************************************************
 *
 * @file       qymodemsend.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by J.D.Medhurst (a.k.a. Tixy)
 * @brief      Implementation of Class for presenting a file as a input stream.
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   ymodem_lib
 * @{
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*!
*   \section Credits
*   This implementation is based on J.D.Medhurst (a.k.a. Tixy) work from
*   <a href="http://yxit.co.uk">Tixy's source code</a>.
*/

#include "qymodem_tx.h"

/**
Class for presenting a file as a input stream.
*/
class QymodemFileStream : public QymodemTx::InStream
{
public    :
    QymodemFileStream()
        : File(0)
    {
    }


    /**
    Open the file stream.

    @param filename	The name of the file to Open.

    @return Zero if successful, or a negative error value if failed.

    */
    int Open(const char* fileName)
    {
        File = fopen(fileName,"rb");
        if(!File)
            return QymodemTx::ErrorInputStreamError;
        // find file size...
        if(fseek(File,0,SEEK_END))
        {
            fclose(File);
            return QymodemTx::ErrorInputStreamError;
        }
        TotalSize = ftell(File);
        if(fseek(File,0,SEEK_SET))
        {
            fclose(File);
            return QymodemTx::ErrorInputStreamError;
        }
        TransferredSize = 0;
        return 0;
    }

    /**
        Close the stream.
        */
    void Close()
    {
        if(File)
        {
            fclose(File);
            File = 0;
        }
    }

    /**
        Return the size of the file.

        @return File size.
        */
    inline size_t Size()
    {
        return TotalSize;
    }

    /**
        Read data from the stream.

        @param[out] data	Pointer to buffer to hold data read from stream.
        @param size			Maximum size of data to read.

        @return Zero if successful, or a negative error value if failed.
        */
    int In(quint8* data, size_t size)
    {
        percent = TotalSize ? ((quint64)TransferredSize*(quint64)100)/(quint64)TotalSize : 0;
        fflush(stdout);
        size=fread(data,sizeof(quint8),size,File);
        if(size)
        {
            TransferredSize += size;
            return size;
        }
        if(TransferredSize!=TotalSize)
            return QymodemTx::ErrorInputStreamError;
        return 0;
    }
private:
    FILE* File;
    size_t TotalSize;
    size_t TransferredSize;

};
