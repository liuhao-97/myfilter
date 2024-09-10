/* GStreamer
 * Copyright (C) 2024 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

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
  GstBaseTransform base_datareschedule;

  // GstElement element;  /**< parent object */
  GstPad *sinkpad;     /**< sink pad */
  GstPad *srcpad;      /**< src pad */
  GstAdapter *adapter; /**< adapter for data */
};

struct _GstDataRescheduleClass
{
  // GstElementClass parent_class; /**< parent class */

  GstBaseTransformClass base_datareschedule_class;
};

GType gst_data_reschedule_get_type(void);

G_END_DECLS

#endif
