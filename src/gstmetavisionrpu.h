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
