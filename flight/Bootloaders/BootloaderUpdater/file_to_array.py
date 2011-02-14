# Convert binary file to a hex encoded array for inclusion in C projects
 
import os
import struct
import sys
import logging
 
class BinToArray:
    def __init__(self):
        pass
 
    def ConvertFileToArray(self, strInFile, strOutFile, integerSize, ignoreBytes, endianNess):
        """ Reads binary file at location strInFile and writes out a C array of hex values
            Parameters - 
                strInFile - Path and filename of binary file to convert
                strOutFile - Path and filename of output. Suggested extension is .c or .cpp
                integerSize - Size in bytes of output array elements. Array generated is always
                    of type uint8, uint16, uint32. These types would need to be defined using
                    typedef if they don't exist, or the user can replace the type name with the
                    appropriate keyword valid for the compiler size conventions
                ignoreBytes - Number of bytes to ignore at the beginning of binary file. Helps
                    strip out file headers and only encode the payload/data.
                endianNess - Only used for integerSize of 2 or 4. 'l' for Little Endian, 'b' for
                    Big Endian
        """
        # Check integerSize value
        if integerSize not in (1, 2, 4):
            logging.debug("Integer Size parameter must be 1, 2 or 4")
            return
        # endif
        # Open input file
        try:
            fileIn = open(strInFile, 'rb')
        except IOError, err:
            logging.debug("Could not open input file %s" % (strInFile))
            return
        # end try
        # Open input file
        try:
            fileOut = open(strOutFile, 'w')
        except IOError, err:
            logging.debug("Could not open output file %s" % (strOutFile))
            return
        # end try
        # Start array definition preamble
        inFileName = os.path.basename(strInFile)
        strVarType = "uint%d_t" % (integerSize * 8)
        fileOut.write("// Array representation of binary file %s\n\n\n" % (inFileName))
        fileOut.write("static %s dataArray[] = {\n" % strVarType)
        # Convert and write array into C file
        fileIn.seek(ignoreBytes)
        if integerSize == 1:
            bufChunk = fileIn.read(20)
            while bufChunk != '':
                fileOut.write("        ")
                for byteVal in bufChunk:
                    fileOut.write("0x%02x, " % ord(byteVal))
                # end for
                fileOut.write("\n")
                bufChunk = fileIn.read(20)
            # end while
        else:
            if   endianNess == 'l' and integerSize == 2:
                endianFormatter = '<H'
                maxWordsPerLine = 10
            elif endianNess == 'l' and integerSize == 4:
                endianFormatter = '<L'
                maxWordsPerLine = 6
            elif endianNess == 'b' and integerSize == 2:
                endianFormatter = '>H'
                maxWordsPerLine = 10
            elif endianNess == 'b' and integerSize == 4:
                endianFormatter = '>L'
                maxWordsPerLine = 6
            # endif
            bufChunk = fileIn.read(integerSize)
            i = 0
            fileOut.write("        ")
            while bufChunk != '':
                wordVal = struct.unpack(endianFormatter, bufChunk)
                if integerSize == 2:
                    fileOut.write("0x%04x, " % wordVal)
                else:
                    fileOut.write("0x%08x, " % wordVal)
                # endif
                i += 1
                if i == maxWordsPerLine:
                    fileOut.write("\n        ")
                    i = 0
                # endif
                bufChunk = fileIn.read(integerSize)
            # end while
        # end if
        # Close array definition
        fileOut.write("\n    };\n")
        fileIn.close()
        fileOut.close()
 
if __name__ == "__main__":
    print(sys.argv[0])
    logging.basicConfig(level=logging.DEBUG)
    converter = BinToArray()
    converter.ConvertFileToArray(
        sys.argv[1], sys.argv[2], 4, 0, 'l')
