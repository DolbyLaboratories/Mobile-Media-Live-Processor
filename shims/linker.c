
#ifdef _WIN32
	#include <Windows.h>
#else
	#include <dlfcn.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "linker.h"

void *lnk_symbol(void *lib, const char *name) {
	#ifdef _WIN32
	return GetProcAddress((HMODULE)lib, name);
	#else
	return dlsym(lib, name);
	#endif
}

void *lnk_open(const char *name) {
	char fullname[128];
	#ifdef _WIN32
	sprintf_s(fullname, 128, "%s.dll", name);
	return LoadLibrary(fullname);
	#else
	#if __APPLE__
	sprintf(fullname, "lib%s.dylib", name);
	#else
	sprintf(fullname, "lib%s.so", name);
	#endif
	void *ptr;
	if ((ptr = dlopen(fullname, RTLD_NOW|RTLD_LOCAL))) {
		return ptr;
	}
	printf("dlopen %s failed: %s\n", fullname, dlerror());
	return NULL;
	#endif
}
