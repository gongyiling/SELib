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
#define SELIB_FS_HANDLE FILE*

#ifndef SELIB_FWRITE
#define SELIB_FWRITE(ptr, elementSize, elementCount, fileHandle) fwrite(ptr, elementSize, elementCount, fileHandle)
#endif

#ifndef SELIB_FREAD
#define SELIB_FREAD(ptr, elementSize, elementCount, fileHandle) fread(ptr, elementSize, elementCount, fileHandle)
#endif

#define SELIB_LIB_VERSION		"1.0"

#include "SEAnim.h"

#ifdef __cplusplus
}
#endif

#endif