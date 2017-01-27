#define _CRT_SECURE_NO_WARNINGS
#ifdef LIVE_DEV
#include "twinstick.h"
#include <fonts.c>
#include <stb_sprintf.h>
#endif
global_variable b32 GlobalPause;
global_variable u64 GlobalFramesToRegainControl;

internal void
BounceScreenEdges(player *Player, v2 ImgSize, f32 BounceFactor)
{
	aabb_collision_sides EdgeCollision = RectInRectEdgeCheck(Player->P, Player->Radius, Player->Radius, ImgSize);
	if(EdgeCollision.L)
	{
		Player->P.X = Player->Radius;
		Player->dP.X *= BounceFactor;
	}
	if(EdgeCollision.R)
	{
		Player->P.X = ImgSize.X - Player->Radius;
		Player->dP.X *= BounceFactor;
	}
	if(EdgeCollision.B)
	{
		Player->P.Y = Player->Radius;
		Player->dP.Y *= BounceFactor;
	}
	if(EdgeCollision.T)
	{
		Player->P.Y = ImgSize.Y - Player->Radius;
		Player->dP.Y *= BounceFactor;
	}
}

internal void
PropelPlayerFromP(player *P1, v2 FromP, f32 Speed)
{
	v2 Diff = V2Sub(P1->P, FromP);
	v2 Normalized = Norm(Diff);
	P1->dP = V2Add(P1->dP, V2Mult(Speed, Normalized));
}

internal void
BouncePlayers(player *P1, player *P2, f32 BounceFactor)
{
	f32 Distance = Dist(P1->P, P2->P);
	f32 Separation = Distance - (P1->Radius + P2->Radius);
	if(Separation < 0)
	{
		v2 Diff1 = V2Sub(P1->P, P2->P);
		v2 Diff2 = V2Sub(P2->P, P1->P);
		v2 Normal1 = Norm(Diff1);
		v2 Normal2 = Norm(Diff2);
		P1->P = V2Add(P1->P, V2Mult(-0.5f*Separation, Normal1));
		P2->P = V2Add(P2->P, V2Mult(-0.5f*Separation, Normal2));
		P1->dP = V2Add(V2Mult(BounceFactor, Normal1), P1->dP);
		P2->dP = V2Add(V2Mult(BounceFactor, Normal2), P2->dP);
	}
}

internal void
BounceObstacle(player *Obstacle, player *P1, player *P2, f32 BounceFactor)
{
	for(uint i = 0; i < 2; ++i)
	{
		player *Player = i ? P2 : P1;
		f32 Distance = Dist(Obstacle->P, Player->P);
		f32 Separation = Distance - (Obstacle->Radius + Player->Radius);
		if(Separation < 0)
		{
			v2 Diff = V2Sub(Player->P, Obstacle->P);
			v2 Normal = Norm(Diff);
			Player->P = V2Add(Player->P, V2Mult(-Separation, Normal));
			Player->dP = V2Add(V2Mult(BounceFactor, Normal), Player->dP);
		}
	}
}


internal void
FireBulletsWhenTriggered(player *P1, controller *C1, f32 BulletSpeed)
{
	if(Tapped(C1->Button.RB) && (P1->AimDir.X || P1->AimDir.Y))
	{
		for(int BI = 0; BI < NUM_BULLETS; ++BI)
		{
			if(P1->BulletState[BI] == BS_Free)
			{
				P1->BulletState[BI] = BS_Firing;
				P1->BulletP[BI] = V2Add(P1->P, V2Mult(P1->Radius, P1->AimDir));
				P1->BulletdP[BI] = V2Add(P1->dP, V2Mult(BulletSpeed, P1->AimDir));
				break;
			}
		}
	}
}

internal b32
UpdateBulletStateAndCollision(player *Shooter, player *Target, player *Obstacle, b32 ShowObs, v2 OutOfBounds, f32 BulletPower)
{
	b32 Collided = 0;
	for(int BI = 0; BI < NUM_BULLETS; ++BI)
	{
		if(Shooter->BulletState[BI] != BS_Free)
		{
			if(Shooter->BulletState[BI] == BS_Hit)
			{
				Shooter->BulletState[BI] = BS_Free;
				continue;
			}
			else if(Shooter->BulletState[BI] == BS_Firing)
			{
				Shooter->BulletP[BI] = V2Add(Shooter->BulletP[BI], Shooter->BulletdP[BI]);
				if(ShowObs && (DistSq(Shooter->BulletP[BI], Obstacle->P) < (Target->Radius * Target->Radius)))
				{
					Shooter->BulletState[BI] = BS_Hit;
					Collided = 1;
				}
				else if(DistSq(Shooter->BulletP[BI], Target->P) < (Target->Radius * Target->Radius)) 
				{
					Shooter->BulletState[BI] = BS_Hit;
					PropelPlayerFromP(Target, Shooter->BulletP[BI], Len(Shooter->BulletdP[BI])*BulletPower);
					Target->Col = WHITE;
					Collided = 1;
				}
				// NOTE: negative so that bullet has to be fully outside screen
				else if(RectInRectCollisionCheck(Shooter->BulletP[BI],
							-Shooter->BulletRadius, -Shooter->BulletRadius, OutOfBounds))
				{
					Shooter->BulletState[BI] = BS_Free;
				}
			}
		}
	}
	return Collided;
}

internal void
DrawBullets(image_buffer *ScreenBuffer, player P1)
{
	for(int BI = 0; BI < NUM_BULLETS; ++BI)
	{
		if(P1.BulletState[BI] == BS_Firing)
		{
			DrawCircleFilled(ScreenBuffer, P1.BulletP[BI], P1.BulletRadius, P1.Col);
		}
		else if(P1.BulletState[BI] == BS_Hit)
		{
			DrawCircleFilled(ScreenBuffer, P1.BulletP[BI], 2.f * P1.BulletRadius, WHITE);
		}
	}
}

internal inline u32
NumBulletsFree(player P1)
{
	u32 NumBullets = 0;
	for(int BI = 0; BI < NUM_BULLETS; ++BI)
	{
		if(P1.BulletState[BI] == BS_Free) ++NumBullets;
	}
	return NumBullets;
}

internal void
DrawBulletStatus(image_buffer *Buffer, player P1)
{
	u32 NumSymbols = NumBulletsFree(P1);
	if(NumSymbols)
	{
		f32 SymbolWidth = 10.f;
		f32 SpaceWidth = 0.2f * SymbolWidth;
		u32 NumSpaces = NumSymbols - 1;
		f32 TotalWidth = ((f32)NumSymbols * SymbolWidth) + ((f32)NumSpaces * SpaceWidth);
		v2 StartP = P1.P;
		StartP.X -= 0.5f*TotalWidth;
		StartP.Y -= SymbolWidth;

		for(u32 i = 0; i < NumSymbols; ++i)
		{
			v2 P = StartP;
			P.X += (f32)(i)*SymbolWidth + (f32)(i)*SpaceWidth; 
			DrawRectDimsFilled(Buffer, P, SymbolWidth, SymbolWidth, WHITE);
		}
	}
}

internal void
ResetPlayerPositions(player *P1, player *P2, player *Ob, v2 Centre, f32 Distance)
{
	P1->P = Centre;
	P1->P.X -= (Distance + 1.f);
	P1->dP.Y = 0.f;
	P1->dP.X = -BNC_Player;
	P1->ddP = ZeroV2;

	P2->P = Centre;
	P2->P.X += (Distance + 1.f);
	P2->dP.Y = 0.f;
	P2->dP.X = BNC_Player;
	P2->ddP = ZeroV2;

	Ob->P = Centre;
	Ob->P.Y += (5.f*Distance);
	Ob->dP.Y = 0.f;
	Ob->ddP.X = 0.f;
	Ob->F = 0;

	GlobalFramesToRegainControl = 20;
}

internal void
ResetGame(memory *Memory)
{
	Memory->IsInitialized = 0;
}

#define SCORE_WHEN_OUT(Outer, Scorer)\
	if(DistSq(ScreenCentre, Outer->P) > (ArenaRadius * ArenaRadius))\
	{\
		DrawRectangleFilled(ScreenBuffer, Origin, ImgSize, WHITE);\
		ResetPlayerPositions(Outer, Scorer, Obstacle, ScreenCentre, State->InitialPlayerRadius);\
		++Scorer->Score;\
	}

UPDATE_AND_RENDER(UpdateAndRender)
{
	state *State = (state *)Memory->PermanentStorage;
	v2 Origin = {0, 0};
	v2 ImgSize = {(f32)ScreenBuffer->Width, (f32)ScreenBuffer->Height};
	v2 ScreenCentre = V2Mult(0.5f, ImgSize);

	player *P1 = &State->Player[0];
	player *P2 = &State->Player[1];
	player *Obstacle = &State->Player[2];
	controller *C1 = &Input.New->Controllers[1];
	controller *C2 = &Input.New->Controllers[2];
		State->InitialPlayerRadius = 30.f;
		State->InitialBulletRadius = 10.f;
		State->BulletSpeed = 18.f;

	if(!Memory->IsInitialized)
	{
		GlobalPause = 1;
		State->FrameCount = 0;
		*P1 = ZeroPlayer;
		*P2 = ZeroPlayer;
		P1->Radius = State->InitialPlayerRadius;
		P2->Radius = State->InitialPlayerRadius;
		Obstacle->Radius = State->InitialPlayerRadius;
		P1->BulletRadius = State->InitialBulletRadius;
		P2->BulletRadius = State->InitialBulletRadius;

		State->ShowObstacle = 0;

		ResetPlayerPositions(P1, P2, Obstacle, ScreenCentre, State->InitialPlayerRadius);

		/////////////////////////////
		Memory->IsInitialized = 1;
	}
	P1->Col = BLUE;
	P1->LineCol = LIGHT_BLUE;
	P2->Col = RED;
	P2->LineCol = LIGHT_RED;
	Obstacle->Col = GREY;
	Obstacle->LineCol = LIGHT_GREY;
	/* u64 F = State->FrameCount; */

	f32 ArenaRadius = ScreenCentre.Y - State->InitialPlayerRadius;

	if(Tapped(C1->Button.Start) || Tapped(C2->Button.Start))
	{
		GlobalPause = !GlobalPause;
	}

	// Move Players
	if(!GlobalPause)
	{
		if(GlobalFramesToRegainControl) --GlobalFramesToRegainControl;
		if(!GlobalFramesToRegainControl)
		{
			P1->ddP = V2(C1->LStickAverageX, C1->LStickAverageY);
			P2->ddP = V2(C2->LStickAverageX, C2->LStickAverageY);
		}

		f32 SinPeriod = 25.f;
		++Obstacle->F;
		f32 Friction = 0.8f;
		f32 AccelFactor = 2.4f;
		P1->dP = V2Mult(Friction, P1->dP);
		P1->dP = V2Add(P1->dP, V2Mult(AccelFactor, P1->ddP));
		P1->P = V2Add(P1->P, P1->dP);
		P2->dP = V2Mult(Friction, P2->dP);
		P2->dP = V2Add(P2->dP, V2Mult(AccelFactor, P2->ddP));
		P2->P = V2Add(P2->P, P2->dP);
		v2 pObstacleP = Obstacle->P;
		Obstacle->P.Y = ScreenCentre.Y +
			(ArenaRadius - 2.5f * Obstacle->Radius) * Cos(((f32)Obstacle->F)/SinPeriod);
		Obstacle->dP = V2Sub(Obstacle->P, pObstacleP);

	BounceScreenEdges(P1, ImgSize, BNC_Wall);
	BounceScreenEdges(P2, ImgSize, BNC_Wall);
	BouncePlayers(P1, P2, BNC_Player);
	if(State->ShowObstacle) BounceObstacle(Obstacle, P1, P2, 30.f);

	// Player Aiming
	// NOTE: Sticks are from -1 to 1 - can give less powerful shot
	if(C1->RStickAverageX || C1->RStickAverageY) P1->AimDir = V2(C1->RStickAverageX, C1->RStickAverageY);
	else P1->AimDir = V2(C1->LStickAverageX, C1->LStickAverageY);
	if(C2->RStickAverageX || C2->RStickAverageY) P2->AimDir = V2(C2->RStickAverageX, C2->RStickAverageY);
	else P2->AimDir = V2(C2->LStickAverageX, C2->LStickAverageY);

	if(!GlobalFramesToRegainControl)
	{	// Fire bullet
		// NOTE: takes a slot if one is free, otherwise does nothing
		FireBulletsWhenTriggered(P1, C1, State->BulletSpeed);
	}FireBulletsWhenTriggered(P2, C2, State->BulletSpeed);

	// Update bullet position
	f32 BulletPower = 1.3f;
	UpdateBulletStateAndCollision(P1, P2, Obstacle, State->ShowObstacle, ImgSize, BulletPower);
	UpdateBulletStateAndCollision(P2, P1, Obstacle, State->ShowObstacle, ImgSize, BulletPower);
	}

	// Draw background ///////////////////////
	f32 LinesFrac = 0.5f;
	f32 CentreFrac = 0.15f;
	f32 StrokeWidth = 2.f;
	DrawRectangleFilled(ScreenBuffer, Origin, ImgSize, LIGHT_GREY);
	DrawCircleFilled(ScreenBuffer, ScreenCentre, ArenaRadius, LIGHT_YELLOW);
	DrawCircleFilled(ScreenBuffer, ScreenCentre, CentreFrac*ArenaRadius, LIGHT_GREY);
	DrawCircleLine(ScreenBuffer, ScreenCentre, ArenaRadius, GREY);
	DrawCircleLine(ScreenBuffer, ScreenCentre, ArenaRadius+1.f, GREY);

	DrawRectangleFilled(ScreenBuffer, V2(ScreenCentre.X - LinesFrac*ArenaRadius, ScreenCentre.Y-StrokeWidth),
									  V2(ScreenCentre.X + LinesFrac*ArenaRadius, ScreenCentre.Y+StrokeWidth), LIGHT_GREY);
	DrawRectangleFilled(ScreenBuffer, V2(ScreenCentre.X-StrokeWidth, ScreenCentre.Y - LinesFrac*ArenaRadius),
									  V2(ScreenCentre.X+StrokeWidth, ScreenCentre.Y + LinesFrac*ArenaRadius), LIGHT_GREY);
	DrawRectangleFilled(ScreenBuffer, V2(ScreenCentre.X - CentreFrac*ArenaRadius, ScreenCentre.Y-StrokeWidth),
									  V2(ScreenCentre.X + CentreFrac*ArenaRadius, ScreenCentre.Y+StrokeWidth), LIGHT_YELLOW);
	DrawRectangleFilled(ScreenBuffer, V2(ScreenCentre.X-StrokeWidth, ScreenCentre.Y - CentreFrac*ArenaRadius),
									  V2(ScreenCentre.X+StrokeWidth, ScreenCentre.Y + CentreFrac*ArenaRadius), LIGHT_YELLOW);
	/////////////////////////////////////////////////////////////

	// Draw Crosshair
	f32 CrosshairDistance = State->InitialPlayerRadius * 1.35f;
	DrawCircleFilled(ScreenBuffer, V2Add(P1->P, V2Mult(CrosshairDistance, P1->AimDir)),
			0.3f*State->InitialBulletRadius, P1->Col);
	DrawCircleFilled(ScreenBuffer, V2Add(P2->P, V2Mult(CrosshairDistance, P2->AimDir)),
			0.3f*State->InitialBulletRadius, P2->Col);


	// DrawPlayers
	//Obstacle
	if(State->ShowObstacle)
	{
		DrawCircleFilled(ScreenBuffer, Obstacle->P, Obstacle->Radius, Obstacle->Col);
		DrawCircleLine(ScreenBuffer, Obstacle->P, Obstacle->Radius, Obstacle->LineCol);
	}
	//P1
	DrawCircleFilled(ScreenBuffer, P1->P, P1->Radius, P1->Col);
	DrawCircleLine(ScreenBuffer, P1->P, P1->Radius, P1->LineCol);
	DrawBulletStatus(ScreenBuffer, *P1);
	//P2
	DrawCircleFilled(ScreenBuffer, P2->P, P1->Radius, P2->Col);
	DrawCircleLine(ScreenBuffer, P2->P, P2->Radius, P2->LineCol);
	DrawBulletStatus(ScreenBuffer, *P2);

	// Draw bullets
	DrawBullets(ScreenBuffer, *P1);
	DrawBullets(ScreenBuffer, *P2);

	// Win Condition
	SCORE_WHEN_OUT(P1, P2);
	SCORE_WHEN_OUT(P2, P1);

	// Draw Score
	f32 FontSize = 25.f;
	f32 P1ScorePX = 40.f;
	f32 P2ScorePX = ImgSize.X - 142.f;
	char P1Score[32], P2Score[32];
	stbsp_sprintf(P1Score, "ScoRe  %d", P1->Score);
	DrawString(ScreenBuffer, &State->DefaultFont, P1Score, FontSize, P1ScorePX, 1.5f * ScreenCentre.Y, P1->Col);
	stbsp_sprintf(P2Score, "ScoRe  %d", P2->Score);
	DrawString(ScreenBuffer, &State->DefaultFont, P2Score, FontSize, P2ScorePX, 1.5f * ScreenCentre.Y, P2->Col);

	// Draw Pause Screen when appropriate
	if(GlobalPause)
	{
		b32 BPressed = 0;
		b32 XPressed = 0;
		b32 YPressed = 0;
#define EitherController(Action, BUTTON) (Action(C1->Button.BUTTON) || Action(C2->Button.BUTTON))
		if(EitherController( , B.EndedDown) ) {
			GlobalPause = 0;
			BPressed = 1;
		}
		if(EitherController(Tapped, A)) {
			State->ShowObstacle = !State->ShowObstacle;
			/* Include Trap */
		}
		if(EitherController( , X.EndedDown)) {
			ResetPlayerPositions(P1, P2, Obstacle, ScreenCentre, State->InitialPlayerRadius);
			XPressed = 1;
		}
		if(EitherController( , Y.EndedDown)) {
			ResetGame(Memory);
			YPressed = 1;
	 	}
#undef EitherController

		f32 PauseWidth = 0.65f;
		f32 EdgeWidth = 3.f;
		f32 LeftEdge = ScreenCentre.X - 0.5f*PauseWidth*ImgSize.X;
		f32 RightEdge = ScreenCentre.X + 0.5f*PauseWidth*ImgSize.X;

		DrawRectangleFilled(ScreenBuffer, V2(LeftEdge, 0.f), V2(RightEdge, ImgSize.Y), PreMultiplyColour(GREY, 0.7f));
		DrawRectangleFilled(ScreenBuffer, V2(LeftEdge - EdgeWidth, 0.f), V2(LeftEdge, ImgSize.Y), GREY);
		DrawRectangleFilled(ScreenBuffer, V2(RightEdge, 0.f), V2(RightEdge + EdgeWidth, ImgSize.Y), GREY);
		DrawString(ScreenBuffer, &State->DefaultFont, "TWINSTICK SUMO", 3.f*FontSize, 0.41f*ScreenCentre.X, 0.7f*ImgSize.Y, WHITE);
		DrawString(ScreenBuffer, &State->DefaultFont, "PAUSed", 3.f*FontSize, 0.7f*ScreenCentre.X, -0.07f*ImgSize.Y, WHITE);

		f32 ButtonDist = 0.2f;
		f32 ButtonRad = 25.f;
		f32 DarkSection = 0.7f;

		v2 XPos = V2(ScreenCentre.X - ButtonDist*ArenaRadius, ScreenCentre.Y);
		v2 BPos = V2(ScreenCentre.X + ButtonDist*ArenaRadius, ScreenCentre.Y);
		v2 APos = V2(ScreenCentre.X, ScreenCentre.Y - ButtonDist*ArenaRadius);
		v2 YPos = V2(ScreenCentre.X, ScreenCentre.Y + ButtonDist*ArenaRadius);

		DrawString(ScreenBuffer, &State->DefaultFont, "Reset\nPositions", FontSize, XPos.X - 150.f, XPos.Y - 20.f, WHITE);
		DrawString(ScreenBuffer, &State->DefaultFont, "Resume\nPlaying", FontSize, BPos.X + 45.f, BPos.Y - 20.f, WHITE);
		DrawString(ScreenBuffer, &State->DefaultFont, "Add Obstacle", FontSize, APos.X - 80.f, APos.Y - 90.f, WHITE);
		DrawString(ScreenBuffer, &State->DefaultFont, "Reset Game", FontSize, YPos.X - 65.f, YPos.Y + 20.f, WHITE);

		DrawCircleFilled(ScreenBuffer, XPos, ButtonRad, BLUE);
		DrawCircleFilled(ScreenBuffer, BPos, ButtonRad, RED);
		DrawCircleFilled(ScreenBuffer, APos, ButtonRad, GREEN);
		DrawCircleFilled(ScreenBuffer, YPos, ButtonRad, YELLOW);

		colour XCentreCol = XPressed ? DARK_BLUE : LIGHT_BLUE;
		colour BCentreCol = BPressed ? DARK_RED : LIGHT_RED;
		colour ACentreCol = State->ShowObstacle ? DARK_GREEN : LIGHT_GREEN;
		colour YCentreCol = YPressed ? DARK_YELLOW : LIGHT_YELLOW;

		DrawCircleFilled(ScreenBuffer, XPos, DarkSection*ButtonRad, XCentreCol);
		DrawCircleFilled(ScreenBuffer, BPos, DarkSection*ButtonRad, BCentreCol);
		DrawCircleFilled(ScreenBuffer, APos, DarkSection*ButtonRad, ACentreCol);
		DrawCircleFilled(ScreenBuffer, YPos, DarkSection*ButtonRad, YCentreCol);

		DrawCircleLine(ScreenBuffer, XPos, ButtonRad, DARK_BLUE);
		DrawCircleLine(ScreenBuffer, BPos, ButtonRad, DARK_RED);
		DrawCircleLine(ScreenBuffer, APos, ButtonRad, DARK_GREEN);
		DrawCircleLine(ScreenBuffer, YPos, ButtonRad, DARK_YELLOW);
		DrawCircleLine(ScreenBuffer, XPos, ButtonRad-1.f, DARK_BLUE);
		DrawCircleLine(ScreenBuffer, BPos, ButtonRad-1.f, DARK_RED);
		DrawCircleLine(ScreenBuffer, APos, ButtonRad-1.f, DARK_GREEN);
		DrawCircleLine(ScreenBuffer, YPos, ButtonRad-1.f, DARK_YELLOW);

	}

////////////////////////////////////////////////////////////
	/* DrawCrosshair(ScreenBuffer, State->MouseP, 10.0f, BLUE); */
	/* colour CheckPxCol = GetPixelColour(ScreenBuffer, State->MouseP); */
	
///////////////////////////////////////////////////

	char Message[512];
	stbsp_sprintf(Message, "%.1ffps", 1.f/(State->dt));
			/* "ARGB: (%.2f, %.2f, %.2f, %.2f)", */
			/* CheckPxCol.A, CheckPxCol.R, CheckPxCol.G, CheckPxCol.B); */
	DrawString(ScreenBuffer, &State->DefaultFont, Message, 17, 10, 10, WHITE);
	/* *_Times = Times; */
}
