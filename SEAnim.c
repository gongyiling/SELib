#include "SELib.h"
#include "SEAnim.h"

int SELIB_API LoadSEAnim(SEAnim_File_t *dest, SELIB_FS_HANDLE handle)
{
	memset(dest, 0, sizeof(SEAnim_File_t));
	dest->isLoaded = 0;

	char magicToCompare[] = SEANIM_MAGIC;
	SELIB_FREAD(dest->magic, 1, 6, handle); // read magic, it's always 6 bytes

	if (strncmp(dest->magic, magicToCompare, 6) != 0)
		return SEANIM_WRONG_MAGIC;

	SELIB_FREAD(&dest->version, 2, 1, handle); // read version

	if (dest->version != SEANIM_VERSION)
		return SEANIM_WRONG_VERSION;

	SELIB_FREAD(&dest->header, sizeof(SEAnim_Header_t), 1, handle); // read header

	dest->bone = (uint8_t**)SELIB_CALLOC(dest->header.boneCount, sizeof(uint8_t*)); // allocate a string array for bone names
	for (uint32_t i = 0; i < dest->header.boneCount; i++)
	{
		__SELib_internal_ReadNullTerminatedString(&dest->bone[i], handle); // read bone name
		SELib_Printf("Bone %u: %s\n", i, dest->bone[i]);
	}

	int numSize = (dest->header.boneCount <= 0xFF ? 1 : (dest->header.boneCount <= 0xFFFF ? 2 : 4)); // determine bone index length in bytes depending on bone count
	dest->boneModifiers = (SEAnim_BoneAnimModifier_t *)SELIB_CALLOC(dest->header.boneAnimModifierCount, sizeof(SEAnim_BoneAnimModifier_t)); // allocate an array for bone modifiers
	for (uint8_t i = 0; i < dest->header.boneAnimModifierCount; i++)
	{
		__SELib_internal_ReadVariableLengthNumber(numSize, &dest->boneModifiers[i].index, handle);	// read the bone index for bone modifier
		SELIB_FREAD(&dest->boneModifiers[i].animTypeOverride, 1, 1, handle);						// read the anim type override for bone modifier
		SELib_Printf("Bone Anim Modifier %u: Bone %u | Value: %u\n", i, dest->boneModifiers[i].index, dest->boneModifiers[i].animTypeOverride);
	}

	numSize = (dest->header.frameCount <= 0xFF ? 1 : (dest->header.frameCount <= 0xFFFF ? 2 : 4)); // determine frame index length in bytes depending on frame count
	dest->boneData = (SEAnim_BoneData_t*)SELIB_CALLOC(dest->header.boneCount, sizeof(SEAnim_BoneData_t)); // allocate an array for bone data
	for (uint32_t i = 0; i < dest->header.boneCount; i++)
	{
		SELIB_FREAD(&dest->boneData[i].flags, 1, 1, handle);		// read flags for bone
		if (dest->header.dataPresenceFlags & SEANIM_BONE_LOC)		// does animation has bone position data?
		{
			__SELib_internal_ReadVariableLengthNumber(numSize, &dest->boneData[i].locKeyCount, handle);		// read the count for bone position keyframes
			dest->boneData[i].loc = (SEAnim_BoneLocData_t*)SELIB_CALLOC(dest->boneData[i].locKeyCount, sizeof(SEAnim_BoneLocData_t)); // allocate an array for bone position keyframe data
			for (uint32_t o = 0; o < dest->boneData[i].locKeyCount; o++)
			{
				__SELib_internal_ReadVariableLengthNumber(numSize, &dest->boneData[i].loc[o].frame, handle); // read frame index
				__SELib_internal_ReadVector3FromFile(dest->header.dataPropertyFlags, &dest->boneData[i].loc[o].loc, handle); // read keyframe data

				SELib_Printf("Frame %u (Bone %d): loc %f %f %f\n", dest->boneData[i].loc[o].frame, i, dest->boneData[i].loc[o].loc[0], dest->boneData[i].loc[o].loc[1], dest->boneData[i].loc[o].loc[2]);
			}
		}
		if (dest->header.dataPresenceFlags & SEANIM_BONE_ROT)		// does animation has bone rotation data?
		{
			__SELib_internal_ReadVariableLengthNumber(numSize, &dest->boneData[i].rotKeyCount, handle);		// read the count for bone rotation keyframes
			dest->boneData[i].quats = (SEAnim_BoneRotData_t*)SELIB_CALLOC(dest->boneData[i].rotKeyCount, sizeof(SEAnim_BoneRotData_t)); // allocate an array for bone rotation keyframe data
			for (uint32_t o = 0; o < dest->boneData[i].rotKeyCount; o++)
			{
				__SELib_internal_ReadVariableLengthNumber(numSize, &dest->boneData[i].quats[o].frame, handle); // read frame index
				__SELib_internal_ReadQuatFromFile(dest->header.dataPropertyFlags, &dest->boneData[i].quats[o].rot, handle); // read keyframe data
				SELib_Printf("Frame %u (Bone %d): rot %f %f %f %f\n", dest->boneData[i].quats[o].frame, i, dest->boneData[i].quats[o].rot[0], dest->boneData[i].quats[o].rot[1], dest->boneData[i].quats[o].rot[2], dest->boneData[i].quats[o].rot[3]);
			}
		}
		if (dest->header.dataPresenceFlags & SEANIM_BONE_SCALE)		// does animation has bone scale data?
		{
			__SELib_internal_ReadVariableLengthNumber(numSize, &dest->boneData[i].scaleKeyCount, handle);		// read the count for bone scale keyframes
			dest->boneData[i].scale = (SEAnim_BoneScaleData_t*)SELIB_CALLOC(dest->boneData[i].scaleKeyCount, sizeof(SEAnim_BoneScaleData_t)); // allocate an array for bone scale keyframe data
			for (uint32_t o = 0; o < dest->boneData[i].scaleKeyCount; o++)
			{
				__SELib_internal_ReadVariableLengthNumber(numSize, &dest->boneData[i].scale[o].frame, handle); // read frame index
				__SELib_internal_ReadVector3FromFile(dest->header.dataPropertyFlags, &dest->boneData[i].scale[o].scale, handle); // read keyframe data
				SELib_Printf("Frame %u (Bone %d): scale %f %f %f\n", dest->boneData[i].scale[o].frame, i, dest->boneData[i].scale[o].scale[0], dest->boneData[i].scale[o].scale[1], dest->boneData[i].scale[o].scale[2]);
			}
		}
	}

	if (dest->header.dataPresenceFlags & SEANIM_PRESENCE_NOTE)		// does animation has notetracks?
	{
		dest->notes = (SEAnim_Note_t*)SELIB_CALLOC(dest->header.noteCount, sizeof(SEAnim_Note_t)); // allocate an array for notes
		dest->noteCount = dest->header.noteCount;
		for (uint32_t i = 0; i < dest->header.noteCount; i++)
		{
			__SELib_internal_ReadVariableLengthNumber(numSize, &dest->notes[i].frame, handle);	// read frame index
			__SELib_internal_ReadNullTerminatedString(&dest->notes[i].name, handle);			// read note name
			SELib_Printf("Note %u (f: %u): %s\n", i, dest->notes[i].frame, dest->notes[i].name);
		}
	}

	if (dest->header.dataPresenceFlags & SEANIM_PRESENCE_CUSTOM)	// does animation has a custom data block?
	{
		SELIB_FREAD(&dest->customDataBlockSize, 4, 1, handle);		// read data block size
		dest->customDataBlockBuf = (uint8_t *)SELIB_CALLOC(dest->customDataBlockSize, 1); // allocate the memory for the data block
		SELIB_FREAD(dest->customDataBlockBuf, 1, dest->customDataBlockSize, handle); // read the custom data block
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
	for (uint32_t i = 0; i < ptr->header.boneCount; i++)
	{
		SELIB_FREE(ptr->boneData[i].loc);
		SELIB_FREE(ptr->boneData[i].quats);
		SELIB_FREE(ptr->boneData[i].scale);
		SELIB_FREE(ptr->bone[i]);
	}
	SELIB_FREE(ptr->boneData);
	SELIB_FREE(ptr->boneModifiers);
	SELIB_FREE(ptr->bone);
	for (uint32_t i = 0; i < ptr->header.noteCount; i++)
	{
		SELIB_FREE(ptr->notes[i].name);
	}
	SELIB_FREE(ptr->notes);
	SELIB_FREE(ptr->customDataBlockBuf);
}