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
  PROP_SINGLE_FRAME_SIZE,
  PROP_ACCUMULATE_FRAME_NUMBER,
  PROP_SILENT
};

/**
 * @brief Each frame data size in input buffer.
 */
#define DEFAULT_SINGLE_FRAME_SIZE 1

/**
 * @brief Accumulate frame size in output buffer.
 */
#define DEFAULT_ACCUMULATE_FRAME_NUMBER 1

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
  g_object_class_install_property(gobject_class, PROP_SINGLE_FRAME_SIZE,
                                  g_param_spec_uint("single-frame-size", "single frame size (byte)",
                                                    "single frame size (byte)", 1, G_MAXUINT,
                                                    DEFAULT_SINGLE_FRAME_SIZE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstTensorReschedule::frames-out:
   *
   * The number of frames in outgoing buffer. (buffer is a sinle tensor instance)
   * GstTensorReschedule calculates the size of outgoing frames and pushes a buffer to source pad.
   */
  g_object_class_install_property(gobject_class, PROP_ACCUMULATE_FRAME_NUMBER,
                                  g_param_spec_uint("accumulate-frame-number", "accumulate frame number",
                                                    "accumulate frame number", 1, G_MAXUINT,
                                                    DEFAULT_ACCUMULATE_FRAME_NUMBER, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

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
  filter->single_frame_size = DEFAULT_SINGLE_FRAME_SIZE;
  filter->accumulate_frame_number = DEFAULT_ACCUMULATE_FRAME_NUMBER;
}

static void
gst_data_reschedule_set_property(GObject *object, guint prop_id,
                                 const GValue *value, GParamSpec *pspec)
{
  GstDataReschedule *filter = GST_DATA_RESCHEDULE(object);

  switch (prop_id)
  {
  case PROP_SINGLE_FRAME_SIZE:
    filter->single_frame_size = g_value_get_uint(value);
    break;
  case PROP_ACCUMULATE_FRAME_NUMBER:
    filter->accumulate_frame_number = g_value_get_uint(value);
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
  case PROP_SINGLE_FRAME_SIZE:
    g_value_set_uint(value, filter->single_frame_size);
    break;
  case PROP_ACCUMULATE_FRAME_NUMBER:
    g_value_set_uint(value, filter->accumulate_frame_number);
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
  gsize single_frame_size, accumulate_frame_number, accumulate_frame_size;
  gsize buffersize = gst_buffer_get_size(buf); // Get the size of the buffer data

  filter = GST_DATA_RESCHEDULE(parent);
  adapter = filter->adapter;
  single_frame_size = filter->single_frame_size;
  accumulate_frame_number = filter->accumulate_frame_number;
  accumulate_frame_size = single_frame_size * accumulate_frame_number;

  // Check if single_frame_size is not equal to buffersize
  if (single_frame_size != buffersize)
  {
    g_print("Warning: Single frame size (%zu) is not equal to buffer size (%zu)\n",
            single_frame_size, buffersize);
    g_assert(single_frame_size == buffersize);
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
    // Create a new buffer to hold the processed data

    GstBuffer *outbuf = gst_buffer_new_and_alloc(accumulate_frame_size);

    // Map the buffer for writing
    GstMapInfo map;
    if (gst_buffer_map(outbuf, &map, GST_MAP_WRITE))
    {
      // Write data into the buffer (in this case just copy the input data)
      memcpy(map.data, data, accumulate_frame_size);

      // Unmap the buffer after writing
      gst_buffer_unmap(outbuf, &map);
    }

    // Push the new buffer downstream
    ret = gst_pad_push(filter->srcpad, outbuf);

    g_print("reach %zu bytes.\n", accumulate_frame_size);

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
