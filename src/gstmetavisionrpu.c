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


#include "gstmetavisionrpu.h"

GST_META_VISION_RPU_DLL_API GType gst_meta_vision_rpu_api_get_type(void)
{
	static volatile GType type;
	static const gchar *tags[] = { NULL };

	if (g_once_init_enter(&type))
	{
		GType _type = gst_meta_api_type_register("GstMetaVisionRpuAPI", tags);
		g_once_init_leave(&type, _type);
	}

	return type;
}

gboolean gst_meta_vision_rpu_init(GstMeta *meta, gpointer params, GstBuffer *buffer)
{
	GstMetaVisionRpu *meta_vision_rpu = (GstMetaVisionRpu*)meta;

	meta_vision_rpu->data = NULL;
	meta_vision_rpu->size = 0;

	return TRUE;
}

gboolean gst_meta_vision_rpu_transform(GstBuffer *dest_buf, GstMeta *src_meta, GstBuffer *src_buf, GQuark type, gpointer data)
{
	GstMetaVisionRpu *dst_meta_vision_rpu = GST_META_VISION_RPU_ADD(dest_buf);
	GstMetaVisionRpu *src_meta_vision_rpu = (GstMetaVisionRpu*)src_meta;

	dst_meta_vision_rpu->data = g_memdup(src_meta_vision_rpu->data, src_meta_vision_rpu->size);
	dst_meta_vision_rpu->size = src_meta_vision_rpu->size;

	return TRUE;
}

void gst_meta_vision_rpu_free(GstMeta *meta, GstBuffer *buffer)
{
	GstMetaVisionRpu *meta_vision_rpu = (GstMetaVisionRpu*)meta;

	g_free(meta_vision_rpu->data);
}

GST_META_VISION_RPU_DLL_API const GstMetaInfo *gst_meta_vision_rpu_get_info(void)
{
	static const GstMetaInfo *meta_info = NULL;

	if (g_once_init_enter(&meta_info))
	{
		const GstMetaInfo *meta = gst_meta_register(
			gst_meta_vision_rpu_api_get_type(),
			"GstMetaVisionRpu",
			sizeof(GstMetaVisionRpu),
			gst_meta_vision_rpu_init,
			gst_meta_vision_rpu_free,
			gst_meta_vision_rpu_transform
		);

		g_once_init_leave(&meta_info, meta);
	}

	return meta_info;
}
