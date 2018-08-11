/*

How to load SEAnims

	SEAnim_File_t seanimFile; // create the SEAnim var
	FILE* f;
	f = fopen("ch_idle.seanim","rb");
	int r = LoadSEAnim(&seanimFile, f);

That's it! Simple as that.
You can access every bit of information that was stored in the SEAnim in the var.

How to save SEAnims
// TODO

*/


#ifndef __SEANIM_H
#define __SEANIM_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "SELib.h"

// Error codes
#define SEANIM_OK 0
#define SEANIM_WRONG_MAGIC 1
#define SEANIM_WRONG_VERSION 2
#define SEANIM_UNKNOWN_ERROR -1

#define SEANIM_VERSION			1
#define SEANIM_MAGIC			"SEAnim"
#define SEANIM_SPEC_VERSION		"1.1.0"
#define SEANIM_LIB_VERSION		"1.0"

	// Specifies how the data is interpreted by the importer
	enum SEAnim_AnimationType
	{
		// Animation translations are set to this exact value each frame
		SEANIM_TYPE_ABSOLUTE,
		// This animation is applied to existing animation data in the scene
		SEANIM_TYPE_ADDITIVE,
		// Animation translations are based on rest position in scene
		SEANIM_TYPE_RELATIVE,
		// This animation is relative and contains delta data (Whole model movement) Delta tag name must be set!
		SEANIM_TYPE_DELTA
	};

	// Specifies the data present for each frame of every bone
	enum SEAnim_DataPresenceFlags
	{
		// These describe what type of keyframe data is present for the bones
		SEANIM_BONE_LOC = 1 << 0,
		SEANIM_BONE_ROT = 1 << 1,
		SEANIM_BONE_SCALE = 1 << 2,

		// If any of the above flags are set, then bone keyframe data is present, thus this comparing against this mask will return true
		SEANIM_PRESENCE_BONE = SEANIM_BONE_LOC | SEANIM_BONE_ROT | SEANIM_BONE_SCALE,

		// The file contains notetrack data
		SEANIM_PRESENCE_NOTE = 1 << 6,
		// The file contains a custom data block
		SEANIM_PRESENCE_CUSTOM = 1 << 7,
	};

	enum SEAnim_BoneFlags {
		SEANIM_BONE_NORMAL = 0,
		SEANIM_BONE_COSMETIC = 1 << 0
	};

	enum SEAnim_PropertyFlags
	{
		SEANIM_PRECISION_HIGH = 1 << 0 // Use double precision floating point vectors instead of single precision
		//RESERVED_1		= 1 << 1, // ALWAYS FALSE
		//RESERVED_2		= 1 << 2, // ALWAYS FALSE
		//RESERVED_3		= 1 << 3, // ALWAYS FALSE
		//RESERVED_4		= 1 << 4, // ALWAYS FALSE
		//RESERVED_5		= 1 << 5, // ALWAYS FALSE
		//RESERVED_6		= 1 << 6, // ALWAYS FALSE
		//RESERVED_7		= 1 << 7, // ALWAYS FALSE
	};

	enum SEAnim_Flags
	{
		SEANIM_LOOPED = 1 << 0, // The animation is a looping animation
								//RESERVED_0		= 1 << 1, // ALWAYS FALSE
								//RESERVED_1		= 1 << 2, // ALWAYS FALSE
								//RESERVED_2		= 1 << 3, // ALWAYS FALSE
								//RESERVED_3		= 1 << 4, // ALWAYS FALSE
								//RESERVED_4		= 1 << 5, // ALWAYS FALSE
								//RESERVED_5		= 1 << 6, // ALWAYS FALSE
								//RESERVED_6		= 1 << 7, // ALWAYS FALSE
	};

	typedef struct SEAnim_BoneAnimModifier_s
	{
		uint32_t index; // Index of the bone
		uint8_t animTypeOverride; // AnimType to use for that bone, and its children recursively
	} SEAnim_BoneAnimModifier_t;

	typedef struct SEAnim_BoneLocData_s
	{
		uint32_t frame;
		vec3_t loc;
	} SEAnim_BoneLocData_t;

	typedef struct SEAnim_BoneRotData_s
	{
		uint32_t frame;
		quat_t rot;
	} SEAnim_BoneRotData_t;

	typedef struct SEAnim_BoneScaleData_s
	{
		uint32_t frame;
		vec3_t scale; // 1.0 is the default scale, 2.0 is twice as big, and 0.5 is half size
	} SEAnim_BoneScaleData_t;

	//
	// Pooled String Support Will Be Added Later Maybe
	//
	typedef struct SEAnim_Note_s
	{
		uint32_t frame;
		char *name;
	} SEAnim_Note_t;

	typedef struct SEAnim_BoneData_s
	{
		/*
		Currently the only supported values 'flags' are:
		0 = DEFAULT
		1 = COSMETIC
		*/
		uint8_t flags;

		uint32_t locKeyCount;
		SEAnim_BoneLocData_t *loc;

		uint32_t rotKeyCount;
		SEAnim_BoneRotData_t *quats;
		
		uint32_t scaleKeyCount;
		SEAnim_BoneScaleData_t *scale;

	} SEAnim_BoneData_t;

	typedef struct SEAnim_Header_s
	{
		// Contains the size of the header block in bytes, any extra data is ignored
		uint16_t headerSize; //currently 0x1C

		// The type of animation data that is stored, matches an SEANIM_TYPE
		uint8_t animType;

		// Bitwise flags that define the properties for the animation itself
		uint8_t animFlags;

		// Bitwise flags that define which data blocks are present, and properties for those data blocks
		uint8_t dataPresenceFlags;

		// Bitwise flags containing property information pertaining regarding the data in the file
		uint8_t dataPropertyFlags;

		// RESERVED - Should be 0
		uint8_t reserved1[2];

		float framerate;

		// frameCount describes the length of the animation (in frames)
		// It is used to determine the size of frame_t
		// This should be equal to the largest frame number in the anim (Including keys & notes) plus 1
		// Ex: An anim with keys on frame 0, 5 and 8 - frameCount would be 9
		//     An anim with keys on frame 4, 7, and 14 - frameCount would be 15
		uint32_t frameCount;

		// Is 0 if ( presenceFlags & SEANIM_PRESENCE_BONE ) is false
		uint32_t boneCount;
		uint8_t boneAnimModifierCount; // The number of animType modifier bones

									   // RESERVED - Should be 0
		uint8_t reserved2[3];

		// Is 0 if ( presenceFlags & SEANIM_PRESENCE_NOTE ) is false
		uint32_t noteCount;
	} SEAnim_Header_t;

	typedef struct SEAnim_File_s
	{
		char magic[6];		// 'SEAnim'
		uint16_t version;	// The file version - the current version is 0x1
		SEAnim_Header_t header;

		uint8_t						**bone;
		SEAnim_BoneAnimModifier_t	*boneModifiers;
		SEAnim_BoneData_t			*boneData;

		uint32_t				noteCount;
		SEAnim_Note_t			*notes;

		uint32_t customDataBlockSize;
		uint8_t *customDataBlockBuf;

	} SEAnim_File_t;

	int SELIB_API LoadSEAnim(SEAnim_File_t *dest, SELIB_FS_HANDLE handle);

	int SELIB_API SaveSEAnim(SEAnim_File_t *src, SELIB_FS_HANDLE destHandle);

	void SELIB_API FreeSEAnim(SEAnim_File_t *ptr);
	

#ifdef __cplusplus
}
#endif

#endif


