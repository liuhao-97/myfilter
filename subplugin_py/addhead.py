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


class Addhead(GstBase.BaseTransform):
    GST_PLUGIN_NAME = "addhead"

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
        super(Addhead, self).__init__()
        print("Initialized addhead plugin")
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


    # This method is called when the buffer is being processed
    def do_transform_ip(self, buf):
        print("Received buffer at PTS:", buf.pts)

        # Map the buffer for reading
        success, map_info = buf.map(Gst.MapFlags.READ)
        if not success:
            print("Failed to map buffer")
            return Gst.FlowReturn.ERROR

        # Print the buffer data (first 20 bytes for example)
        data = map_info.data
        print("old Buffer data:", data[:])

        # Unmap the buffer when done
        buf.unmap(map_info)
   
        # new_data = b'20' + b'1' + data[:]
        all_num = struct.pack('B', self.header_value)
        print(all_num)
        indx = struct.pack('B', self.idx_in_tensor)
        print(indx)
        new_data = all_num + indx + data[:]
        # print(new_data)
 
        # new_buffer = Gst.Buffer.new_allocate(None, len(new_data), None)
        # success, map_info = new_buffer.map(Gst.MapFlags.WRITE)
        # if not success:
        #     print("Failed to map buffer")
        #     return Gst.FlowReturn.ERROR
        # new_buffer.fill(0, new_data)
        # success, new_map_info = new_buffer.map(Gst.MapFlags.READ)
        # if not success:
        #     print("Failed to map buffer")
        #     return Gst.FlowReturn.ERROR
        # print("new Buffer data:", new_map_info.data[:])
        # new_buffer.unmap(new_map_info)
        
        new_memory= Gst.Memory.new_wrapped(0, new_data, len(new_data), 0, None, None)
        buf.replace_all_memory(new_memory)
        success, map_info = buf.map(Gst.MapFlags.READ)
        if success:
            print("New Buffer data:", map_info.data[:])
            buf.unmap(map_info)


        return Gst.FlowReturn.OK  # Pass the buffer along the pipeline

# Register the element as a GStreamer plugin
GObject.type_register(Addhead)
__gstelementfactory__ = (Addhead.GST_PLUGIN_NAME, Gst.Rank.NONE, Addhead)

