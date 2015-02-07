equals(copydata, 1) {

    win32 {

        # copy core DLLs
        # libstdc++-6.dll must be the one provided by MinGW
        # libwinpthread-1.dll is provided by MinGW
        DLLS = \
            gst-inspect-1.0.exe \
            gst-launch-1.0.exe \
            libffi-6.dll \
            libglib-2.0-0.dll \
            libgmodule-2.0-0.dll \
            libgobject-2.0-0.dll \
            libgstreamer-1.0-0.dll \
            libgstinterfaces-1.0-0.dll \
            libintl-8.dll \
            libxml2-2.dll \
            libz.dll

        # copy DLLs needed by plugins
        DLLS += \
            libgstapp-1.0-0.dll \
            libgstaudio-1.0-0.dll \
            libgstbase-1.0-0.dll \
            libgstcodecparsers-1.0-23.dll \
            libgstcontroller-1.0-0.dll \
            libgstpbutils-1.0-0.dll \
            libgstvideo-1.0-0.dll \
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

        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_LIBRARY_PATH/gstreamer-1.0/$$dll\") $$addNewline()
        for(dll, DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$(GSTREAMER_SDK_DIR)/lib/gstreamer-1.0/$$dll\") \
                $$targetPath(\"$$GCS_LIBRARY_PATH/gstreamer-1.0/$$dll\") $$addNewline()
        }

    }

    # add make target
    POST_TARGETDEPS += copydata

    data_copy.target = copydata
    QMAKE_EXTRA_TARGETS += data_copy
}
