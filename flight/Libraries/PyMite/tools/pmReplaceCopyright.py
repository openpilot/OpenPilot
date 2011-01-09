# This file is Copyright 2009, 2010 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
#
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING up one directory from this.

## @file
#  @copydoc pmReplaceCopyright

## @package pmReplaceCopyright
#  @brief This program naively replaces the copyright header of a c, h or py file.
#
#  Once this program is used on the source tree, it will no longer work
#  because the copyright header will have changed size.


import os, sys


PoaC_COPYRIGHT = """# This file is Copyright 2003, 2006, 2007, 2009 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
#
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING up one directory from this.
"""

PM_COPYRIGHT = """# This file is Copyright 2003, 2006, 2007, 2009 Dean Hall.
#
# This file is part of the PyMite VM.
# The PyMite VM is free software: you can redistribute it and/or modify
# it under the terms of the GNU GENERAL PUBLIC LICENSE Version 2.
#
# The PyMite VM is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU GENERAL PUBLIC LICENSE Version 2
# is seen in the file COPYING in this directory.
"""

COPYRIGHT = PM_COPYRIGHT


def replace_copyright(fn):
    ext = os.path.splitext(fn)[1]
    if ext == ".c" or ext == ".h" or ".c." in fn:
        lines = open(fn, 'rb').readlines()
        if lines[0] == "/*\n" and lines[5] == " */\n":
            lines[0:6] = ("/*\n", COPYRIGHT, "*/\n")
        else:
            print "Did not modify %s" % fn

    elif ext == ".py":
        lines = open(fn, 'rb').readlines()
        if lines[0] == "#\n" and lines[5] == "#\n":
            lines[0:6] = (COPYRIGHT, )
        else:
            print "Did not modify %s" % fn

    open(fn, 'wb').writelines(lines)


if __name__ == "__main__":
    map(replace_copyright, sys.argv[1:])
