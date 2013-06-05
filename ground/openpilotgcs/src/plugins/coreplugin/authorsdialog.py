#!/usr/bin/env python
#
# Helper function to generate a QML list of contributors
#
# (c) 2013, The OpenPilot Team, http://www.openpilot.org
# See also: The GNU Public License (GPL) Version 3
#

import optparse
import sys

def create_qml_file(args):
    """This function reads input and template files and writes output QML file"""

    assert args.infile is not None
    assert args.outfile is not None
    assert args.template is not None

    with open(args.infile, "rt") as input_file:
        names = input_file.readlines()

    names_list = ""
    for name in names:
        if name.strip():
            names_list += "    ListElement { name: \"" + name.strip() + "\" }\n"

    with open(args.template, "rt") as template_file, open(args.outfile, "wt") as output_file:
        template = template_file.read()
        output_file.write(template.replace("${LIST_ELEMENTS}", names_list.rstrip()))

    return 0

def main():
    """Helper function to generate a QML list of contributors"""

    parser = optparse.OptionParser(description = main.__doc__);

    parser.add_option('--infile', action='store',
                        help='name of input file, one name per line');
    parser.add_option('--outfile', action='store',
                        help='name of output QML file');
    parser.add_option('--template', action='store',
                        help='name of QML template file');
    (args, positional_args) = parser.parse_args()

    if (len(positional_args) != 0) or (len(sys.argv) == 1):
        parser.error("incorrect number of arguments, try --help for help")

    return create_qml_file(args)

if __name__ == "__main__":
    sys.exit(main())
