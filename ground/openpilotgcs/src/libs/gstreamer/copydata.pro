equals(copydata, 1) {

    win32 {

            #gst-inspect-1.0.exe \
            #gst-launch-1.0.exe \

        # copy core DLLs
        # libstdc++-6.dll must be the one provided by MinGW
        # libwinpthread-1.dll is provided by MinGW
        DLLS = \
            liba52-0.dll \
            libffi-6.dll \
            libglib-2.0-0.dll \
            libgio-2.0-0.dll \
            libgmodule-2.0-0.dll \
            libgobject-2.0-0.dll \
            libgstreamer-1.0-0.dll \
            libintl-8.dll \
            libxml2-2.dll \
            libz.dll

        # copy DLLs needed by plugins
        DLLS += \
            libgstapp-1.0-0.dll \
            libgstbase-1.0-0.dll \
            libgstcodecparsers-1.0-0.dll \
            libgstcontroller-1.0-0.dll \
            libgstpbutils-1.0-0.dll \
            libgsttag-1.0-0.dll \
            libgstvideo-1.0-0.dll \
            libgstaudio-1.0-0.dll \
            libgstfft-1.0-0.dll \
            libgstnet-1.0-0.dll \
            libgstrtp-1.0-0.dll \
            libgstrtsp-1.0-0.dll \
            libgstsdp-1.0-0.dll \
            liborc-0.4-0.dll \
            libcairo-2.dll \
            libfontconfig-1.dll \
            libexpat-1.dll \
            libfreetype-6.dll \
            libpng16-16.dll \
            libpixman-1-0.dll \
            libpango-1.0-0.dll \
            libpangocairo-1.0-0.dll \
            libpangoft2-1.0-0.dll \
            libpangowin32-1.0-0.dll \
            libharfbuzz-0.dll \
            libbz2.dll \
            libgcc_s_sjlj-1.dll

        for(dll, DLLS) {
            addCopyFileTarget($${dll},$$(GSTREAMER_SDK_DIR)/bin,$${GCS_APP_PATH})
        }

        # copy plugin DLLs
        DLLS = \
            libgstapp.dll \
            libgstplayback.dll \
            libgstautoconvert.dll \
            libgstautodetect.dll \
            libgstcoreelements.dll \
            libgstdeinterlace.dll \
            libgstrawparse.dll \
            libgstvideomixer.dll \
            libgstvideoparsersbad.dll \
            libgstvideorate.dll \
            libgstvideoconvert.dll \
            libgstvideoscale.dll \
            libgstvideotestsrc.dll \
            libgstpango.dll \
            libgstdebugutilsbad.dll \
            libgstd3dvideosink.dll \
            #libgstwinks.dll \
            libgstwinscreencap.dll \
            libgstaudiotestsrc.dll \
            libgstaudiovisualizers.dll \
            libgstdirectsoundsrc.dll \
            libgstdirectsoundsink.dll \
            libgstrtsp.dll \
            libgstudp.dll \
            libgsttcp.dll \
            libgstrtpmanager.dll \
            libgstrtp.dll \
            libgstlibav.dll

        for(dll, DLLS) {
            addCopyFileTarget($${dll},$$(GSTREAMER_SDK_DIR)/lib/gstreamer-1.0,$${GCS_LIBRARY_PATH}/gstreamer-1.0)
        }

    }

}
