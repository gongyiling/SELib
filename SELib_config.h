#ifndef __SELIB_CONFIG_H
#define __SELIB_CONFIG_H

// General config

// Enable debug prints (uses printf)
//#define SELIB_DEBUG_PRINT

// Define custom printf function
//#define SELIB_PRINTF_FUNC printf


// SEAnim config

// Don't use high precision (eg. if using SEAnims on low-memory devices)
#define DONT_USE_HIGH_PRECISION


// Define attributes of all API symbols declarations, e.g. for DLL under Windows.
//#define SEANIM_API __declspec( dllexport )
//#define SEANIM_API __declspec( dllimport )

// Define assertion handler.
//#define SEANIM_ASSERT(_EXPR) assert(_EXPR)

// Define custom malloc
//#define SEANIM_MALLOC malloc

// Define custom calloc
//#define SEANIM_CALLOC calloc

// Define custom realloc
//#define SEANIM_REALLOC realloc

// Define custom free
//#define SEANIM_FREE free

///////////////////////////////
// Custom filesystem defines //
///////////////////////////////

// The handle used by the file system
//#define SEANIM_FS_HANDLE FILE*

// The fwrite function used by the file system
//#define SEANIM_FWRITE(ptr, elementSize, elementCount, fileHandle) fwrite(ptr, elementSize, elementCount, fileHandle)

// The fread function used by the file system
//#define SEANIM_FREAD(ptr, elementSize, elementCount, fileHandle) fread(ptr, elementSize, elementCount, fileHandle)

#endif