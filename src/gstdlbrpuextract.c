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



#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>

#include <gst/gst.h>
#include <gst/video/video-format.h>
#include <gst/video/gstvideodecoder.h>


#include "lean_parser.h"
#include "lp_linker.h"
#include "gstdlbrpuextract.h"
#include "gstmetavisionrpu.h"


 /* Dolby Vision MMB Live Processor Plugin */


 /*
	"src" pad is RPU out (with dummy image data),
	"passthru" pad is STREAM(=input) out.
 */

GST_DEBUG_CATEGORY_STATIC( gst_rx_debug_category );
#define GST_CAT_DEFAULT gst_rx_debug_category

#define IN_CAPS "video/x-h265, stream-format=byte-stream"
static GstStaticPadTemplate gst_rx_sink_template = GST_STATIC_PAD_TEMPLATE(
	"sink", GST_PAD_SINK, GST_PAD_ALWAYS, GST_STATIC_CAPS( IN_CAPS ) );
static GstStaticPadTemplate gst_rx_passthru_template = GST_STATIC_PAD_TEMPLATE(
	"passthru", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS( IN_CAPS ) );
#define VIDEO_OUT_CAPS GST_VIDEO_CAPS_MAKE("{ GRAY8 }")

G_DEFINE_TYPE_WITH_CODE(
	GstBerRX,
	gst_rpux,
	GST_TYPE_VIDEO_DECODER,
	GST_DEBUG_CATEGORY_INIT(
		gst_rx_debug_category, "dlbrpux", 0, "debug category for dlbrpux element" ) );




static void gst_rpux_set_property( GObject* object, guint property_id, const GValue* value, GParamSpec* pspec )
{
	GstBerRX* rx = GST_RX( object );
	GST_DEBUG_OBJECT( rx, "set_property" );
	switch( property_id )
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, pspec );
		break;
	}
}
static void gst_rpux_get_property( GObject* object, guint property_id, GValue* value, GParamSpec* pspec )
{
	GstBerRX* rx = GST_RX( object );
	GST_DEBUG_OBJECT( rx, "get_property" );
	switch( property_id )
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, pspec );
		break;
	}
}

static void gst_rpux_finalize( GObject* object )
{
	GstBerRX* rx = GST_RX( object );
	lean_parser_destroy( rx->p_lean_parser );
}



static gint find_frame( gconstpointer a, gconstpointer b ) 
{
	GstVideoCodecFrame* frame = ( GstVideoCodecFrame* )a;
	lp_access_unit_t* decoded_frame = ( lp_access_unit_t* )b;
	return ( int64_t )decoded_frame->pts - frame->pts;
}

static void unref_frame( gpointer frame )
{
	gst_video_codec_frame_unref( frame );
}


gint compare_decoding_order_cnt( gconstpointer a, gconstpointer b )
{
	TimeStamp* t1 = ( TimeStamp* )( a );
	TimeStamp* t2 = ( TimeStamp* )( b );

	if( t1->doc > t2->doc )
	{
		return 1;
	}
	else if( t1->doc < t2->doc )
	{
		return -1;
	}
	return 0;
}


void on_bump( lp_access_unit_t* p_au, enum_au_seis_t fun, void* rx_ )
{
	GstBerRX *rx = GST_RX( rx_ );
	if (p_au->sps_hevc->vui_parameters_present_flag) {
		static bool done = false;
		if (!done++) {
			bool override_frame_rate = true;
			GstCaps *incaps = gst_pad_get_current_caps(GST_VIDEO_DECODER( rx )->sinkpad);
			if (incaps) {
				gint num, denom;
				GstStructure *structure = gst_caps_get_structure(incaps, 0);
				if (gst_structure_get_fraction(structure, "framerate", &num, &denom)) {
					if (!!num) {
						override_frame_rate = false;
					}
				}
			}
			if (override_frame_rate) {
				rx->codec_state->caps = gst_caps_make_writable (rx->codec_state->caps);
				gst_caps_set_simple(rx->codec_state->caps, "framerate", GST_TYPE_FRACTION,
									p_au->sps_hevc->vui.vui_time_scale, p_au->sps_hevc->vui.vui_num_units_in_tick, NULL);
				if (!gst_pad_set_caps(GST_VIDEO_DECODER( rx )->srcpad, rx->codec_state->caps)) {
					GST_DEBUG_OBJECT(rx, "gst_pad_set_caps override failed");
				}
			}
		}
	}
	
	GList* frames = gst_video_decoder_get_frames( GST_VIDEO_DECODER( rx ) );
	GList* tmp = g_list_find_custom( frames, p_au, find_frame );
	GstVideoCodecFrame* frame;
	if( !tmp )
	{
		frame = gst_video_decoder_get_oldest_frame( GST_VIDEO_DECODER( rx ) );
	}
	else
	{
		frame = ( GstVideoCodecFrame* )tmp->data;
	}
	g_list_free_full( frames, unref_frame );

	static char dummybuff[ 32 * 32 ] = { 0 };
	if( GST_FLOW_OK != gst_video_decoder_allocate_output_frame( GST_VIDEO_DECODER( rx ), frame ) )
	{
		return;
	}
	gst_buffer_fill( frame->output_buffer, 0, dummybuff, sizeof( dummybuff ) );

	// set pts and dts
	guint length = gst_queue_array_get_length( rx->clock_times );
	for( guint i = 0; i < length; i++ )
	{
		TimeStamp* t = ( TimeStamp* )( gst_queue_array_peek_nth_struct( rx->clock_times, i ) );
		if( t->doc == p_au->doc )
		{
			GST_DEBUG_OBJECT( rx, " --> out pts = %f    dts = %f    \n", ( double )( t->pts ) / 1000000000.0, ( double )( t->dts ) / 1000000000.0 );
			frame->pts = GST_BUFFER_PTS( frame->output_buffer ) = t->pts;
			frame->dts = GST_BUFFER_DTS( frame->output_buffer ) = GST_CLOCK_TIME_NONE;
			gst_queue_array_drop_struct( rx->clock_times, i, NULL );
			break;
		}
	}

	if( p_au->rpu && p_au->rpu_blob_len )
	{
		GstMetaVisionRpu* meta = GST_META_VISION_RPU_ADD( frame->output_buffer );
		meta->data = g_malloc( p_au->rpu_blob_len );
		memcpy( meta->data, p_au->rpu, p_au->rpu_blob_len );
		meta->size = p_au->rpu_blob_len;
	}
	gst_video_decoder_finish_frame( GST_VIDEO_DECODER( rx ), frame );
}


static void gst_decoder_finalize( GObject* object )
{
	GstBerRX* rx = GST_RX( object );
	GST_DEBUG_OBJECT( rx, "finalize" );

	/* clean up object here */
	lean_parser_push_bytes( rx->p_lean_parser, NULL, 0, -1, -1, on_bump, NULL, NULL, rx );
	lean_parser_destroy( rx->p_lean_parser );
	
	gst_queue_array_free( rx->clock_times );

	if( rx->codec_state != NULL )
	{
		gst_video_codec_state_unref( rx->codec_state );
	}

	G_OBJECT_CLASS( gst_rpux_parent_class )->finalize( object );
}

static gboolean gst_decoder_start( GstVideoDecoder* decoder )
{
	GstBerRX* rx = GST_RX( decoder );
	GST_DEBUG_OBJECT( rx, "start" );
	return TRUE;
}
static gboolean gst_decoder_stop( GstVideoDecoder* decoder )
{
	GstBerRX* rx = GST_RX( decoder );
	GST_DEBUG_OBJECT( rx, "stop" );
	return TRUE;
}

static gboolean gst_decoder_set_format( GstVideoDecoder* decoder, GstVideoCodecState* state )
{
	GstBerRX* rx = GST_RX( decoder );
	GST_DEBUG_OBJECT( rx, "set_format" );
	if( rx->codec_state != NULL )
	{
		gst_video_codec_state_unref( rx->codec_state );
	}
	rx->codec_state = gst_video_codec_state_ref( state );

	gst_video_decoder_set_output_state( GST_VIDEO_DECODER( rx ), GST_VIDEO_FORMAT_GRAY8,
		32, 32, rx->codec_state );

	return TRUE;
}

static gboolean gst_decoder_flush( GstVideoDecoder* decoder )
{
	GstBerRX* rx = GST_RX( decoder );
	GST_DEBUG_OBJECT( rx, "flush" );
	lean_parser_push_bytes( rx->p_lean_parser, NULL, 0, -1, -1, on_bump, NULL, NULL, rx );
	return TRUE;
}

static GstFlowReturn gst_decoder_finish( GstVideoDecoder* decoder )
{
	GstBerRX* rx = GST_RX( decoder );
	GST_DEBUG_OBJECT( rx, "finish" );

	if( rx->p_lean_parser )
	{
		lean_parser_push_bytes( rx->p_lean_parser, NULL, 0, -1, -1, on_bump, NULL, NULL, rx );
	}

	return GST_FLOW_OK;
}

static GstFlowReturn gst_decoder_handle_frame( GstVideoDecoder* decoder, GstVideoCodecFrame* frame )
{
	GstMapInfo info;
	int32_t err;

	GstBerRX* rx = GST_RX( decoder );

	gst_video_codec_frame_ref( frame );
	gst_buffer_map( frame->input_buffer, &info, GST_MAP_READ );

	TimeStamp time_stamp;
	time_stamp.pts = GST_BUFFER_PTS( frame->input_buffer );
	time_stamp.dts = GST_BUFFER_DTS( frame->input_buffer );
	time_stamp.doc = rx->access_unit_cnt++;
	GST_DEBUG_OBJECT( rx, " --> in  pts = %f    dts = %f    \n", ( double )( time_stamp.pts ) / 1000000000.0, ( double )( time_stamp.dts ) / 1000000000.0 );
	gst_queue_array_push_tail_struct( rx->clock_times, ( gpointer )( &time_stamp ) );
	
	lean_parser_return_t ret = lean_parser_push_bytes( rx->p_lean_parser, info.data, ( uint32_t )info.size,
													  GST_BUFFER_PTS( frame->input_buffer ), GST_BUFFER_DTS( frame->input_buffer ),
													  on_bump, NULL, NULL, rx );
	if( ret == lp_Error )
	{
		const char* err;
		lean_parser_last_error( rx->p_lean_parser, &err );
		GST_DEBUG_OBJECT( rx, "lean_parser_push_bytes() error: %s", err );
	}

	if( gst_pad_is_linked( rx->passthru ) )
	{
		GstMapInfo info_out;
		GstBuffer* buffer_out = gst_buffer_copy( frame->input_buffer );
		gst_pad_push( rx->passthru, buffer_out );
	}

	gst_video_codec_frame_unref( frame );

	return GST_FLOW_OK;
}


static GstFlowReturn gst_decoder_drain( GstVideoDecoder* decoder )
{
	GstBerRX* rx = GST_RX( decoder );
	GST_DEBUG_OBJECT( rx, "drain" );
	return GST_FLOW_OK;
}



static gboolean gst_decoder_sink_event( GstVideoDecoder* decoder, GstEvent* event )
{
	GstBerRX* rx = GST_RX( decoder );
	GST_DEBUG_OBJECT( rx, "sink_event %i", GST_EVENT_TYPE( event ) );

	switch( GST_EVENT_TYPE( event ) )
	{
	case GST_EVENT_CAPS:
		if( gst_pad_is_linked( rx->passthru ) )
		{
			GstCaps* caps;
			if( gst_pad_has_current_caps( decoder->srcpad ) )
			{
				caps = gst_caps_copy( gst_pad_get_current_caps( decoder->srcpad ) );
				gst_pad_set_caps( rx->passthru, caps );
				gst_pad_push_event( rx->passthru, gst_event_new_caps( caps ) );
			}
			else
			{
				gst_event_parse_caps( event, &caps );
				GstCaps* outcaps = gst_caps_copy( caps );
				gst_pad_set_caps( rx->passthru, outcaps );
				gst_pad_push_event( rx->passthru, gst_event_new_caps( outcaps ) );
				gst_caps_unref( outcaps );
			}
		}
		break;
			
	case GST_EVENT_QOS:
		(void)(0);
	case GST_EVENT_EOS:
		(void)(0);
	case GST_EVENT_SEGMENT:
		(void)(0);
	default:
		if (gst_pad_is_linked( rx->passthru)) {
			GstEvent* ce = gst_event_copy(event);
			gst_pad_push_event(rx->passthru, ce);
		}
		break;
	}

	return GST_VIDEO_DECODER_CLASS( gst_rpux_parent_class )->sink_event( decoder, event );
}

static void gst_rpux_class_init( GstBerRXClass* klass )
{
	GObjectClass* gobject_class = G_OBJECT_CLASS( klass );
	GstVideoDecoderClass* decoder_class = GST_VIDEO_DECODER_CLASS( klass );

	gst_element_class_add_static_pad_template( GST_ELEMENT_CLASS( klass ), &gst_rx_sink_template );
	gst_element_class_add_pad_template( GST_ELEMENT_CLASS( klass ), gst_pad_template_new( "src", GST_PAD_SRC, GST_PAD_ALWAYS, gst_caps_from_string( VIDEO_OUT_CAPS ) ) );
	gst_element_class_add_pad_template( GST_ELEMENT_CLASS( klass ), gst_pad_template_new( "passthru", GST_PAD_SRC, GST_PAD_ALWAYS, gst_caps_from_string( IN_CAPS ) ) );

	gst_element_class_set_static_metadata(
		GST_ELEMENT_CLASS( klass ),
		"Dolby Vision RPU Extractor",
		"dvdec",
		"Extracts Dolby Vision RPUs from AVC / HEVC streams",
		"Dolby Laboratories <contact@dolby.com>" );

	klass->default_sink_event = decoder_class->sink_event;

	gobject_class->finalize = gst_rpux_finalize;
	gobject_class->set_property = gst_rpux_set_property;
	gobject_class->get_property = gst_rpux_get_property;

	decoder_class->start = gst_decoder_start;
	decoder_class->stop = gst_decoder_stop;
	decoder_class->set_format = gst_decoder_set_format;
	decoder_class->flush = gst_decoder_flush;
	decoder_class->finish = gst_decoder_finish;
	decoder_class->drain = gst_decoder_drain;
	decoder_class->handle_frame = gst_decoder_handle_frame;
	decoder_class->sink_event = gst_decoder_sink_event;
}

static void gst_rpux_init( GstBerRX* rx )
{
	rx->p_lean_parser = lean_parser_create( lpCodec_HEVC );
	rx->nal_buffer_size = 0;
	rx->nal_buffer = NULL;

	rx->clock_times = gst_queue_array_new_for_struct( sizeof( TimeStamp ), 64 );
	rx->access_unit_cnt = 0;

	rx->passthru = gst_pad_new_from_static_template( &gst_rx_passthru_template, "passthru" );
	gst_element_add_pad( GST_ELEMENT( rx ), rx->passthru );
}


static gboolean plugin_init( GstPlugin *plugin )
{
	#ifdef LEAN_PARSER_OPTIONAL
	if (!lp_try_link()) {
		return FALSE;
	}
	#endif
	return gst_element_register( plugin, "dlbrpux", GST_RANK_NONE, GST_TYPE_RX );
}





#ifndef VERSION
#define VERSION "0.0.0"
#endif
#ifndef PACKAGE
#define PACKAGE "Dolby Vision RPU Extractor"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "Dolby Vision RPU Extractor"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://dolby.com"
#endif

GST_PLUGIN_DEFINE(
	GST_VERSION_MAJOR,
	GST_VERSION_MINOR,
	dlbrpux,
	"Dolby Vision RPU Extractor",
	plugin_init,
	VERSION,
	"Proprietary",
	PACKAGE_NAME,
	GST_PACKAGE_ORIGIN )
