#!/usr/bin/env python

# This file is Copyright 2007, 2009, 2010 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
#
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING in this directory.

"""
==================
Interactive PyMite
==================

An interactive command line that runs on a host computer that is connected
to a target device that is running PyMite.  The host computer compiles the
interactive statement and converts it to a form that PyMite can handle,
sends that over the connection where the target device loads and interprets it.
The target device then packages any result, sends it to the host computer
and the host computer prints the result.
"""

## @file
#  @copybrief ipm_host

## @package ipm_host
#  @brief Interactive PyMite
#
#
#  An interactive command line that runs on a host computer that is connected
#  to a target device that is running PyMite.  The host computer compiles the
#  interactive statement and converts it to a form that PyMite can handle,
#  sends that over the connection where the target device loads and interprets it.
#  The target device then packages any result, sends it to the host computer
#  and the host computer prints the result.

import cmd, code, dis, getopt, os, subprocess, sys
import pmImgCreator


__usage__ = """USAGE:
    ipm.py -f pmfeaturesfilename -[d|s /dev/tty] --[desktop | serial=/dev/tty [baud=19200]]

    -h          Prints this usage message.
    --help

    -f <fn>     Specify the file containing the PM_FEATURES dict to use
    -d          Specifies a desktop connection; uses pipes to send/receive bytes
    --desktop   to/from the target, which is the vm also running on the desktop.
                ipm spawns the vm and runs ipm-desktop as a subprocess.

    -s <port> [<baud>] Specifies the port (device) for a serial connection.
                <port> resembles `com5` on Win32 or `/dev/cu.usbmodem1912`.
                Optional argument, <baud>, defaults to 19200.

    --serial=<port> Specifies the port (device) for a serial connection.

    --baud=<baud>   Specifies the baud rate for a serial connection.

REQUIREMENTS:

    - pySerial package from http://pyserial.sourceforge.net/
    """

NEED_PYSERIAL = "Install the pySerial module from http://pyserial.sourceforge.net/"
if not sys.platform.lower().startswith("win"):
    PMVM_EXE = "../platform/desktop/main.out"
else:
    PMVM_EXE = "../platform/windows/main.exe"
IPM_PROMPT = "ipm> "
IPM_PROMPT2 = ".... "
COMPILE_FN = "<ipm>"
COMPILE_MODE = "single"
INIT_MESSAGE = """Python-on-a-Chip is Copyright 2003, 2006, 2007, 2009, 2010 Dean Hall and others.
Python-on-a-Chip is licensed under the GNU LESSER GENERAL PUBLIC LICENSE V 2.1
PyMite is Copyright 2003, 2006, 2007, 2009, 2010 Dean Hall.
PyMite is licensed under the GNU GENERAL PUBLIC LICENSE V 2.
This software is offered with NO WARRANTY.  See LICENSE for details.
"""
HELP_MESSAGE = """Type the Python code that you want to run on the target device.
If you see no prompt, type two consecutive returns to exit multiline mode.
Type Ctrl+C to interrupt and Ctrl+D to quit (or Ctrl+Z <enter> on Win32).
"""

REPLY_TERMINATOR = '\x04'

if sys.platform.lower().startswith("win"):
    EOF_KEY = 'Z'
else:
    EOF_KEY = 'D'


class Connection(object):
    def open(self,): raise NotImplementedError
    def read(self,): raise NotImplementedError
    def write(self, msg): raise NotImplementedError
    def close(self,): raise NotImplementedError


class PipeConnection(Connection):
    """Provides ipm-host to target connection over stdio pipes on the desktop.
    This connection should work on any POSIX-compliant OS.
    The ipm-device must be spawned as a subprocess
    (the executable created when PyMite was built with PLATFORM=desktop).
    """
    def __init__(self, target=PMVM_EXE):
        self.open(target)


    def open(self, target):
        self.child = subprocess.Popen(target,
                                      bufsize=-1,
                                      stdin=subprocess.PIPE,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE,
                                      )


    def read(self,):
        # If the child process is not alive, read in everything from the buffer.
        # It will usually be an exception message from the target
        # TODO

        # Collect all characters up to and including the ipm reply terminator
        chars = []
        c = ''
        while c != REPLY_TERMINATOR:
            c = self.child.stdout.read(1)
            if c == '':
                # DEBUG: uncomment the next line to print the child's return val
                #print "DEBUG: child returncode = %s\n" % hex(self.child.poll())
                break
            chars.append(c)
        msg = "".join(chars)
        return msg


    def write(self, msg):
        self.child.stdin.write(msg)
        self.child.stdin.flush()


    def close(self,):
        self.write("\0")


class SerialConnection(Connection):
    """Provides ipm-host to target connection over a serial device.
    This connection should work on any platform that PySerial supports.
    The ipm-device must be running at the same baud rate (19200 default).
    """

    def __init__(self, serdev="/dev/cu.SLAB_USBtoUART", baud=19200):
        try:
            import serial
        except Exception, e:
            print NEED_PYSERIAL
            raise e

        self.s = serial.Serial(serdev, baud)
        self.s.setTimeout(4)


    def read(self,):
        # Collect all characters up to and including the ipm reply terminator
        # Issue #110 Readline with eol is not available on all platforms
        # return self.s.readline(eol=REPLY_TERMINATOR)
        b = bytearray()
        c = None
        while c != REPLY_TERMINATOR:
            c = self.s.read(1)
            if len(c) == 0:
                break
            b.append(c)
        return str(b)


    def write(self, msg):
        self.s.write(msg)
        self.s.flush()


    def close(self,):
        self.s.close()


class Interactive(cmd.Cmd):
    """The interactive command line parser accepts typed input line-by-line.
    If a statement requires multiple lines to complete,  the input
    is collected until two sequential end-of-line characters are received.
    """
    ipmcommands = ("?", "help", "load",)


    def __init__(self, conn, pmfn):
        cmd.Cmd.__init__(self,)
        self.prompt = IPM_PROMPT
        self.conn = conn
        self.pic = pmImgCreator.PmImgCreator(pmfn)


    def do_help(self, *args):
        """Prints the help message.
        """
        self.stdout.write(HELP_MESSAGE)


    def do_load(self, *args):
        """Loads a module from the host to the target device.
        """

        # Ensure the filename arg names a python source file
        fn = args[0]
        if not os.path.exists(fn):
            self.stdout.write('File "%s" does not exist in %s.\n'
                              % (fn, os.getcwd()))
            return
        if not fn.endswith(".py"):
            self.stdout.write('Error using "load <module>": '
                              'module must be a ".py" source file.\n')
            return

        src = open(fn).read()
        code = compile(src, fn, "exec")

        img = self.pic.co_to_str(code)

        self.conn.write(img)
        self.stdout.write(self.conn.read())


    def onecmd(self, line):
        """Gathers one interactive line of input (gets more lines as needed).
        """
        # Ignore empty line, continue interactive prompt
        if not line:
            return

        # Handle ctrl+D (End Of File) input, stop interactive prompt
        if line == "EOF":
            self.conn.close()

            # Do this so OS prompt is on a new line
            self.stdout.write("\n")

            # Quit the run loop
            self.stop = True
            return True

        # Handle ipm-specific commands
        if line.split()[0] in Interactive.ipmcommands:
            cmd.Cmd.onecmd(self, line)
            return

        # Gather input from the interactive line
        try:
            codeobj = code.compile_command(line, COMPILE_FN, COMPILE_MODE)

            # If the line was incomplete, get more input and try to compile it
            if not codeobj:

                # Restore the newline chopped by cmd.py:140
                line += "\n"

                while not line.endswith("\n\n") or not codeobj:
                    self.stdout.write(IPM_PROMPT2)
                    line += self.stdin.readline()
                    codeobj = code.compile_command(line,
                                                   COMPILE_FN,
                                                   COMPILE_MODE)

        except Exception, e:
            self.stdout.write("%s:%s\n" % (e.__class__.__name__, e))
            return

        # DEBUG: Uncomment the next line to print the statement's bytecodes
        #dis.disco(codeobj)

        # Convert to a code image
        try:
            codeimg = self.pic.co_to_str(codeobj)

        # Print any conversion errors
        except Exception, e:
            self.stdout.write("%s:%s\n" % (e.__class__.__name__, e))

        # Otherwise send the image and print the reply
        else:

            # DEBUG: Uncomment the next line to print the size of the code image
            # print "DEBUG: len(codeimg) = ", len(codeimg)
            # DEBUG: Uncomment the next line to print the code image
            # print "DEBUG: codeimg = ", repr(codeimg)

            try:
                self.conn.write(codeimg)
            except Exception, e:
                self.stdout.write(
                    "Connection write error, type Ctrl+%s to quit.\n" % EOF_KEY)

            rv = self.conn.read()
            if rv == '':
                self.stdout.write(
                    "Connection read error, type Ctrl+%s to quit.\n" % EOF_KEY)
            else:
                if rv.endswith(REPLY_TERMINATOR):
                    self.stdout.write(rv[:-1])
                else:
                    self.stdout.write(rv)


    def run(self,):
        """Runs the command loop and handles keyboard interrupts (ctrl+C).
        The command loop is what calls self.onecmd().
        """

        print INIT_MESSAGE,
        print HELP_MESSAGE,

        self.stop = False
        while not self.stop:
            try:
                self.cmdloop()
            except KeyboardInterrupt, ki:
                print "\n", ki.__class__.__name__
                # TODO: check connection?


def parse_cmdline():
    """Parses the command line for options.
    """
    baud = 19200
    Conn = PipeConnection
    serdev = None

    try:
        opts, args = getopt.getopt(sys.argv[1:], "dhsf:",
            ["desktop", "help", "serial=", "baud="])
    except Exception, e:
        print __usage__
        sys.exit()

    if not opts:
        print __usage__
        sys.exit()

    for opt in opts:
        if opt[0] == "-d" or opt[0] == "--desktop":
            Conn = PipeConnection
        elif opt[0] == "-s":
            Conn = SerialConnection
            serdev = args[0]
            if len(args) > 1:
                baud = int(args[1])
        elif opt[0] == "--serial":
            Conn = SerialConnection
            serdev = opt[1]
        elif opt[0] == "--baud":
            assert serdev, "--serial must be specified before --baud."
            baud = int(opt[1])
        elif opt[0] == "-f":
            pmfeatures_filename = opt[1]
        else:
            print __usage__
            sys.exit(0)

    if Conn == SerialConnection:
        c = Conn(serdev, baud)
    else:
        c = Conn()

    return (c, pmfeatures_filename)


def main():
    conn, pmfeatures_filename = parse_cmdline()
    i = Interactive(conn, pmfeatures_filename)
    i.run()


def ser_test():
    """Test ipm over serial connection directly.
    """
    try:
        import serial
    except Exception, e:
        print NEED_PYSERIAL
        raise e

    pic = pmImgCreator.PmImgCreator("../platform/desktop/pmfeatures.py")
    serconn = serial.Serial("/dev/cu.SLAB_USBtoUART", 19200)
    serconn.setTimeout(2)

    testcode = (
        'print "Hello"\n',
        'import sys\n',
        'print sys.heap()\n',
        )

    for line in testcode:
        print "compiling ``%s``" % line
        codeobj = compile(line, COMPILE_FN, COMPILE_MODE)
        codeimg = pic.co_to_str(codeobj)
        print "codeimg is %d bytes" % len(codeimg)
        print "sending codeimg..."
        serconn.write(codeimg)
        reply = serconn.readline(eol=REPLY_TERMINATOR)
        print "reply is %d bytes" % len(reply)
        print "reply is:\n%s" % reply


if __name__ == "__main__":
    main()
