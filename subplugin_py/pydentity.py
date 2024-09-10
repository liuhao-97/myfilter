#!/usr/bin/env python3
#
# $ GST_PLUGIN_PATH=./ GST_DEBUG=python:4 gst-launch-1.0 videotestsrc  ! pydentity ! ximagesink

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstBase', '1.0')

from gi.repository import Gst, GObject, GstBase
Gst.init(None)

ANY_SRC_TEMPLATE = Gst.PadTemplate.new("src",
                                       Gst.PadDirection.SRC,
                                       Gst.PadPresence.ALWAYS,
                                       Gst.Caps.from_string("ANY"))
ANY_SINK_TEMPLATE = Gst.PadTemplate.new("sink",
                                        Gst.PadDirection.SINK,
                                        Gst.PadPresence.ALWAYS,
                                       Gst.Caps.from_string("ANY"))
#
# Simple Identity in python
#
class Pydentity(GstBase.BaseTransform):
    GST_PLUGIN_NAME = "pydentity"
    __gstmetadata__ = ('identity in python',
                       'Generic',
                       'python identity element',
                       'Jason Kim')
    __gproperties__ = {}
    __gsttemplates__ = (ANY_SRC_TEMPLATE, ANY_SINK_TEMPLATE)

    def __init__(self):
        super().__init__()
        self.cnt = 0

    def do_transform_ip(self, buffer):
        if self.cnt == 40:
            Gst.info("buffer.pts: %s" % (Gst.TIME_ARGS(buffer.pts)))
            self.cnt = 0

        self.cnt += 1

        return Gst.FlowReturn.OK

GObject.type_register(Pydentity)
__gstelementfactory__ = (Pydentity.GST_PLUGIN_NAME, Gst.Rank.NONE, Pydentity)