// SEAnimInfo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <time.h>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include "PxTransform.h"

using namespace physx;

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

struct bone
{
	const char* bone_name;
	int parent_index;
	PxTransform ref_transform;
};

PxQuat mq(float pitch, float yaw, float roll)
{
	return from_rotator(pitch, yaw, roll);
}

template< class T, class U >
static T Lerp(const U& Alpha, const T& A, const T& B)
{
	return (T)(A + Alpha * (B - A));
}

static void lerp_transform(PxTransform* transforms, int frame_count, int start_frame, int end_frame, bool lerp_loc)
{
	if (start_frame == -1 && frame_count == end_frame)
	{
		return;
	}
	if (start_frame == -1)
	{
		for (int i = 0; i < end_frame; i++)
		{
			if (lerp_loc)
			{
				transforms[i].p = transforms[end_frame].p;
			}
			else
			{
				transforms[i].q = transforms[end_frame].q;
			}
		}
	}
	else if (end_frame == frame_count)
	{
		for (int i = start_frame + 1; i < end_frame; i++)
		{
			if (lerp_loc)
			{
				transforms[i].p = transforms[start_frame].p;
			}
			else
			{
				transforms[i].q = transforms[start_frame].q;
			}
		}
	}
	else
	{
		float inv = 1.0f / (end_frame - start_frame);
		for (int i = start_frame + 1; i < end_frame; i++)
		{
			float t = i * inv;
			if (lerp_loc)
			{
				transforms[i].p = Lerp(t, transforms[start_frame].p, transforms[end_frame].p);
			}
			else
			{
				// TODO using slerp.
				transforms[i].q = slerp(t, transforms[start_frame].q, transforms[end_frame].q);
			}
		}
	}
}

void print_vec3(const PxVec3& v)
{
	printf("%f,%f,%f\n", v.x, v.y, v.z);
}

void print_pose_info(const char* filename)
{
	SEAnim_File_t seanim;
	memset(&seanim, 0, sizeof(SEAnim_File_t));
	FILE* f;
	fopen_s(&f, filename, "rb");
	if (!f)
	{
		return;
	}

	const int r = LoadSEAnim(&seanim, f);
	fclose(f);
	if (r != 0)
	{
		return;
	}

	if (seanim.header.animType != SEANIM_TYPE_RELATIVE)
	{
		return;
	}

	if (seanim.header.frameCount <= 1)
	{
		return;
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
		{"tag_origin", -1, PxTransform(PxIdentity)},		// 0
		{"j_mainroot", 0, PxTransform(PxVec3(-2.565400, -0.000000, 97.028000), mq(90, 0, 90))},		// 1
		{"j_spinelower", 1, PxTransform(PxVec3(8.973823, 2.565400,-0.000165), mq(0, 0, 0))},	// 2
		{"j_spineupper", 2, PxTransform(PxVec3(15.875000, 0,0), mq(0, 0, 0))},	// 3
		{"j_spine4", 3, PxTransform(PxVec3(11.508736, 0, 0), mq(0, 0, 0))},		// 4
		{"j_neck", 4, PxTransform(PxVec3(22.409206, -2.574906, 0.075798), mq(0.185064,16.826536,0.056054))},			// 5
		{"j_head", 5, PxTransform(PxIdentity)},			// 6
		{"j_clavicle_le", 4, PxTransform(PxIdentity)},	// 7
		{"j_shoulder_le", 7, PxTransform(PxIdentity)},	// 8
		{"j_elbow_le", 8, PxTransform(PxIdentity)},		// 9
		{"j_wrist_le", 9, PxTransform(PxIdentity)},		// 10
		{"j_calvicle_ri", 4, PxTransform(PxIdentity)},	// 11
		{"j_shoulder_ri", 11, PxTransform(PxIdentity)},	// 12
		{"j_elbow_ri", 12, PxTransform(PxIdentity)},		// 13
		{"j_wrist_ri", 13, PxTransform(PxIdentity)},		// 14
		{"j_hip_le",1, PxTransform(PxIdentity)},			// 15
		{"j_knee_le", 15, PxTransform(PxIdentity)},		// 16
		{"j_ankle_le", 16, PxTransform(PxIdentity)},		// 17
		{"j_ball_le", 17, PxTransform(PxIdentity)},		// 18
		{"j_hip_ri", 1, PxTransform(PxIdentity)},		// 19
		{"j_knee_ri", 19, PxTransform(PxIdentity)},		// 20
		{"j_ankle_ri", 20, PxTransform(PxIdentity)},		// 21
		{"j_ball_ri", 21, PxTransform(PxIdentity)}		// 22
	};

	static const int MAX_FRAME_COUNT = 1000;

	static PxTransform bone_transforms[_countof(bones)][MAX_FRAME_COUNT];
	for (int i = 0; i < _countof(bones); i++)
	{
		PxTransform* bone_transform = bone_transforms[i];
		for (int j = 0; j < seanim.header.frameCount; j++)
		{
			bone_transform[j] = PxTransform(PxIdentity);
		}
		int bone_index = find_bone(seanim, bones[i].bone_name);
		if (bone_index >= 0)
		{
			const SEAnim_BoneData_t& bone_data = seanim.boneData[bone_index];
			int last_frame = -1;
			for (int j = 0; j < bone_data.locKeyCount; j++)
			{
				const vec3_t& p = bone_data.loc[j].loc;
				int frame = bone_data.loc[j].frame;
				bone_transform[frame].p = PxVec3(p[0], p[1], p[2]);
				if (last_frame + 1 != frame)
				{
					lerp_transform(bone_transform, seanim.header.frameCount, last_frame, frame, true);
				}
				last_frame = frame;
			}
			if (last_frame + 1 != seanim.header.frameCount)
			{
				lerp_transform(bone_transform, seanim.header.frameCount, last_frame, seanim.header.frameCount, true);
			}

			last_frame = -1;
			for (int j = 0; j < bone_data.rotKeyCount; j++)
			{
				const quat_t& q = bone_data.quats[j].rot;
				int frame = bone_data.quats[j].frame;
				bone_transform[frame].q = PxQuat(q[0], q[1], q[2], q[3]);
				if (last_frame + 1 != frame)
				{
					lerp_transform(bone_transform, seanim.header.frameCount, last_frame, frame, false);
				}
				last_frame = frame;
			}
			if (last_frame + 1 != seanim.header.frameCount)
			{
				lerp_transform(bone_transform, seanim.header.frameCount, last_frame, seanim.header.frameCount, false);
			}

			// https://github.com/dtzxporter/SETools/blob/2714356ff4a8dddbf80f9e76a3ad5b9e181554ba/SEToolsPlugin.py#L1352
			const PxTransform& ref_transform = bones[i].ref_transform;
			printf("bone_name:%s\n",  bones[i].bone_name);
			print_vec3(to_rotator(ref_transform.q));
			for (int j = 0; j < seanim.header.frameCount; j++)
			{
				bone_transform[j].p = ref_transform.p + bone_transform[j].p;
				print_vec3(to_rotator(bone_transform[j].q));
				bone_transform[j].q = ref_transform.q * bone_transform[j].q;
				print_vec3(to_rotator(bone_transform[j].q));
			}
			int parent_index = bones[i].parent_index;
			if (parent_index >= 0)
			{
				PxTransform* parent_bone_transform = bone_transforms[parent_index];
				for (int j = 0; j < seanim.header.frameCount; j++)
				{
					bone_transform[j] = parent_bone_transform[j] * bone_transform[j];
				}
			}
		}
	}

	static const int features_bone_index[] =
	{
		6,	// j_head
		10,	// j_wrist_le
		14,	// j_wrist_ri
		18,	// j_ball_le
		22	// j_ball_ri
	};

	for (int i = 0; i < seanim.header.frameCount; i++)
	{
		printf("%s\t", filename);
		for (int j = 0; j < _countof(features_bone_index); j++)
		{
			const PxTransform& transform = bone_transforms[features_bone_index[j]][i];
			printf("%.2f,%.2f,%.2f\t", transform.p.x, transform.p.y, transform.p.z);
		}
		printf("\n");
	}
}

void print_all_pose_info(const char* dir)
{
	for (const fs::directory_entry& dir_entry :
		fs::recursive_directory_iterator(dir))
	{
		if (dir_entry.is_regular_file())
		{
			const std::string filename = dir_entry.path().string();
			print_pose_info(filename.c_str());
		}
	}
}

int main(int argc, char *argv[])
{
	//if (argc < 2)
	//{
	//	printf("Drag and drop a SEAnim on the exe to print info\n");
	//	return 0;
	//}

	print_pose_info("G:/CODResources/sdr/com/strafe/jog/8.seanim");
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

