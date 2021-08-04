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

#include "linker.h"
#include "slbc_mmb.h"
#include "slbc_mmb_linker.h"


typedef struct {
	const char *(*get_api_ver)();
	const char *(*get_build_info)();
	slbc_t *(*create)(slbc_config_t *);
	void (*destroy)(slbc_t *);
	bool (*process)(slbc_t *, uint16_t *, uint8_t *, size_t);
	bool (*flush)(slbc_t *);
	const char *(*last_error)(slbc_t *);
} slbc_mmb_symbols_t;

static slbc_mmb_symbols_t slbc_symbols;

bool slbc_mmb_try_link() {
	void *handle;
	if ((handle = lnk_open("slbc_mmb"))) {
		slbc_symbols.create = lnk_symbol(handle, "slbc_create");
		slbc_symbols.get_api_ver = lnk_symbol(handle, "slbc_get_api_ver");
		slbc_symbols.get_build_info = lnk_symbol(handle, "slbc_get_build_info");
		slbc_symbols.destroy = lnk_symbol(handle, "slbc_destroy");
		slbc_symbols.flush = lnk_symbol(handle, "slbc_flush");
		slbc_symbols.last_error = lnk_symbol(handle, "slbc_last_error");
		slbc_symbols.process = lnk_symbol(handle, "slbc_process");
		return true;
	}
	return false;
}

const char *slbc_get_api_ver(void) {
	return slbc_symbols.get_api_ver();
}
const char *slbc_get_build_info(void) {
	return slbc_symbols.get_build_info();
}
slbc_t *slbc_create(slbc_config_t *ps_slbc_config) {
	return slbc_symbols.create(ps_slbc_config);
}
void slbc_destroy(slbc_t *slbc_ptr) {
	slbc_symbols.destroy(slbc_ptr);
}
bool slbc_process(slbc_t *slbc_ptr, uint16_t *p_img, uint8_t *rpu_ptr, size_t rpu_length) {
	return slbc_symbols.process(slbc_ptr, p_img, rpu_ptr, rpu_length);
}
bool slbc_flush(slbc_t *slbc_ptr) {
	return slbc_symbols.flush(slbc_ptr);
}
const char *slbc_last_error(slbc_t *slbc_ptr) {
	return slbc_symbols.last_error(slbc_ptr);
}
