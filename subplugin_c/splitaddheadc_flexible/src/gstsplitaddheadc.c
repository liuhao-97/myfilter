/**
 * SECTION:element-splitaddheadc
 *
 * FIXME:Describe splitaddheadc here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! splitaddheadc ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gstsplitaddheadc.h"
// #include <gst/base/gstadapter.h>
#include <gst/gstelement.h>

GST_DEBUG_CATEGORY_STATIC(gst_splitaddheadc_debug);
#define GST_CAT_DEFAULT gst_splitaddheadc_debug

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
  PROP_SINGLE_SMALL_FRAME_SIZE,
  PROP_ACCUMULATE_LARGE_FRAME_NUMBER,
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

#define gst_splitaddheadc_parent_class parent_class
G_DEFINE_TYPE(GstSplitaddheadc, gst_splitaddheadc, GST_TYPE_ELEMENT);

static void gst_splitaddheadc_set_property(GObject *object,
                                           guint prop_id, const GValue *value, GParamSpec *pspec);
static void gst_splitaddheadc_get_property(GObject *object,
                                           guint prop_id, GValue *value, GParamSpec *pspec);
static void gst_splitaddheadc_finalize(GObject *object);

static gboolean gst_splitaddheadc_sink_event(GstPad *pad,
                                             GstObject *parent, GstEvent *event);
static GstFlowReturn gst_splitaddheadc_chain(GstPad *pad,
                                             GstObject *parent, GstBuffer *buf);

/* GObject vmethod implementations */

/* initialize the splitaddheadc's class */
static void
gst_splitaddheadc_class_init(GstSplitaddheadcClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *)klass;
  gstelement_class = (GstElementClass *)klass;

  gobject_class->set_property = gst_splitaddheadc_set_property;
  gobject_class->get_property = gst_splitaddheadc_get_property;
  gobject_class->finalize = gst_splitaddheadc_finalize;

  /**
   * GstTensorReschedule::frames-in:
   *
   *
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
   *
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
                                       "Splitaddheadc",
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
gst_splitaddheadc_init(GstSplitaddheadc *filter)
{
  filter->sinkpad = gst_pad_new_from_static_template(&sink_factory, "sink");
  gst_pad_set_event_function(filter->sinkpad,
                             GST_DEBUG_FUNCPTR(gst_splitaddheadc_sink_event));
  gst_pad_set_chain_function(filter->sinkpad,
                             GST_DEBUG_FUNCPTR(gst_splitaddheadc_chain));
  GST_PAD_SET_PROXY_CAPS(filter->sinkpad);
  gst_element_add_pad(GST_ELEMENT(filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template(&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS(filter->srcpad);
  gst_element_add_pad(GST_ELEMENT(filter), filter->srcpad);

  filter->silent = FALSE;

  filter->single_large_frame_size = DEFAULT_SINGLE_LARGE_FRAME_SIZE;
  filter->single_small_frame_size = DEFAULT_SINGLE_SMALL_FRAME_SIZE;
  filter->accumulate_large_frame_number = DEFAULT_ACCUMULATE_LARGE_FRAME_NUMBER;
  filter->accumulate_small_frame_number = DEFAULT_ACCUMULATE_SMALL_FRAME_NUMBER;
}

static void
gst_splitaddheadc_set_property(GObject *object, guint prop_id,
                               const GValue *value, GParamSpec *pspec)
{
  GstSplitaddheadc *filter = GST_SPLITADDHEADC(object);

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
gst_splitaddheadc_get_property(GObject *object, guint prop_id,
                               GValue *value, GParamSpec *pspec)
{
  GstSplitaddheadc *filter = GST_SPLITADDHEADC(object);

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
gst_splitaddheadc_sink_event(GstPad *pad, GstObject *parent,
                             GstEvent *event)
{
  GstSplitaddheadc *filter;
  gboolean ret;

  filter = GST_SPLITADDHEADC(parent);

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
gst_splitaddheadc_chain(GstPad *pad, GstObject *parent, GstBuffer *buf)
{
  GstSplitaddheadc *filter;
  GstFlowReturn ret = GST_FLOW_OK;
  gsize single_large_frame_size, single_small_frame_size;
  gsize accumulate_large_frame_number, accumulate_small_frame_number;
  gsize accumulate_frame_size;
  gsize new_single_large_frame_size, new_single_small_frame_size;
  gsize buffersize = gst_buffer_get_size(buf); // Get the size of the buffer data

  filter = GST_SPLITADDHEADC(parent);
  single_large_frame_size = filter->single_large_frame_size;
  single_small_frame_size = filter->single_small_frame_size;
  accumulate_large_frame_number = filter->accumulate_large_frame_number;
  accumulate_small_frame_number = filter->accumulate_small_frame_number;
  accumulate_frame_size = single_large_frame_size * accumulate_large_frame_number + single_small_frame_size * accumulate_small_frame_number;

  if ((accumulate_small_frame_number == 0 && single_small_frame_size != 0) || (accumulate_small_frame_number != 0 && single_small_frame_size == 0))
  {
    g_print("Warning: Either both Single small frame size (%zu) and Accumulate small frame number (%zu) should be 0 or both should be non-zero\n",
            single_small_frame_size, accumulate_small_frame_number);
    g_assert((accumulate_small_frame_number == 0 && single_small_frame_size == 0) || (accumulate_small_frame_number != 0 && single_small_frame_size != 0));
  }

  new_single_large_frame_size = single_large_frame_size + 2;
  if (accumulate_small_frame_number != 0)
  {
    new_single_small_frame_size = single_small_frame_size + 2;
  }
  else
  {
    new_single_small_frame_size = 0;
  }

  // Check if accumulate_frame_size is not equal to buffersize
  if (accumulate_frame_size != buffersize)
  {
    g_print("Warning: accumulate frame size (%zu) is not equal to buffer size (%zu)\n",
            accumulate_frame_size, buffersize);
    g_assert(accumulate_frame_size == buffersize);
  }
  else
  {
    // Process normally if the sizes match
    // Your processing logic here
  }

  /* Map the buffer for reading */
  GstMapInfo map_info;
  gboolean success = gst_buffer_map(buf, &map_info, GST_MAP_READ);
  if (!success)
  {
    g_print("Failed to map buffer\n");
    return GST_FLOW_ERROR;
  }

  /* Get the data */
  guint8 *data = map_info.data;
  gsize size = map_info.size;

  /* Unmap the buffer when done */
  gst_buffer_unmap(buf, &map_info);
  guint indx;
  guint8 pack_accumulate_large_frame_number = (guint8)filter->accumulate_large_frame_number;
  guint8 pack_accumulate_small_frame_number = (guint8)filter->accumulate_small_frame_number;
  for (indx = 1; indx <= filter->accumulate_large_frame_number; indx++)
  {
    guint8 pack_indx = (guint8)indx;
    g_print("%u\n", indx);

    /* Compute the segment of data */
    gsize offset = (indx - 1) * filter->single_large_frame_size;
    gsize segment_size = filter->single_large_frame_size;
    if (offset + segment_size > size)
    {
      /* Handle error: data is not enough */
      g_print("Not enough data in buffer\n");
      return GST_FLOW_ERROR;
    }

    guint8 *pack_data_segment = data + offset;

    /* Prepare new data: pack_accumulate_frame_number + pack_indx + pack_data_segment */
    guint8 *new_data = g_malloc(new_single_large_frame_size);
    if (!new_data)
    {
      /* Handle error: memory allocation failed */
      g_print("Memory allocation failed\n");
      return GST_FLOW_ERROR;
    }
    new_data[0] = pack_accumulate_large_frame_number;
    new_data[1] = pack_indx;
    memcpy(new_data + 2, pack_data_segment, filter->single_large_frame_size);

    /* Create a new buffer */
    GstBuffer *new_buffer = gst_buffer_new_allocate(NULL, new_single_large_frame_size, NULL);
    if (!new_buffer)
    {
      g_print("Failed to allocate new buffer\n");
      g_free(new_data);
      return GST_FLOW_ERROR;
    }

    /* Map new buffer for writing */
    GstMapInfo map_info_new;
    success = gst_buffer_map(new_buffer, &map_info_new, GST_MAP_WRITE);
    if (!success)
    {
      g_print("Failed to map buffer for new buffer\n");
      gst_buffer_unref(new_buffer);
      g_free(new_data);
      return GST_FLOW_ERROR;
    }

    /* Fill the new buffer with new_data */
    memcpy(map_info_new.data, new_data, new_single_large_frame_size);

    /* Unmap the new buffer */
    gst_buffer_unmap(new_buffer, &map_info_new);

    /* Free new_data */
    g_free(new_data);

    /* Push the new buffer downstream */
    GstFlowReturn ret = gst_pad_push(filter->srcpad, new_buffer);
    if (ret != GST_FLOW_OK)
    {
      g_print("Failed to push buffer downstream\n");
      return ret;
    }
  }

  if (accumulate_small_frame_number != 0)
  {
    for (indx = 1; indx <= filter->accumulate_small_frame_number; indx++)
    {
      guint8 pack_indx = (guint8)indx;
      g_print("%u\n", indx);

      /* Compute the segment of data */
      gsize offset = filter->accumulate_large_frame_number * filter->single_large_frame_size;
      gsize segment_size = filter->single_small_frame_size;
      if (offset + segment_size > size)
      {
        /* Handle error: data is not enough */
        g_print("Not enough data in buffer\n");
        return GST_FLOW_ERROR;
      }

      guint8 *pack_data_segment = data + offset;

      /* Prepare new data: pack_accumulate_frame_number + pack_indx + pack_data_segment */
      guint8 *new_data = g_malloc(new_single_small_frame_size);
      if (!new_data)
      {
        /* Handle error: memory allocation failed */
        g_print("Memory allocation failed\n");
        return GST_FLOW_ERROR;
      }
      new_data[0] = pack_accumulate_small_frame_number;
      new_data[1] = pack_accumulate_large_frame_number + pack_accumulate_small_frame_number;
      memcpy(new_data + 2, pack_data_segment, filter->single_small_frame_size);

      /* Create a new buffer */
      GstBuffer *new_buffer = gst_buffer_new_allocate(NULL, new_single_small_frame_size, NULL);
      if (!new_buffer)
      {
        g_print("Failed to allocate new buffer\n");
        g_free(new_data);
        return GST_FLOW_ERROR;
      }

      /* Map new buffer for writing */
      GstMapInfo map_info_new;
      success = gst_buffer_map(new_buffer, &map_info_new, GST_MAP_WRITE);
      if (!success)
      {
        g_print("Failed to map buffer for new buffer\n");
        gst_buffer_unref(new_buffer);
        g_free(new_data);
        return GST_FLOW_ERROR;
      }

      /* Fill the new buffer with new_data */
      memcpy(map_info_new.data, new_data, new_single_small_frame_size);

      /* Unmap the new buffer */
      gst_buffer_unmap(new_buffer, &map_info_new);

      /* Free new_data */
      g_free(new_data);

      /* Push the new buffer downstream */
      GstFlowReturn ret = gst_pad_push(filter->srcpad, new_buffer);
      if (ret != GST_FLOW_OK)
      {
        g_print("Failed to push buffer downstream\n");
        return ret;
      }
    }
  }

  // return GST_FLOW_CUSTOM_SUCCESS;
  return ret;
}

static void
gst_splitaddheadc_finalize(GObject *object)
{
  GstSplitaddheadc *filter = GST_SPLITADDHEADC(object);

  G_OBJECT_CLASS(gst_splitaddheadc_parent_class)->finalize(object);
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
  return gst_element_register(plugin, "splitaddheadc", GST_RANK_NONE,
                              GST_TYPE_SPLITADDHEADC);
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

/* gstreamer looks for this structure to register splitaddheadc
 *
 * exchange the string 'Template splitaddheadc' with your splitaddheadc description
 */
GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  splitaddheadc,
                  "splitaddheadc",
                  plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
