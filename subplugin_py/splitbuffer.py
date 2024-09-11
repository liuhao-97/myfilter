#!/usr/bin/env python3
import gi
gi.require_version('Gst', '1.0')
# gi.require_version('GObject', '2.0')
gi.require_version('GstBase', '1.0')

from gi.repository import Gst, GObject, GstBase
import struct

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


class Splitbuffer(GstBase.BaseTransform):
    GST_PLUGIN_NAME = "splitbuffer"

    __gstmetadata__ = (
        "Print Data Plugin",  # Name
        "Generic",  # Class type Transform
        "A plugin that prints received data",  # Description
        "Your Name"  # Author
    )
    __gsttemplates__ = (ANY_SRC_TEMPLATE, ANY_SINK_TEMPLATE)

    __gproperties__ = {
        "header-value": (
            GObject.TYPE_UINT,  # Property type (unsigned int)
            "Header Value",     # Property name
            "The value to prepend as the header",  # Description
            0, 255,             # Allowed range (0-255 because it's a byte)
            20,                 # Default value
            GObject.ParamFlags.READWRITE  # Property is readable and writable
        ),
        "idx-in-tensor": (
            GObject.TYPE_UINT,  # Property type (unsigned int)
            "idx in tensor Value",     # Property name
            "idx in tensor Value",  # Description
            1, 255,             # Allowed range (0-255 because it's a byte)
            1,                 # Default value
            GObject.ParamFlags.READWRITE  # Property is readable and writable
        ),
    }

    def __init__(self):
        super(Splitbuffer, self).__init__()
        print("Initialized Splitbuffer plugin")
        # Initialize the property
        self.header_value = 20
        self.idx_in_tensor = 1


    def do_get_property(self, prop):
        if prop.name == "header-value":
            return self.header_value
        elif prop.name == "idx-in-tensor":
            return self.idx_in_tensor
        else:
            raise AttributeError("Unknown property: %s" % prop.name)
       
    def do_set_property(self, prop, value):
        if prop.name == "header-value":
            self.header_value = value
        elif prop.name == "idx-in-tensor":
            self.idx_in_tensor = value
        else:
            raise AttributeError("Unknown property: %s" % prop.name)
        

    def do_transform_ip(self, buf):
        print("Received buffer at PTS:", buf.pts)

        # Map the buffer for reading
        success, map_info = buf.map(Gst.MapFlags.READ)
        if not success:
            print("Failed to map buffer")
            return Gst.FlowReturn.ERROR

        # Access the data from the buffer
        data = map_info.data
        print(f"Original buffer data length: {len(data)}")

        # Unmap the original buffer after reading
        buf.unmap(map_info)

        # Split the buffer data into two parts
        half_point = len(data) // 2
        data_part_1 = data[:half_point]
        data_part_2 = data[half_point:]

        print(f"Split at index {half_point}. Part 1 length: {len(data_part_1)}, Part 2 length: {len(data_part_2)}")

        # Create a new buffer for the first part of the data
        buffer_part_1 = Gst.Buffer.new_allocate(None, len(data_part_1), None)
        success, map_info_part_1 = buffer_part_1.map(Gst.MapFlags.WRITE)
        if not success:
            print("Failed to map buffer for Part 1")
            return Gst.FlowReturn.ERROR
        buffer_part_1.fill(0, data_part_1)
        buffer_part_1.unmap(map_info_part_1)

        # Create a new buffer for the second part of the data
        buffer_part_2 = Gst.Buffer.new_allocate(None, len(data_part_2), None)
        success, map_info_part_2 = buffer_part_2.map(Gst.MapFlags.WRITE)
        if not success:
            print("Failed to map buffer for Part 2")
            return Gst.FlowReturn.ERROR
        buffer_part_2.fill(0, data_part_2)
        buffer_part_2.unmap(map_info_part_2)

        # Now push the two parts sequentially
        print("Pushing Part 1")
        self.srcpad.push(buffer_part_1)

        print("Pushing Part 2")
        self.srcpad.push(buffer_part_2)

        return Gst.FlowReturn.OK



# Register the element as a GStreamer plugin
GObject.type_register(Splitbuffer)
__gstelementfactory__ = (Splitbuffer.GST_PLUGIN_NAME, Gst.Rank.NONE, Splitbuffer)

