#ifndef _GST_SPLITADDHEADC_H_
#define _GST_SPLITADDHEADC_H_

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
// #include <gst/base/gstadapter.h>
#include <gst/gstelement.h>

G_BEGIN_DECLS

#define GST_TYPE_SPLITADDHEADC (gst_splitaddheadc_get_type())
#define GST_SPLITADDHEADC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_SPLITADDHEADC, GstSplitaddheadc))
#define GST_SPLITADDHEADC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_SPLITADDHEADC, GstSplitaddheadcClass))
#define GST_IS_SPLITADDHEADC(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_SPLITADDHEADC))
#define GST_IS_SPLITADDHEADC_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_SPLITADDHEADC))

typedef struct _GstSplitaddheadc GstSplitaddheadc;
typedef struct _GstSplitaddheadcClass GstSplitaddheadcClass;

struct _GstSplitaddheadc
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  guint single_frame_size;       /**< number of frames in input buffer */
  guint accumulate_frame_number; /**< number of frames in output buffer */

  gboolean silent;

  // GstAdapter *adapter;
};

struct _GstSplitaddheadcClass
{
  GstElementClass parent_class; /**< parent class */
};

/**
 * @brief Function to get type of Splitaddheadc.
 */
GType gst_splitaddheadc_get_type(void);

G_END_DECLS

#endif /* __GST_MYFILTER_H__ */
