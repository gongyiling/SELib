// SEAnimInfo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <time.h>

time_t startClock = 0;
time_t endClock = 0;


int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Drag and drop a SEAnim on the exe to print info\n");
		return 0;
	}
	startClock = clock();
	SEAnim_File_t seanim;
	memset(&seanim, 0, sizeof(SEAnim_File_t));
	FILE* f;
	fopen_s(&f, argv[1], "rb");
	int r = LoadSEAnim(&seanim, f);
	fclose(f);
	if (r != 0)
	{
		printf("Error: %d\n", r);
		return 1;
	}
	endClock = clock();
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
	printf("----------------------\n");
	for (unsigned int i = 0; i < seanim.header.boneCount; i++)
	{
		printf("Bone %u: %s %s\n", i, seanim.bone[i], seanim.boneData[i].flags & SEANIM_BONE_COSMETIC ? "(cosmetic)" : "");
	}
	printf("----------------------\n");
	for (unsigned int i = 0; i < seanim.header.noteCount; i++)
	{
		printf("Note %u: %s\n", i, seanim.notes[i].name);
	}
	FreeSEAnim(&seanim);
    return 0;
}

