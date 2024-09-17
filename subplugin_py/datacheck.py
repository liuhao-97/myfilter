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


class Datacheck(GstBase.BaseTransform):
    GST_PLUGIN_NAME = "datacheck"

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
        "start": (
            GObject.TYPE_UINT,  # Property type (unsigned int)
            "start",     # Property name
            "data generate start from",  # Description
            0, 255,             # Allowed range (0-255 because it's a byte)
            0,                 # Default value
            GObject.ParamFlags.READWRITE  # Property is readable and writable
        ),
        "datasize": (
            GObject.TYPE_UINT,  # Property type (unsigned int)
            "data size",     # Property name
            "how many byte to generate",  # Description
            1, 42949672,             # Allowed range 
            1,                 # Default value
            GObject.ParamFlags.READWRITE  # Property is readable and writable
        ),
        "ifseq": (
            GObject.TYPE_UINT,  # Property type (unsigned int)
            "if_sequential",     # Property name
            "if generate from 1 to 255 iter",  # Description
            0, 1,             # Allowed range 
            0,                 # Default value
            GObject.ParamFlags.READWRITE  # Property is readable and writable
        ),
        "large-tensor-datasize": (
            GObject.TYPE_UINT,  # Property type (unsigned int)
            "data size",     # Property name
            "how many byte the single large tensor",  # Description
            1, 42949672,             # Allowed range 
            1,                 # Default value
            GObject.ParamFlags.READWRITE  # Property is readable and writable
        ),
        "small-tensor-datasize": (
            GObject.TYPE_UINT,  # Property type (unsigned int)
            "data size",     # Property name
            "how many byte the small tensor",  # Description
            0, 65536,             # Allowed range 
            0,                 # Default value
            GObject.ParamFlags.READWRITE  # Property is readable and writable
        ),
    }

    def __init__(self):
        super(Datacheck, self).__init__()
        print("Initialized Datacheck plugin")
        # Initialize the property
        self.header_value = 3
        self.idx_in_tensor = 1
        self.datasize = 1
        self.ifseq = 0
        self.start = 0
        self.large_tensor_datasize = 1
        self.small_tensor_datasize = 0

    def do_get_property(self, prop):
        if prop.name == "header-value":
            return self.header_value
        elif prop.name == "idx-in-tensor":
            return self.idx_in_tensor
        elif prop.name == "datasize":
            return self.datasize
        elif prop.name == "ifseq":
            return self.ifseq
        elif prop.name == "start":
            return self.start
        elif prop.name == "large-tensor-datasize":
            return self.large_tensor_datasize
        elif prop.name == "small-tensor-datasize":
            return self.small_tensor_datasize
        else:
            raise AttributeError("Unknown property: %s" % prop.name)     

    def do_set_property(self, prop, value):
        if prop.name == "header-value":
            self.header_value = value
        elif prop.name == "idx-in-tensor":
            self.idx_in_tensor = value
        elif prop.name == "datasize":
            self.datasize = value
        elif prop.name == "ifseq":
            self.ifseq = value
        elif prop.name == "start":
            self.start = value
        elif prop.name == "large-tensor-datasize":
            self.large_tensor_datasize = value
        elif prop.name == "small-tensor-datasize":
            self.small_tensor_datasize = value
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
    
        all_data=b''
        for i in range(self.datasize):
            if self.ifseq == 1:
                all_data=all_data+struct.pack('B', (self.start +i)%255)
            else:
                all_data=all_data+struct.pack('B', self.start)
        
        check_data=b''
        if self.header_value != 0:
            check_data=check_data+struct.pack('B', self.header_value)
            check_data=check_data+struct.pack('B', self.idx_in_tensor)

        if self.small_tensor_datasize == 0:
            single_frame_size = self.large_tensor_datasize
            check_data=check_data+all_data[single_frame_size*(self.idx_in_tensor-1):single_frame_size*self.idx_in_tensor]
        else:
            if self.idx_in_tensor != self.header_value:
                single_frame_size = self.large_tensor_datasize
                check_data=check_data+all_data[single_frame_size*(self.idx_in_tensor-1):single_frame_size*self.idx_in_tensor]
            else:
                check_data=check_data+all_data[-self.small_tensor_datasize:]
        if check_data != data:
            print("data check failed")
        else:
            print("data check success")

        # Unmap the buffer when done
        buf.unmap(map_info)

        return Gst.FlowReturn.OK  # Pass the buffer along the pipeline

# Register the element as a GStreamer plugin
GObject.type_register(Datacheck)
__gstelementfactory__ = (Datacheck.GST_PLUGIN_NAME, Gst.Rank.NONE, Datacheck)

