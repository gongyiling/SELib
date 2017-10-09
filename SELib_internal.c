#include "SELib.h"
#include "SELib_internal.h"

inline void __SELib_internal_ReadVariableLengthNumber(int length, uint32_t *destination, SELIB_FS_HANDLE handle)
{
	static uint8_t v8 = 0;
	static uint16_t v16 = 0;
	switch (length)
	{
	case 1:
		SELIB_FREAD(&v8, 1, 1, handle);
		*destination = (uint32_t)v8;
		break;
	case 2:
		SELIB_FREAD(&v16, 2, 1, handle);
		*destination = (uint32_t)v16;
		break;
	case 4:
		SELIB_FREAD(destination, 4, 1, handle);
		break;
	default:
		SELIB_ASSERT(0); // Wrong number size, it's important to fix it if you encounter that
		break;
	}
}

inline void __SELib_internal_ReadNullTerminatedString(uint8_t **dest, SELIB_FS_HANDLE handle)
{
	(*dest) = (uint8_t*)SELIB_CALLOC(32, sizeof(uint8_t));  // alloc a 32-byte chunk
	(*dest)[0] = 0xcc; // this is so we dont get a null-terminator before the for loop below begins

	for(int i = 0; ; i++) // read them chars until we encounter a null terminator
	{
		if(i % 32 == 0 && i != 0) (*dest) = SELIB_REALLOC((*dest), i + 32); // reallocate the char array and add an another 32-byte chunk
		SELIB_FREAD(&((*dest)[i]), 1, 1, handle); // read a char from file
		if ((*dest)[i] == 0) break;
	}

	(*dest) = SELIB_REALLOC((*dest), strlen(*dest)+1);
}

inline void __SELib_internal_ReadVector3FromFile(uint8_t animationFlags, vec3_t * destination, SELIB_FS_HANDLE handle)
{
#ifndef DONT_USE_HIGH_PRECISION
	if (animationFlags & SEANIM_PRECISION_HIGH)
	{
		SELIB_FREAD(destination, sizeof(vec3_t), 1, handle);
	}
	else {
		float in[3];
		vec3_t res;
		SELIB_FREAD(in, sizeof(float), 3, handle);
		for (int pp = 0; pp < 3; pp++)
		{
			res[pp] = (double)in[pp];
		}
		memcpy(destination, res, sizeof(vec3_t));
	}
#else
	if (animationFlags & SEANIM_PRECISION_HIGH)
	{
		double in[3];
		vec3_t res;
		SELIB_FREAD(in, sizeof(double), 3, handle);
		for (int pp = 0; pp < 3; pp++)
		{
			res[pp] = (float)in[pp];
		}
		memcpy(destination, res, sizeof(vec3_t));
	}
	else {
		SELIB_FREAD(destination, sizeof(vec3_t), 1, handle);
	}
#endif
}

inline void __SELib_internal_ReadQuatFromFile(uint8_t animationFlags, quat_t * destination, SELIB_FS_HANDLE handle)
{
#ifndef DONT_USE_HIGH_PRECISION
	if (animationFlags & SEANIM_PRECISION_HIGH)
	{
		SELIB_FREAD(destination, sizeof(quat_t), 1, handle);
	}
	else {
		float in[4];
		quat_t res;
		SELIB_FREAD(in, sizeof(float), 4, handle);
		for (int i = 0; i < 4; i++)
		{
			res[i] = (double)in[i];
		}
		memcpy(destination, res, 4 * sizeof(double));
	}
#else
	if (animationFlags & SEANIM_PRECISION_HIGH)
	{
		double in[4];
		quat_t res;
		SELIB_FREAD(in, sizeof(double), 4, handle);
		for (int i = 0; i < 4; i++)
		{
			res[i] = (float)in[i];
		}
		memcpy(destination, res, 4 * sizeof(quat_t));
	}
	else {
		SELIB_FREAD(destination, sizeof(quat_t), 1, handle);
	}
#endif
}

inline void __SELib_internal_WriteVariableLengthNumber(int numberLength, uint32_t * source, SELIB_FS_HANDLE handle)
{
	static uint8_t v8 = 0;
	static uint16_t v16 = 0;
	switch (numberLength)
	{
	case 1:
		v8 = *source;
		SELIB_FWRITE(&v8, 1, 1, handle);
		break;
	case 2:
		v16 = *source;
		SELIB_FWRITE(&v16, 2, 1, handle);
		break;
	case 4:
		SELIB_FWRITE(source, 4, 1, handle);
		break;
	default:
		SELIB_ASSERT(0); // Wrong number size, it's important to fix it if you encounter that
		break;
	}
}

inline void __SELib_internal_WriteNullTerminatedString(uint8_t ** source, SELIB_FS_HANDLE handle)
{
	for (int i = 0;; i++)
	{
		SELIB_FWRITE(&(*source)[i], 1, 1, handle);
		if ((*source)[i] == 0) break;
	}
}

inline void __SELib_internal_WriteVector3ToFile(uint8_t animationFlags, vec3_t * source, SELIB_FS_HANDLE handle)
{
#ifndef DONT_USE_HIGH_PRECISION
	if (animationFlags & SEANIM_PRECISION_HIGH)
	{
		SELIB_FWRITE(source, sizeof(vec3_t), 1, handle);
	}
	else {
		float out[3];
		for (int i = 0; i < 3; i++)
		{
			out[i] = (*source)[i];
		}
		SELIB_FWRITE(out, sizeof(float), 3, handle);
	}
#else
	if (animationFlags & SEANIM_PRECISION_HIGH)
	{
		double out[3];
		for (int i = 0; i < 3; i++)
		{
			out[i] = (*source)[i];
		}
		SELIB_FWRITE(out, sizeof(double), 3, handle);
	}
	else {
		SELIB_FWRITE(source, sizeof(vec3_t), 1, handle);
	}
#endif
}

inline void __SELib_internal_WriteQuatToFile(uint8_t animationFlags, quat_t * source, SELIB_FS_HANDLE handle)
{
#ifndef DONT_USE_HIGH_PRECISION
	if (animationFlags & SEANIM_PRECISION_HIGH)
	{
		SELIB_FWRITE(source, sizeof(quat_t), 1, handle);
	}
	else {
		float out[4];
		for (int i = 0; i < 4; i++)
		{
			out[i] = (*source)[i];
		}
		SELIB_FWRITE(out, sizeof(float), 4, handle);
	}
#else
	if (animationFlags & SEANIM_PRECISION_HIGH)
	{
		double out[4];
		for (int i = 0; i < 4; i++)
		{
			out[i] = (*source)[i];
		}
		SELIB_FWRITE(out, sizeof(double), 4, handle);
	}
	else {
		SELIB_FWRITE(source, sizeof(quat_t), 1, handle);
	}
#endif
}
