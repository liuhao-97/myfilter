#ifndef _GST_SLEEPC_H_
#define _GST_SLEEPC_H_

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include <gst/gstelement.h>

G_BEGIN_DECLS

#define GST_TYPE_SLEEPC (gst_sleepc_get_type())
#define GST_SLEEPC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_SLEEPC, GstSleepc))
#define GST_SLEEPC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_SLEEPC, GstSleepcClass))
#define GST_IS_SLEEPC(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_SLEEPC))
#define GST_IS_SLEEPC_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_SLEEPC))

typedef struct _GstSleepc GstSleepc;
typedef struct _GstSleepcClass GstSleepcClass;

struct _GstSleepc
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  guint ns; /**< number of frames in input buffer */

  gboolean silent;

  // GstAdapter *adapter;
};

struct _GstSleepcClass
{
  GstElementClass parent_class; /**< parent class */
};

/**
 * @brief Function to get type of Sleepc.
 */
GType gst_sleepc_get_type(void);

G_END_DECLS

#endif /* __GST_MYFILTER_H__ */
