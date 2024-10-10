/**
 * SECTION:element-timestampc
 *
 * FIXME:Describe timestampc here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! timestampc ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttimestampc.h"
#include <gst/gstelement.h>
#include <time.h>

GST_DEBUG_CATEGORY_STATIC(gst_timestampc_debug);
#define GST_CAT_DEFAULT gst_timestampc_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_POSITION,
  PROP_SILENT
};

/**
 * @brief Each frame data size in input buffer.
 */
#define DEFAULT_POSITION 0

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

#define gst_timestampc_parent_class parent_class
G_DEFINE_TYPE(GstTimestampc, gst_timestampc, GST_TYPE_ELEMENT);

static void gst_timestampc_set_property(GObject *object,
                                        guint prop_id, const GValue *value, GParamSpec *pspec);
static void gst_timestampc_get_property(GObject *object,
                                        guint prop_id, GValue *value, GParamSpec *pspec);
static void gst_timestampc_finalize(GObject *object);

static gboolean gst_timestampc_sink_event(GstPad *pad,
                                          GstObject *parent, GstEvent *event);
static GstFlowReturn gst_timestampc_chain(GstPad *pad,
                                          GstObject *parent, GstBuffer *buf);

/* GObject vmethod implementations */

/* initialize the timestampc's class */
static void
gst_timestampc_class_init(GstTimestampcClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *)klass;
  gstelement_class = (GstElementClass *)klass;

  gobject_class->set_property = gst_timestampc_set_property;
  gobject_class->get_property = gst_timestampc_get_property;
  gobject_class->finalize = gst_timestampc_finalize;

  /**
   * GstTensorReschedule::frames-in:
   *
   *
   */

  g_object_class_install_property(gobject_class, PROP_POSITION,
                                  g_param_spec_uint("position", "position",
                                                    "position", 0, G_MAXUINT,
                                                    DEFAULT_POSITION, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class, PROP_SILENT,
                                  g_param_spec_boolean("silent", "Silent", "Produce verbose output ?",
                                                       FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple(gstelement_class,
                                       "Timestampc",
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
gst_timestampc_init(GstTimestampc *filter)
{
  filter->sinkpad = gst_pad_new_from_static_template(&sink_factory, "sink");
  gst_pad_set_event_function(filter->sinkpad,
                             GST_DEBUG_FUNCPTR(gst_timestampc_sink_event));
  gst_pad_set_chain_function(filter->sinkpad,
                             GST_DEBUG_FUNCPTR(gst_timestampc_chain));
  GST_PAD_SET_PROXY_CAPS(filter->sinkpad);
  gst_element_add_pad(GST_ELEMENT(filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template(&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS(filter->srcpad);
  gst_element_add_pad(GST_ELEMENT(filter), filter->srcpad);

  filter->silent = FALSE;

  filter->position = DEFAULT_POSITION;
}

static void
gst_timestampc_set_property(GObject *object, guint prop_id,
                            const GValue *value, GParamSpec *pspec)
{
  GstTimestampc *filter = GST_TIMESTAMPC(object);

  switch (prop_id)
  {
  case PROP_POSITION:
    filter->position = g_value_get_uint(value);
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
gst_timestampc_get_property(GObject *object, guint prop_id,
                            GValue *value, GParamSpec *pspec)
{
  GstTimestampc *filter = GST_TIMESTAMPC(object);

  switch (prop_id)
  {
  case PROP_POSITION:
    g_value_set_uint(value, filter->position);
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
gst_timestampc_sink_event(GstPad *pad, GstObject *parent,
                          GstEvent *event)
{
  GstTimestampc *filter;
  gboolean ret;

  filter = GST_TIMESTAMPC(parent);

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
gst_timestampc_chain(GstPad *pad, GstObject *parent, GstBuffer *buf)
{
  GstTimestampc *filter;
  gsize position;
  struct timespec receive_timestamp;
  // float timenow;
  // gsize buffersize = gst_buffer_get_size(buf); // Get the size of the buffer data

  filter = GST_TIMESTAMPC(parent);
  position = filter->position;

  clock_gettime(CLOCK_REALTIME, &receive_timestamp);
  g_print("position: %zu, timestamp: %ld.%ld\n", position, receive_timestamp.tv_sec, receive_timestamp.tv_nsec);
  // g_print("position: %zu, timestamp: %.9f %.9f\n", position, timenow);

  return gst_pad_push(filter->srcpad, buf);
}

static void
gst_timestampc_finalize(GObject *object)
{
  GstTimestampc *filter = GST_TIMESTAMPC(object);

  G_OBJECT_CLASS(gst_timestampc_parent_class)->finalize(object);
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
  return gst_element_register(plugin, "timestampc", GST_RANK_NONE,
                              GST_TYPE_TIMESTAMPC);
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

/* gstreamer looks for this structure to register timestampc
 *
 * exchange the string 'Template timestampc' with your timestampc description
 */
GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  timestampc,
                  "timestampc",
                  plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
