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

/*! @defgroup ves_muxer_buf ves_muxer buffer based library documentation
 *
 * @{
 */

/**
*  @page page_ves_muxer_buf Dolby Vision Buffer Based VES Muxer Library
*
*  The VES Muxer combines the base-layer(BL) VES, enhancement-layer(EL) VES and
*  metadata(RPU) substreams into the Dolby Vision COMBO VES for the single VES
*  implementation or the enhancement-layer VES and metadata substreams
*  into the Dolby Vision enhancement-layer VES for the dual VES implementation.
*
*  1. Normal calling sequence of the buffer based API
*         dv_ves_mux_create()
*         dv_ves_mux_init()
*         dv_ves_mux_bl_process()
*         dv_ves_mux_el_process()
*         dv_ves_mux_rpu_process()
*         dv_ves_mux_flush（）
*         dv_ves_mux_destroy()
*     Refer to vdr_muxer_test.c as an API usage example.
*
*  2. Two ways of deriving PTS for muxing
*     - Picture Timing SEI
*     - POC from slice header
*
*     VES muxer derives PTS of incoming BL EL and RPU and interlace them into the output
*     elementary stream accordingly. VES muxer can either derive BL/EL PTS from Picture
*     timing SEI message(if present in the elementary stream) or POC from slice header(always
*     present in an elementary stream)
*
*  3. Input handling
*     - VES muxer does not allocate any buffer internally. The API user shall push complete
*       NALU buffers into VES muxer by calling dv_ves_mux_bl_process(), dv_ves_mux_el_process()
*       or dv_ves_mux_rpu_process().
*     - As long as VES muxer finished processing the input NALU buffer, VES muxer will invoke
*       the callback function dv_ves_mux_inbuf_release_cb_func_t() to return the buffer to API user.
*       The API user shall impelement the callback function to handle the release input buffer.
*
*  4. Output handling
*     - As long as VES muxer is ready to output muxed stream, it invokes the user defined callback
*       function dv_ves_mux_mux_outdata_cb_func_t(). The API user shall implement the callback
*       function to handle the output bit stream buffer.
*     - VES muxer may call the output callback function multiple times to output a complete NALU, as
*       VES muxer may just insert 0x7E01 or 0x7C01 to muxed EL or RPU in the callback.
*
*  5. Additional features supported by VES muxer
*     - VES muxer can insert AUD if incoming bit stream doesn't have AUD.
*     - VES muxer can fix incorrectly inserted AUD.
*     - VES muxer can reorder non-VCL NALUs before first VCL so that the order of these NALUs conforms to
*       AVC/HEVC specifications.
*
*  6. Demuxing is not supported in buffer based API, use ves_splitter library instead.
*
*  @file  dv_ves_mux.h
*  @brief Dolby Vision VES muxer buffer based API header file
*
* $Id$
*/

#ifndef __DV_VES_MUX_H__
#define __DV_VES_MUX_H__

#include "dv_cdefs.h"

/*! @brief Macro to handle DLL import/export */
#ifdef LIBVES_MUX_EXPORTS
/* export symbols */
#ifdef WIN32
#define DV_VES_MUX_API __declspec(dllexport)
#else
#define DV_VES_MUX_API __attribute__((visibility("default")))
#endif
#else
/* import symbols */
#ifdef WIN32
#ifdef DV_VES_MUX_DLL
#define DV_VES_MUX_API __declspec(dllimport)
#else
#define DV_VES_MUX_API
#endif
#else
#define DV_VES_MUX_API
#endif
#endif /* DV_VES_MUX_EXPORTS */


#ifdef __cplusplus
extern "C" {
#endif

    /*! @defgroup ves_muxer_buf_data_types Enumerations and data structures
     *
     * @{
     */


    /*! @brief Input buffer release callback function prototype
        @param[in] *p_ctx     pointer to context
        @param[in]  es_id     identify the Dolby Vision ES type
        @param[in] *p_es_buf  pointer to the ES buffer. It is one of the
                              input arguments to the dv_ves_mux_process() call
    */
    typedef void(*dv_ves_mux_inbuf_release_cb_func_t)
    (
        void    *p_ctx,
        dv_es_t  es_id,
        uint8_t *p_es_buf
    );


    /*! @brief Mux output data callback function prototype
        @param[in] *p_ctx           pointer to context
        @param[in] *p_dv_ves_buf    pointer to the mux output data
        @param[in]  dv_ves_buf_len  size of the mux output data in bytes
    */
    typedef void(*dv_ves_mux_mux_outdata_cb_func_t)
    (
        void     *p_ctx,
        uint8_t  *p_dv_ves_buf,
        uint32_t  dv_ves_buf_len
    );

    /*! @brief RPU reordering setting */
    typedef enum rpu_reorder_type_e {
        RPU_REORDER_TYPE_NONE  = 0, /**< @brief disable RPU reordering */
        RPU_REORDER_TYPE_BL    = 1, /**< @brief reorder RPU based on Base Layer (primary_vid) decoding order */
        RPU_REORDER_TYPE_EL    = 2  /**< @brief reorder RPU based on Enhancement Layer (seconary_vid) decoding order */
    } rpu_reorder_type_t;

    /*! @brief bit field to control VES muxer output */
    typedef enum mpeg_muxer_cfg_bit_s
    {
        MPEG_MUXER_CFG_BIT_DEFAULT                           = 0,        /**< @brief default cfg. insert aud and discard aud in wrong position */
        MPEG_MUXER_CFG_BIT_BL_OUT_REORDER                    = (1 << 0), /**< @brief reorder nal before 1st vcl of au in bl, used for fixing streams with non-conforming non-VCL NALU order */
        MPEG_MUXER_CFG_BIT_EL_OUT_REORDER                    = (1 << 1), /**< @brief reorder nal before 1st vcl of au in el, used for fixing streams with non-conforming non-VCL NALU order */
        MPEG_MUXER_CFG_BIT_NO_BL_AUD_INSERTION               = (1 << 2), /**< @brief [Obsolete, takes no effect, kept only for backward compatibility] */
        MPEG_MUXER_CFG_BIT_NO_EL_AUD_INSERTION               = (1 << 3), /**< @brief [Obsolete, takes no effect, kept only for backward compatibility] */
    } mpeg_muxer_cfg_bit_t;

    /*! @brief Dolby Vision VES muxer configuration structure
    */
    typedef struct _dv_ves_mux_conf_s_
    {
        uint32_t                            mpeg_muxer_config_bits;
        uint8_t                             f_secondary_vid_present;
        /**<@brief indicate if there is a secondary VES present. Here is how
                   this flag should be set.
                   1. DV COMBO VES (BL + EL + metadata):        1
                   2. DV COMBO VES w/o EL (BL + metadata):      0
                   3. DV enhancement-layer VES (EL + metadata): 0

                   Note: For use case (3), EL will be treated as BL, dv_ves_mux_bl_process() should be called
         */
        dv_es_codec_t                       primary_vid_codec;
                    /**<@brief primary (usually BL) video codec type (AVC | HEVC) */
        dv_es_codec_t                       secondary_vid_codec;
                  /**<@brief secondary (usually EL) video codec type (AVC | HEVC) */
        rpu_reorder_type_t                  rpu_reorder_type;
                                             /**<@brief reorder RPU by NONE/BL/EL */
        msg_log_func_t                      pf_msg_log;
        dv_ves_mux_inbuf_release_cb_func_t  pf_inbuf_release_cb;
                                /**<@brief input buffer release callback function */
        void                               *p_inbuf_release_cb_ctx;
                        /**<@brief input buffer release callback function context */
        dv_ves_mux_mux_outdata_cb_func_t    pf_mux_outdata_cb;
                                     /**<@brief mux output data callback function */
        void                               *p_mux_outdata_cb_ctx;
                             /**<@brief mux output data callback function context */
    } dv_ves_mux_conf_t;


    /*! @brief Dolby Vision VES muxer return code definition
    */
    typedef enum _dv_ves_mux_rc_e_
    {
        DV_VES_MUX_OK = 0,
        DV_VES_MUX_UNINITIALIZE,                 /**< @brief module is not initialized yet */
        DV_VES_MUX_INVALID_CONTEXT_HANDLE,       /**< @brief invalid context handle */
        DV_VES_MUX_INVALID_ARG,                  /**< @brief invalid argument */
        DV_VES_MUX_INVALID_VES_TYPE,             /**< @brief invalid VES type */
        DV_VES_MUX_OUT_OF_MEM,                   /**< @brief out of memory */
        DV_VES_MUX_ES_INPUT_QUEUE_FULL,          /**< @brief ES input queue is full */
        DV_VES_MUX_MPEG_PARSER_ERR,
    #ifdef ENABLE_VP9
        DV_VES_MUX_VP9_PARSER_ERR,
        DV_VES_MUX_VP9_WRITER_ERR,
    #endif
        DV_VES_MUX_UNKNOWN_ERR
    } dv_ves_mux_rc_t;


    /*! @brief Dolby Vision VES muxer handle
    */
    typedef void  *dv_ves_mux_handle_t;

    /*!
    * @}
    */

    /*! @defgroup ves_muxer_buf_api API functions
     *
     * @{
     */

    /*! @brief Get the Dolby Vision VES muxer module
               API version string.

        @return
            A pointer to the API version string.
    */
    DV_VES_MUX_API const char *dv_ves_mux_get_api_ver
        (
        void
        );


    /*! @brief Get the Dolby Vision VES muxer module
               alogrithm version string.

        @return
            A pointer to the algorithm version string.
    */
    DV_VES_MUX_API const char *dv_ves_mux_get_algo_ver
        (
        void
        );

    /*! @brief Get the Dolby Vision VES muxer build info string.

        @return
            A pointer to the build info string.
    */
    DV_VES_MUX_API const char *dv_ves_mux_get_build_info
        (
        void
        );


    /*! @brief Create the Dolby Vision VES muxer.

        @return
            @li non-zero if successful, a handle for the Dolby Vision VES muxer.
            @li NULL     if there was an error.
    */
    DV_VES_MUX_API dv_ves_mux_handle_t dv_ves_mux_create
        (
        void
        );


    /*! @brief Destroy the Dolby Vision VES muxer.

        @param[in] h_ves_mux  handle for the Dolby Vision VES muxer

        @return
            @li DV_VES_MUX_OK if successful.
            @li non-zero      if there was an error.
    */
    DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_destroy
        (
        dv_ves_mux_handle_t  h_ves_mux
        );


    /*! @brief Initialize and configure the Dolby Vision VES muxer.

        @param[in] h_ves_mux  handle for the Dolby Vision VES muxer
        @param[in] p_conf     pointer to the configuration block

        @return
            @li DV_VES_MUX_OK if successful.
            @li non-zero      if there was an error.
    */
    DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_init
        (
        dv_ves_mux_handle_t   h_ves_mux,
        dv_ves_mux_conf_t    *p_conf
        );


    /*! @brief Reset the Dolby Vision VES muxer to the state after calling dv_ves_mux_init().

        @param[in] h_ves_mux  handle for the Dolby Vision VES muxer

        @return
            @li DV_VES_MUX_OK if successful.
            @li non-zero      if there was an error.
    */
    DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_reset
        (
        dv_ves_mux_handle_t  h_ves_mux
        );


    /*! @brief Push the BL VES data into the Dolby Vision VES muxer.

               This API expects a complete BL NAL unit as input.

        @param[in] h_ves_mux    handle for the Dolby Vision VES muxer
        @param[in] p_ves_data   pointer to a complete BL NAL unit (AVC/HEVC Annex B),
                                this buffer will be released by dv_ves_mux_inbuf_release_cb_func_t callback function
        @param[in] ves_size     BL NAL unit size in bytes

        @return
            @li DV_VES_MUX_OK if successful.
            @li non-zero      if there was an error.
    */
    DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_bl_process
        (
        dv_ves_mux_handle_t   h_ves_mux,
        uint8_t              *p_ves_data,
        uint32_t              ves_size
        );

    /*! @brief Push the EL VES data into the Dolby Vision VES muxer.

               This API expects a complete EL NAL unit as input.

        @param[in] h_ves_mux    handle for the Dolby Vision VES muxer
        @param[in] p_ves_data   pointer to a complete EL NAL unit (AVC/HEVC Annex B),
                                this buffer will be released by dv_ves_mux_inbuf_release_cb_func_t callback function
        @param[in] ves_size     EL NAL unit data size in bytes

        @return
            @li DV_VES_MUX_OK if successful.
            @li non-zero      if there was an error.
    */
    DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_el_process
        (
        dv_ves_mux_handle_t   h_ves_mux,
        uint8_t              *p_ves_data,
        uint32_t              ves_size
        );

    /*! @brief Push the RPU metadata NAL unit into the Dolby Vision VES muxer.

               This API expects a complete RPU metadata NAL unit as input.

        @param[in] h_ves_mux     handle for the Dolby Vision VES muxer
        @param[in] p_rpu_data     pointer to a complete RPU metadata NAL unit
                                 this buffer will be released by dv_ves_mux_inbuf_release_cb_func_t callback function
        @param[in] rpu_data_size  RPU metadata size

        @return
            @li DV_VES_MUX_OK if successful.
            @li non-zero      if there was an error.
    */
    DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_rpu_process
        (
        dv_ves_mux_handle_t   h_ves_mux,
        uint8_t              *p_rpu_data,
        uint32_t              rpu_data_size
        );

    /*! @brief Flush the input to the output

        @param[in] h_ves_mux     handle for the Dolby Vision VES muxer

        @return
            @li DV_VES_MUX_OK if successful.
            @li non-zero      if there was an error.
    */
    DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_flush
        (
        dv_ves_mux_handle_t   h_ves_mux
        );

    /*! @brief Get the error string for the given Dolby Vision VES muxer
               error code.

        @param[in]  err_code the error code

        @return
            @li NULL     if the error code is not recognized.
            @li non-zero if the error code is valid, a pointer to error string.
    */
    DV_VES_MUX_API const char *dv_ves_mux_get_errstr
        (
        uint32_t  err_code
        );

    /*!
     * @}
     */


#ifdef __cplusplus
}
#endif

/*!
 * @}
 */

#endif /* __DV_VES_MUX_H__ */

