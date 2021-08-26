/*
* BSD 3-Clause License
*
* Copyright (c) 2018-2021, Dolby Laboratories
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice, this
*   list of conditions and the following disclaimer.
*
* * Redistributions in binary form must reproduce the above copyright notice,
*   this list of conditions and the following disclaimer in the documentation
*   and/or other materials provided with the distribution.
*
* * Neither the name of the copyright holder nor the names of its
*   contributors may be used to endorse or promote products derived from
*   this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

void *lnk_symbol(void *lib, const char *name);
void *lnk_open(const char *name);



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
