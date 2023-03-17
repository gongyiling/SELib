// SEAnimInfo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <time.h>
#include <filesystem>
#include <vector>
#include <unordered_map>
time_t startClock = 0;
time_t endClock = 0;

namespace fs = std::filesystem;

int find_bone(const SEAnim_File_t& seanim, const char* bone_name)
{
	for (int i = 0; i < seanim.header.boneCount; i++)
	{
		if (strcmp((const char*)seanim.bone[i], bone_name) == 0)
		{
			return i;
		}
	}
	return -1;
}

static double calc_distance(const vec3_t& a, const vec3_t& b)
{
	double x = a[0] - b[0];
	double y = a[1] - b[1];
	double z = a[2] - b[2];
	return std::sqrt(x * x + y * y + z * z);
}

struct accinfo
{
	std::string filename;
	int frame_count;
	int bone_count;
	float start_speed = 0.0f;
	float end_speed = 0.0f;
	float framerate = 0.0f;
	bool operator <(const accinfo& other) const
	{
		return filename < other.filename;
	}
};

static double calc_speed(const SEAnim_BoneData_t& root_bone, int start_frame, int stop_frame, float framerate)
{
	double distance = calc_distance(root_bone.loc[stop_frame].loc, root_bone.loc[start_frame].loc);
	return distance / (root_bone.loc[stop_frame].frame - root_bone.loc[start_frame].frame) * framerate;
}

void matching(const char* dir)
{
	std::vector<accinfo> accinfos;
	for (const fs::directory_entry& dir_entry :
		fs::recursive_directory_iterator(dir))
	{
		if (dir_entry.is_regular_file())
		{
			SEAnim_File_t seanim;
			memset(&seanim, 0, sizeof(SEAnim_File_t));
			FILE* f;
			const std::string filename = dir_entry.path().string();
			fopen_s(&f, filename.c_str(), "rb");
			if (!f)
			{
				continue;
			}

			const int r = LoadSEAnim(&seanim, f);
			fclose(f);
			if (r != 0)
			{
				continue;
			}

			if (seanim.header.animType != SEANIM_TYPE_RELATIVE)
			{
				continue;
			}

			if (seanim.header.frameCount <= 1)
			{
				continue;
			}

			float start_speed = 0.0f;
			float end_speed = 0.0f;
			const int root_bone_index = find_bone(seanim, "tag_origin");
			if (root_bone_index >= 0)
			{
				const SEAnim_BoneData_t& root_bone = seanim.boneData[root_bone_index];

				if (root_bone.locKeyCount >= 3)
				{
					start_speed = calc_speed(root_bone, 0, 2, seanim.header.framerate);
					end_speed = calc_speed(root_bone, root_bone.locKeyCount - 3, root_bone.locKeyCount - 1, seanim.header.framerate);
				}
			}

			accinfos.push_back(accinfo{ filename, (int)seanim.header.frameCount, (int)seanim.header.boneCount, start_speed, end_speed, seanim.header.framerate });
		}
	}
	std::sort(accinfos.begin(), accinfos.end());
	printf("filename\tbone_count\tframe_count\tstart_speed\tend_speed\tframerate\n");
	for (size_t i = 0; i < accinfos.size(); i++)
	{
		const accinfo& info = accinfos[i];
		printf("%s\t%d\t%d\t%f\t%f\t%f\n", info.filename.c_str(), info.bone_count, info.frame_count, info.start_speed, info.end_speed, info.framerate);
	}
}

struct PxQuat
{

	double x, y, z, w;
};


struct tranform
{
	vec3_t pos;
	quat_t rot;

	static const tranform identity;
};


void mul(quat_t& p, const quat_t& q)
{
	const float tx = w * q.x + q.w * x + y * q.z - q.y * z;
	const float ty = w * q.y + q.w * y + z * q.x - q.z * x;
	const float tz = w * q.z + q.w * z + x * q.y - q.x * y;

	w = w * q.w - q.x * x - y * q.y - q.z * z;
	x = tx;
	y = ty;
	z = tz;
}

const vec3_t rotate(const quat_t& quat, const vec3_t& v)
{
	const float vx = 2.0f * v.x;
	const float vy = 2.0f * v.y;
	const float vz = 2.0f * v.z;
	const float w2 = w * w - 0.5f;
	const float dot2 = (x * vx + y * vy + z * vz);
	return PxVec3((vx * w2 + (y * vz - z * vy) * w + x * dot2), (vy * w2 + (z * vx - x * vz) * w + y * dot2),
		(vz * w2 + (x * vy - y * vx) * w + z * dot2));
}

const tranform tranform::identity = { {0, 0, 0}, {0, 0, 0, 1} };


struct bone
{
	const char* bone_name;
	int parent_index;
};

void print_pose_info(const char* dir)
{
	std::vector<accinfo> accinfos;
	for (const fs::directory_entry& dir_entry :
		fs::recursive_directory_iterator(dir))
	{
		if (dir_entry.is_regular_file())
		{
			SEAnim_File_t seanim;
			memset(&seanim, 0, sizeof(SEAnim_File_t));
			FILE* f;
			const std::string filename = dir_entry.path().string();
			fopen_s(&f, filename.c_str(), "rb");
			if (!f)
			{
				continue;
			}

			const int r = LoadSEAnim(&seanim, f);
			fclose(f);
			if (r != 0)
			{
				continue;
			}

			if (seanim.header.animType != SEANIM_TYPE_RELATIVE)
			{
				continue;
			}

			if (seanim.header.frameCount <= 1)
			{
				continue;
			}
			/*
			* tag_origin
			*		j_mainroot
			*			|j_spinelower
			*				j_spineupper
			*					j_spine4
			*						j_neck
			*							j_head
			*						j_clavicle_le
			*							j_shoulder_le
			*								j_elbow_le
			*									j_wrist_le
			*						j_calvicle_ri
			*							j_shoulder_ri
			*								j_elbow_ri
			*									j_wrist_ri
			*			|j_hip_le
			*				j_knee_le
			*					j_ankle_le
			*						j_ball_le
			*			|j_hip_ri
			*				j_knee_ri
			*					j_ankle_ri
			*						j_ball_ri
			*/

			static const bone bones[] = {
				{"tag_origin", -1},		// 0
				{"j_mainroot", 0},		// 1
				{"j_spinelower", 1},	// 2
				{"j_spineupper", 2},	// 3
				{"j_spine4", 3},		// 4
				{"j_neck", 4},			// 5
				{"j_head", 5},			// 6
				{"j_clavicle_le", 4},	// 7
				{"j_shoulder_le", 7},	// 8
				{"j_elbow_le", 8},		// 9
				{"j_wrist_le", 9},		// 10
				{"j_calvicle_ri", 4},	// 11
				{"j_shoulder_ri", 11},	// 12
				{"j_elbow_ri", 12},		// 13
				{"j_wrist_ri", 13},		// 14
				{"j_hip_le",1},			// 15
				{"j_knee_le", 15},		// 16
				{"j_ankle_le", 16},		// 17
				{"j_ball_le", 17},		// 18
				{"j_hip_ri", 1},		// 19
				{"j_knee_ri", 19},		// 20
				{"j_ankle_ri", 20},		// 21
				{"j_ball_ri", 21}		// 22
			};

			int bone_indices[_countof(bones)];
			for (int i = 0; i < _countof(bones); i++)
			{
				bone_indices[i] = find_bone(seanim, bones[i].bone_name);
			}

			static tranform bone_transforms[_countof(bones)][1000];
			for (int i = 0; i < _countof(bones); i++)
			{
				if (bone_indices[i] >= 0)
				{
					const SEAnim_BoneData_t& root_bone = seanim.boneData[bone_indices[i]];
				}
				else
				{
					for (int j = 0; j < seanim.header.frameCount; j++)
					{
						bone_transforms[i][j] = tranform::identity;
					}
				}
			}
			
			float start_speed = 0.0f;
			float end_speed = 0.0f;

			if (root_bone_index >= 0)
			{
				const SEAnim_BoneData_t& root_bone = seanim.boneData[root_bone_index];

				if (root_bone.locKeyCount >= 3)
				{
					start_speed = calc_speed(root_bone, 0, 2, seanim.header.framerate);
					end_speed = calc_speed(root_bone, root_bone.locKeyCount - 3, root_bone.locKeyCount - 1, seanim.header.framerate);
				}
			}

			accinfos.push_back(accinfo{ filename, (int)seanim.header.frameCount, (int)seanim.header.boneCount, start_speed, end_speed, seanim.header.framerate });
		}
	}
	std::sort(accinfos.begin(), accinfos.end());
	printf("filename\tbone_count\tframe_count\tstart_speed\tend_speed\tframerate\n");
	for (size_t i = 0; i < accinfos.size(); i++)
	{
		const accinfo& info = accinfos[i];
		printf("%s\t%d\t%d\t%f\t%f\t%f\n", info.filename.c_str(), info.bone_count, info.frame_count, info.start_speed, info.end_speed, info.framerate);
	}
}

int main(int argc, char *argv[])
{
	//if (argc < 2)
	//{
	//	printf("Drag and drop a SEAnim on the exe to print info\n");
	//	return 0;
	//}

	matching("G:/CODResources/Animations/seanims");
	return 0;

	SEAnim_File_t seanim;
	memset(&seanim, 0, sizeof(SEAnim_File_t));
	FILE* f;
	fopen_s(&f, argv[1], "rb");
	startClock = clock();
	int r = LoadSEAnim(&seanim, f);
	const auto& root_bone = seanim.boneData[25];

	for (int i = 0; i < root_bone.locKeyCount; i++)
	{
		printf("%d,", root_bone.loc[i].frame);
	}
	for (int i = 0; i < root_bone.locKeyCount; i++)
	{
		printf("%f,", root_bone.loc[i].loc[0]);
	}
	endClock = clock();
	fclose(f);
	if (r != 0)
	{
		printf("Error: %d\n", r);
		return 1;
	}

	float operationTime = (endClock - startClock) / (float)CLOCKS_PER_SEC;
	printf("SEAnim processed in %f seconds\n", operationTime);
	printf("----------------------\n");
	printf("Header size: %u\n", seanim.header.headerSize);
	printf("Anim Type: %u\n", seanim.header.animType);
	printf("Anim Flags: %u\n", seanim.header.animFlags);
	printf("Data Presence Flags: %u\n", seanim.header.dataPresenceFlags);
	printf("Data Property Flags: %u\n", seanim.header.dataPropertyFlags);
	printf("Frame Rate: %f\n", seanim.header.framerate);
	printf("Frame Count: %u\n", seanim.header.frameCount);
	printf("Bone Count: %u\n", seanim.header.boneCount);
	printf("Bone Anim Modifier Count: %u\n", seanim.header.boneAnimModifierCount);
	printf("Note Count: %u\n", seanim.header.noteCount);
	//printf("----------------------\n");
	for (unsigned int i = 0; i < seanim.header.boneCount; i++)
	{
		//printf("Bone %u: %s %s\n", i, seanim.bone[i], seanim.boneData[i].flags & SEANIM_BONE_COSMETIC ? "(cosmetic)" : "");
	}
	//printf("----------------------\n");
	for (unsigned int i = 0; i < seanim.header.noteCount; i++)
	{
		//printf("Note %u: %s\n", i, seanim.notes[i].name);
	}

	startClock = clock();
	FILE *fs;
	fopen_s(&fs, "test_copy.seanim", "wb");
	static char *customdata = "Saved with SEAnimInfo";
	seanim.header.dataPresenceFlags |= SEANIM_PRESENCE_CUSTOM;
	seanim.customDataBlockBuf = new uint8_t[23];
	memcpy(seanim.customDataBlockBuf, customdata, 23);
	seanim.customDataBlockSize = 23;
	SaveSEAnim(&seanim, fs);
	fclose(fs);
	endClock = clock();
	operationTime = (endClock - startClock) / (float)CLOCKS_PER_SEC;
	printf("SEAnim saved in %f seconds\n", operationTime);
	FreeSEAnim(&seanim);
    return 0;
}

