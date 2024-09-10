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


class Reorderbuffer(GstBase.BaseTransform):
    GST_PLUGIN_NAME = "reorderbuffer"

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
        super(Reorderbuffer, self).__init__()
        print("Initialized Reorderbuffer plugin")
        # Initialize the property
        self.header_value = 20
        self.idx_in_tensor = 1
        # Internal buffer to accumulate data
        self.accumulated_data = bytearray()
        self.threshold_size = 64  # Example threshold size in bytes (e.g., wait for 1KB of data)


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

        # Accumulate the buffer data
        self.accumulated_data.extend(data)
        buf.unmap(map_info)

        # Check if we've accumulated enough data
        if len(self.accumulated_data) >= self.threshold_size:
            print(f"Accumulated {len(self.accumulated_data)} bytes, processing now...")

            # # Process the accumulated data and create a new buffer
            # new_memory = Gst.Memory.new_wrapped(0, self.accumulated_data, len(self.accumulated_data), 0, None, None)
            # new_buffer = Gst.Buffer.new()
            # new_buffer.append_memory(new_memory)

            new_memory= Gst.Memory.new_wrapped(0, self.accumulated_data, len(self.accumulated_data), 0, None, None)
            buf.replace_all_memory(new_memory)
            success, map_info = buf.map(Gst.MapFlags.READ)
            if success:
                print("New Buffer data:", map_info.data[:])
                buf.unmap(map_info)

            # Clear the internal buffer after processing
            self.accumulated_data.clear()

            # # Push the new buffer downstream
            # result = self.srcpad.push(new_buffer)
            # if result != Gst.FlowReturn.OK:
            #     print("Error pushing buffer downstream")
            #     return result
            return Gst.FlowReturn.OK  

        else:

            # Post a custom message to the bus to request the pipeline pause
            print("Condition met: sending a PAUSE request to the pipeline")
            
            ret = Gst.Element.do_change_state(self, Gst.StateChange.PLAYING_TO_PAUSED)



  

# Register the element as a GStreamer plugin
GObject.type_register(Reorderbuffer)
__gstelementfactory__ = (Reorderbuffer.GST_PLUGIN_NAME, Gst.Rank.NONE, Reorderbuffer)

