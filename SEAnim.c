#include "SELib.h"
#include "SEAnim.h"

#define SELib_Log 0

#define SELib_Printf if(SELib_Log) printf

int SELIB_API LoadSEAnim(SEAnim_File_t *dest, SELIB_FS_HANDLE handle)
{
	memset(dest, 0, sizeof(SEAnim_File_t));
	dest->isLoaded = 0;

	char magicToCompare[] = SEANIM_MAGIC;
	SELIB_FREAD(dest->magic, 1, 6, handle);

	if (strncmp(dest->magic, magicToCompare, 6) != 0)
		return SEANIM_WRONG_MAGIC;

	SELIB_FREAD(&dest->version, 2, 1, handle);

	if (dest->version != SEANIM_VERSION)
		return SEANIM_WRONG_VERSION;

	SELIB_FREAD(&dest->header, sizeof(SEAnim_Header_t), 1, handle);

	dest->bone = (uint8_t**)SELIB_CALLOC(dest->header.boneCount, sizeof(uint8_t*));
	for (uint32_t i = 0; i < dest->header.boneCount; i++)
	{
		dest->bone[i] = (uint8_t*)SELIB_CALLOC(1, sizeof(uint8_t));
		dest->bone[i][0] = 0xcc;
		int stri = 0;
		char started = 0;
		while (dest->bone[i][stri] != 0)
		{
			if (started) stri++;
			else started = 1;
			dest->bone[i] = SELIB_REALLOC(dest->bone[i], stri + 1);
			SELIB_FREAD(&dest->bone[i][stri], 1, 1, handle);
		}

		SELib_Printf("Bone %u: %s\n", i, dest->bone[i]);
	}

	int numSize = (dest->header.boneCount <= 0xFF ? 1 : (dest->header.boneCount <= 0xFFFF ? 2 : 4));
	dest->boneModifiers = (SEAnim_BoneAnimModifier_t *)SELIB_CALLOC(dest->header.boneAnimModifierCount, sizeof(SEAnim_BoneAnimModifier_t));
	for (uint8_t i = 0; i < dest->header.boneAnimModifierCount; i++)
	{
		uint8_t v8 = 0;
		uint16_t v16 = 0;
		uint32_t v32 = 0;
		switch (numSize)
		{
		case 1:
			SELIB_FREAD(&v8, 1, 1, handle);
			dest->boneModifiers[i].index = (uint32_t)v8;
			break;
		case 2:
			SELIB_FREAD(&v16, 2, 1, handle);
			dest->boneModifiers[i].index = (uint32_t)v16;
			break;
		case 4:
			SELIB_FREAD(&v32, 4, 1, handle);
			dest->boneModifiers[i].index = (uint32_t)v32;
			break;
		default:
			SELIB_ASSERT(0); // should never ever happen.... ever
			break;
		}
		SELIB_FREAD(&dest->boneModifiers[i].animTypeOverride, 1, 1, handle);
		SELib_Printf("Bone Anim Modifier %u: Bone %u | Value: %u\n", i, dest->boneModifiers[i].index, dest->boneModifiers[i].animTypeOverride);
	}

	numSize = (dest->header.frameCount <= 0xFF ? 1 : (dest->header.frameCount <= 0xFFFF ? 2 : 4));
	dest->boneData = (SEAnim_BoneData_t*)SELIB_CALLOC(dest->header.boneCount, sizeof(SEAnim_BoneData_t));
	for (uint32_t i = 0; i < dest->header.boneCount; i++)
	{
		SELIB_FREAD(&dest->boneData[i].flags, 1, 1, handle);
		if (dest->header.dataPresenceFlags & SEANIM_BONE_LOC)
		{
			uint8_t v8 = 0;
			uint16_t v16 = 0;
			uint32_t v32 = 0;
			switch (numSize)
			{
			case 1:
				SELIB_FREAD(&v8, 1, 1, handle);
				dest->boneData[i].locKeyCount = (uint32_t)v8;
				break;
			case 2:
				SELIB_FREAD(&v16, 2, 1, handle);
				dest->boneData[i].locKeyCount = (uint32_t)v16;
				break;
			case 4:
				SELIB_FREAD(&v32, 4, 1, handle);
				dest->boneData[i].locKeyCount = (uint32_t)v32;
				break;
			default:
				SELIB_ASSERT(0); // should never ever happen.... ever
				break;
			}

			dest->boneData[i].loc = (SEAnim_BoneLocData_t*)SELIB_CALLOC(dest->boneData[i].locKeyCount, sizeof(SEAnim_BoneLocData_t));
			for (uint32_t o = 0; o < dest->boneData[i].locKeyCount; o++)
			{
				switch (numSize)
				{
				case 1:
					SELIB_FREAD(&v8, 1, 1, handle);
					dest->boneData[i].loc[o].frame = (uint32_t)v8;
					break;
				case 2:
					SELIB_FREAD(&v16, 2, 1, handle);
					dest->boneData[i].loc[o].frame = (uint32_t)v16;
					break;
				case 4:
					SELIB_FREAD(&v32, 4, 1, handle);
					dest->boneData[i].loc[o].frame = (uint32_t)v32;
					break;
				default:
					SELIB_ASSERT(0); // should never ever happen.... ever
					break;
				}
				if (dest->header.dataPropertyFlags & SEANIM_PRECISION_HIGH)
				{
					SELIB_FREAD(dest->boneData[i].loc[o].loc, sizeof(vec3_t), 1, handle);
				}
				else {
					float in[3];
					vec3_t res;
					SELIB_FREAD(in, sizeof(float), 3, handle);
					for (int pp = 0; pp < 3; pp++)
					{
						res[pp] = (double)in[pp];
					}
					memcpy(dest->boneData[i].loc[o].loc, res, 3 * sizeof(double));
				}
				SELib_Printf("Frame %u (Bone %d): loc %f %f %f\n", dest->boneData[i].loc[o].frame, i, dest->boneData[i].loc[o].loc[0], dest->boneData[i].loc[o].loc[1], dest->boneData[i].loc[o].loc[2]);
			}
		}
		if (dest->header.dataPresenceFlags & SEANIM_BONE_ROT)
		{
			uint8_t v8 = 0;
			uint16_t v16 = 0;
			uint32_t v32 = 0;
			switch (numSize)
			{
			case 1:
				SELIB_FREAD(&v8, 1, 1, handle);
				dest->boneData[i].rotKeyCount = (uint32_t)v8;
				break;
			case 2:
				SELIB_FREAD(&v16, 2, 1, handle);
				dest->boneData[i].rotKeyCount = (uint32_t)v16;
				break;
			case 4:
				SELIB_FREAD(&v32, 4, 1, handle);
				dest->boneData[i].rotKeyCount = (uint32_t)v32;
				break;
			default:
				SELIB_ASSERT(0); // should never ever happen.... ever
				break;
			}
			dest->boneData[i].quats = (SEAnim_BoneRotData_t*)SELIB_CALLOC(dest->boneData[i].rotKeyCount, sizeof(SEAnim_BoneRotData_t));
			for (uint32_t o = 0; o < dest->boneData[i].rotKeyCount; o++)
			{
				switch (numSize)
				{
				case 1:
					SELIB_FREAD(&v8, 1, 1, handle);
					dest->boneData[i].quats[o].frame = (uint32_t)v8;
					break;
				case 2:
					SELIB_FREAD(&v16, 2, 1, handle);
					dest->boneData[i].quats[o].frame = (uint32_t)v16;
					break;
				case 4:
					SELIB_FREAD(&v32, 4, 1, handle);
					dest->boneData[i].quats[o].frame = (uint32_t)v32;
					break;
				default:
					SELIB_ASSERT(0); // should never ever happen.... ever
					break;
				}
				if (dest->header.dataPropertyFlags & SEANIM_PRECISION_HIGH)
				{
					SELIB_FREAD(dest->boneData[i].quats[o].rot, sizeof(quat_t), 1, handle);
				}
				else {
					float in[4];
					quat_t res;
					SELIB_FREAD(in, sizeof(float), 4, handle);
					for (int pp = 0; pp < 4; pp++)
					{
						res[pp] = (double)in[pp];
					}
					memcpy(dest->boneData[i].quats[o].rot, res, 4 * sizeof(double));
				}
				SELib_Printf("Frame %u (Bone %d): rot %f %f %f %f\n", dest->boneData[i].quats[o].frame, i, dest->boneData[i].quats[o].rot[0], dest->boneData[i].quats[o].rot[1], dest->boneData[i].quats[o].rot[2], dest->boneData[i].quats[o].rot[3]);
			}
		}
		if (dest->header.dataPresenceFlags & SEANIM_BONE_SCALE)
		{
			uint8_t v8 = 0;
			uint16_t v16 = 0;
			uint32_t v32 = 0;
			switch (numSize)
			{
			case 1:
				SELIB_FREAD(&v8, 1, 1, handle);
				dest->boneData[i].scaleKeyCount = (uint32_t)v8;
				break;
			case 2:
				SELIB_FREAD(&v16, 2, 1, handle);
				dest->boneData[i].scaleKeyCount = (uint32_t)v16;
				break;
			case 4:
				SELIB_FREAD(&v32, 4, 1, handle);
				dest->boneData[i].scaleKeyCount = (uint32_t)v32;
				break;
			default:
				SELIB_ASSERT(0); // should never ever happen.... ever
				break;
			}
			dest->boneData[i].scale = (SEAnim_BoneScaleData_t*)SELIB_CALLOC(dest->boneData[i].scaleKeyCount, sizeof(SEAnim_BoneScaleData_t));
			for (uint32_t o = 0; o < dest->boneData[i].scaleKeyCount; o++)
			{
				switch (numSize)
				{
				case 1:
					SELIB_FREAD(&v8, 1, 1, handle);
					dest->boneData[i].scale[o].frame = (uint32_t)v8;
					break;
				case 2:
					SELIB_FREAD(&v16, 2, 1, handle);
					dest->boneData[i].scale[o].frame = (uint32_t)v16;
					break;
				case 4:
					SELIB_FREAD(&v32, 4, 1, handle);
					dest->boneData[i].scale[o].frame = (uint32_t)v32;
					break;
				default:
					SELIB_ASSERT(0); // should never ever happen.... ever
					break;
				}
				if (dest->header.dataPropertyFlags & SEANIM_PRECISION_HIGH)
				{
					SELIB_FREAD(dest->boneData[i].scale[o].scale, sizeof(vec3_t), 1, handle);
				}
				else {
					float in[3];
					vec3_t res;
					SELIB_FREAD(in, sizeof(float), 3, handle);
					for (int pp = 0; pp < 3; pp++)
					{
						res[pp] = (double)in[pp];
					}
					memcpy(dest->boneData[i].scale[o].scale, res, 3 * sizeof(double));
				}
				SELib_Printf("Frame %u (Bone %d): scale %f %f %f\n", dest->boneData[i].scale[o].frame, i, dest->boneData[i].scale[o].scale[0], dest->boneData[i].scale[o].scale[1], dest->boneData[i].scale[o].scale[2]);
			}
		}
	}

	if (dest->header.dataPresenceFlags & SEANIM_PRESENCE_NOTE)
	{
		dest->notes = (SEAnim_Note_t*)SELIB_CALLOC(dest->header.noteCount, sizeof(SEAnim_Note_t));
		dest->noteCount = dest->header.noteCount;
		for (uint32_t i = 0; i < dest->header.noteCount; i++)
		{
			uint8_t v8 = 0;
			uint16_t v16 = 0;
			uint32_t v32 = 0;
			switch (numSize)
			{
			case 1:
				SELIB_FREAD(&v8, 1, 1, handle);
				dest->notes[i].frame = (uint32_t)v8;
				break;
			case 2:
				SELIB_FREAD(&v16, 2, 1, handle);
				dest->notes[i].frame = (uint32_t)v16;
				break;
			case 4:
				SELIB_FREAD(&v32, 4, 1, handle);
				dest->notes[i].frame = (uint32_t)v32;
				break;
			default:
				SELIB_ASSERT(0); // should never ever happen.... ever
				break;
			}
			dest->notes[i].name = (uint8_t*)SELIB_CALLOC(1, sizeof(uint8_t));
			dest->notes[i].name[0] = 0xcc;
			int stri = 0;
			char started = 0;
			while (dest->notes[i].name[stri] != 0 )
			{
				if (started) stri++;
				else started = 1;
				dest->notes[i].name = SELIB_REALLOC(dest->notes[i].name, stri + 1);
				SELIB_FREAD(&dest->notes[i].name[stri], 1, 1, handle);
			}
			SELib_Printf("Note %u (f: %u): %s\n", i, dest->notes[i].frame, dest->notes[i].name);
		}
	}

	if (dest->header.dataPresenceFlags & SEANIM_PRESENCE_CUSTOM)
	{
		SELIB_FREAD(&dest->customDataBlockSize, 4, 1, handle);
		dest->customDataBlockBuf = (uint8_t *)SELIB_CALLOC(dest->customDataBlockSize, 1);
		SELIB_FREAD(dest->customDataBlockBuf, 1, dest->customDataBlockSize, handle);
	}
	dest->isLoaded = 1;
	return SEANIM_OK;
}

int SELIB_API SaveSEAnim(SEAnim_File_t * src, SELIB_FS_HANDLE destHandle)
{
	// TODO
	return 0;
}

void SELIB_API FreeSEAnim(SEAnim_File_t *ptr)
{
	for (int i = 0; i < ptr->header.boneCount; i++)
	{
		SELIB_FREE(ptr->boneData[i].loc);
		SELIB_FREE(ptr->boneData[i].quats);
		SELIB_FREE(ptr->boneData[i].scale);
		SELIB_FREE(ptr->bone[i]);
	}
	SELIB_FREE(ptr->boneData);
	SELIB_FREE(ptr->boneModifiers);
	SELIB_FREE(ptr->bone);
	for (int i = 0; i < ptr->header.noteCount; i++)
	{
		SELIB_FREE(ptr->notes[i].name);
	}
	SELIB_FREE(ptr->notes);
	SELIB_FREE(ptr->customDataBlockBuf);
}