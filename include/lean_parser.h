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



#ifdef SHARED_LP_LIB
	#ifndef _WIN32
		#define LEAN_PARSER_API __attribute__ ((visibility ("default")))
	#else
		#define LEAN_PARSER_API __declspec(dllexport)
	#endif
#else
	#define LEAN_PARSER_API
#endif

#ifdef __cplusplus
extern "C" {
#endif



typedef struct
{
	uint32_t general_profile_space;
	uint32_t general_tier_flag;
	uint32_t general_profile_idc;
	uint8_t general_profile_compatibility_flag[32];
	uint8_t general_progressive_source_flag;
	uint8_t general_interlaced_source_flag;
	uint8_t general_non_packed_constraint_flag;
	uint8_t general_frame_only_constraint_flag;
	uint32_t general_level_idc;
	uint8_t sub_layer_profile_present_flag[8];
	uint8_t sub_layer_level_present_flag[8];
	uint32_t sub_layer_profile_space[8];
	uint8_t sub_layer_tier_flag[8];
	uint32_t sub_layer_profile_idc[8];
	uint8_t sub_layer_profile_compatibility_flag[8][32];
	uint8_t sub_layer_progressive_source_flag[8];
	uint8_t sub_layer_interlaced_source_flag[8];
	uint8_t sub_layer_non_packed_constraint_flag[8];
	uint8_t sub_layer_frame_only_constraint_flag[8];
	uint32_t sub_layer_level_idc[8];
} h265_ptl_t;

typedef struct
{
	int8_t i_id;
	int32_t i_max_sub_layers;
	int32_t i_max_layers;
	bool b_temporal_id_nesting;

	int32_t i_max_dec_pic_buffering;
	int32_t i_num_reorder_pics;
	int32_t i_max_latency_increase;

	int32_t i_num_hrd_params;
	int32_t i_vps_max_layer_id;
	int32_t i_vps_max_op_sets;

	bool b_vps_timing_info_present;
	uint32_t ui_vps_num_units_in_tick;
	uint32_t ui_vps_time_scale;
	bool b_vps_poc_proportional_to_timing;
	int32_t i_vps_num_ticks_poc_diff_one;

	h265_ptl_t ptl;
} h265_vps_t;


typedef struct
{
	uint8_t scaling_list_pred_mode_flag[ 4 ][ 6 ];
	uint32_t scaling_list_pred_matrix_id_delta[ 4 ][ 6 ];
	int32_t scaling_list_dc_coef_minus8[ 4 ][ 6 ];
	int32_t scaling_list_delta_coeff[ 4 ][ 6 ][ 64 ];
} h265_scaling_list_t;

typedef struct
{
	uint32_t bit_rate_value_minus1[ 32 ];
	uint32_t cpb_size_value_minus1[ 32 ];
	uint32_t cpb_size_du_value_minus1[ 32 ];
	uint32_t bit_rate_du_value_minus1[ 32 ];
	uint32_t cbr_flag[ 32 ];
} h265_sublayer_hrd_t;

typedef enum { nal = 0, vcl = 1 } h265_sublayer_hrd_type_t;
typedef struct
{
	uint8_t nal_hrd_parameters_present_flag;
	uint8_t vcl_hrd_parameters_present_flag;
	uint8_t sub_pic_hrd_params_present_flag;
	uint32_t tick_divisor_minus2;
	uint32_t du_cpb_removal_delay_increment_length_minus1;
	uint8_t sub_pic_cpb_params_in_pic_timing_sei_flag;
	uint32_t dpb_output_delay_du_length_minus1;
	uint32_t bit_rate_scale;
	uint32_t cpb_size_scale;
	uint32_t cpb_size_du_scale;
	uint32_t initial_cpb_removal_delay_length_minus1;
	uint32_t au_cpb_removal_delay_length_minus1;
	uint32_t dpb_output_delay_length_minus1;
	uint8_t fixed_pic_rate_general_flag[ 8 ];
	uint8_t fixed_pic_rate_within_cvs_flag[ 8 ];
	uint32_t elemental_duration_in_tc_minus1[ 8 ];
	uint8_t low_delay_hrd_flag[ 8 ];
	uint32_t cpb_cnt_minus1[ 8 ];

	h265_sublayer_hrd_t subLayerHRD[ 2 ][ 8 ];
} h265_hrd_t;

typedef struct
{
	uint8_t aspect_ratio_info_present_flag;
	uint32_t aspect_ratio_idc;
	uint32_t sar_width;
	uint32_t sar_height;
	uint8_t overscan_info_present_flag;
	uint8_t overscan_appropriate_flag;
	uint8_t video_signal_type_present_flag;
	uint32_t video_format;
	uint8_t video_full_range_flag;
	uint8_t colour_description_present_flag;
	uint32_t colour_primaries;
	uint32_t transfer_characteristics;
	uint32_t matrix_coeffs;
	uint8_t chroma_loc_info_present_flag;
	uint32_t chroma_sample_loc_top;
	uint32_t chroma_sample_loc_bottom;
	uint8_t neutral_chroma_indication_flag;
	uint8_t field_seq_flag;
	uint8_t frame_field_info_present_flag;
	uint8_t default_display_window_flag;
	uint32_t def_disp_win_left_offset;
	uint32_t def_disp_win_right_offset;
	uint32_t def_disp_win_top_offset;
	uint32_t def_disp_win_bottom_offset;
	uint8_t vui_timing_info_present_flag;
	uint32_t vui_num_units_in_tick;
	uint32_t vui_time_scale;
	uint8_t vui_poc_proportional_to_timing_flag;
	uint32_t vui_num_ticks_poc_diff_one_minus1;
	uint8_t vui_hrd_parameters_present_flag;
	uint8_t bitstream_restriction_flag;
	uint8_t tiles_fixed_structure_flag;
	uint8_t motion_vectors_over_pic_boundaries_flag;
	uint8_t restricted_ref_pic_lists_flag;
	uint32_t min_spatial_segmentation_idc;
	uint32_t max_bytes_per_pic_denom;
	uint32_t max_bitsRead_per_min_cu_denom;
	uint32_t log2_max_mv_length_horizontal;
	uint32_t log2_max_mv_length_vertical;
	h265_hrd_t hrd;
} h265_vui_t;

typedef struct
{
	int32_t NumNegativePics;
	int32_t NumPositivePics;
	int32_t NumDeltaPocs;
	int32_t DeltaPocS0[ 16 ];
	int32_t DeltaPocS1[ 16 ];
	int32_t UsedByCurrPicS0[ 16 ];
	int32_t UsedByCurrPicS1[ 16 ];
} h265_short_term_reference_picture_set_t;


typedef struct
{
	uint32_t sps_video_parameter_set_id;
	uint32_t sps_max_sub_layers_minus1;
	uint8_t sps_temporal_id_nesting_flag;
	uint32_t sps_seq_parameter_set_id;
	uint32_t chroma_format_idc;
	uint8_t separate_colour_plane_flag;
	uint32_t pic_width_in_luma_samples;
	uint32_t pic_height_in_luma_samples;
	uint32_t max_cu_depth, max_cu_size;
	uint8_t conformance_window_flag;
	uint32_t conf_win_left_offset;
	uint32_t conf_win_right_offset;
	uint32_t conf_win_top_offset;
	uint32_t conf_win_bottom_offset;
	uint32_t bit_depth_luma_minus8;
	uint32_t bit_depth_chroma_minus8;
	uint32_t log2_max_pic_order_cnt_lsb_minus4;
	uint8_t sps_sub_layer_ordering_info_present_flag;
	uint32_t sps_max_dec_pic_buffering_minus1[ 8 ];
	uint32_t sps_max_num_reorder_pics[ 8 ];
	uint32_t sps_max_latency_increase_plus1[ 8 ];
	uint32_t log2_min_luma_coding_block_size_minus3;
	uint32_t log2_diff_max_min_luma_coding_block_size;
	uint32_t log2_min_transform_block_size_minus2;
	uint32_t log2_diff_max_min_transform_block_size;
	uint32_t max_transform_hierarchy_depth_inter;
	uint32_t max_transform_hierarchy_depth_intra;
	uint32_t scaling_list_enabled_flag;
	uint8_t sps_scaling_list_data_present_flag;
	uint8_t amp_enabled_flag;
	uint8_t sample_adaptive_offset_enabled_flag;
	uint8_t pcm_enabled_flag;
	uint32_t pcm_sample_bit_depth_luma_minus1;
	uint32_t pcm_sample_bit_depth_chroma_minus1;
	uint32_t log2_min_pcm_luma_coding_block_size_minus3;
	uint32_t log2_diff_max_min_pcm_luma_coding_block_size;
	uint8_t pcm_loop_filter_disabled_flag;
	uint32_t num_short_term_ref_pic_sets;
	uint8_t long_term_ref_pics_present_flag;
	uint32_t num_long_term_ref_pics_sps;
	uint32_t lt_ref_pic_poc_lsb_sps[ 33 ];
	uint8_t used_by_curr_pic_lt_sps_flag[ 33 ];
	uint8_t sps_temporal_mvp_enabled_flag;
	uint8_t strong_intra_smoothing_enabled_flag;
	uint8_t vui_parameters_present_flag;
	uint8_t sps_extension_flag;

	h265_ptl_t ptl;
	h265_scaling_list_t sld;
	h265_vui_t vui;
	h265_short_term_reference_picture_set_t refPictureSets[ 64+1 ];
} h265_sps_t;

typedef struct
{
	uint32_t pps_pic_parameter_set_id;
	uint32_t pps_seq_parameter_set_id;
	uint32_t flags;
	uint32_t num_extra_slice_header_bits;
} h265_pps_t;



typedef enum
{
	lp_h265PicType_IDR,
	lp_h265PicType_CRA,
	lp_h265PicType_BLA,
	lp_h265PicType_RASL,
	lp_h265PicType_RADL,
	lp_h265PicType_TRAIL,

	lp_h265PicType_Unknown
	
} h265_picture_type_t;


/*
  ------------------------------- AVC --------------------------------
 */
typedef struct
{
	int32_t i_cpb_cnt_minus1;
	int32_t i_bit_rate_scale;
	int32_t i_cpb_size_scale;
	int32_t rgi_bit_rate_value_minus1[32];
	int32_t rgi_cpb_size_value_minus1[32];
	bool b_cbr_flag[32];
	int32_t i_initial_cpb_removal_delay_length_minus1;
	int32_t i_cpb_removal_delay_length_minus1;
	int32_t i_dpb_output_delay_length_minus1;
	int32_t i_time_offset_length;
} h264_hrd_parameter_t;

typedef struct
{
	bool valid;
	bool aspect_ratio_info_present_flag;
	int32_t aspect_ratio_idc;
	int32_t sar_width;
	int32_t sar_height;
	bool overscan_info_present_flag;
	bool overscan_appropriate_flag;
	bool video_signal_type_present_flag;
	int32_t video_format;
	bool video_full_range_flag;
	bool colour_description_present_flag;
	int32_t colour_primaries;
	int32_t transfer_characteristics;
	int32_t matrix_coefficients;
	bool chroma_loc_info_present_flag;
	int32_t chroma_sample_loc_type_top_field;
	int32_t chroma_sample_loc_type_bottom_field;
	bool timing_info_present_flag;
	uint32_t num_units_in_tick;
	int32_t time_scale;
	bool fixed_frame_rate_flag;
	bool nal_hrd_parameters_present_flag;
	bool vcl_hrd_parameters_present_flag;
	bool low_delay_hrd_flag;
	bool pic_struct_present_flag;
	bool bitstream_restriction_flag;
	bool motion_vectors_over_pic_boundaries_flag;
	int32_t max_bytes_per_pic_denom;
	int32_t max_bits_per_mb_denom;
	int32_t log2_max_mv_length_horizontal;
	int32_t log2_max_mv_length_vertical;
	int32_t num_reorder_frames;
	int32_t max_dec_frame_buffering;
	h264_hrd_parameter_t hrd_param;
} h264_vui_t;


typedef struct
{
#define MAX_NUM_REF_FRAMES_IN_PIC_ORDER_CNT_CYCLE			255

	uint32_t profile_idc;
	bool constraint_set0_flag;
	bool constraint_set1_flag;
	bool constraint_set2_flag;
	bool constraint_set3_flag;
	bool constraint_set4_flag;
	bool constraint_set5_flag;
	uint32_t level_idc;
	uint32_t seq_parameter_set_id;
	uint32_t log2_max_frame_num_minus4;
	uint32_t pic_order_cnt_type;
	uint32_t log2_max_pic_order_cnt_lsb_minus4;
	bool delta_pic_order_always_zero_flag;
	int32_t offset_for_non_ref_pic;
	int32_t offset_for_top_to_bottom_field;
	uint32_t num_ref_frames_in_pic_order_cnt_cycle;
	uint32_t offset_for_ref_frame[MAX_NUM_REF_FRAMES_IN_PIC_ORDER_CNT_CYCLE];
	uint32_t num_ref_frames;
	bool gaps_in_frame_num_value_allowed_flag;
	uint32_t pic_width_in_mbs;
	uint32_t pic_height_in_map_units;
	bool frame_mbs_only_flag;
	bool mb_adaptive_frame_field_flag;
	bool direct_8x8_inference_flag;
	bool frame_cropping_flag;
	uint32_t frame_crop_left_offset;
	uint32_t frame_crop_right_offset;
	uint32_t frame_crop_top_offset;
	uint32_t frame_crop_bottom_offset;
	bool vui_parameters_present_flag;

	/* for (ui_profile_idc >= 100) only */
	uint32_t chroma_format_idc;
	bool residual_colour_transform_flag;
	uint16_t bit_depth_luma_minus_8;
	uint16_t bit_depth_chroma_minus_8;
	bool qp_prime_y_zero_transform_bypass_flag;
	bool seq_scaling_matrix_present_flag;
	int32_t seq_scaling_list_present_flag[8];
	uint16_t scaling_list_4x4[6][16];
	uint16_t scaling_list_8x8[2][64];
	bool use_default_matrix4x4[6];
	bool use_default_matrix8x8[2];
	bool valid;

	h264_vui_t vui;
} h264_sps_t;

typedef struct
{
	uint32_t pps_pic_parameter_set_id;
	uint32_t pps_seq_parameter_set_id;
	bool valid;
	
	bool b_entropy_coding_mode_flag;
	bool b_pic_order_present_flag;
	uint32_t ui_num_slice_groups_minus1;
	uint32_t ui_slice_group_map_type;
	uint32_t ui_num_ref_idx_l0_active_minus1;
	uint32_t ui_num_ref_idx_l1_active_minus1;
	bool b_weighted_pred_flag;
	uint32_t ui_weighted_bipred_idc;
	bool i_pic_init_qp_minus26;
	bool i_pic_init_qs_minus26;
	int32_t rgi_chroma_qp_index_offset[1];
	bool b_deblocking_filter_control_present_flag;
	bool b_constrained_intra_pred_flag;
	bool b_redundant_pic_cnt_present_flag;

} h264_pps_t;


typedef enum
{
	lp_h264PicType_IDR,
	lp_h264PicType_nonIDR,
	lp_h264PicType_Unknown
	
} h264_picture_type_t;









typedef enum {
	lpBaseLayer, lpEnhLayer
} lp_auLayer_t;

typedef enum {
	lpCodec_AVC,
	lpCodec_HEVC
} lp_codec_type_e;

typedef struct {
	uint8_t *p_data_raw;   ///< with start code, header, in ebsp
	uint32_t len_raw;
	uint8_t *p_data_ebsp;  ///< with payload only, no start code/header, ebsp
	uint32_t len_ebsp;
	uint8_t *p_data_rbsp;  ///< with payload only, no start code/header, rbsp (ep bytes removed)
	uint32_t len_rbsp;
	bool newAU;
} lp_nal_data_t;

typedef struct {
	int32_t poc;
	uint32_t doc;

	int64_t pts, dts;
	lp_auLayer_t layer;

	bool concatenationFlag;
	uint64_t ui64_dpb_output_time; ///< DPB output time in units of vui_time_scale as derived from picture timing SEI message.
	                               ///< If no picture timing SEI is available for this picture, the value is ~0u.
	union {
		struct {
			h265_sps_t *sps_hevc;
			h265_pps_t *pps_hevc;
			h265_vps_t *vps_hevc;
			h265_picture_type_t pictureType_hevc;
		};
		struct {
			h264_sps_t *sps_avc;
			h264_pps_t *pps_avc;
			h264_picture_type_t pictureType_avc;
		};
	};

	uint8_t *sei;   ///< Pointer to sei. use callback to go to next sei data \sa enum_au_seis_t
	uint32_t sei_blob_len;
	uint8_t *rpu;   ///< Pointer to rpu.
	uint32_t rpu_blob_len;
	uint32_t rpu_blob_len_ebsp;
	uint32_t slice_types;  ///< slice type [bit 0,1,2] occurring in that AU
	
} lp_access_unit_t;




typedef void lean_parser_t;
typedef enum { lp_NeedMore, lp_Done, lp_Error } lean_parser_return_t;

///< Signature of callback function to enumerate SEI blobs attached to an AU
typedef bool(*enum_au_seis_t)(lean_parser_t *p, lp_access_unit_t *au);
///< Signature of callback function called when a frame is output in display order
typedef void(*on_bump_t)(lp_access_unit_t *, enum_au_seis_t fun, void *);
///< Signature of callback function called when an Access Unit is done parsing in decode order
typedef void(*on_access_unit_t)(lp_access_unit_t *, void *);
///< Signature of callback function called when a NAL unit is received in decode order
typedef void(*on_nal_unit_t)(lp_auLayer_t layer, int32_t nalu_type, lp_nal_data_t *ps_data, void *);


LEAN_PARSER_API lean_parser_t *lean_parser_create(lp_codec_type_e codec);
LEAN_PARSER_API bool lean_parser_destroy(lean_parser_t *p);
LEAN_PARSER_API bool lean_parser_version(int32_t *major, int32_t *minor, int32_t *build);
LEAN_PARSER_API const char *lean_parser_version_str();

LEAN_PARSER_API bool lean_parser_last_error(lean_parser_t *p, const char **buffer);
LEAN_PARSER_API void lean_parser_wait_for_rap(lean_parser_t *p, bool wait); // default true

LEAN_PARSER_API lean_parser_return_t
lean_parser_push_bytes(lean_parser_t *p, uint8_t *bytes, int32_t i_length, int64_t pts, int64_t dts,
	on_bump_t on_bump, on_access_unit_t on_access_unit, on_nal_unit_t on_nal_unit, void *user);

#define lpUnknownPTS ((int64_t)-0x8000000000000000ll)
#define lpUnknownDTS ((int64_t)-0x8000000000000000ll)

#ifdef __cplusplus
} // extern "C"
#endif
