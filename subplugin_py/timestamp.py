#!/usr/bin/env python3
import gi
gi.require_version('Gst', '1.0')
# gi.require_version('GObject', '2.0')
gi.require_version('GstBase', '1.0')

from gi.repository import Gst, GObject, GstBase
import struct
import time

# Initialize GStreamer
Gst.init(None)

ANY_SRC_TEMPLATE = Gst.PadTemplate.new("src",
                                       Gst.PadDirection.SRC,
                                       Gst.PadPresence.ALWAYS,
                                       Gst.Caps.from_string("ANY"))
ANY_SINK_TEMPLATE = Gst.PadTemplate.new("sink",
                                        Gst.PadDirection.SINK,
                                        Gst.PadPresence.ALWAYS,
                                       Gst.Caps.from_string("ANY"))


class Timestamp(GstBase.BaseTransform):
    GST_PLUGIN_NAME = "timestamp"

    __gstmetadata__ = (
        "Print Data Plugin",  # Name
        "Generic",  # Class type Transform
        "A plugin that prints received data",  # Description
        "Your Name"  # Author
    )
    __gsttemplates__ = (ANY_SRC_TEMPLATE, ANY_SINK_TEMPLATE)

    __gproperties__ = {
        "position": (
            GObject.TYPE_UINT,  # Property type (unsigned int)
            "position",     # Property name
            "how many byte to print at head and tail",  # Description
            0, 255,             # Allowed range (0-255 because it's a byte)
            20,                 # Default value
            GObject.ParamFlags.READWRITE  # Property is readable and writable
        ),
    }

    def __init__(self):
        super(Timestamp, self).__init__()
        print("Initialized Timestamp plugin")
        self.position=20

    def do_get_property(self, prop):
        if prop.name == "position":
            return self.position
        else:
            raise AttributeError("Unknown property: %s" % prop.name)
       

    def do_set_property(self, prop, value):
        if prop.name == "position":
            self.position = value
        else:
            raise AttributeError("Unknown property: %s" % prop.name)


    # This method is called when the buffer is being processed
    def do_transform_ip(self, buf):
        # print("Received buffer at PTS:", buf.pts)
        timenow = time.time()
        print(f"position: {self.position}, timestamp: {timenow}")
        # Map the buffer for reading
        success, map_info = buf.map(Gst.MapFlags.READ)
        if not success:
            print("Failed to map buffer")
            return Gst.FlowReturn.ERROR

        
        # Unmap the buffer when done
        buf.unmap(map_info)

        return Gst.FlowReturn.OK  # Pass the buffer along the pipeline

# Register the element as a GStreamer plugin
GObject.type_register(Timestamp)
__gstelementfactory__ = (Timestamp.GST_PLUGIN_NAME, Gst.Rank.NONE, Timestamp)

