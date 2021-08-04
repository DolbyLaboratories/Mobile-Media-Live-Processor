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


#ifndef _GST_META_VISION_RPU_H_
#define _GST_META_VISION_RPU_H_

#include <gst/gst.h>
#include <gst/gstmeta.h>

#ifdef WIN32
	#ifdef EXPORT_GST_META_VISION_RPU
		#define GST_META_VISION_RPU_DLL_API __declspec(dllexport)
	#else
		#define GST_META_VISION_RPU_DLL_API __declspec(dllimport)
	#endif
#else
	#define GST_META_VISION_RPU_DLL_API
#endif

G_BEGIN_DECLS

typedef struct _GstMetaVisionRpu GstMetaVisionRpu;

struct _GstMetaVisionRpu {
	GstMeta meta;
	guint8 *data;
	gint size;
};

GST_META_VISION_RPU_DLL_API GType gst_meta_vision_rpu_api_get_type(void);
GST_META_VISION_RPU_DLL_API const GstMetaInfo* gst_meta_vision_rpu_get_info(void);
#define GST_META_VISION_RPU_GET(buf) ((GstMetaVisionRpu*)gst_buffer_get_meta(buf,gst_meta_vision_rpu_api_get_type()))
#define GST_META_VISION_RPU_ADD(buf) ((GstMetaVisionRpu*)gst_buffer_add_meta(buf,gst_meta_vision_rpu_get_info(),(gpointer)NULL))

G_END_DECLS

#endif
