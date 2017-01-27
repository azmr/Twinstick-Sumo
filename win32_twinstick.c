#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include "twinstick.h"
#define USING_INPUT
#include <fonts.c>
#include <win32.h>
#include <live_edit.h>
#include <stb_sprintf.h>
/* #include <loop_edit.h> */

global_variable b32 GlobalRunning;
global_variable WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};
#include <win32_gfx.h>
#include <win32_input.h>
#if !defined(LIVE_DEV)
#include "twinstick.c"
#endif

#ifdef LIVE_DEV
typedef UPDATE_AND_RENDER(update_and_render);
#endif

int CALLBACK
WinMain(HINSTANCE Instance,
		HINSTANCE PrevInstance,
		LPSTR CommandLine,
		int ShowCode)
{
	// UNUSED:
	ShowCode; CommandLine; PrevInstance;

	win32_window Window;
	GlobalRunning = 1;
	if(!Win32BasicWindow(Instance, &Window, 960, 540, "Twinstick Sumo"))
	{
		GlobalRunning = 0;
	}

#define MemSize (Megabytes(64))
	memory Memory = {0};
	// TODO: WORK HERE
	Memory.PermanentStorageSize = MemSize;
	Memory.PermanentStorage = VirtualAlloc(0, MemSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	
	if(!Memory.PermanentStorage)
	{
		OutputDebugStringA("Memory not allocated properly");
		GlobalRunning = 0;
	}

	/* win32_loop_info LoopInfo = Win32InitLoop(MemSize); */

	Win32LoadXInput();
#ifdef LIVE_DEV
	char *LibFnNames[] = {"UpdateAndRender"};
	win32_library Lib = Win32Library(LibFnNames, 0, 1,
									 0, "twinstick.dll", "twinstick_temp.dll", "lock.tmp");
#endif

	win32_image_buffer Win32Buffer = {0};
	Win32ResizeDIBSection(&Win32Buffer, 960, 540);

	// ASSETS
	state *State = (state *)Memory.PermanentStorage;
	void *FontBuffer = VirtualAlloc(0, 1<<24, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	INIT_FONT(OldSchool, "OldSchool.ttf", FontBuffer, 1<<24);
	/* INIT_FONT(ArialBD, "c:/windows/fonts/arialbd.ttf", FontBuffer, 1<<25); */
	State->DefaultFont = OldSchool;

	//////////

	win32_frame_timing FrameTimer = Win32InitFrameTimer(Window.TargetSecondsPerFrame);

	input Input = {0};
	input_state Inputs[2] = {0};
	Input.New = &Inputs[0];
	Input.Old = &Inputs[1];

	b32 Fullscreen = 0;
	while(GlobalRunning)
	{
		// TODO: failed on pressing escape
		FrameTimer.Start = Win32GetWallClock();
		/* FrameTimer = Win32StartFrameTimer(FrameTimer); */
		old_new_controller Keyboard = UpdateController(Input, 0);
		/* controller_input Keyboards[2]; */
		/* Keyboard.New = &Keyboards[0]; */
		/* Keyboard.Old = &Keyboards[1]; */

		Win32ProcessPendingMessages(Keyboard.New);
		// TODO: zero controllers
		Win32ProcessXInputControllers(&Input);
		Win32GetWindowDimensionAndOffset(&Window, Win32Buffer.Width, Win32Buffer.Height);
		Win32UpdateMouse(Window.Handle, Input.New, Window.OffsetX, Window.OffsetY, Win32Buffer.Height);
		if(Fullscreen)
		{
			// TODO: Finish fixing
			Input.New->MouseX /= 2;
			Input.New->MouseY /= 2;
			// TODO: this is a hack, fix it properly!
			Input.New->MouseX += 240;
			Input.New->MouseY += 134;
		}

		// NOTE: just uses the first part of the Windows version as the generic version
		image_buffer GameImageBuffer = *(image_buffer *) &Win32Buffer;


#ifdef LIVE_DEV
		Win32ReloadLibOnRecompile(&Lib); 
		update_and_render *UpdateAndRender = ((update_and_render *)Lib.Functions[0].Function);
		if(!UpdateAndRender) { break; }
#endif
		Fullscreen = Win32DisplayBufferInWindow(&Win32Buffer, Window);

		// TODO: decide on struct-contained vs global OS functions
		Memory.FreeFileMemory = Win32FreeFileMemory;
		Memory.ReadEntireFile = Win32ReadEntireFile;
		Memory.WriteEntireFile = Win32WriteEntireFile;

		UpdateAndRender(&GameImageBuffer, &Memory, Input);
		State->dt = FrameTimer.SecondsElapsedForFrame;

		FrameTimer = Win32WaitForFrameEnd(FrameTimer);
		FrameTimer = Win32EndFrameTimer(FrameTimer);
	}
	// TODO:? free timer resolution
}
