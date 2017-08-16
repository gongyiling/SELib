#ifndef __SELIB_CONFIG_H
#define __SELIB_CONFIG_H

// General config

// Enable debug prints (uses printf)
//#define SELIB_DEBUG_PRINT

// Define custom printf function
//#define SELIB_PRINTF_FUNC printf


// SEAnim config

// Don't use high precision (eg. if using SEAnims on low-memory devices)
//#define DONT_USE_HIGH_PRECISION


// Define attributes of all API symbols declarations, e.g. for DLL under Windows.
//#define SELIB_API __declspec( dllexport )
//#define SELIB_API __declspec( dllimport )

// Define assertion handler.
//#define SELIB_ASSERT(_EXPR) assert(_EXPR)

// Define custom malloc
//#define SELIB_MALLOC malloc

// Define custom calloc
//#define SELIB_CALLOC calloc

// Define custom realloc
//#define SELIB_REALLOC realloc

// Define custom free
//#define SELIB_FREE free

///////////////////////////////
// Custom filesystem defines //
///////////////////////////////

// The handle used by the file system
//#define SELIB_FS_HANDLE FILE*

// The fwrite function used by the file system
//#define SELIB_FWRITE(ptr, elementSize, elementCount, fileHandle) fwrite(ptr, elementSize, elementCount, fileHandle)

// The fread function used by the file system
//#define SELIB_FREAD(ptr, elementSize, elementCount, fileHandle) fread(ptr, elementSize, elementCount, fileHandle)

#endif