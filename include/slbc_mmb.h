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

#ifndef slbc_mmb_h
#define slbc_mmb_h

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef SHARED_SLBCMMB_LIB
	#ifndef _WIN32
		#define SLBC_API  __attribute__ ((visibility ("default")))
	#else
		#define SLBC_API __declspec(dllexport)
	#endif
#else
	#ifdef _WIN32
		#define SLBC_API __declspec(dllimport)
	#else
		#define SLBC_API
	#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t *p_y, *p_cb, *p_cr;
	uint32_t rpu_data_length;
	uint8_t *rpu_data;
} slbc_result_t;

typedef void (*slbc_cb_func_t)(slbc_result_t *p_output, void *p_data_callback_handle);
typedef enum { SlbcMmbOp_OutputSDR, SlbcMmbOp_Output81 } slbc_mmb_operation_t;

/*! @brief this structure is passed to initialization function as SLBC library configuration parameters */
typedef struct
{
	void *p_data_callback_handle;     /**< @brief the handle which is to be passed to the callback function as the first argument */
	slbc_cb_func_t pf_data_callback;  /**< @brief pointer to the callback function, must not be NULL */

	uint32_t width;                   /**< @brief width in pixels of the input signal */
	uint32_t height;                  /**< @brief height in pixels of the input signal */
	float fps;                        /**< @brief frame rate of the input signal */
	
	slbc_mmb_operation_t operation;   /**< @brief specify whether to convert to SDR, to 8.4 (passthru w/ optional metadata creation) or to 8.1 */
	
} slbc_config_t;

typedef void slbc_t;

/*! @brief Get the Dolby Vision libslbc version string.
 *  @return A pointer to the version string.
*/
SLBC_API const char *slbc_get_api_ver(void);

/*! @brief Get the Dolby Vision libslbc build info string.
 *  @return A pointer to the string containing the build info.
*/
SLBC_API const char *slbc_get_build_info(void);

/*! @brief Create a SLBC context
 *  @param ps_slbc_config Pointer to configuration structure
 *  @return NULL in case of error \sa slbc_last_error
*/
SLBC_API slbc_t *slbc_create(slbc_config_t *ps_slbc_config);

/*! @brief Destroy an SLBC context
 *  @param slbc_ptr Handle to context to destroy
*/
SLBC_API void slbc_destroy(slbc_t *slbc_ptr);

/*! @brief Process one  frame
 *  @param slbc_ptr Handle to context
 *  @param p_img Planar 4:2:0 image data (Dolby Vision Profile 8.4, YCbCr 10bit, length shall be sizeof(uint16_t)*width*height*3/2)
 *  @param rpu_ptr RPU blob (Dolby Vision Metadata)
 *  @param rpu_length Length in bytes of the RPU blob
 *  @return true when successful or false \sa slbc_last_error
*/
SLBC_API bool slbc_process(slbc_t *slbc_ptr, uint16_t *p_img, uint8_t *rpu_ptr, size_t rpu_length);

/*! @brief Flush the library
 *  @param slbc_ptr Handle to context
 *  @return true when successful or false \sa slbc_last_error
*/
SLBC_API bool slbc_flush(slbc_t *slbc_ptr);

/*! @brief Get last error
 *  @param slbc_ptr Handle to current context
 *  @return Textual representation of the error
*/
SLBC_API const char *slbc_last_error(slbc_t *slbc_ptr);


#ifdef __cplusplus
} // extern "C"
#endif
#endif
