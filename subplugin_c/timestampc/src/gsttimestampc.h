#ifndef _GST_TIMESTAMPC_H_
#define _GST_TIMESTAMPC_H_

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include <gst/gstelement.h>

G_BEGIN_DECLS

#define GST_TYPE_TIMESTAMPC (gst_timestampc_get_type())
#define GST_TIMESTAMPC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TIMESTAMPC, GstTimestampc))
#define GST_TIMESTAMPC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TIMESTAMPC, GstTimestampcClass))
#define GST_IS_TIMESTAMPC(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TIMESTAMPC))
#define GST_IS_TIMESTAMPC_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TIMESTAMPC))

typedef struct _GstTimestampc GstTimestampc;
typedef struct _GstTimestampcClass GstTimestampcClass;

struct _GstTimestampc
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  guint position; /**< number of frames in input buffer */

  gboolean silent;

  // GstAdapter *adapter;
};

struct _GstTimestampcClass
{
  GstElementClass parent_class; /**< parent class */
};

/**
 * @brief Function to get type of Timestampc.
 */
GType gst_timestampc_get_type(void);

G_END_DECLS

#endif /* __GST_MYFILTER_H__ */
