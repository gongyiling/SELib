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

int find_root_bone(const SEAnim_File_t& seanim)
{
	for (int i = 0; i < seanim.header.boneCount; i++)
	{
		if (strcmp((const char*)seanim.bone[i], "tag_origin") == 0)
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
			const int root_bone_index = find_root_bone(seanim);
			if (root_bone_index < 0)
			{
				continue;
			}
			const SEAnim_BoneData_t& root_bone = seanim.boneData[root_bone_index];
			float start_speed = 0.0f;
			float end_speed = 0.0f;
			if (root_bone.locKeyCount >= 3)
			{
				start_speed = calc_speed(root_bone, 0, 2, seanim.header.framerate);
				end_speed = calc_speed(root_bone, root_bone.locKeyCount - 3, root_bone.locKeyCount - 1, seanim.header.framerate);
			}
			accinfos.push_back(accinfo{ filename, (int)root_bone.locKeyCount, (int)seanim.header.boneCount, start_speed, end_speed });
		}
	}
	std::sort(accinfos.begin(), accinfos.end());
	printf("filename\tbone_count\tframe_count\n");
	for (size_t i = 0; i < accinfos.size(); i++)
	{
		const accinfo& info = accinfos[i];
		printf("%s\t%d\t%d\t%f\t%f\n", info.filename.c_str(), info.bone_count, info.frame_count, info.start_speed, info.end_speed);
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

