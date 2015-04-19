Tips:
- Measuring video latency : display time on vide + film video output -> the time between two frames is the latency

Limitations:
- It is not possible to view a web cam in two different gadgets (same is *not* true for DirectSound sources)
  but it is not really an issue, as it possible to tee a vide source in the pipeline itself

Issues:
 - The ksvideo source has a bug in 1.4.5 : https://bugzilla.gnome.org/show_bug.cgi?id=734264
   Not sure it has an impact, need to test with older gstreamer version


0:02:23.040677081  4920   50CAE360 ERROR                rtspsrc gstrtspsrc.c:5896:gst_rtspsrc_prepare_transports: failed  to allocate udp ports
"VideoWidget - bus_sync_handler (3704) : rtspsrc0 : error ()"
0:02:23.044302841  4920   50CAE360 ERROR                rtspsrc gstrtspsrc.c:6737:gst_rtspsrc_open_from_sdp:<rtspsrc0> setup failed