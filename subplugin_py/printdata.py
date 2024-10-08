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


class Printdata(GstBase.BaseTransform):
    GST_PLUGIN_NAME = "printdata"

    __gstmetadata__ = (
        "Print Data Plugin",  # Name
        "Generic",  # Class type Transform
        "A plugin that prints received data",  # Description
        "Your Name"  # Author
    )
    __gsttemplates__ = (ANY_SRC_TEMPLATE, ANY_SINK_TEMPLATE)

    __gproperties__ = {
        "headtail": (
            GObject.TYPE_UINT,  # Property type (unsigned int)
            "headtail",     # Property name
            "how many byte to print at head and tail",  # Description
            0, 255,             # Allowed range (0-255 because it's a byte)
            20,                 # Default value
            GObject.ParamFlags.READWRITE  # Property is readable and writable
        ),
    }

    def __init__(self):
        super(Printdata, self).__init__()
        print("Initialized printdata plugin")
        self.headtail=20

    def do_get_property(self, prop):
        if prop.name == "headtail":
            return self.headtail
        else:
            raise AttributeError("Unknown property: %s" % prop.name)
       

    def do_set_property(self, prop, value):
        if prop.name == "headtail":
            self.headtail = value
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

        # Print the buffer data 
        data = map_info.data
        unpacked_byte_list=[]
        for i in data:
            unpacked_byte = struct.unpack('B', bytes([i]))  # 'i' must be passed as a single byte
            unpacked_byte_list.append(unpacked_byte[0])
        # print("Buffer data:", data[:])
        if self.headtail > 0:
            print(f"first {self.headtail} Buffer data:", unpacked_byte_list[0:self.headtail])
            print(f"last {self.headtail} Buffer data:", unpacked_byte_list[-self.headtail:])
        else:
            print("Buffer data:", unpacked_byte_list[:])
        print(f"length {len(unpacked_byte_list)}")
        # Unmap the buffer when done
        buf.unmap(map_info)
   
        # new_data = b'h' + data[:]

        # new_memory= Gst.Memory.new_wrapped(0, new_data, len(new_data), 0, None, None)
        # buf.replace_all_memory(new_memory)
        # success, map_info = buf.map(Gst.MapFlags.READ)
        # if success:
        #     print("New Buffer data:", map_info.data[:])
        #     buf.unmap(map_info)

        return Gst.FlowReturn.OK  # Pass the buffer along the pipeline

# Register the element as a GStreamer plugin
GObject.type_register(Printdata)
__gstelementfactory__ = (Printdata.GST_PLUGIN_NAME, Gst.Rank.NONE, Printdata)

