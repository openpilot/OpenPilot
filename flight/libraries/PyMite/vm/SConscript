import sys


Import("env", "vars")

SOURCES = Glob("*.c")
PMSTDLIB_SOURCES = ["../lib/list.py",
                    "../lib/dict.py",
                    "../lib/__bi.py",
                    "../lib/sys.py",
                    "../lib/string.py",]
if env["IPM"] == True:
    PMSTDLIB_SOURCES.append("../lib/ipm.py")


img_sources = Command(["pmstdlib_img.c", "pmstdlib_nat.c"], [PMSTDLIB_SOURCES],
    "%s src/tools/pmImgCreator.py -f src/platform/%s/pmfeatures.py -c -s " \
    "-o src/vm/pmstdlib_img.c --native-file=src/vm/pmstdlib_nat.c $SOURCES" \
    % (sys.executable, vars.args["PLATFORM"]))
if sys.platform != "win32":
    env['ARFLAGS'] = "rcs"
lib = env.Library("pmvm_%s" % vars.args["PLATFORM"], SOURCES + img_sources)
env.Precious(lib)


Return("lib")
