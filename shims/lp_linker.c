
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include "linker.h"
#include "lean_parser.h"
#include "lp_linker.h"


typedef struct {
	lean_parser_t *(*create)(lp_codec_type_e codec);
	bool (*destroy)(lean_parser_t *p);
	bool (*version)(int32_t *, int32_t *, int32_t *);
	const char *(*version_str)();
	bool (*last_error)(lean_parser_t *p, const char **buffer);
	void (*wait_for_rap)(lean_parser_t *p, bool wait);
	lean_parser_return_t (*push_bytes)(lean_parser_t *, uint8_t *, int32_t, uint64_t, uint64_t, on_bump_t,
									   on_access_unit_t, on_nal_unit_t, void *);
} leanParser_symbols_t;

static leanParser_symbols_t symbols;

bool lp_try_link() {
	void *handle;
	if ((handle = lnk_open("leanParser"))) {
		symbols.create = lnk_symbol(handle, "lean_parser_create");
		symbols.destroy = lnk_symbol(handle, "lean_parser_destroy");
		symbols.version = lnk_symbol(handle, "lean_parser_version");
		symbols.version_str = lnk_symbol(handle, "lean_parser_version_str");
		symbols.last_error = lnk_symbol(handle, "lean_parser_last_error");
		symbols.wait_for_rap = lnk_symbol(handle, "lean_parser_wait_for_rap");
		symbols.push_bytes = lnk_symbol(handle, "lean_parser_push_bytes");
		return true;
	}
	return false;
}



LEAN_PARSER_API lean_parser_t *lean_parser_create(lp_codec_type_e codec){
	return symbols.create(codec);
}
LEAN_PARSER_API bool lean_parser_destroy(lean_parser_t *p){
	return symbols.destroy(p);
}
LEAN_PARSER_API bool lean_parser_version(int32_t *major, int32_t *minor, int32_t *build){
	return symbols.version(major, minor, build);
}
LEAN_PARSER_API const char *lean_parser_version_str(){
	return symbols.version_str();
}
LEAN_PARSER_API bool lean_parser_last_error(lean_parser_t *p, const char **buffer){
	return symbols.last_error(p, buffer);
}
LEAN_PARSER_API void lean_parser_wait_for_rap(lean_parser_t *p, bool wait) {
	symbols.wait_for_rap(p, wait);
}
LEAN_PARSER_API lean_parser_return_t
lean_parser_push_bytes(lean_parser_t *p, uint8_t *bytes, int32_t i_length, uint64_t pts, uint64_t dts,
					   on_bump_t on_bump, on_access_unit_t on_access_unit, on_nal_unit_t on_nal_unit, void *user) {
	return symbols.push_bytes(p, bytes, i_length, pts, dts, on_bump, on_access_unit, on_nal_unit, user);
}


