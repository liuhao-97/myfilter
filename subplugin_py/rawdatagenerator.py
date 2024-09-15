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


class Rawdatagenerator(GstBase.BaseSrc):
    GST_PLUGIN_NAME = "rawdatagenerator"

    __gstmetadata__ = (
        "rawdatagenerator",  # Name
        "Src",  # Class type Transform
        "Custom test src element",  # Description
        "Your Name"  # Author
    )

    __gsttemplates__ = ANY_SRC_TEMPLATE

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
        "datasize": (
            GObject.TYPE_UINT,  # Property type (unsigned int)
            "data size",     # Property name
            "how many byte to generate",  # Description
            1, 100000000,             # Allowed range 
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
    }

    def __init__(self):
        super(Rawdatagenerator, self).__init__()
        print("Initialized Rawdatagenerator plugin")
        # Initialize the property
        self.header_value = 3
        self.idx_in_tensor = 1
        self.datasize = 1
        self.ifseq = 0
        
        self.data_sent = False
        self.set_live(True)


    def do_get_property(self, prop):
        if prop.name == "header-value":
            return self.header_value
        elif prop.name == "idx-in-tensor":
            return self.idx_in_tensor
        elif prop.name == "datasize":
            return self.datasize
        elif prop.name == "ifseq":
            return self.ifseq
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
        else:
            raise AttributeError("Unknown property: %s" % prop.name)
        

    def do_create(self, offset, length):
        if not self.data_sent:
            # data = b'Hello, this is a custom source!\n' 
            data=b''
            if self.header_value != 0:
                data=data+struct.pack('B', self.header_value)
                data=data+struct.pack('B', self.idx_in_tensor)
            for i in range(self.datasize):
                # data=data+struct.pack('B', i%3)
                if self.ifseq == 1:
                    data=data+struct.pack('B', i%255)
                else:
                    data=data+struct.pack('B', self.idx_in_tensor)

            buffer = Gst.Buffer.new_allocate(None, len(data), None)
            buffer.fill(0, data)   
            self.data_sent = True  # Set the flag to indicate the data was sent
            return Gst.FlowReturn.OK, buffer
        
        return Gst.FlowReturn.EOS, None
    

# Register the element as a GStreamer plugin
GObject.type_register(Rawdatagenerator)
__gstelementfactory__ = (Rawdatagenerator.GST_PLUGIN_NAME, Gst.Rank.NONE, Rawdatagenerator)

