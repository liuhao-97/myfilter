#ifndef _GST_DATA_RESCHEDULE_H_
#define _GST_DATA_RESCHEDULE_H_

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include <gst/base/gstadapter.h>
#include <gst/gstelement.h>

G_BEGIN_DECLS

#define GST_TYPE_DATA_RESCHEDULE (gst_data_reschedule_get_type())
#define GST_DATA_RESCHEDULE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_DATA_RESCHEDULE, GstDataReschedule))
#define GST_DATA_RESCHEDULE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_DATA_RESCHEDULE, GstDataRescheduleClass))
#define GST_IS_DATA_RESCHEDULE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_DATA_RESCHEDULE))
#define GST_IS_DATA_RESCHEDULE_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_DATA_RESCHEDULE))

typedef struct _GstDataReschedule GstDataReschedule;
typedef struct _GstDataRescheduleClass GstDataRescheduleClass;

struct _GstDataReschedule
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  gboolean silent;
};

struct _GstDataRescheduleClass
{
  GstElementClass parent_class; /**< parent class */
};

/**
 * @brief Function to get type of data_reschedule.
 */
GType gst_data_reschedule_get_type(void);

G_END_DECLS

#endif /* __GST_MYFILTER_H__ */
