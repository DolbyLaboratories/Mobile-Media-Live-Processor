/**
 * Dolby Vision Mobile Media Live Processing
 * Copyright (C) 2020-2021, Dolby Laboratories
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "linker.h"
#include "dv_ves_mux.h"
#include "vesmux_linker.h"


typedef struct {
	const char *(*get_api_ver)(void);
	const char *(*get_algo_ver)(void);
	const char *(*get_build_info)(void);
	dv_ves_mux_handle_t *(*create)(void);
	dv_ves_mux_rc_t (*destroy)(dv_ves_mux_handle_t  h_ves_mux);
	dv_ves_mux_rc_t (*init)(dv_ves_mux_handle_t   h_ves_mux, dv_ves_mux_conf_t *p_conf);
	dv_ves_mux_rc_t (*reset)(dv_ves_mux_handle_t h_ves_mux);
	dv_ves_mux_rc_t (*bl_process)(dv_ves_mux_handle_t h_ves_mux, uint8_t *p_ves_data, uint32_t ves_size);
	dv_ves_mux_rc_t (*el_process)(dv_ves_mux_handle_t h_ves_mux, uint8_t *p_ves_data, uint32_t ves_size);
	dv_ves_mux_rc_t (*rpu_process)(dv_ves_mux_handle_t h_ves_mux, uint8_t *p_rpu_data, uint32_t rpu_data_size);
	dv_ves_mux_rc_t (*flush)(dv_ves_mux_handle_t h_ves_mux);
	const char *(*get_errstr)(uint32_t err_code);
} vesMuxer_symbols_t;

static vesMuxer_symbols_t vm_symbols;

bool ves_muxer_try_link() {
	void *handle;
	if ((handle = lnk_open("vesmux"))) {
		vm_symbols.get_api_ver = lnk_symbol(handle, "dv_ves_mux_get_api_ver");
		vm_symbols.get_algo_ver = lnk_symbol(handle, "dv_ves_mux_get_algo_ver");
		vm_symbols.get_build_info = lnk_symbol(handle, "dv_ves_mux_get_build_info");
		vm_symbols.create = lnk_symbol(handle, "dv_ves_mux_create");
		vm_symbols.destroy = lnk_symbol(handle, "dv_ves_mux_destroy");
		vm_symbols.init = lnk_symbol(handle, "dv_ves_mux_init");
		vm_symbols.reset = lnk_symbol(handle, "dv_ves_mux_reset");
		vm_symbols.bl_process = lnk_symbol(handle, "dv_ves_mux_bl_process");
		vm_symbols.el_process = lnk_symbol(handle, "dv_ves_mux_el_process");
		vm_symbols.rpu_process = lnk_symbol(handle, "dv_ves_mux_rpu_process");
		vm_symbols.flush = lnk_symbol(handle, "dv_ves_mux_flush");
		vm_symbols.get_errstr = lnk_symbol(handle, "dv_ves_mux_get_errstr");
		return true;
	}
	return false;
}





DV_VES_MUX_API const char *dv_ves_mux_get_api_ver(void) {
	return vm_symbols.get_api_ver();
}

DV_VES_MUX_API const char *dv_ves_mux_get_algo_ver(void) {
	return vm_symbols.get_algo_ver();
}

DV_VES_MUX_API const char *dv_ves_mux_get_build_info(void) {
	return vm_symbols.get_build_info();
}

DV_VES_MUX_API dv_ves_mux_handle_t dv_ves_mux_create(void) {
	return vm_symbols.create();
}
DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_destroy(dv_ves_mux_handle_t h_ves_mux) {
	return vm_symbols.destroy(h_ves_mux);
}
DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_init(dv_ves_mux_handle_t h_ves_mux, dv_ves_mux_conf_t *p_conf) {
	return vm_symbols.init(h_ves_mux, p_conf);
}
DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_reset(dv_ves_mux_handle_t h_ves_mux) {
	return vm_symbols.reset(h_ves_mux);
}
DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_bl_process(dv_ves_mux_handle_t h_ves_mux, uint8_t *p_ves_data, uint32_t ves_size) {
	return vm_symbols.bl_process(h_ves_mux, p_ves_data, ves_size);
}
DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_el_process(dv_ves_mux_handle_t h_ves_mux, uint8_t *p_ves_data, uint32_t ves_size) {
	return vm_symbols.el_process(h_ves_mux, p_ves_data, ves_size);
}
DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_rpu_process(dv_ves_mux_handle_t h_ves_mux, uint8_t *p_rpu_data, uint32_t rpu_data_size) {
	return vm_symbols.rpu_process(h_ves_mux, p_rpu_data, rpu_data_size);
}
DV_VES_MUX_API dv_ves_mux_rc_t dv_ves_mux_flush(dv_ves_mux_handle_t h_ves_mux) {
	return vm_symbols.flush(h_ves_mux);
}
DV_VES_MUX_API const char *dv_ves_mux_get_errstr(uint32_t err_code) {
	return vm_symbols.get_errstr(err_code);
}

