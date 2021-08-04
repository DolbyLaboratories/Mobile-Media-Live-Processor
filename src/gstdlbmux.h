/**
 * This program is protected under international and U.S. copyright laws as
 * an unpublished work. This program is confidential and proprietary to the
 * copyright owners. Reproduction or disclosure, in whole or in part, or the
 * production of derivative works therefrom without the express permission of
 * the copyright owners is prohibited.
 *
 *      Copyright (C) 2021 by Dolby Laboratories.
 *                All rights reserved.
*/


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

GST_EXPORT GType gst_dlb_muxer_pad_get_type(void);


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

	GstQueueArray* clock_times;
};

struct _GstDlbMuxerClass
{
	GstAggregatorClass parent_class;
};

GST_EXPORT GType gst_dlb_muxer_get_type(void);

G_END_DECLS

