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



G_BEGIN_DECLS

/// GstDlbMuxerPad
#define GST_TYPE_dlb_muxer_PAD            (gst_dlb_muxer_pad_get_type())
#define GST_dlb_muxer_PAD(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_dlb_muxer_PAD, GstDlbMuxerPad))
#define GST_dlb_muxer_PAD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_dlb_muxer_PAD, GstDlbMuxerPadClass))
#define GST_dlb_muxer_PAD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),GST_TYPE_dlb_muxer_PAD, GstDlbMuxerPadClass))
#define GST_IS_dlb_muxer_PAD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_dlb_muxer_PAD))
#define GST_IS_dlb_muxer_PAD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_dlb_muxer_PAD))

typedef struct _GstDlbMuxerPad GstDlbMuxerPad;
typedef struct _GstDlbMuxerPadClass GstDlbMuxerPadClass;

struct _GstDlbMuxerPad {
	GstAggregatorPad parent;
};
struct _GstDlbMuxerPadClass {
	GstAggregatorPadClass parent_class;
};

GType gst_dlb_muxer_pad_get_type(void);


/// GstDlbMuxer
#define GST_TYPE_dlb_muxer               (gst_dlb_muxer_get_type())
#define GST_dlb_muxer(obj)               (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_dlb_muxer, GstDlbMuxer))
#define GST_dlb_muxer_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_dlb_muxer,GstDlbMuxerClass))
#define GST_dlb_muxer_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS(obj), GST_TYPE_dlb_muxer, GstDlbMuxerClass))
#define GST_IS_dlb_muxer(obj)            (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_dlb_muxer))
#define GST_IS_dlb_muxer_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_dlb_muxer))
#define GST_dlb_muxer_CAST(obj)          ((GstDlbMuxer *)(obj))

/// GST DlbMuxer Structs
typedef struct _GstDlbMuxer      GstDlbMuxer;
typedef struct _GstDlbMuxerClass GstDlbMuxerClass;
typedef struct _TimeStamp TimeStamp;

struct _TimeStamp
{
	GstClockTime pts;
	GstClockTime dts;
	GstClockTime duration;
};

struct _GstDlbMuxer 
{
	GstAggregator parent;
	dv_ves_mux_handle_t ves_muxer;
	guint32 cnt_eos;
	gint ebsp_buf_length;
	guint8* ebsp_buf;
	gboolean rbsp_to_ebsp;

	GstQueueArray* clock_times;
};

struct _GstDlbMuxerClass
{
	GstAggregatorClass parent_class;
};

GType gst_dlb_muxer_get_type(void);

G_END_DECLS

