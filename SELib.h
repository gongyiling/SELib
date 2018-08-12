#ifndef __SELIB_H
#define __SELIB_H

#include "SELib_config.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Define attributes of all API symbols declarations, e.g. for DLL under Windows.
#ifndef SELIB_API
#define SELIB_API
#endif

// Define assertion handler.
#ifndef SELIB_ASSERT
#include <assert.h>
#define SELIB_ASSERT(_EXPR) assert(_EXPR)
#endif

// Define malloc
#ifndef SELIB_MALLOC
#define SELIB_MALLOC malloc
#endif

// Define calloc
#ifndef SELIB_CALLOC
#define SELIB_CALLOC calloc
#endif

// Define realloc
#ifndef SELIB_REALLOC
#define SELIB_REALLOC realloc
#endif

// Define free
#ifndef SELIB_FREE
#define SELIB_FREE free
#endif

////////////////////////
// Filesystem defines //
////////////////////////
#ifndef SELIB_FS_HANDLE 
#define SELIB_FS_HANDLE FILE*
#endif

#ifndef SELIB_FWRITE
#define SELIB_FWRITE(ptr, elementSize, elementCount, fileHandle) fwrite(ptr, elementSize, elementCount, fileHandle)
#endif

#ifndef SELIB_FREAD
#define SELIB_FREAD(ptr, elementSize, elementCount, fileHandle) fread(ptr, elementSize, elementCount, fileHandle)
#endif

#define SELIB_LIB_VERSION		"2.0"

#ifndef DONT_USE_HIGH_PRECISION
	typedef double vec2_t[2];
	typedef double vec3_t[3];
	typedef double quat_t[4]; // X Y Z W (Normalized)
#else
	typedef float vec2_t[2];
	typedef float vec3_t[3];
	typedef float quat_t[4]; // X Y Z W (Normalized)
#endif

//#ifndef DONT_USE_HIGH_PRECISION
	typedef double vec2d_t[2];
	typedef double vec3d_t[3];
	typedef double quatd_t[4]; // X Y Z W (Normalized)
//#else
	typedef float vec2f_t[2];
	typedef float vec3f_t[3];
	typedef float quatf_t[4]; // X Y Z W (Normalized)
//#endif

	typedef uint8_t vec2b_t[2];
	typedef uint8_t vec3b_t[3];
	typedef uint8_t quatb_t[4];

#include "SELib_internal.h"

#include "SEAnim.h"
#include "SEModel.h"

#ifdef __cplusplus
}
#endif

#endif