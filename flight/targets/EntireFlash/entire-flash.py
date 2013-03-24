#!/usr/bin/env python
#
# Helper function to generate an entire flash image.
#
# (c) 2013, The OpenPilot Team, http://www.openpilot.org
# See also: The GNU Public License (GPL) Version 3
#

import optparse
import sys

def append(verbose, text, data):
    """Appends data to global image, updates global offset, prints details if verbose is set"""
    global image, offset
    if verbose:
        print text, "0x%08x" % len(data), "@ 0x%08x" % offset, "%8d bytes" % len(data)
    image  += data
    offset += len(data)

def create_entire_flash(args):
    """Generates entire flash image, prints image info and writes image to a file"""
    global image, offset

    assert args.bl_bank_base is not None
    assert args.bl_bank_size is not None
    assert args.fw_bank_base is not None
    assert args.fw_bank_size is not None
    assert args.fw_desc_size is not None
    assert args.bl_bin_path is not None
    assert args.fw_bin_path is not None
    assert args.fwinfo_bin_path is not None

    image = ""
    offset = 0
    pad = chr(0xff)

    try:
        bl_bin_file = open(args.bl_bin_path, "rb")
        bl_bin = bl_bin_file.read()
        bl_bin_file.close()

        fw_bin_file = open(args.fw_bin_path, "rb")
        fw_bin = fw_bin_file.read()
        fw_bin_file.close()

        fwinfo_bin_file = open(args.fwinfo_bin_path, "rb")
        fwinfo_bin = fwinfo_bin_file.read()
        fwinfo_bin_file.close()

        pre_pad_size  = args.fw_bank_base - args.bl_bank_base - args.bl_bank_size
        post_pad_size = args.fw_bank_size - args.fw_desc_size - len(fw_bin)

        append(args.verbose, "Bootloader image:   ", bl_bin)
        append(args.verbose, "Padding:            ", pad * pre_pad_size)
        append(args.verbose, "Firmware image:     ", fw_bin)
        append(args.verbose, "Padding:            ", pad * post_pad_size)
        append(args.verbose, "Firmware info blob: ", fwinfo_bin)

        if args.verbose:
            print "Entire flash image size:", offset, "bytes"

        if args.outfile:
            of = open(args.outfile, "wb")
            of.write(image)
            of.close()

    except:
        import traceback
        traceback.print_exc()
        return -2

    return 0

def main():
    """Entire image data will be written to OUTFILE (if given) as concatenation of:
 - bootloader + padding + firmware + padding + firmware info
    """

    # Parse command line.
    class RawDescriptionHelpFormatter(optparse.IndentedHelpFormatter):
        """optparse formatter function to pretty print raw epilog"""
        def format_epilog(self, epilog):
            if epilog:
                return "\n" + epilog + "\n"
            else:
                return ""

    parser = optparse.OptionParser(
        formatter=RawDescriptionHelpFormatter(),
        description = "Helper function to generate an entire flash image.",
        epilog = main.__doc__);

    parser.add_option('--bl-bank-base', type='long', action='store',
                        help='start of bootloader flash');
    parser.add_option('--bl-bank-size', type='long', action='store',
                        help='should include BD_INFO region');
    parser.add_option('--fw-bank-base', type='long', action='store',
                        help='start of firmware flash');
    parser.add_option('--fw-bank-size', type='long', action='store',
                        help='should include FW_DESC_SIZE');
    parser.add_option('--fw-desc-size', type='long', action='store',
                        help='firmware description area size');
    parser.add_option('--bl-bin-path', action='store',
                        help='path to bootloader image');
    parser.add_option('--fw-bin-path', action='store',
                        help='path to firmware image');
    parser.add_option('--fwinfo-bin-path', action='store',
                        help='path to firmware info blob');
    parser.add_option('--outfile', action='store',
                        help='name of entire flash output file');
    parser.add_option('--verbose', action='store_true',
                        help='print addresses and sizes');
    (args, positional_args) = parser.parse_args()

    if (len(positional_args) != 0) or (len(sys.argv) == 1):
        parser.error("incorrect number of arguments, try --help for help")

    return create_entire_flash(args)

if __name__ == "__main__":
    sys.exit(main())
