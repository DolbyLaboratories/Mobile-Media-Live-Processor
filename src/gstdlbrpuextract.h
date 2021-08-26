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


#ifndef _GST_RX_H_
#define _GST_RX_H_

#include <gst/gst.h>
#include <gst/gstmeta.h>
#include <gst/base/gstqueuearray.h>

G_BEGIN_DECLS

#define GST_TYPE_RX (gst_rpux_get_type())
#define GST_RX(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_RX, GstBerRX))
#define GST_RX_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_RX, GstBerRXClass))


typedef struct _GstBerRX GstBerRX;
typedef struct _GstBerRXClass GstBerRXClass;
typedef struct _TimeStamp TimeStamp;

struct _TimeStamp
{
	GstClockTime pts;
	GstClockTime dts;
	guint32 doc;
}; 

struct _GstBerRX 
{
	GstVideoDecoder base_decoder;
	lean_parser_t *p_lean_parser;
	GstVideoCodecState *codec_state;
	gpointer nal_buffer;
	gsize nal_buffer_size;
	GstPad *passthru;

	GstQueueArray* clock_times;
	guint32 access_unit_cnt;
};

struct _GstBerRXClass 
{
	GstVideoDecoderClass base_rpux_class;
	gboolean (*default_sink_event)(GstVideoDecoder * decoder, GstEvent * event);
};

GType gst_rpux_get_type( void );

G_END_DECLS

#endif // _GST_RX_H_
