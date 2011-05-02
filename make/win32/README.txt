This set of scripts is to provide a unix-like build environment on Windows.
---------------------------------------------------------------------------

Why do I need that?
-------------------
It allows to use the "Big Hammer", that is, to build whole OpenPilot system
with a single command "make all" using the top level Makefile originally
written for Linux and Mac only.

Also any routine task automation could use the same set of scripts and
commands on all platforms (Linux, Mac and Windows) if scripts are written in
the shell language. It is particularly important for cross-paltform projects
like the OpenPilot.


How to install this?
--------------------
Fortunately, it requires only few small text files since all others
components should already be installed on your system as parts of msysGit,
QtSDK and CodeSourcery G++ packages required to build the OpenPilot.

It is expected that you have the following tools installed into the listed
locations (but any other locations are fine as well):

 - Python            in C:\Python27
 - CodeSourcery G++  in C:\CodeSourcery
 - QtSDK             in C:\Qt\2010.05
 - msysGit           in C:\Program Files\Git

Also it is assumed that you have the C:\Program Files\Git\cmd\ directory in
the PATH. Usually this is the case for msysGit installation if you have
chosen the 2nd option: put only git and gitk in the PATH (it is recommended
option).

Now you need to copy two files to your msysGit installation folders.
Assuming that you installed the msysGit into C:\Program Files\Git\,
you have to copy:

   make\win32\make    ->  C:\Program Files\Git\bin\
   make\win32\sh.cmd  ->  C:\Program Files\Git\cmd\

If you have msysGit installed into another directory, you need to update
paths accordingly. Also if you have tools installed into different
directories and they are not in the PATH, then you may want to update paths
in the sh.cmd script too (it is self-documented).


How to use it?
--------------

Interactive mode:

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

Note the current git branch in parentheses (master), if it exists.
The path format is also printed according to MSYS notation. And you
have to use forward slashes in paths, not backslashes.

3) Enter make command with required options and target list:

   user@pc /d/Work/OpenPilot/git (master)
   $ make all

The building should be started, and you will have full system including
ground software and flight firmware built in the end.

4) To build parts of the system you can use, for example, such commands:

   user@pc /d/Work/OpenPilot/git (master)
   $ make -j2 USE_BOOTLOADER=YES GCS_BUIL_CONF=release gcs coptercontrol bl_coptercontrol

or to completely remove the build directory:

   user@pc /d/Work/OpenPilot/git (master)
   $ make all_clean


Batch mode:

1) Create a shell script file containing all required commands, for instance:

   #!/bin/sh
   # This is the cc_make_release.sh file used to build CC release software
   cd D:/Work/OpenPilot/git
   make -j2 USE_BOOTLOADER=YES GCS_BUIL_CONF=release gcs coptercontrol bl_coptercontrol
   echo RC=$?

2) Run it typing:

   C:\> sh cc_make_release.sh

3) Of course, a lot of other shell commands can be used in scripts.
