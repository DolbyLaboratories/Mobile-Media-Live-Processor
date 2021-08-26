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

#include <stdbool.h>
#include <stdio.h>

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/base/gstaggregator.h>
#include <gst/base/gstqueuearray.h>
#include <gmodule.h>

#include "dv_ves_mux.h"
#include "vesmux_linker.h"
#include "gstdlbmux.h"
#include "gstmetavisionrpu.h"


 /* Dolby Vision MMB Live Muxer Plugin */
static GstStaticPadTemplate sink_pad = GST_STATIC_PAD_TEMPLATE( "sink_0",
	GST_PAD_SINK,
	GST_PAD_REQUEST,
	GST_STATIC_CAPS( "video/x-h265" )
);
static GstStaticPadTemplate rpu_pad = GST_STATIC_PAD_TEMPLATE( "sink_rpu",
	GST_PAD_SINK,
	GST_PAD_REQUEST,
	GST_STATIC_CAPS( "video/x-h265; video/x-raw" )
);
static GstStaticPadTemplate src_pad = GST_STATIC_PAD_TEMPLATE( "src",
	GST_PAD_SRC,
	GST_PAD_ALWAYS,
	GST_STATIC_CAPS( "video/x-h265" )
);




G_DEFINE_TYPE( GstDlbMuxerPad, gst_dlb_muxer_pad, GST_TYPE_AGGREGATOR_PAD );
enum
{
	PROP_PAD_0,
};

static void gst_dlb_muxer_pad_init( GstDlbMuxerPad* pad )
{

}

static void gst_dlb_muxer_pad_get_property( GObject* object, guint prop_id, GValue* value, GParamSpec* pspec )
{
	// GstDlbMuxerPad *dlb_muxer_pad = GST_dlb_muxer_PAD(object);
	switch( prop_id )
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
		break;
	}
}


static void gst_dlb_muxer_pad_set_property( GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec )
{
	// GstDlbMuxerPad *dlb_muxer_pad = GST_dlb_muxer_PAD(object);
	switch( prop_id )
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
		break;
	}
}


static void gst_dlb_muxer_pad_class_init( GstDlbMuxerPadClass* klass )
{
	GObjectClass* gobject_class = ( GObjectClass* )klass;
	// GstAggregatorPadClass *aggregatorpad_class = (GstAggregatorPadClass *) klass;

	gobject_class->get_property = gst_dlb_muxer_pad_get_property;
	gobject_class->set_property = gst_dlb_muxer_pad_set_property;
}


G_DEFINE_TYPE( GstDlbMuxer, gst_dlb_muxer, GST_TYPE_AGGREGATOR );
GST_DEBUG_CATEGORY_STATIC( dlbmuxer_debug );
#define GST_CAT_DEFAULT dlbmuxer_debug


enum { PROP_0, };

static void gst_dlb_muxer_set_property( GObject* object, guint prop_id,
	const GValue* value, GParamSpec* pspec )
{
	// GstDlbMuxer *dlb_muxer = GST_dlb_muxer (object);
	switch( prop_id )
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
		break;
	}
}

static void gst_dlb_muxer_get_property( GObject* object, guint prop_id,
	GValue* value, GParamSpec* pspec )
{
	// GstDlbMuxer *dlb_muxer = GST_dlb_muxer (object);
	switch( prop_id )
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
		break;
	}
}


static gint gst_dlb_muxer_RBSPtoEBSP( guint8 *ebsp_buf, guint8 *rbsp_buf, gint rbsp_length )
{
	gint j = 0, i;
	gint zero_cnt = 0;
	for( i = 0; i < rbsp_length; i++ )
	{
		if( zero_cnt == 2 )
		{
			if( !( rbsp_buf[ i ] & 0xFC ) )
			{
				ebsp_buf[ j++ ] = 0x03;
				zero_cnt = 0;
			}
		}
		ebsp_buf[ j++ ] = rbsp_buf[ i ];
		zero_cnt += rbsp_buf[ i ] == 0;
	}

	return j;
}


static GstFlowReturn gst_dlb_muxer_aggregate( GstAggregator* aggregator, gboolean timeout )
{
	GstElement* element = GST_ELEMENT( aggregator );
	GstDlbMuxer* muxer = GST_dlb_muxer( aggregator );
	gboolean bitstream_data = TRUE, rpu_data = TRUE;
	gpointer first = NULL, second = NULL;
	GstFlowReturn ret = GST_FLOW_OK;
	gboolean rpu_is_metadata = FALSE;

	gst_element_set_locked_state( element, TRUE );

	gint i = 0;
	GList* iter;
	for( iter = element->sinkpads; iter; iter = iter->next, i++ )
	{
		switch( i )
		{
		case 0:
			first = iter->data;
			break;
		case 1:
			second = iter->data;
			break;
		default: // Two sinks, not more
			ret = GST_FLOW_ERROR;
			goto bail;
		}
	}

	if( !first || !second )
	{
		// Two sinks, not less
		ret = GST_FLOW_ERROR;
		goto bail;
	}

	GstCaps* caps = gst_pad_get_current_caps( second );
	if( caps && gst_caps_get_size( caps ) > 0 )
	{
		GstStructure* s = gst_caps_get_structure( caps, 0 );
		const gchar* str = gst_structure_get_name( s );
		if( ( str != NULL ) && ( strcmp( str, "video/x-raw" ) == 0 ) )
		{
			rpu_is_metadata = TRUE;
		}
	}

	for( ; bitstream_data || rpu_data;)
	{
		dv_ves_mux_rc_t vesmux_result;
		GstBuffer* incoming_buffer;
		GstAggregatorPad* pad;

		pad = ( GstAggregatorPad* )first;
		incoming_buffer = gst_aggregator_pad_peek_buffer( pad );

		if( incoming_buffer )
		{
			TimeStamp time_stamp;
			GstMapInfo mapinfo = { 0 };
			gst_buffer_map( incoming_buffer, &mapinfo, GST_MAP_READ );
			// save pts and dts
			time_stamp.pts = GST_BUFFER_PTS( incoming_buffer );
			time_stamp.dts = GST_BUFFER_DTS( incoming_buffer );
			time_stamp.duration = GST_BUFFER_DURATION( incoming_buffer );
			gst_queue_array_push_tail_struct( muxer->clock_times, ( gpointer )( &time_stamp ) );

			vesmux_result = dv_ves_mux_bl_process( muxer->ves_muxer, mapinfo.data, ( uint32_t )( mapinfo.size ) );
			if( DV_VES_MUX_OK == vesmux_result )
			{
				gst_aggregator_pad_pop_buffer( pad );
			}
			else if( DV_VES_MUX_ES_INPUT_QUEUE_FULL == vesmux_result )
			{
				if( !rpu_data )
				{
					bitstream_data = FALSE;
				}
			}
			else
			{
				ret = GST_FLOW_ERROR;
				goto bail;
			}
			gst_buffer_unref( incoming_buffer );
		}
		else
		{
			bitstream_data = FALSE;
		}

		if( rpu_is_metadata )
		{
			pad = ( GstAggregatorPad* )second;
			incoming_buffer = gst_aggregator_pad_peek_buffer( pad );

			if( incoming_buffer )
			{
				GstMetaVisionRpu* meta = GST_META_VISION_RPU_GET( incoming_buffer );
				if( !meta )
				{
					ret = GST_FLOW_ERROR;
					goto bail;
				}
				gint ebspSize = gst_dlb_muxer_RBSPtoEBSP( muxer->ebsp_buf + 4, meta->data, meta->size );
				*( ( uint32_t* )muxer->ebsp_buf ) = 0x1000000;  // adding start code
				ebspSize += 4;
				vesmux_result = dv_ves_mux_rpu_process( muxer->ves_muxer, muxer->ebsp_buf, ebspSize );
				if( DV_VES_MUX_OK == vesmux_result )
				{
					gst_aggregator_pad_pop_buffer( pad );
				}
				else if( DV_VES_MUX_ES_INPUT_QUEUE_FULL == vesmux_result )
				{
					if( !bitstream_data )
					{
						rpu_data = FALSE;
						GST_DEBUG_OBJECT( muxer, "rpu- but no bitstream-data (1)" );
					}
				}
				else if( vesmux_result != DV_VES_MUX_OK )
				{
					ret = GST_FLOW_ERROR;
					goto bail;
				}
				gst_buffer_unref( incoming_buffer );
			}
			else
			{
				rpu_data = FALSE;
			}
		}
		else
		{
			pad = ( GstAggregatorPad* )second;
			incoming_buffer = gst_aggregator_pad_peek_buffer( pad );

			if( incoming_buffer )
			{
				GstMapInfo mapinfo = { 0 };
				gst_buffer_map( incoming_buffer, &mapinfo, GST_MAP_READ );
				vesmux_result = dv_ves_mux_rpu_process( muxer->ves_muxer, mapinfo.data, ( uint32_t )( mapinfo.size ) );
				if( DV_VES_MUX_OK == vesmux_result )
				{
					gst_aggregator_pad_pop_buffer( pad );
				}
				else if( DV_VES_MUX_ES_INPUT_QUEUE_FULL == vesmux_result )
				{
					if( !bitstream_data )
					{
						rpu_data = FALSE;
						GST_DEBUG_OBJECT( muxer, "rpu- but no bitstream-data (2)" );
					}
				}
				else if( vesmux_result != DV_VES_MUX_OK )
				{
					ret = GST_FLOW_ERROR;
					goto bail;
				}
				gst_buffer_unref( incoming_buffer );
			}
			else
			{
				rpu_data = FALSE;
			}
		}
	}
bail:
	gst_element_set_locked_state( element, FALSE );
	if( 2 == muxer->cnt_eos ) {
		GST_DEBUG_OBJECT( muxer, "aggregate EOS" );
		dv_ves_mux_flush( muxer->ves_muxer );
		gst_pad_push_event( muxer->parent.srcpad, gst_event_new_eos( ) );
		return GST_FLOW_EOS;
	}

	return ret;
}


void dv_ves_mux_inbuf_release_cb( void* p_ctx, dv_es_t es_id, uint8_t* p_es_buf )
{
	GstDlbMuxer* muxer = p_ctx;
	GST_DEBUG_OBJECT( muxer, "vesmuxer release signal" );
}


void dv_ves_mux_mux_outdata_cb( void* p_ctx, uint8_t* p_dv_ves_buf, uint32_t dv_ves_buf_len )
{
	GstDlbMuxer* muxer = p_ctx;

	GstCaps* outcaps = gst_pad_get_current_caps( muxer->parent.srcpad );
	if( outcaps == NULL )
	{
		GST_DEBUG_OBJECT( muxer, "no output caps, source pad has been deactivated" );
		return;
	}
	gst_caps_unref( outcaps );

	GstMapInfo info_out;
	GstBuffer* buffer_out = gst_buffer_new_allocate( NULL, dv_ves_buf_len, NULL );

	TimeStamp* t = ( TimeStamp* )( gst_queue_array_pop_head_struct( muxer->clock_times ) );
	GST_DEBUG_OBJECT( muxer, " --> out pts = %f    dts = %f    \n", ( double )( t->pts ) / 1000000000.0, ( double )( t->dts ) / 1000000000.0 );
	GST_BUFFER_PTS( buffer_out ) = t->pts;
	GST_BUFFER_DTS( buffer_out ) = t->dts;
	GST_BUFFER_DURATION( buffer_out ) = t->duration;

	gst_buffer_map( buffer_out, &info_out, GST_MAP_WRITE );
	memcpy( info_out.data, p_dv_ves_buf, dv_ves_buf_len );
	gst_buffer_unmap( buffer_out, &info_out );

	gst_pad_push( muxer->parent.srcpad, buffer_out );
	GST_DEBUG_OBJECT( muxer, "vesmuxer output" );
	// static int i=0;
	// printf("MUX %i\n", i++);
}


gboolean gst_dlb_muxer_sink_event( GstAggregator* aggregator, GstAggregatorPad* aggregator_pad, GstEvent* event )
{
	GstDlbMuxer* muxer = GST_dlb_muxer( aggregator );
	gchar* pad_name = gst_pad_get_name( aggregator_pad );

	switch( GST_EVENT_TYPE( event ) )
	{
	case GST_EVENT_EOS:
		GST_DEBUG_OBJECT( muxer, "flush vesmuxer" );
		muxer->cnt_eos++;
		break;
	case GST_EVENT_FLUSH_START:
	case GST_EVENT_FLUSH_STOP:
		break;
	default:
		break;
	}

	g_free( pad_name );
	// gst_object_unref(gstmapper);
	return GST_AGGREGATOR_CLASS( gst_dlb_muxer_parent_class )->sink_event( aggregator, aggregator_pad, event );
}


static GstPad* gst_dlb_muxer_request_new_pad( GstElement* element, GstPadTemplate* templ, const gchar* name, const GstCaps* caps ) {
	GstPad* pad;
	pad = gst_pad_new_from_template( templ, name );
	gst_element_add_pad( element, pad );
	return pad;
}


static void gst_dlb_muxer_release_pad( GstElement* element, GstPad* pad ) {
	gst_element_remove_pad( element, pad );
}


static void gst_dlb_muxer_finalize( GObject* object )
{
	GstDlbMuxer* muxer = GST_dlb_muxer( object );

	dv_ves_mux_destroy( muxer->ves_muxer );
	muxer->ves_muxer = NULL;
	g_free( muxer->ebsp_buf );
	muxer->ebsp_buf = NULL;

	gst_queue_array_free( muxer->clock_times );
}


GstFlowReturn gst_dlb_muxer_update_src_caps( GstAggregator* self, GstCaps* caps, GstCaps** ret )
{
	// GstDlbMuxer *muxer = GST_dlb_muxer(self);
	return GST_AGGREGATOR_CLASS( gst_dlb_muxer_parent_class )->update_src_caps( self, caps, ret );
}


GstCaps* gst_dlb_muxer_fixate_src_caps( GstAggregator* self, GstCaps* caps )
{
	// GstDlbMuxer *muxer = GST_dlb_muxer(self);
	return GST_AGGREGATOR_CLASS( gst_dlb_muxer_parent_class )->fixate_src_caps( self, caps );
}


static void gst_dlb_muxer_class_init( GstDlbMuxerClass* klass )
{
	GObjectClass* gobject_class;
	GstElementClass* gstelement_class;
	GstAggregatorClass* gstaggregator_class;

	gobject_class = ( GObjectClass* )klass;
	gstelement_class = ( GstElementClass* )klass;
	gstaggregator_class = ( GstAggregatorClass* )klass;

	gobject_class->set_property = gst_dlb_muxer_set_property;
	gobject_class->get_property = gst_dlb_muxer_get_property;
	gobject_class->finalize = gst_dlb_muxer_finalize;

	gstaggregator_class->aggregate = gst_dlb_muxer_aggregate;
	gstaggregator_class->sink_event = gst_dlb_muxer_sink_event;
	gstaggregator_class->update_src_caps = gst_dlb_muxer_update_src_caps;
	gstaggregator_class->fixate_src_caps = gst_dlb_muxer_fixate_src_caps;
	gstelement_class->request_new_pad = gst_dlb_muxer_request_new_pad;
	gstelement_class->release_pad = gst_dlb_muxer_release_pad;

	gst_element_class_set_static_metadata( gstelement_class,
		"Dolby Vision Muxer Plugin",
		"Generic",
		"Dolby Vision Muxer Plugin",
		"Dolby Laboratories / contact@dolby.com" );

	gst_element_class_add_static_pad_template_with_gtype( gstelement_class, &rpu_pad, GST_TYPE_dlb_muxer_PAD );
	gst_element_class_add_static_pad_template_with_gtype( gstelement_class, &sink_pad, GST_TYPE_dlb_muxer_PAD );
	gst_element_class_add_static_pad_template_with_gtype( gstelement_class, &src_pad, GST_TYPE_AGGREGATOR_PAD );

	GST_DEBUG_CATEGORY_INIT( dlbmuxer_debug, "dlbmuxer", 0, "Dolby Vision Muxer" );
}


static void gst_dlb_muxer_init( GstDlbMuxer* muxer ) 
{
	muxer->ves_muxer = dv_ves_mux_create( );
	dv_ves_mux_conf_t ves_cfg = {
		.mpeg_muxer_config_bits = 0,
		.f_secondary_vid_present = 0,
		.primary_vid_codec = DV_HEVC_CODEC,
		.secondary_vid_codec = DV_HEVC_CODEC,
		.rpu_reorder_type = RPU_REORDER_TYPE_BL,
		.pf_msg_log = ( msg_log_func_t )g_print,
		.pf_inbuf_release_cb = dv_ves_mux_inbuf_release_cb,
		.p_inbuf_release_cb_ctx = muxer,
		.pf_mux_outdata_cb = dv_ves_mux_mux_outdata_cb,
		.p_mux_outdata_cb_ctx = muxer,
	};

	dv_ves_mux_rc_t ves_ret;
	if( DV_VES_MUX_OK != ( ves_ret = dv_ves_mux_init( muxer->ves_muxer, &ves_cfg ) ) )
	{
		GST_ERROR_OBJECT( muxer, "ves muxer initialization failed with code %i.", ves_ret );
	}

	muxer->cnt_eos = 0;
	muxer->ebsp_buf_length = 10000000;
	muxer->ebsp_buf = g_malloc( muxer->ebsp_buf_length );

	muxer->clock_times = gst_queue_array_new_for_struct( sizeof( TimeStamp ), 64 );
}




#define PACKAGE "dlb_muxer"
#define VERSION "0.0.0"
#define LICENSE "Proprietary"
#define DESCRIPTION "Dolby Vision Muxer"
#define BINARY_PACKAGE "Dolby Vision Muxer Plugin"
#define URL "http://dolby.com"

static gboolean dlb_muxer_plugin_init( GstPlugin* plugin ) {
	#ifdef VES_MUX_OPTIONAL
	if (!ves_muxer_try_link()) {
		return FALSE;
	}
	#endif
	return gst_element_register( plugin, "dlbmux", GST_RANK_NONE, GST_TYPE_dlb_muxer );
}


GST_PLUGIN_DEFINE( GST_VERSION_MAJOR,
	GST_VERSION_MINOR,
	dlbmux,
	DESCRIPTION,
	dlb_muxer_plugin_init,
	VERSION,
	LICENSE,
	BINARY_PACKAGE,
	URL )
