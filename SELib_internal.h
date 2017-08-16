#ifndef __SELIB_INTERNAL_H
#define __SELIB_INTERNAL_H

#include "SELib.h"

#ifndef SELIB_PRINTF_FUNC
#define SELIB_PRINTF_FUNC printf
#endif

#ifdef SELIB_DEBUG_PRINT
#define SELib_Printf SELIB_PRINTF_FUNC
#else
#define SELib_Printf
#endif

// Read a number from file and cast it to UInt32
extern inline void __SELib_internal_ReadVariableLengthNumber(int numberLength, uint32_t *destination, SELIB_FS_HANDLE handle);

// Read a null-terminated string from file of any length
extern inline void __SELib_internal_ReadNullTerminatedString(uint8_t **dest, SELIB_FS_HANDLE handle);

// Read a 3-dimensional vector from file (will cast to either float or double, depending on data flag and the setting in config)
extern inline void __SELib_internal_ReadVector3FromFile(uint8_t animationFlags, vec3_t *destination, SELIB_FS_HANDLE handle);

// Read a quaternion from file 
extern inline void __SELib_internal_ReadQuatFromFile(uint8_t animationFlags, quat_t *destination, SELIB_FS_HANDLE handle);

#endif