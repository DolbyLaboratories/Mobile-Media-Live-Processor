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

#include <gst/gst.h>
#include <gst/video/video-format.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


#ifdef HAVE_TIMING
	#include "timing.h"
#endif

#include "slbc_mmb.h"
#include "slbc_mmb_linker.h"
#include "gstdlbmapper.h"
#include "gstmetavisionrpu.h"

GST_DEBUG_CATEGORY_STATIC(GST_DLB_MAPPER_debug_category);
#define GST_CAT_DEFAULT GST_DLB_MAPPER_debug_category


static GstStaticPadTemplate GST_DLB_MAPPER_sink_template =
	GST_STATIC_PAD_TEMPLATE("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
							GST_STATIC_CAPS("video/x-raw, format = {I420,I420_10LE,I420_12LE}"));
static GstStaticPadTemplate GST_DLB_MAPPER_rpu_sink_template =
	GST_STATIC_PAD_TEMPLATE("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
							GST_STATIC_CAPS("video/x-raw"));
#define RPU_OUT_CAPS GST_VIDEO_CAPS_MAKE("{ GRAY8 }")


#define VIDEO_OUT_CAPS GST_VIDEO_CAPS_MAKE("{I420, I420_12LE, I420_10LE}")



G_DEFINE_TYPE_WITH_CODE(
	gstDlbMapper,
	GST_DLB_MAPPER,
	GST_TYPE_ELEMENT,
	GST_DEBUG_CATEGORY_INIT(
		GST_DLB_MAPPER_debug_category, "dlbmapper", 0, "debug category for dlbmapper element"));

enum {
	PROP_0,
	PROP_OPERATION,
};


const char *INPUT_VID = "vid";
const char *INPUT_RPU = "rpu";
const char *OUTPUT_BS = "bs";
const char *OUTPUT_RPU = "rpu_out";

static GstElementClass *___mapper_element_class__ = 0;


void slbc_callback(slbc_result_t *p420_ptr, void *mp) {
	gstDlbMapper *gstmapper = (gstDlbMapper *)mp;
	
	GstMapInfo info_out = { 0 };
	int32_t bytes = gstmapper->output81 ? sizeof(uint16_t) : sizeof(uint8_t);
	GstBuffer *buffer_out = gst_buffer_new_allocate(NULL, bytes * gstmapper->width*gstmapper->height*3/2, NULL);
	gst_buffer_map(buffer_out, &info_out, GST_MAP_WRITE);
	int32_t chroma_stride = bytes * gstmapper->width/2;
	
	GST_BUFFER_PTS(buffer_out) = gstmapper->ui64_current_pts;
	GST_BUFFER_DTS(buffer_out) = gstmapper->ui64_current_dts;
	
	uint8_t *ptr;
	ptr = (uint8_t *)info_out.data;
	memcpy(ptr, p420_ptr->p_y, bytes * gstmapper->width*gstmapper->height);
	ptr += bytes * gstmapper->width * gstmapper->height;
	
	for (int32_t k=0; k<gstmapper->height/2; k++) {
		memcpy(ptr, p420_ptr->p_cb, bytes * gstmapper->width/2);
		p420_ptr->p_cb += chroma_stride;
		ptr += chroma_stride;
	}
	for (int32_t k=0; k<gstmapper->height/2; k++) {
		memcpy(ptr, p420_ptr->p_cr, bytes * gstmapper->width/2);
		p420_ptr->p_cr += chroma_stride;
		ptr += chroma_stride;
	}
	
	if (p420_ptr->rpu_data && p420_ptr->rpu_data_length) {
		GstMetaVisionRpu *meta = GST_META_VISION_RPU_ADD( buffer_out );
		meta->data = g_malloc( p420_ptr->rpu_data_length );
		memcpy( meta->data, p420_ptr->rpu_data, p420_ptr->rpu_data_length );
		meta->size = p420_ptr->rpu_data_length;
	}
	
	gst_buffer_unmap(buffer_out, &info_out);
	gst_pad_push(gstmapper->srcpad, buffer_out);
	
	if (p420_ptr->rpu_data && p420_ptr->rpu_data_length) {
		if (gstmapper->rpupad) {
			GstBuffer *rpu_buffer = gst_buffer_new_allocate(NULL, 32*32, NULL);
			GST_BUFFER_PTS(rpu_buffer) = gstmapper->ui64_current_pts;
	  		GST_BUFFER_DTS(rpu_buffer) = gstmapper->ui64_current_dts;
			// GstMapInfo info_rpu = { 0 };
			// gst_buffer_map(rpu_buffer, &info_rpu, GST_MAP_WRITE);
			
			GstMetaVisionRpu *meta = GST_META_VISION_RPU_ADD( rpu_buffer );
			meta->data = g_malloc( p420_ptr->rpu_data_length-4 );
			memcpy( meta->data, p420_ptr->rpu_data+4, p420_ptr->rpu_data_length-4 );
			meta->size = p420_ptr->rpu_data_length-4;

			gst_pad_push(gstmapper->rpupad, rpu_buffer);
			// gst_buffer_unmap(rpu_buffer, &info_rpu);
		}
	}
	GST_DEBUG_OBJECT(gstmapper, "output %1.5f", (float)GST_BUFFER_PTS( buffer_out ) / 1000000000.f);
}

void GST_DLB_MAPPER_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec)
{
	gstDlbMapper *gstmapper = GST_DLB_MAPPER(object);

	switch (prop_id) {
	case PROP_OPERATION:
		if (!strcmp("8.1", g_value_get_string(value))) {
			gstmapper->output81 = true;
			
			if (gstmapper->output81) {
				if (!gstmapper->rpupad) {
					GstPadTemplate *p_template = gst_element_class_get_pad_template(___mapper_element_class__, "output_rpu_pad_template");
					gstmapper->rpupad = gst_pad_new_from_template(p_template, OUTPUT_RPU);
					gst_pad_set_active(gstmapper->rpupad, TRUE);
					gst_element_add_pad(GST_ELEMENT(object), gstmapper->rpupad);
				}
			} else if (gstmapper->rpupad) {
				gst_pad_set_active(gstmapper->rpupad, FALSE);
				gst_element_remove_pad(GST_ELEMENT(object), gstmapper->rpupad);
			}
		}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}


static void GST_DLB_MAPPER_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	gstDlbMapper *gstmapper = GST_DLB_MAPPER(object);

	switch (prop_id) {
	case PROP_OPERATION:
		g_value_set_string(value, gstmapper->output81 ? "8.1":"sdr");
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}


static GstFlowReturn on_collect_data_available(GstCollectPads *pads, gpointer user_data)
{
	gstDlbMapper *gstmapper = (gstDlbMapper*)user_data;
	GstBuffer *p_buffer_vid = NULL, *p_buffer_rpu = NULL;

	// handle flushing state
	g_mutex_lock(&gstmapper->mtx_flushing);
	if (gstmapper->b_is_flushing) {
		GST_DEBUG_OBJECT(gstmapper, "flush in on_collect_data_available");
		g_mutex_unlock(&gstmapper->mtx_flushing);
		return GST_FLOW_FLUSHING;
	}
	g_mutex_unlock(&gstmapper->mtx_flushing);

	GST_COLLECT_PADS_STREAM_LOCK( gstmapper->collect );

	p_buffer_vid = gst_collect_pads_peek( gstmapper->collect, gstmapper->collect_data[ 0 ] );
	p_buffer_rpu = gst_collect_pads_peek( gstmapper->collect, gstmapper->collect_data[ 1 ] );
	
	if (!p_buffer_vid && !p_buffer_rpu) {
		slbc_flush(gstmapper->slbc_ptr);
		gst_pad_push_event(gstmapper->srcpad, gst_event_new_eos());
		if (gstmapper->rpupad && gst_pad_is_linked(gstmapper->rpupad)) {
			gst_pad_push_event(gstmapper->rpupad, gst_event_new_eos());
		}
		GST_COLLECT_PADS_STREAM_UNLOCK(gstmapper->collect);
		GST_DEBUG_OBJECT(gstmapper, "send EOS in on_collect_data_available");
		return GST_FLOW_EOS;
	}
	GST_COLLECT_PADS_STREAM_UNLOCK(gstmapper->collect);

	// if (!p_buffer_rpu) {
	// 	GST_DEBUG_OBJECT(gstmapper, "no rpu buffer");
	// }
	// if (!p_buffer_vid) {
	// 	GST_DEBUG_OBJECT(gstmapper, "no video buffer");
	// }
	if (!p_buffer_vid || !p_buffer_rpu) {
		return GST_FLOW_OK;// ??
	}
	/*
	GstClockTime pts;
	GstClockTime dts;
	pts = GST_BUFFER_PTS( p_buffer_vid );
	dts = GST_BUFFER_DTS( p_buffer_vid );
	fprintf( stdout, " --> in vid  pts = %f    dts = %f    \n", ( double )( pts ) / 1000000000.0, ( double )( dts ) / 1000000000.0 );
	pts = GST_BUFFER_PTS( p_buffer_rpu );
	dts = GST_BUFFER_DTS( p_buffer_rpu );
	fprintf( stdout, " --> in rpu  pts = %f    dts = %f    \n", ( double )( pts ) / 1000000000.0, ( double )( dts ) / 1000000000.0 );
	*/

	if( ( GST_BUFFER_PTS( p_buffer_vid ) == GST_CLOCK_TIME_NONE ) || ( GST_BUFFER_PTS( p_buffer_rpu ) == GST_CLOCK_TIME_NONE ) ||
		( GST_BUFFER_PTS( p_buffer_vid ) == GST_BUFFER_PTS( p_buffer_rpu ) ) )
	{
		gst_collect_pads_pop( gstmapper->collect, gstmapper->collect_data[ 0 ] );
		gst_collect_pads_pop( gstmapper->collect, gstmapper->collect_data[ 1 ] );
	}
	else if( GST_BUFFER_PTS( p_buffer_vid ) > GST_BUFFER_PTS( p_buffer_rpu ) )
	{
		gst_collect_pads_pop( gstmapper->collect, gstmapper->collect_data[ 1 ] );
		GST_DEBUG_OBJECT(gstmapper, "drop RPU b%1.5f vs r%1.5f", (float)GST_BUFFER_PTS( p_buffer_vid )/1000000000.f, (float)GST_BUFFER_PTS( p_buffer_rpu )/1000000000.f);
		return GST_FLOW_OK;
	}
	else
	{
		g_assert( GST_BUFFER_PTS( p_buffer_vid ) < GST_BUFFER_PTS( p_buffer_rpu ) );
		gst_collect_pads_pop( gstmapper->collect, gstmapper->collect_data[ 0 ] );
		GST_DEBUG_OBJECT(gstmapper, "drop VID b%1.5f vs r%1.5f", (float)GST_BUFFER_PTS( p_buffer_vid )/1000000000.f, (float)GST_BUFFER_PTS( p_buffer_rpu )/1000000000.f);
		return GST_FLOW_OK;
	}
	
	GstMetaVisionRpu *gst_rpu_meta_data = GST_META_VISION_RPU_GET(p_buffer_rpu);
	g_assert(gst_rpu_meta_data != NULL);

	// FIXME: determine if this frame has shotcut info flag.
	// if so, send an event downstream for info.
	if (0)
	{
		GstEvent* event = gst_event_new_custom(GST_EVENT_CUSTOM_DOWNSTREAM, gst_structure_new("GstForceKeyUnit", NULL, NULL));
		gst_pad_push_event(gstmapper->srcpad, event);
	}

	GstMapInfo info_vid = { 0 };
	GstMapInfo info_rpu = { 0 };

	gst_buffer_map(p_buffer_vid, &info_vid, GST_MAP_READ);
	if (p_buffer_rpu != NULL) {
		gst_buffer_map(p_buffer_rpu, &info_rpu, GST_MAP_READ);
	}
	
	int32_t pts_vid = GST_BUFFER_PTS(p_buffer_vid);
	int32_t pts_rpu = GST_BUFFER_PTS(p_buffer_rpu);

	gstmapper->ui64_current_pts = GST_BUFFER_PTS(p_buffer_vid);
	gstmapper->ui64_current_dts = GST_BUFFER_DTS(p_buffer_vid);

	if (!gstmapper->slbc_ptr) {
		slbc_config_t config = {
			.pf_data_callback = slbc_callback,
			.p_data_callback_handle = (void *)gstmapper,
			.width = gstmapper->width,
			.height = gstmapper->height,
			.operation = gstmapper->output81 ? SlbcMmbOp_Output81 : SlbcMmbOp_OutputSDR
		};
		gstmapper->slbc_ptr = slbc_create(&config);
		if (!gstmapper->slbc_ptr) {
			GST_DEBUG_OBJECT(gstmapper, "SLBC init failed: %s", slbc_last_error(NULL));
			return GST_FLOW_ERROR;
		}
		
		GstCaps *caps = gst_caps_new_simple(
			"video/x-raw",
			"format", G_TYPE_STRING, gstmapper->output81 ? "I420_10LE" : "I420",
			"width", G_TYPE_INT, gstmapper->width,
			"height", G_TYPE_INT, gstmapper->height,
			"interlace-mode", G_TYPE_STRING, "progressive",
			"colorimetry", G_TYPE_STRING, gstmapper->output81 ? "bt2020" /* smpte240m? */: "bt709",
			"framerate", GST_TYPE_FRACTION, gstmapper->fps_num, gstmapper->fps_denom,
	 		NULL
		);

		gboolean b = gst_pad_set_caps(gstmapper->srcpad, caps);
		gst_caps_unref(caps);
		GST_DEBUG_OBJECT(gstmapper, "Set src caps");
		
		if (gstmapper->rpupad && gst_pad_is_linked(gstmapper->rpupad)) {
			GstCaps *caps_rpu = gst_caps_new_simple(
				"video/x-raw",
				"format", G_TYPE_STRING, "Gray8",
				"width", G_TYPE_INT, 32,
				"height", G_TYPE_INT, 32,
				NULL
			);
			gst_pad_set_caps(gstmapper->rpupad, caps_rpu);
			GST_DEBUG_OBJECT(gstmapper, "Set RPU caps");
		}
	}
	
	#if 0
	FILE *f = fopen("/Users/anedd/vid/trash/damn.yuv", "wb");
	fwrite((uint16_t *)info_vid.data, 2, gstmapper->width*gstmapper->height*3/2, f);
	fclose(f);
	#endif
	

	//double tic = tictoc();
	if (!slbc_process(gstmapper->slbc_ptr, (uint16_t *)info_vid.data, gst_rpu_meta_data->data, gst_rpu_meta_data->size)) {
		GST_DEBUG_OBJECT(gstmapper, "SLBC process failed: %s", slbc_last_error(gstmapper->slbc_ptr));
	}
	//printf("SLBC call duration = %1.5fsec\n", tictoc() - tic);
	
	gst_buffer_unmap(p_buffer_vid, &info_vid);
	gst_buffer_unref(p_buffer_vid);

	if (p_buffer_rpu) {
		gst_buffer_unmap(p_buffer_rpu, &info_rpu);
		gst_buffer_unref(p_buffer_rpu);
	}
	
	GST_DEBUG_OBJECT(gstmapper, "did frame pts=%i", pts_vid);
	return GST_FLOW_OK;
}

static GstStateChangeReturn GST_DLB_MAPPER_change_state(GstElement *element, GstStateChange transition)
{
	gstDlbMapper *gstmapper = GST_DLB_MAPPER(element);
	GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

	GST_DEBUG_OBJECT(gstmapper, "GST_DLB_MAPPER_change_state");

	switch (transition)
	{
	case GST_STATE_CHANGE_READY_TO_PAUSED:
		if (gst_pad_is_linked(gstmapper->sinkpad[0]) == TRUE) {
			gstmapper->collect_data[0] = gst_collect_pads_add_pad(
				gstmapper->collect, gstmapper->sinkpad[0], sizeof(GstCollectData), NULL, TRUE);
		}
		if (gst_pad_is_linked(gstmapper->sinkpad[1]) == TRUE) {
			gstmapper->collect_data[1] = gst_collect_pads_add_pad(
				gstmapper->collect, gstmapper->sinkpad[1], sizeof(GstCollectData), NULL, TRUE);
		}
		gst_collect_pads_start(gstmapper->collect);
		break;
	case GST_STATE_CHANGE_PAUSED_TO_READY:
		gst_collect_pads_stop(gstmapper->collect);
		break;
	default:
		break;
	}

	ret = GST_ELEMENT_CLASS(GST_DLB_MAPPER_parent_class)->change_state(element, transition);
	if (ret != GST_STATE_CHANGE_SUCCESS) {
		return ret;
	}

	switch (transition)
	{
	case GST_STATE_CHANGE_PAUSED_TO_READY:
		if (gst_pad_is_linked(gstmapper->sinkpad[0]) == TRUE) {
			gst_collect_pads_remove_pad(gstmapper->collect, gstmapper->sinkpad[0]);
		}
		if (gst_pad_is_linked(gstmapper->sinkpad[1]) == TRUE) {
			gst_collect_pads_remove_pad(gstmapper->collect, gstmapper->sinkpad[1]);
		}
		break;
	default:
		break;
	}

	return ret;
}


// dlbcomposer will not accept any input frames after this call until gst_dlbmapper_on_flush_stop got called
static void gst_dlbmapper_on_flush_start(gstDlbMapper *mapper)
{
	g_mutex_lock(&mapper->mtx_flushing);
	mapper->b_is_flushing = TRUE;

	GST_DEBUG_OBJECT(mapper, "gstdlbcomposer: GST_EVENT_FLUSH_START");

	GST_COLLECT_PADS_STREAM_LOCK(mapper->collect);
	gst_collect_pads_set_flushing(mapper->collect, TRUE);
	GST_COLLECT_PADS_STREAM_UNLOCK(mapper->collect);

	slbc_flush(mapper->slbc_ptr);
	g_mutex_unlock(&mapper->mtx_flushing);
}



static void gst_dlbmapper_on_flush_stop(gstDlbMapper* mapper)
{
	g_mutex_lock(&mapper->mtx_flushing);
	mapper->b_is_flushing = FALSE;

	GST_DEBUG_OBJECT(mapper, "GST_EVENT_FLUSH_STOP");

	GST_COLLECT_PADS_STREAM_LOCK(mapper->collect);
	gst_collect_pads_set_flushing(mapper->collect, FALSE);
	GST_COLLECT_PADS_STREAM_UNLOCK(mapper->collect);

	g_mutex_unlock(&mapper->mtx_flushing);
}


static gboolean
GST_DLB_MAPPER_sink_event(GstCollectPads* pads, GstCollectData* data, GstEvent* event, gpointer user_data)
{
	gstDlbMapper *gstmapper = (gstDlbMapper*)gst_pad_get_parent_element(data->pad);

	GstCaps *caps = NULL;
	gchar *pad_name = gst_pad_get_name(data->pad);

	gboolean bRet = FALSE;

	switch (GST_EVENT_TYPE(event))
	{
	case GST_EVENT_CAPS:
		GST_DEBUG_OBJECT(gstmapper, "GST_DLB_MAPPER_sink_event CAPS");

		gst_event_parse_caps(event, &caps);
		GstStructure *structure = gst_caps_get_structure(caps, 0);

		if (g_strcmp0(pad_name, INPUT_VID) == 0)
		{
			gchar *buffer;
			gst_structure_get_int(structure, "width", &gstmapper->width);
			gst_structure_get_int(structure, "height", &gstmapper->height);
			gst_structure_get_fraction( structure, "framerate", &gstmapper->fps_num, &gstmapper->fps_denom );
			if( gstmapper->fps_num == 0) 
			{
				gstmapper->fps_num = 60000; // make x265enc link
				gstmapper->fps_denom = 1001;
			}
			GstCaps *outcaps = gst_caps_copy(caps);
			gst_caps_set_simple( outcaps,
				"format", G_TYPE_STRING, gstmapper->output81 ? "I420_10LE" : "I420",
				"framerate", GST_TYPE_FRACTION, gstmapper->fps_num, gstmapper->fps_denom, // FIXME: must be set/kept upstream
				"pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,NULL);

			gst_pad_set_caps(gstmapper->srcpad, outcaps);
			gst_caps_unref(outcaps);
		} else if (!g_strcmp0(pad_name, INPUT_RPU)) {
			int z=0;
		}
			
		bRet = gst_collect_pads_event_default(pads, data, event, FALSE);
		break;
	case GST_EVENT_SEGMENT:
		GST_DEBUG_OBJECT(gstmapper, "GST_DLB_MAPPER_sink_event SEGMENT");
		/* fallthrough */
	case GST_EVENT_STREAM_START:
		/*
		if (g_strcmp0(pad_name, INPUT_VID) == 0)
		{
			const GstSegment *p_seg;
			gst_event_ref(event);
			gst_pad_event_default(data->pad, GST_OBJECT(gstmapper), event);
			gst_event_parse_segment(event, &p_seg);
			#ifdef _DEBUG
			gint picno_start = (int32_t)(23.976f * ((float)p_seg->start / 1000000000.0f) + 0.5f);
			gint picno_end = (int32_t)(23.976f * ((float)p_seg->stop / 1000000000.0f) + 0.5f);
			fprintf(stdout, "NEW_SEGMENT [%d, %d ] \n", picno_start, picno_end);
			#endif
			gst_event_unref(event);
		}
		bRet = gst_collect_pads_event_default(pads, data, event, FALSE);
		**/
		bRet = gst_collect_pads_event_default( pads, data, event, TRUE );
		if( g_strcmp0( pad_name, INPUT_VID ) == 0 )
		{
			gst_event_ref( event );
			gst_pad_push_event( gstmapper->srcpad, event );
		}
		bRet = TRUE;
	break;
	case GST_EVENT_EOS:
		GST_DEBUG_OBJECT(gstmapper, "GST_DLB_MAPPER_sink_event EOS");
		// let the collect pads handle it, but not send downstreams afterwards since we are going to do that
		bRet = gst_collect_pads_event_default(pads, data, event, TRUE);
		gst_event_ref(event);
		
		g_mutex_lock(&gstmapper->mtx_flushing);
		gstmapper->eos_cnt += 1;
	  	g_mutex_unlock(&gstmapper->mtx_flushing);
		// gst_pad_push_event(gstmapper->srcpad, event);
		break;
	case GST_EVENT_FLUSH_START:
		// let the collect pads handle it, but not send downstreams afterwards since we are going to do that
		bRet = gst_collect_pads_event_default(pads, data, event, TRUE);
		gst_event_ref(event);
		if (g_strcmp0(pad_name, INPUT_VID) == 0)
		{
			gst_dlbmapper_on_flush_start(gstmapper);
		}
		gst_pad_push_event(gstmapper->srcpad, event);
		bRet = TRUE;
		break;
	case GST_EVENT_FLUSH_STOP:
		// let the collect pads handle it, but not send downstreams afterwards since we are going to do that
		bRet = gst_collect_pads_event_default(pads, data, event, TRUE);
		gst_event_ref(event);
		if (g_strcmp0(pad_name, INPUT_VID) == 0)
		{
			gst_event_ref(event);
			gst_dlbmapper_on_flush_stop(gstmapper);
		}
		gst_pad_push_event(gstmapper->srcpad, event);
		bRet = TRUE;
		break;
	default:
		// let the collect pads handle it and send it downstreams afterwards
		bRet = gst_collect_pads_event_default(pads, data, event, FALSE);
			
		break;
	}

	if (GST_IS_EVENT(event) && gstmapper->rpupad && gst_pad_is_linked(gstmapper->rpupad)) {
		GstEvent *e = gst_event_copy(event);
		gst_pad_push_event(gstmapper->rpupad, e);
	}
	
	g_free(pad_name);
	gst_object_unref(gstmapper);

	return bRet;
}

static void GST_DLB_MAPPER_finalize(GObject* object)
{
	gstDlbMapper *gstmapper = GST_DLB_MAPPER(object);

	GST_DEBUG_OBJECT(gstmapper, "finalize");

	slbc_destroy(gstmapper->slbc_ptr);
	
	gst_object_unref(gstmapper->collect);
}


static void GST_DLB_MAPPER_class_init(gstDlbMapperClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GstElementClass* gstelement_class = GST_ELEMENT_CLASS(klass);

	___mapper_element_class__ = gstelement_class;

	gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass), &GST_DLB_MAPPER_sink_template);

	gst_element_class_add_pad_template(
		GST_ELEMENT_CLASS(klass),
		gst_pad_template_new("output_pad_template", GST_PAD_SRC, GST_PAD_ALWAYS, gst_caps_from_string(VIDEO_OUT_CAPS)));
	gst_element_class_add_pad_template( GST_ELEMENT_CLASS(klass),
		gst_pad_template_new("output_rpu_pad_template", GST_PAD_SRC, GST_PAD_SOMETIMES, gst_caps_from_string("video/x-h265")));

	size_t slbclen1 = strlen(slbc_get_api_ver());
	gchar *base_descr = "Tonemaps Dolby Vision Images (SLBC %s)";
	gchar *descr = malloc(slbclen1+strlen(base_descr)-2+1);
	sprintf(descr, base_descr, slbc_get_api_ver());
	
	gst_element_class_set_static_metadata(
		GST_ELEMENT_CLASS(klass),
		"Dolby Vision Mapper",
		"Generic", descr,
		"Dolby Laboratories <contact@dolby.com>");
	free(descr);
	
	gstelement_class->change_state = GST_DLB_MAPPER_change_state;
	gobject_class->set_property = GST_DLB_MAPPER_set_property;
	gobject_class->get_property = GST_DLB_MAPPER_get_property;
	gobject_class->finalize = GST_DLB_MAPPER_finalize;
	
	g_object_class_install_property(
		gobject_class, PROP_OPERATION,
		g_param_spec_string("operation", "Operation Mode", "Specify \"sdr\" or \"8.1\" ", "sdr", G_PARAM_READWRITE));
}

static void GST_DLB_MAPPER_init(gstDlbMapper *gstmapper)
{
	gstmapper->sinkpad[0] = gst_pad_new_from_static_template(&GST_DLB_MAPPER_sink_template, INPUT_VID);
	gstmapper->sinkpad[1] = gst_pad_new_from_static_template(&GST_DLB_MAPPER_rpu_sink_template, INPUT_RPU);

	GstPadTemplate *p_template = gst_element_class_get_pad_template(___mapper_element_class__, "output_pad_template");
	gstmapper->srcpad = gst_pad_new_from_template(p_template, OUTPUT_BS);

	gst_element_add_pad(GST_ELEMENT(gstmapper), gstmapper->sinkpad[0]);
	gst_element_add_pad(GST_ELEMENT(gstmapper), gstmapper->sinkpad[1]);
	gst_element_add_pad(GST_ELEMENT(gstmapper), gstmapper->srcpad);

	gstmapper->collect = gst_collect_pads_new();
	gst_collect_pads_set_function(gstmapper->collect, on_collect_data_available, gstmapper);
	gst_collect_pads_set_event_function(gstmapper->collect, GST_DLB_MAPPER_sink_event, NULL);

	GST_DEBUG_OBJECT(gstmapper, "init");

	gstmapper->width = 0;
	gstmapper->height = 0;
	gstmapper->b_is_flushing = FALSE;
	gstmapper->frame = 0;
	gstmapper->slbc_ptr = NULL;
	gstmapper->ui64_current_pts = -1;
	gstmapper->ui64_current_dts = -1;
	gstmapper->eos_cnt = 0;
	gstmapper->output81 = false;
	gstmapper->rpupad = NULL;
}




static gboolean plugin_init(GstPlugin *plugin) {
	#ifdef SLBC_MMB_OPTIONAL
	if (!slbc_mmb_try_link()) {
		return FALSE;
	}
	#endif
	return gst_element_register(plugin, "dlbmapper", GST_RANK_NONE, GST_TYPE_DLB_MAPPER);
}

#ifndef VERSION
#define VERSION "0.0.0"
#endif
#ifndef PACKAGE
#define PACKAGE "Dolby Vision Mapper"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "Dolby Vision Mapper"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://www.dolby.com/"
#endif

GST_PLUGIN_DEFINE(
	GST_VERSION_MAJOR,
	GST_VERSION_MINOR,
	dlbmapper,
	"Dolby Vision Mapper",
	plugin_init,
	VERSION,
	"Proprietary",
	PACKAGE_NAME,
	GST_PACKAGE_ORIGIN
)
