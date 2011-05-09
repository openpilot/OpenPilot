This set of scripts is to provide a unix-like build environment on Windows.
---------------------------------------------------------------------------

1. Why do I need it?
2. How to install?
3. How to use it?
3.1. Interactive mode
3.2. Batch mode
4. Advanced usage
5. Limitations of use
6. Credits and license


1. Why do I need it?
--------------------
It allows to use the "Big Hammer", that is, to build whole OpenPilot system
with a single command "make all" using the top level Makefile originally
written for Linux and Mac only.

Also any routine task automation could use the same set of scripts and commands
on all platforms (Linux, Mac and Windows) if scripts are written in the shell
language. It is particularly important for cross-paltform projects like the
OpenPilot.


2. How to install?
------------------
Fortunately, it requires only few small text files since all others components
should already be installed on your system as parts of msysGit, QtSDK and
CodeSourcery G++ packages required to build the OpenPilot.

It is expected that you have the following tools installed into the listed
locations (but any other locations are fine as well):

 - Python            in C:\Python27
 - CodeSourcery G++  in C:\CodeSourcery
 - QtSDK             in C:\Qt\2010.05
 - msysGit           in %ProgramFiles%\Git
 - Unicode NSIS      in %ProgramFiles%\NSIS\Unicode

Also it is assumed that you have the C:\Program Files\Git\cmd\ directory in
the PATH. Usually this is the case for msysGit installation if you have chosen
the 2nd option: put only git and gitk in the PATH (it is recommended option).

Now you need to copy two files to your msysGit installation folders.
Assuming that you installed the msysGit into C:\Program Files\Git\,
you have to copy:

   make\winx86\make    ->  C:\Program Files\Git\bin\
   make\winx86\sh.cmd  ->  C:\Program Files\Git\cmd\

If you have msysGit installed into another directory, you need to update paths
accordingly. Also if you have tools installed into different directories and
they are not in the PATH, then you may want to update paths in the sh.cmd
script too (it is self-documented).


3. How to use it?
-----------------

3.1. Interactive mode
---------------------

1) Type:

   C:\> sh

and the bash prompt should appear:

   user@pc /
   $

2) Enter your OpenPilot directory:

   user@pc /
   $ cd D:/Work/OpenPilot/git

   user@pc /d/Work/OpenPilot/git (master)
   $

Note the current git branch in parentheses (master), if it exists. The path
format is also printed according to MSYS notation. And you have to use forward
slashes in paths, not backslashes.

3) Enter make command with required options and target list:

   user@pc /d/Work/OpenPilot/git (master)
   $ make all

The building should be started, and you will have full system including ground
software and flight firmware built in the end.

4) To build parts of the system you can use, for example, such commands:

   user@pc /d/Work/OpenPilot/git (master)
   $ make -j2 USE_BOOTLOADER=YES GCS_BUIL_CONF=release gcs coptercontrol bl_coptercontrol

or to completely remove the build directory:

   user@pc /d/Work/OpenPilot/git (master)
   $ make all_clean


3.2. Batch mode
---------------

1) Create a shell script file containing all required commands, for instance:

   #!/bin/sh
   # This is the cc_make_release.sh file used to build CC release software
   cd D:/Work/OpenPilot/git
   make -j2 USE_BOOTLOADER=YES GCS_BUIL_CONF=release gcs coptercontrol bl_coptercontrol
   echo RC=$?

2) Run it typing:

   C:\> sh cc_make_release.sh

3) Of course, a lot of other shell commands can be used in scripts.


4. Advanced usage
-----------------

It is possible to go further and integrate shell scripting into Windows system
like any other executables. This allows:

 - double click on any .sh file in the Explorer window to execute it;
 - type name of .sh file with any arguments on the command line to run script;
 - omit .sh extension typing names since it is now recognized automatically;
 - call .sh scripts even from .bat and .cmd files as Windows command;
 - execute scripts which are in any directory in the PATH;
 - return and check exit code from .sh scripts to .bat or .cmd batch files.

In short, you may have quite powerful and cross-platform bash scripting on
Windows.

In order to integrate bash scripting into Windows system you need to:

 - double click on the included shell_script.reg file to register .sh
   extension in the system. Thus, any click on .sh script will execute it
   automatically assuming that the sh.cmd is in the PATH;
 - register .sh extension as an executable file type, so you can omit the
   .sh typing commands. To do so open "My Computer" properties dialog, choose
   the "Advanced" tab, "Environment variables", in the "System variables"
   find the variable called "PATHEXT". It contains the list of "executable"
   file extensions separated by semicolon. You want to add a ";.SH" to the
   end of its value. Then click OK to apply.

Now any .sh script can be run just by typing its name, optionally with
parameters.

As an example, you can create a shell script named make.sh in the cmd/
subdirectory of Git installation with the following content:

   exec /bin/make $*

and then build the OpenPilot software typing

   make all

directly from Windows command line or from a batch file.

You also may want to rename or remove "C:\Program Files\Git\etc\motd" file
to get rid of git bash welcome message on every script invocation.


5. Limitations of use
---------------------

Currently there may be some problems running scripts which contain spaces in
file names or located in directories which contain spaces in full paths.
It results in in strange "file not found" or other errors.

It is recommended to avoid using such names with spaces.


6. Credits and license
----------------------

This set of scripts uses the MSYS package included into the msysGit
distribution and MinGW make included into the QtSDK package.

The sh.cmd, shell_script.reg and this README.txt files were written
by Oleg Semyonov (os-openpilot-org@os-propo.info) for the OpenPilot
project and are licensed under CC-BY-SA terms:

    http://creativecommons.org/licenses/by-sa/3.0/

Feel free to contact me for additions and improvements.

Happy bashing!
