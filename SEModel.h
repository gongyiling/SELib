#ifndef __SEMODEL_H
#define __SEMODEL_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "SELib.h"

	// Error codes
#define SEMODEL_OK 0
#define SEMODEL_WRONG_MAGIC 1
#define SEMODEL_WRONG_VERSION 2
#define SEMODEL_UNKNOWN_ERROR -1

#define SEMODEL_VERSION			1
#define SEMODEL_MAGIC			"SEModel"
#define SEMODEL_SPEC_VERSION	"1.0"
#define SEMODEL_LIB_VERSION		"1.0"

	enum SEMODEL_PRESENCE_FLAGS
	{
		// Whether or not this model contains a bone block
		SEMODEL_PRESENCE_BONE = 1 << 0,
		// Whether or not this model contains submesh blocks
		SEMODEL_PRESENCE_MESH = 1 << 1,
		// Whether or not this model contains a material reference block
		SEMODEL_PRESENCE_MATERIALS = 1 << 2,

		// RESERVED_0		= 1 << 3, // ALWAYS FALSE
		// RESERVED_1		= 1 << 4, // ALWAYS FALSE
		// RESERVED_2		= 1 << 5, // ALWAYS FALSE
		// RESERVED_3		= 1 << 6, // ALWAYS FALSE

		// Whether or not this model contains a custom data block
		SEMODEL_PRESENCE_CUSTOM = 1 << 7,
	};

	enum SEMODEL_BONEPRESENCE_FLAGS
	{
		// Whether or not bones contain global matricies
		SEMODEL_PRESENCE_GLOBAL_MATRIX = 1 << 0,
		// Whether or not bones contain local matricies
		SEMODEL_PRESENCE_LOCAL_MATRIX = 1 << 1,

		// Whether or not bones contain scales
		SEMODEL_PRESENCE_SCALES = 1 << 2,

		// RESERVED_0		= 1 << 3, // ALWAYS FALSE
		// RESERVED_1		= 1 << 4, // ALWAYS FALSE
		// RESERVED_2		= 1 << 5, // ALWAYS FALSE
		// RESERVED_3		= 1 << 6, // ALWAYS FALSE
		// RESERVED_4		= 1 << 7, // ALWAYS FALSE
	};

	enum SEMODEL_MESHPRESENCE_FLAGS
	{
		// Whether or not meshes contain at least 1 uv layer
		SEMODEL_PRESENCE_UVSET = 1 << 0,

		// Whether or not meshes contain vertex normals
		SEMODEL_PRESENCE_NORMALS = 1 << 1,

		// Whether or not meshes contain vertex colors
		SEMODEL_PRESENCE_COLOR = 1 << 2,

		// Whether or not meshes contain at least 1 weight set
		SEMODEL_PRESENCE_WEIGHTS = 1 << 3,

		// RESERVED_0		= 1 << 4, // ALWAYS FALSE
		// RESERVED_1		= 1 << 5, // ALWAYS FALSE
		// RESERVED_2		= 1 << 6, // ALWAYS FALSE
		// RESERVED_3		= 1 << 7, // ALWAYS FALSE
	};
#pragma pack(push, 1)
	typedef struct SEModel_BoneData_s
	{
		uint8_t flags;

		int32_t boneParent; // Is -1 if bone is a root bone, otherwise, index of parent bone.

		vec3f_t global_position;
		quatf_t global_rotation;
		
		vec3f_t local_position;
		quatf_t local_rotation;
		
		vec3f_t scale;	// 1.0 is the default scale, 2.0 is twice as big, and 0.5 is half size
	} SEModel_BoneData_t;

	typedef struct SEModel_VertexBuffer_s
	{
		vec3f_t *position; // [Vertex]

		vec2f_t **uvCoords; // [UV Set][Vertex]

		vec3f_t *normal; // [Vertex]

		quatb_t *color; // [Vertex]

		uint32_t **boneIndex; // [Vertex][Bone Index]
		float **boneWeight; // [Vertex][Bone Index]

		int32_t *matIndex;	// [UV Set]

	} SEModel_VertexBuffer_t;

	typedef struct SEModel_FaceData_s
	{
		uint32_t Indicies[3];
	} SEModel_FaceData_t;

	typedef struct SEModel_MeshData_s
	{
		uint8_t flags;

		// The count of UV sets per vertex in this mesh
		uint8_t uvSetCount;
		// The maximum skin influcence for this mesh
		uint8_t maxSkinInfluence;

		// The count of verticies in the mesh
		uint32_t vertexCount;

		// The count of faces in the mesh, faces match D3DPT_TRIANGLELIST (DirectX) and VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST (Vulkan)
		uint32_t faceCount; 

		// The unified vertex buffer
		SEModel_VertexBuffer_t vertexBuffer;

		// A collection of tris that make up this mesh
		SEModel_FaceData_t *faceData;
	} SEModel_MeshData_t;

	typedef struct SEModel_Header_s
	{
		// Contains the size of the header block in bytes, any extra data is ignored, ignoring headerSize itself.
		uint16_t headerSize; // Currently 0x14

		// Bitwise flags that define which data blocks are present, matches SEMODEL_PRESENCE_FLAGS enum
		uint8_t dataPresenceFlags;

		// Bitwise flags that define which data is present in the boneData block, matches SEMODEL_BONEPRESENCE_FLAGS enum
		uint8_t boneDataPresenceFlags;

		// Bitwise flags that define which data is present in the meshData blocks, matches SEMODEL_MESHPRESENCE_FLAGS enum
		uint8_t meshDataPresenceFlags;

		// Is 0 if ( dataPresenceFlags & SEANIM_PRESENCE_BONE ) is false
		uint32_t boneCount;
		// Is 0 if ( dataPresenceFlags & SEMODEL_PRESENCE_MESH ) is false
		uint32_t meshCount;
		// Is 0 if ( dataPresenceFlags & SEMODEL_PRESENCE_MATERIALS ) is false
		uint32_t matCount;

		// RESERVED - Should be 0
		uint8_t reserved2[3];
	} SEModel_Header_t;

	typedef struct SEModel_SimpleMaterialShader_s
	{
		char *DiffuseMap;
		char *NormalMap;
		char *SpecularMap;

		// Support a color input as well? Otherwise use dynamic shader boolean...
	} SEModel_SimpleMaterialShader_t;

	typedef struct SEModel_MaterialData_s
	{
		char *Name;

		// Material type, 1 = Simple Material Shader
		uint8_t materialType;

		// The material payload, currently, we only have SEModel_SimpleMaterialShader
		SEModel_SimpleMaterialShader_t ShaderData;
	} SEModel_MaterialData_t;

	typedef struct SEModel_File_s
	{
		char magic[7];			// 'SEModel'
		uint16_t version;		// The file version - the current version is 0x1
		SEModel_Header_t header;

		uint8_t **bone;
		SEModel_BoneData_t *boneData; // [Bone]
		SEModel_MeshData_t *meshData; // [Mesh]
		SEModel_MaterialData_t *matData; // [Material]

	} SEModel_File_t;

#pragma pack(pop)

	int SELIB_API LoadSEModel(SEModel_File_t *dest, SELIB_FS_HANDLE handle);

	int SELIB_API SaveSEModel(SEModel_File_t *src, SELIB_FS_HANDLE destHandle);

	void SELIB_API FreeSEModel(SEModel_File_t *ptr);

#ifdef __cplusplus
}
#endif

#endif