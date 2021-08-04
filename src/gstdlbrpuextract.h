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
