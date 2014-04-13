equals(copydata, 1) {

    linux {

        # copy useful executables
        GCS_EXES = \
            gst-inspect-0.10 \
            gst-launch-0.10

        for(exe, GCS_EXES) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$(GSTREAMER_SDK_DIR)/bin/$$exe\") \
                $$targetPath(\"$$GCS_APP_PATH/$$lib\") $$addNewline()
        }

        # copy core libraries
        GCS_LIBS = \
            libffi.so.6 \
            libglib-2.0.so \
            libgmodule-2.0.so \
            libgobject-2.0.so \
            libgstreamer-0.10.so \
            libgstinterfaces-0.10.so \
            libxml2.so.2 \
            libz.so

        # copy LIBS needed by plugins
        GCS_LIBS += \
            libgstapp-0.10.so \
            libgstaudio-0.10.so \
            libgstbase-0.10.so \
            libgstcontroller-0.10.so \
            libgstpbutils-0.10.so \
            libgstvideo-0.10.so \
            liborc-0.4.so

        for(lib, GCS_LIBS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$(GSTREAMER_SDK_DIR)/lib/$$lib\") \
                $$targetPath(\"$$GCS_LIBRARY_PATH/$$lib\") $$addNewline()
        }

        # copy plugin LIBS
        GCS_PLUGIN_LIBS = \
            libgstapp.so \
            libgstautoconvert.so \
            libgstautodetect.so \
            libgstcoreelements.so \
            libgstdecodebin2.so \
            libgstdeinterlace.so \
            libgstffmpegcolorspace.so \
            libgstrawparse.so \
            libgstvideomixer.so \
            libgstvideoparsersbad.so \
            libgstvideorate.so \
            libgstvideoscale.so \
            libgstvideotestsrc.so 

        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_BUILD_TREE/$$GCS_LIBRARY_BASENAME/gstreamer-0.10/$$lib\") $$addNewline()
        for(lib, GCS_PLUGIN_LIBS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$(GSTREAMER_SDK_DIR)/lib/gstreamer-0.10/$$lib\") \
                $$targetPath(\"$$GCS_BUILD_TREE/$$GCS_LIBRARY_BASENAME/gstreamer-0.10/$$lib\") $$addNewline()
        }

    }

    win32 {

        # copy core DLLs
        # libstdc++-6.dll must be the one provided by MinGW
        # libwinpthread-1.dll is provided by MinGW
        DLLS = \
            gst-inspect-0.10.exe \
            gst-launch-0.10.exe \
            libffi-6.dll \
            libglib-2.0-0.dll \
            libgmodule-2.0-0.dll \
            libgobject-2.0-0.dll \
            libgstreamer-0.10-0.dll \
            libgstinterfaces-0.10-0.dll \
            libintl-8.dll \
            libxml2-2.dll \
            libz.dll

        # copy DLLs needed by plugins
        DLLS += \
            libgstapp-0.10-0.dll \
            libgstaudio-0.10-0.dll \
            libgstbase-0.10-0.dll \
            libgstcodecparsers-0.10-23.dll \
            libgstcontroller-0.10-0.dll \
            libgstpbutils-0.10-0.dll \
            libgstvideo-0.10-0.dll \
            liborc-0.4-0.dll

        for(dll, DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$(GSTREAMER_SDK_DIR)/bin/$$dll\") \
                $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }

        # copy plugin DLLs
        DLLS = \
            libgstapp.dll \
            libgstautoconvert.dll \
            libgstautodetect.dll \
            libgstcoreelements.dll \
            libgstd3dvideosink.dll \
            libgstdecodebin2.dll \
            libgstdeinterlace.dll \
            libgstdirectdrawsink.dll \
            libgstdirectsoundsink.dll \
            libgstdshowdecwrapper.dll \
            libgstdshowsrcwrapper.dll \
            libgstdshowvideosink.dll \
            libgstffmpegcolorspace.dll \
            libgstrawparse.dll \
            libgstvideomixer.dll \
            libgstvideoparsersbad.dll \
            libgstvideorate.dll \
            libgstvideoscale.dll \
            libgstvideotestsrc.dll \
            libgstwinks.dll \
            libgstwinscreencap.dll

        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_BUILD_TREE/$$GCS_LIBRARY_BASENAME/gstreamer-0.10/$$dll\") $$addNewline()
        for(dll, DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$(GSTREAMER_SDK_DIR)/lib/gstreamer-0.10/$$dll\") \
                $$targetPath(\"$$GCS_BUILD_TREE/$$GCS_LIBRARY_BASENAME/gstreamer-0.10/$$dll\") $$addNewline()
        }

    }

    # add make target
    POST_TARGETDEPS += copydata

    data_copy.target = copydata
    QMAKE_EXTRA_TARGETS += data_copy
}
