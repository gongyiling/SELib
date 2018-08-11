#include "SEModel.h"

int SELIB_API LoadSEModel(SEModel_File_t * dest, SELIB_FS_HANDLE handle)
{
	memset(dest, 0, sizeof(SEModel_File_t));
	
	SELIB_FREAD(dest->magic, 1, sizeof(SEMODEL_MAGIC) - 1, handle);

	if (strncmp(dest->magic, SEMODEL_MAGIC, sizeof(SEMODEL_MAGIC) - 1) != 0)
		return SEMODEL_WRONG_MAGIC;

	SELIB_FREAD(&dest->version, 2, 1, handle);
	
	if (dest->version != SEMODEL_VERSION)
		return SEMODEL_WRONG_VERSION;

	SELIB_FREAD(&dest->header, sizeof(SEModel_Header_t), 1, handle);

	if (dest->header.dataPresenceFlags & SEMODEL_PRESENCE_BONE)
	{
		dest->bone = (uint8_t**)SELIB_CALLOC(dest->header.boneCount, sizeof(uint8_t*));
		for (uint32_t i = 0; i < dest->header.boneCount; i++)
		{
			__SELib_internal_ReadNullTerminatedString(&dest->bone[i], handle); // read bone name
			SELib_Printf("Bone %u: %s\n", i, dest->bone[i]);
		}

		dest->boneData = (SEModel_BoneData_t *)SELIB_CALLOC(dest->header.boneCount, sizeof(SEModel_BoneData_t));
		for (uint32_t i = 0; i < dest->header.boneCount; i++)
		{
			SEModel_BoneData_t *bone = &dest->boneData[i];
			memset(bone, 0, sizeof(SEModel_BoneData_t));
			SELIB_FREAD(&bone->flags, 1, 1, handle);
			SELIB_FREAD(&bone->boneParent, 4, 1, handle);
			if (dest->header.boneDataPresenceFlags & SEMODEL_PRESENCE_GLOBAL_MATRIX)
			{
				SELIB_FREAD(bone->global_position, sizeof(float), 3, handle);
				SELIB_FREAD(bone->global_rotation, sizeof(float), 4, handle);
			}
			if (dest->header.boneDataPresenceFlags & SEMODEL_PRESENCE_LOCAL_MATRIX)
			{
				SELIB_FREAD(bone->local_position, sizeof(float), 3, handle);
				SELIB_FREAD(bone->local_rotation, sizeof(float), 4, handle);
			}
			if (dest->header.boneDataPresenceFlags & SEMODEL_PRESENCE_SCALES)
			{
				SELIB_FREAD(bone->scale, sizeof(float), 3, handle);
			}
		}
	}
	int ft = ftell(handle);
	if (dest->header.dataPresenceFlags & SEMODEL_PRESENCE_MESH)
	{
		dest->meshData = (SEModel_MeshData_t *)SELIB_CALLOC(dest->header.meshCount, sizeof(SEModel_MeshData_t));
		for (uint32_t i = 0; i < dest->header.meshCount; i++)
		{
			SEModel_MeshData_t *mesh = &dest->meshData[i];
			memset(mesh, 0, sizeof(SEModel_MeshData_t));
			SELIB_FREAD(&mesh->flags, 1, 1, handle);
			SELIB_FREAD(&mesh->uvSetCount, 1, 1, handle);
			SELIB_FREAD(&mesh->maxSkinInfluence, 1, 1, handle);
			SELIB_FREAD(&mesh->vertexCount, 4, 1, handle);
			SELIB_FREAD(&mesh->faceCount, 4, 1, handle);

			uint32_t vc = mesh->vertexCount;
			SEModel_VertexBuffer_t *vb = &mesh->vertexBuffer;
			{
				vb->position = (vec3f_t *)SELIB_CALLOC(vc, sizeof(vec3f_t));
				for (uint32_t o = 0; o < vc; o++)
				{
					SELIB_FREAD(vb->position[o], sizeof(vec3f_t), 1, handle);
				}

				if (dest->header.meshDataPresenceFlags & SEMODEL_PRESENCE_UVSET)
				{
					vb->uvCoords = (vec2f_t **)SELIB_CALLOC(mesh->uvSetCount, sizeof(vec2f_t *));
					for (uint8_t o = 0; o < mesh->uvSetCount; o++)
					{
						vb->uvCoords[o] = (vec2f_t *)SELIB_CALLOC(vc, sizeof(vec2f_t));
						for (uint32_t p = 0; p < vc; p++)
						{
							SELIB_FREAD(vb->uvCoords[o][p], sizeof(vec2f_t), 1, handle);
						}
					}
				}

				if (dest->header.meshDataPresenceFlags & SEMODEL_PRESENCE_NORMALS)
				{
					vb->normal = (vec3f_t *)SELIB_CALLOC(vc, sizeof(vec3f_t));
					for (uint32_t o = 0; o < vc; o++)
					{
						SELIB_FREAD(vb->normal[o], sizeof(vec3f_t), 1, handle);
					}
				}

				if (dest->header.meshDataPresenceFlags & SEMODEL_PRESENCE_COLOR)
				{
					vb->color = (quatb_t *)SELIB_CALLOC(vc, sizeof(quatb_t));
					for (uint32_t o = 0; o < vc; o++)
					{
						SELIB_FREAD(vb->color[o], sizeof(quatb_t), 1, handle);
					}
				}

				if (dest->header.meshDataPresenceFlags & SEMODEL_PRESENCE_WEIGHTS)
				{
					vb->boneIndex = (uint32_t **)SELIB_CALLOC(vc, sizeof(uint32_t *));
					vb->boneWeight = (float **)SELIB_CALLOC(vc, sizeof(float *));
					for (uint32_t o = 0; o < vc; o++)
					{
						vb->boneIndex[o] = (uint32_t *)SELIB_CALLOC(mesh->maxSkinInfluence, sizeof(uint32_t));
						vb->boneWeight[o] = (float *)SELIB_CALLOC(mesh->maxSkinInfluence, sizeof(float));
						for (uint8_t p = 0; p < mesh->maxSkinInfluence; p++)
						{
							//SELIB_FREAD(&vb->boneIndex[o][p], 4, 1, handle);
							int numSize = (dest->header.boneCount <= 0xFF ? 1 : (dest->header.boneCount <= 0xFFFF ? 2 : 4));
							__SELib_internal_ReadVariableLengthNumber(numSize, &vb->boneIndex[o][p], handle);
						}
						for (uint8_t p = 0; p < mesh->maxSkinInfluence; p++)
						{
							SELIB_FREAD(&vb->boneWeight[o][p], sizeof(float), 1, handle);
						}
					}
				}

				if (dest->header.meshDataPresenceFlags & SEMODEL_PRESENCE_UVSET)
				{
					vb->matIndex = (int32_t *)SELIB_CALLOC(mesh->uvSetCount, sizeof(int32_t));
					for (uint8_t o = 0; o < mesh->uvSetCount; o++)
					{
						SELIB_FREAD(&vb->matIndex[o], sizeof(int32_t), 1, handle);
					}
				}

			}

			mesh->faceData = (SEModel_FaceData_t *)SELIB_CALLOC(mesh->faceCount, sizeof(SEModel_FaceData_t));
			int numSize = (mesh->faceCount <= 0xFF ? 1 : (mesh->faceCount <= 0xFFFF ? 2 : 4));
			for (uint32_t i = 0; i < mesh->faceCount; i++)
			{
				for (uint32_t o = 0; o < 3; o++)
				{
					__SELib_internal_ReadVariableLengthNumber(numSize, &mesh->faceData[i].Indicies[o], handle);
				}
			}
		}
	}

	if (dest->header.dataPresenceFlags & SEMODEL_PRESENCE_MATERIALS)
	{
		dest->matData = (SEModel_MaterialData_t *)SELIB_CALLOC(dest->header.matCount, sizeof(SEModel_MaterialData_t));
		for (uint32_t i = 0; i < dest->header.matCount; i++)
		{
			SEModel_MaterialData_t *mat = &dest->matData[i];
			__SELib_internal_ReadNullTerminatedString(&mat->Name, handle);
			SELIB_FREAD(&mat->materialType, 1, 1, handle);
			__SELib_internal_ReadNullTerminatedString(&mat->ShaderData.DiffuseMap, handle);
			__SELib_internal_ReadNullTerminatedString(&mat->ShaderData.NormalMap, handle);
			__SELib_internal_ReadNullTerminatedString(&mat->ShaderData.SpecularMap, handle);
		}
	}

	return SEMODEL_OK;
}

int SELIB_API SaveSEModel(SEModel_File_t * src, SELIB_FS_HANDLE destHandle)
{
	return 0;
}

void SELIB_API FreeSEModel(SEModel_File_t * ptr)
{
	return;
}
