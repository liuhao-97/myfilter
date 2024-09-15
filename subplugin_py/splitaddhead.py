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


class Splitaddhead(GstBase.BaseTransform):
    GST_PLUGIN_NAME = "splitaddhead"

    __gstmetadata__ = (
        "Print Data Plugin",  # Name
        "Generic",  # Class type Transform
        "A plugin that prints received data",  # Description
        "Your Name"  # Author
    )
    __gsttemplates__ = (ANY_SRC_TEMPLATE, ANY_SINK_TEMPLATE)

    __gproperties__ = {
        "accumulate-frame-number": (
            GObject.TYPE_UINT,  # Property type (unsigned int)
            "accumulate_frame_number",     # Property name
            "accumulate_frame_number",  # Description
            1, 255,             # Allowed range (0-255 because it's a byte)
            20,                 # Default value
            GObject.ParamFlags.READWRITE  # Property is readable and writable
        ),
        "single-frame-size": (
            GObject.TYPE_UINT,  # Property type (unsigned int)
            "single-frame-size",     # Property name
            "single-frame-size byte",  # Description no header
            1, 65536,             # Allowed range 
            1,                 # Default value
            GObject.ParamFlags.READWRITE  # Property is readable and writable
        ),
    }

    def __init__(self):
        super(Splitaddhead, self).__init__()
        print("Initialized Splitaddhead plugin")
        # Initialize the property
        self.accumulate_frame_number = 20
        self.single_frame_size = 1


    def do_get_property(self, prop):
        if prop.name == "accumulate-frame-number":
            return self.accumulate_frame_number
        elif prop.name == "single-frame-size":
            return self.single_frame_size
        else:
            raise AttributeError("Unknown property: %s" % prop.name)
       
    def do_set_property(self, prop, value):
        if prop.name == "accumulate-frame-number":
            self.accumulate_frame_number = value
        elif prop.name == "single-frame-size":
            self.single_frame_size = value
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
        # print("old Buffer data:", data[:])
        # print("unpack data:")
        # unpacked_byte_list=[]
        # for i in data:
        #     unpacked_byte = struct.unpack('B', bytes([i]))  # 'i' must be passed as a single byte
        #     unpacked_byte_list.append(unpacked_byte[0])
        # print("Buffer data:", unpacked_byte_list[:])

        # Unmap the buffer when done
        buf.unmap(map_info)
   
        pack_accumulate_frame_number = struct.pack('B', self.accumulate_frame_number)
        for indx in range(1,self.accumulate_frame_number+1):
            pack_indx = struct.pack('B', indx)
            print(indx)
            pack_data_segment=data[(indx-1)*self.single_frame_size : indx*self.single_frame_size]
            new_data = pack_accumulate_frame_number + pack_indx + pack_data_segment
            
            new_buffer = Gst.Buffer.new_allocate(None, len(new_data), None)
            success, map_info_new = new_buffer.map(Gst.MapFlags.WRITE)
            if not success:
                print("Failed to map buffer for new buffer")
                return Gst.FlowReturn.ERROR
            new_buffer.fill(0, new_data)
            new_buffer.unmap(map_info_new)
            self.srcpad.push(new_buffer)
        
        return Gst.FlowReturn.CUSTOM_SUCCESS # Pass the buffer along the pipeline

# Register the element as a GStreamer plugin
GObject.type_register(Splitaddhead)
__gstelementfactory__ = (Splitaddhead.GST_PLUGIN_NAME, Gst.Rank.NONE, Splitaddhead)

