/**
 * Copyright (C) 2021, Dolby Laboratories
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#ifndef _GST_VISION_MAPPER_H_
#define _GST_VISION_MAPPER_H_

#include <gst/base/gstcollectpads.h>
#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_DLB_MAPPER (GST_DLB_MAPPER_get_type())
#define GST_DLB_MAPPER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_DLB_MAPPER, gstDlbMapper))
#define GST_DLB_MAPPER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_DLB_MAPPER, gstDlbMapperClass))
#define GST_IS_DLB_MAPPER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_DLB_MAPPER))
#define GST_IS_DLB_MAPPER_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_DLB_MAPPER))

typedef struct _gstDlbMapper gstDlbMapper;
typedef struct _gstDlbMapperClass gstDlbMapperClass;

struct _gstDlbMapper
{
	GstElement base_dlbcomposer;

	GstPad *sinkpad[2];
	GstPad *srcpad;
	GstPad *rpupad;

	GstCollectPads* collect;
	GstCollectData* collect_data[2];

	gint width;
	gint height;

	slbc_t *slbc_ptr;
	
	gboolean b_is_flushing;
	GMutex mtx_flushing;
	guint frame;
	
	gint fps_denom;
	gint fps_num;
	uint64_t ui64_current_pts;
	uint64_t ui64_current_dts;
	uint32_t eos_cnt;
	bool output81;
};

struct _gstDlbMapperClass
{
	GstElementClass base_dlb_composer_class;
};

GType GST_DLB_MAPPER_get_type(void);

G_END_DECLS

#endif
