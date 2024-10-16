/**
 * SECTION:element-datareschedule
 *
 * FIXME:Describe datareschedule here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! datareschedule ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gstdatareschedule.h"
#include <gst/base/gstadapter.h>
#include <gst/gstelement.h>
#include <time.h>

GST_DEBUG_CATEGORY_STATIC(gst_data_reschedule_debug);
#define GST_CAT_DEFAULT gst_data_reschedule_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SINGLE_LARGE_FRAME_SIZE,
  PROP_ACCUMULATE_LARGE_FRAME_NUMBER,
  PROP_SINGLE_SMALL_FRAME_SIZE,
  PROP_ACCUMULATE_SMALL_FRAME_NUMBER,
  PROP_SILENT
};

/**
 * @brief Each frame data size in input buffer.
 */
#define DEFAULT_SINGLE_LARGE_FRAME_SIZE 1
#define DEFAULT_SINGLE_SMALL_FRAME_SIZE 1

/**
 * @brief Accumulate frame size in output buffer.
 */
#define DEFAULT_ACCUMULATE_LARGE_FRAME_NUMBER 1
#define DEFAULT_ACCUMULATE_SMALL_FRAME_NUMBER 1

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE("sink",
                                                                   GST_PAD_SINK,
                                                                   GST_PAD_ALWAYS,
                                                                   GST_STATIC_CAPS("ANY"));

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE("src",
                                                                  GST_PAD_SRC,
                                                                  GST_PAD_ALWAYS,
                                                                  GST_STATIC_CAPS("ANY"));

#define gst_data_reschedule_parent_class parent_class
G_DEFINE_TYPE(GstDataReschedule, gst_data_reschedule, GST_TYPE_ELEMENT);

static void gst_data_reschedule_set_property(GObject *object,
                                             guint prop_id, const GValue *value, GParamSpec *pspec);
static void gst_data_reschedule_get_property(GObject *object,
                                             guint prop_id, GValue *value, GParamSpec *pspec);
static void gst_data_reschedule_finalize(GObject *object);

static gboolean gst_data_reschedule_sink_event(GstPad *pad,
                                               GstObject *parent, GstEvent *event);
static GstFlowReturn gst_data_reschedule_chain(GstPad *pad,
                                               GstObject *parent, GstBuffer *buf);

/* GObject vmethod implementations */

/* initialize the datareschedule's class */
static void
gst_data_reschedule_class_init(GstDataRescheduleClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *)klass;
  gstelement_class = (GstElementClass *)klass;

  gobject_class->set_property = gst_data_reschedule_set_property;
  gobject_class->get_property = gst_data_reschedule_get_property;
  gobject_class->finalize = gst_data_reschedule_finalize;

  /**
   * GstTensorReschedule::frames-in:
   *
   * The number of frames in incoming buffer.
   * GstTensorReschedule itself cannot get the frames in buffer. (buffer is a sinle tensor instance)
   * GstTensorReschedule calculates the size of single frame with this property.
   */
  g_object_class_install_property(gobject_class, PROP_SINGLE_LARGE_FRAME_SIZE,
                                  g_param_spec_uint("single-large-frame-size", "single large frame size (byte)",
                                                    "single large frame size (byte)", 1, G_MAXUINT,
                                                    DEFAULT_SINGLE_LARGE_FRAME_SIZE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_SINGLE_SMALL_FRAME_SIZE,
                                  g_param_spec_uint("single-small-frame-size", "single small frame size (byte)",
                                                    "single small frame size (byte)", 0, G_MAXUINT,
                                                    DEFAULT_SINGLE_SMALL_FRAME_SIZE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  /**
   * GstTensorReschedule::frames-out:
   *
   * The number of frames in outgoing buffer. (buffer is a sinle tensor instance)
   * GstTensorReschedule calculates the size of outgoing frames and pushes a buffer to source pad.
   */
  g_object_class_install_property(gobject_class, PROP_ACCUMULATE_LARGE_FRAME_NUMBER,
                                  g_param_spec_uint("accumulate-large-frame-number", "accumulate large frame number",
                                                    "accumulate large frame number", 1, G_MAXUINT,
                                                    DEFAULT_ACCUMULATE_LARGE_FRAME_NUMBER, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class, PROP_ACCUMULATE_SMALL_FRAME_NUMBER,
                                  g_param_spec_uint("accumulate-small-frame-number", "accumulate small frame number",
                                                    "accumulate small frame number", 0, G_MAXUINT,
                                                    DEFAULT_ACCUMULATE_SMALL_FRAME_NUMBER, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class, PROP_SILENT,
                                  g_param_spec_boolean("silent", "Silent", "Produce verbose output ?",
                                                       FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple(gstelement_class,
                                       "DataReschedule",
                                       "FIXME:Generic",
                                       "FIXME:Generic Template Element", "root <<user@hostname.org>>");

  gst_element_class_add_pad_template(gstelement_class,
                                     gst_static_pad_template_get(&src_factory));
  gst_element_class_add_pad_template(gstelement_class,
                                     gst_static_pad_template_get(&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void
gst_data_reschedule_init(GstDataReschedule *filter)
{
  filter->sinkpad = gst_pad_new_from_static_template(&sink_factory, "sink");
  gst_pad_set_event_function(filter->sinkpad,
                             GST_DEBUG_FUNCPTR(gst_data_reschedule_sink_event));
  gst_pad_set_chain_function(filter->sinkpad,
                             GST_DEBUG_FUNCPTR(gst_data_reschedule_chain));
  GST_PAD_SET_PROXY_CAPS(filter->sinkpad);
  gst_element_add_pad(GST_ELEMENT(filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template(&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS(filter->srcpad);
  gst_element_add_pad(GST_ELEMENT(filter), filter->srcpad);

  filter->silent = FALSE;

  // Initialize the adapter
  filter->adapter = gst_adapter_new();
  filter->single_large_frame_size = DEFAULT_SINGLE_LARGE_FRAME_SIZE;
  filter->accumulate_large_frame_number = DEFAULT_ACCUMULATE_LARGE_FRAME_NUMBER;
  filter->single_small_frame_size = DEFAULT_SINGLE_SMALL_FRAME_SIZE;
  filter->accumulate_small_frame_number = DEFAULT_ACCUMULATE_SMALL_FRAME_NUMBER;
}

static void
gst_data_reschedule_set_property(GObject *object, guint prop_id,
                                 const GValue *value, GParamSpec *pspec)
{
  GstDataReschedule *filter = GST_DATA_RESCHEDULE(object);

  switch (prop_id)
  {
  case PROP_SINGLE_LARGE_FRAME_SIZE:
    filter->single_large_frame_size = g_value_get_uint(value);
    break;
  case PROP_ACCUMULATE_LARGE_FRAME_NUMBER:
    filter->accumulate_large_frame_number = g_value_get_uint(value);
    break;
  case PROP_SINGLE_SMALL_FRAME_SIZE:
    filter->single_small_frame_size = g_value_get_uint(value);
    break;
  case PROP_ACCUMULATE_SMALL_FRAME_NUMBER:
    filter->accumulate_small_frame_number = g_value_get_uint(value);
    break;
  case PROP_SILENT:
    filter->silent = g_value_get_boolean(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void
gst_data_reschedule_get_property(GObject *object, guint prop_id,
                                 GValue *value, GParamSpec *pspec)
{
  GstDataReschedule *filter = GST_DATA_RESCHEDULE(object);

  switch (prop_id)
  {
  case PROP_SINGLE_LARGE_FRAME_SIZE:
    g_value_set_uint(value, filter->single_large_frame_size);
    break;
  case PROP_ACCUMULATE_LARGE_FRAME_NUMBER:
    g_value_set_uint(value, filter->accumulate_large_frame_number);
    break;
  case PROP_SINGLE_SMALL_FRAME_SIZE:
    g_value_set_uint(value, filter->single_small_frame_size);
    break;
  case PROP_ACCUMULATE_SMALL_FRAME_NUMBER:
    g_value_set_uint(value, filter->accumulate_small_frame_number);
    break;
  case PROP_SILENT:
    g_value_set_boolean(value, filter->silent);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_data_reschedule_sink_event(GstPad *pad, GstObject *parent,
                               GstEvent *event)
{
  GstDataReschedule *filter;
  gboolean ret;

  filter = GST_DATA_RESCHEDULE(parent);

  GST_LOG_OBJECT(filter, "Received %s event: %" GST_PTR_FORMAT,
                 GST_EVENT_TYPE_NAME(event), event);

  switch (GST_EVENT_TYPE(event))
  {
  case GST_EVENT_CAPS:
  {
    GstCaps *caps;

    gst_event_parse_caps(event, &caps);
    /* do something with the caps */

    /* and forward */
    ret = gst_pad_event_default(pad, parent, event);
    break;
  }
  default:
    ret = gst_pad_event_default(pad, parent, event);
    break;
  }
  return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_data_reschedule_chain(GstPad *pad, GstObject *parent, GstBuffer *buf)
{
  GstDataReschedule *filter;
  GstAdapter *adapter;
  GstFlowReturn ret = GST_FLOW_OK;
  gsize single_large_frame_size, accumulate_large_frame_number;
  gsize single_small_frame_size, accumulate_small_frame_number;
  gsize accumulate_frame_size;
  gsize new_single_large_frame_size, new_single_small_frame_size;
  gsize push_out_frame_size;
  gsize buffersize = gst_buffer_get_size(buf); // Get the size of the buffer data

  filter = GST_DATA_RESCHEDULE(parent);
  adapter = filter->adapter;
  single_large_frame_size = filter->single_large_frame_size;
  accumulate_large_frame_number = filter->accumulate_large_frame_number;
  single_small_frame_size = filter->single_small_frame_size;
  accumulate_small_frame_number = filter->accumulate_small_frame_number;

  if ((accumulate_small_frame_number == 0 && single_small_frame_size != 0) || (accumulate_small_frame_number != 0 && single_small_frame_size == 0))
  {
    g_print("Warning: Either both Single small frame size (%zu) and Accumulate small frame number (%zu) should be 0 or both should be non-zero\n",
            single_small_frame_size, accumulate_small_frame_number);
    g_assert((accumulate_small_frame_number == 0 && single_small_frame_size == 0) || (accumulate_small_frame_number != 0 && single_small_frame_size != 0));
  }

  accumulate_frame_size = single_large_frame_size * accumulate_large_frame_number + single_small_frame_size * accumulate_small_frame_number;

  new_single_large_frame_size = single_large_frame_size - 2;
  if (accumulate_small_frame_number != 0)
  {
    new_single_small_frame_size = single_small_frame_size - 2;
  }
  else
  {
    new_single_small_frame_size = 0;
  }

  push_out_frame_size = new_single_large_frame_size * accumulate_large_frame_number + new_single_small_frame_size * accumulate_small_frame_number; // 2 bytes head are removed from each frame

  // Check if single_frame_size is not equal to buffersize
  if (single_large_frame_size != buffersize && single_small_frame_size != buffersize)
  {
    g_print("Warning: None of Single large frame size (%zu) and Single small framesize (%zu) are not equal to buffer size (%zu)\n",
            single_large_frame_size, single_small_frame_size, buffersize);
    g_assert(single_large_frame_size == buffersize || single_small_frame_size == buffersize);
  }
  else
  {
    // Process normally if the sizes match
    // Your processing logic here
  }

  // put buffer into adapter
  gst_adapter_push(adapter, buf);

  // while we can read out accumulate_frame_size bytes, process them
  while (gst_adapter_available(adapter) >= accumulate_frame_size && ret == GST_FLOW_OK)
  {
    const guint8 *data = gst_adapter_map(adapter, accumulate_frame_size);
    // use flowreturn as an error value

    // // Print the data contents in hex format
    // gsize i;
    // g_print("Data contents (size: %zu):\n", accumulate_frame_size);
    // for (i = 0; i < accumulate_frame_size; i++)
    // {
    //   g_print("%02x ", data[i]); // Print each byte in hex format
    //   if ((i + 1) % single_frame_size == 0)
    //   {
    //     g_print("\n"); // New line after every 16 bytes for better readability
    //   }
    // }
    // g_print("\n");

    // Create a new buffer to hold the processed data
    GstBuffer *outbuf = gst_buffer_new_and_alloc(push_out_frame_size);

    // Map the buffer for writing
    GstMapInfo map;
    gsize i;
    gsize j;
    gsize idx;
    if (accumulate_small_frame_number == 0)
    {
      if (gst_buffer_map(outbuf, &map, GST_MAP_WRITE))
      {
        for (i = 0; i < accumulate_large_frame_number; i++)
        {
          idx = data[i * single_large_frame_size + 1] - 1;
          // Copy each frame, skipping the first two bytes
          // FIX
          memcpy(map.data + (idx * new_single_large_frame_size), data + (i * single_large_frame_size) + 2, new_single_large_frame_size);
        }
        // Unmap the buffer after writing
        gst_buffer_unmap(outbuf, &map);
      }
    }

    gsize offset = 0;
    if (accumulate_small_frame_number == 1)
    {
      if (gst_buffer_map(outbuf, &map, GST_MAP_WRITE))
      {
        for (i = 0; i < accumulate_large_frame_number + accumulate_small_frame_number; i++)
        {

          if (data[offset] == 1 && data[offset + 1] == (accumulate_large_frame_number + accumulate_small_frame_number)) // the frame is small frame
          {
            memcpy(map.data + (accumulate_large_frame_number * new_single_large_frame_size), data + offset + 2, new_single_small_frame_size);
            offset += single_small_frame_size;
          }
          else
          {
            idx = data[offset + 1] - 1;
            memcpy(map.data + (idx * new_single_large_frame_size), data + offset + 2, new_single_large_frame_size);
            offset += single_large_frame_size;
          }
        }
        // Unmap the buffer after writing
        gst_buffer_unmap(outbuf, &map);
      }
    }

    struct timespec receive_timestamp;
    clock_gettime(CLOCK_REALTIME, &receive_timestamp);
    g_print("position: 1, timestamp: %ld, ns:%ld\n", receive_timestamp.tv_sec, receive_timestamp.tv_nsec);

    // Push the new buffer downstream
    ret = gst_pad_push(filter->srcpad, outbuf);

    g_print("accumulate %zu bytes.\n", accumulate_frame_size);

    gst_adapter_unmap(adapter);
    gst_adapter_flush(adapter, accumulate_frame_size);
  }
  return ret;

  // if (filter->silent == FALSE)
  //   g_print("I'm plugged, therefore I'm in.\n");

  // /* just push out the incoming buffer without touching it */
  // return gst_pad_push(filter->srcpad, buf);
}

static void
gst_data_reschedule_finalize(GObject *object)
{
  GstDataReschedule *filter = GST_DATA_RESCHEDULE(object);

  // Unref the adapter to release memory
  if (filter->adapter)
  {
    g_object_unref(filter->adapter);
    filter->adapter = NULL;
  }

  G_OBJECT_CLASS(gst_data_reschedule_parent_class)->finalize(object);
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
plugin_init(GstPlugin *plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register(plugin, "datareschedule", GST_RANK_NONE,
                              GST_TYPE_DATA_RESCHEDULE);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef VERSION
#define VERSION "0.0.FIXME"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

/* gstreamer looks for this structure to register datareschedules
 *
 * exchange the string 'Template datareschedule' with your datareschedule description
 */
GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  datareschedule,
                  "data_reschedule",
                  plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
