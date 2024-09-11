#!/usr/bin/env python3
#
# $ GST_PLUGIN_PATH=./ gst-launch-1.0 pyvideotestsrc num-buffers=50 ! ximagesink

import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib, GObject

Gst.init(None)

ANY_SRC_TEMPLATE = Gst.PadTemplate.new("src",
                                       Gst.PadDirection.SRC,
                                       Gst.PadPresence.ALWAYS,
                                       Gst.caps_from_string("ANY"))
DEFAULT_NUM_BUFFER = -1

class PyTestSrc(Gst.Bin):
    '''
    '''
    GST_PLUGIN_NAME = "pyvideotestsrc"
    __gstmetadata__ = ("videotestsrc in python",
                       "Source",
                       "videotestsrc element",
                       "Jason Kim")
    __gproperties__ = {
        "num-buffers": (int,
                        "num-buffers",
                        "Number of Buffers to produce",
                        -1, # unlimited
                        GLib.MAXINT,
                        DEFAULT_NUM_BUFFER,
                        GObject.ParamFlags.READWRITE)
    }
    __gsttemplates__ =(ANY_SRC_TEMPLATE)

    def __init__(self):
        super(PyTestSrc, self).__init__()
        self._videotestsrc = Gst.ElementFactory.make("videotestsrc", "source")
        self._videotestsrc.set_property("num-buffers", DEFAULT_NUM_BUFFER)
        self.add(self._videotestsrc)

        self.add_pad(
            Gst.GhostPad.new_from_template(
                "src",
                self._videotestsrc.get_static_pad("src"),
                ANY_SRC_TEMPLATE
            ))

    def do_get_property(self, prop):
        if prop.name == "num-buffers":
            return self._videotestsrc.get_property(prop.name)
        else:
            raise AttributeError('unknown property %s' % prop.name)

    def do_set_property(self, prop, value):
        if prop.name == "num-buffers":
            return self._videotestsrc.set_property(prop.name, value)
        else:
            raise AttributeError('unknown property %s' % prop.name)


GObject.type_register(PyTestSrc)
__gstelementfactory__ = (PyTestSrc.GST_PLUGIN_NAME, Gst.Rank.NONE, PyTestSrc)