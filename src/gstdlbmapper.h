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
