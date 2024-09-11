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


class Shufflebuffer(GstBase.BaseTransform):
    GST_PLUGIN_NAME = "shufflebuffer"

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
        super(Shufflebuffer, self).__init__()
        print("Initialized Shufflebuffer plugin")
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

        # Map the original buffer for reading
        success, map_info = buf.map(Gst.MapFlags.READ)
        if not success:
            print("Failed to map buffer")
            return Gst.FlowReturn.ERROR

        # Access the data from the original buffer
        data = map_info.data
        data_length = len(data)
        print(f"Original Buffer data (length {data_length}):", data[:])

        # Unmap the original buffer after reading
        buf.unmap(map_info)

        # Split the data into two halves
        half_length = data_length // 2
        first_part = data[:half_length]
        second_part = data[half_length:]

        # Create the first buffer containing the first part of the data
        first_memory = Gst.Memory.new_wrapped(0, first_part, len(first_part), 0, None, None)
        first_buffer = Gst.Buffer.new()
        first_buffer.append_memory(first_memory)

        # Create the second buffer containing the second part of the data
        second_memory = Gst.Memory.new_wrapped(0, second_part, len(second_part), 0, None, None)
        second_buffer = Gst.Buffer.new()
        second_buffer.append_memory(second_memory)

        # Optional: You may want to set the PTS (presentation timestamp) and DTS (decoding timestamp) for these buffers
        first_buffer.pts = buf.pts  # Keeping the same PTS as the original buffer
        second_buffer.pts = buf.pts + Gst.util_uint64_scale_int(1, Gst.SECOND, 2)  # Slightly adjust PTS for the second buffer, e.g., 0.5 seconds later

        # Push the first buffer downstream
        result = self.srcpad.push(first_buffer)
        if result != Gst.FlowReturn.OK:
            print("Error pushing the first buffer")
            return result

        # # Push the second buffer downstream
        # result = self.srcpad.push(second_buffer)
        # if result != Gst.FlowReturn.OK:
        #     print("Error pushing the second buffer")
        #     return result
        
        new_memory= Gst.Memory.new_wrapped(0, second_part, len(second_part), 0, None, None)
        buf.replace_all_memory(new_memory)
        success, map_info = buf.map(Gst.MapFlags.READ)
        if success:
            print("New Buffer data:", map_info.data[:])
            buf.unmap(map_info)

        return Gst.FlowReturn.OK  # Successfully pushed both parts downstream

# Register the element as a GStreamer plugin
GObject.type_register(Shufflebuffer)
__gstelementfactory__ = (Shufflebuffer.GST_PLUGIN_NAME, Gst.Rank.NONE, Shufflebuffer)

