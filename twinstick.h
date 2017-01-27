#ifndef FREEDIVE_H
#include <types.h>
#include <maths.h>
#include <fonts.h>
#include <platform.h>
#include <gfx.h>
#include <memory.h>
#include <input.h>

typedef struct memory
{
	b32 IsInitialized;

	u64 PermanentStorageSize;
	void *PermanentStorage; // NOTE: Required to be cleared to 0 at startup

	u64 TransientStorageSize;
	void *TransientStorage; // NOTE: Required to be cleared to 0 at startup

	debug_platform_free_file_memory *FreeFileMemory;
	debug_platform_read_entire_file *ReadEntireFile;
	debug_platform_write_entire_file *WriteEntireFile;
} memory;

typedef struct player
{
	v2 P;
	v2 dP;
	v2 ddP;
	v2 AimDir;
	colour Col;
	colour LineCol;
	f32 Radius;
	f32 BulletRadius;
	u64 F; // FrameCount
	u8 ControllerNum;
	u8 Score;

#define NUM_BULLETS 3
	u8 BulletState[NUM_BULLETS];
	v2 BulletP[NUM_BULLETS];
	v2 BulletdP[NUM_BULLETS];
} player;
player ZeroPlayer = {0};
typedef enum bullet_states
{
	BS_Free,
	BS_Firing,
	BS_Hit,
	BS_Count,
} bullet_states;

typedef struct state
{
	v2 MouseP;
	f32 dt; // frame time
	font DefaultFont;
	u64 FrameCount;
	player Player[3];
	f32 InitialPlayerRadius;
	f32 InitialBulletRadius;
	f32 HitDist;
	f32 BulletSpeed;
	// Pause Options:
	b32 ShowObstacle;
} state;

#define BNC_Wall (-0.8f)
#define BNC_Player (20.f)

#define UPDATE_AND_RENDER(name) void name(image_buffer *ScreenBuffer, memory *Memory, input Input)
#ifdef TIMING_ON
#define TIME_NOW(n) Times[n] = OS.GetCurrentTime();
#else
#define TIME_NOW(n)
#endif

#define FREEDIVE_H
#endif
