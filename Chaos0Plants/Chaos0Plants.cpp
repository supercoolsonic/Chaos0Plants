// Chaos0Plants.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <cmath>



//Misc
#include "IniFile.hpp"
#include "ObjModels/Plant_Objects.h"





//Structs
struct ObjectThing
{
	ObjectFuncPtr func;
	int16_t list;
	int16_t field_A;
	Rotation3 Rotation;
	NJS_VECTOR Position;
	NJS_OBJECT* object;
};

//Additional SADX Variables
DataArray(CollisionData, stru_C67750, 0xC67750, 1);
DataArray(CollisionData, stru_C673B8, 0xC673B8, 7);





DataPointer(float, CurrentDrawDistance, 0x03ABDC74);

FunctionPointer(void, InitCollision, (ObjectMaster *obj, CollisionData *collisionArray, int count, unsigned __int8 list), 0x41CAF0);


FunctionPointer(void, SetStatus, (ObjectMaster *a1), 0x0049CD60);
FunctionPointer(void, DynCol_LoadObject, (ObjectMaster *a1), 0x0049E170);
FunctionPointer(void, sub_446AF0, (ObjectMaster *a1, int a2), 0x446AF0);

DataPointer(int, DroppedFrames, 0x03B1117C);

//Additional SADX Functions
FunctionPointer(NJS_OBJECT *, DynamicCollision, (NJS_OBJECT *a1, ObjectMaster *a2, ColFlags surfaceFlags), 0x49D6C0);
FunctionPointer(int, rand1, (), 0x6443BF);
DataPointer(int, FramerateSetting, 0x0389D7DC);



//Null Code (Used for debugging purposes)
void __cdecl NullFunction(ObjectMaster *a1)
{
	return;
}

//Standard Display
void __cdecl Basic_Display(ObjectMaster *a2)
{
	EntityData1 *v1; // esi@1
	Angle v2; // eax@2
	Angle v3; // eax@4
	Angle v4; // eax@6

	v1 = a2->Data1;
	if (!MissedFrames)
	{
		SetTextureToLevelObj();
		njPushMatrix(0);
		njTranslateV(0, &v1->Position);
		v2 = v1->Rotation.z;
		if (v2)
		{
			njRotateZ(0, (unsigned __int16)v2);
		}
		v3 = v1->Rotation.x;
		if (v3)
		{
			njRotateX(0, (unsigned __int16)v3);
		}
		v4 = v1->Rotation.y;
		if (v4)
		{
			njRotateY(0, (unsigned __int16)v4);
		}
		ProcessModelNode_AB_Wrapper(v1->Object, 1.0);
		njPopMatrix(1u);
	}
}

//Standard Main
void __cdecl Basic_Main(ObjectMaster *a1)
{
	EntityData1 *v1; // edi@1

	v1 = a1->Data1;
	if (!ClipSetObject_Min(a1))
	{
		if (!ObjectSelectedDebug(a1))
		{
			AddToCollisionList(v1);
		}
		Basic_Display(a1);
	}
}

//Standard Delete Dynamic
void deleteSub_Global(ObjectMaster *a1) {
	if (a1->Data1->Object)
	{
		DynamicCOL_Remove(a1, a1->Data1->Object);
		ObjectArray_Remove(a1->Data1->Object);
	}
	DeleteObject_(a1);
}

void AddToCollision(ObjectMaster *a1, uint8_t col) {
	/*  0 is static
	1 is moving (refresh the colision every frame)
	2 is static, scalable
	3 is moving, scalable   */

	EntityData1 * original = a1->Data1;
	NJS_OBJECT *colobject;

	colobject = ObjectArray_GetFreeObject(); //The collision is a separate object, we add it to the list of object

	//if scalable
	if (col == 2 || col == 3) {
		colobject->evalflags = NJD_EVAL_BREAK | NJD_EVAL_SKIP | NJD_EVAL_HIDE;
		colobject->scl[0] = original->Scale.x;
		colobject->scl[1] = original->Scale.y;
		colobject->scl[2] = original->Scale.z;
	}
	else if (col == 4) {
		colobject->evalflags = NJD_EVAL_BREAK | NJD_EVAL_SKIP | NJD_EVAL_HIDE;
		colobject->scl[0] = 1.0f + original->Scale.x;
		colobject->scl[1] = 1.0f + original->Scale.x;
		colobject->scl[2] = 1.0f + original->Scale.x;
	}
	else if (col == 5) {
		colobject->evalflags = NJD_EVAL_BREAK | NJD_EVAL_SKIP | NJD_EVAL_HIDE;
		colobject->scl[0] = 1.0f + original->Scale.z;
		colobject->scl[1] = 1.0f + original->Scale.z;
		colobject->scl[2] = 1.0f + original->Scale.z;
	}
	else {
		colobject->evalflags = NJD_EVAL_UNIT_SCL | NJD_EVAL_BREAK | NJD_EVAL_SKIP | NJD_EVAL_HIDE; //ignore scale
		colobject->scl[0] = 1.0;
		colobject->scl[1] = 1.0;
		colobject->scl[2] = 1.0;
	}

	//add the rest
	if (col == 4 || col == 1 || col == 5)
	{
		colobject->ang[0] = 0;
		colobject->ang[1] = original->Rotation.y;
		colobject->ang[2] = 0;
	}
	else {
		colobject->ang[0] = original->Rotation.x;
		colobject->ang[1] = original->Rotation.y;
		colobject->ang[2] = original->Rotation.z;
	}
	colobject->pos[0] = original->Position.x;
	colobject->pos[1] = original->Position.y;
	colobject->pos[2] = original->Position.z;

	colobject->basicdxmodel = a1->Data1->Object->basicdxmodel; //object it will use as a collision
	a1->Data1->Object = colobject; //pointer to the collision object into our original object

	if (col == 0 || col == 2) DynamicCOL_Add((ColFlags)1, a1, colobject); //Solid
	else if (col == 1 || col == 3 || col == 4 || col == 5) DynamicCOL_Add((ColFlags)0x8000000, a1, colobject); //Dynamic, solid
}

//Basic drawing call
void DrawObjModel(ObjectMaster *a1, NJS_MODEL_SADX *m, bool scalable) {
	if (!MissedFrames) {
		njSetTexture((NJS_TEXLIST*)&BEACH01_TEXLIST); //Current heroes level texlist is always onto Emerald Coast
		njPushMatrix(0);
		njTranslateV(0, &a1->Data1->Position);
		njRotateXYZ(nullptr, a1->Data1->Rotation.x, a1->Data1->Rotation.y, a1->Data1->Rotation.z);
		if (scalable) njScale(nullptr, a1->Data1->Scale.x, a1->Data1->Scale.y, a1->Data1->Scale.z);
		else njScale(nullptr, 1, 1, 1);
		DrawQueueDepthBias = -6000.0f;
		DrawModel(m);
		DrawQueueDepthBias = 0;
		njPopMatrix(1u);
	}
}


void __cdecl Load_GRAS0(ObjectMaster *a1)
{
	{
		EntityData1 *v1;

		v1 = a1->Data1;
		v1->Object = &Object_OGRAS0;
		a1->MainSub = Basic_Main;
		a1->DisplaySub = Basic_Display;
		a1->DeleteSub = (void(__cdecl *)(ObjectMaster *))nullsub;

	}
}

void __cdecl Load_GRAS1(ObjectMaster *a1)
{
	{
		EntityData1 *v1;

		v1 = a1->Data1;
		v1->Object = &Object_OGRAS1;
		a1->MainSub = Basic_Main;
		a1->DisplaySub = Basic_Display;
		a1->DeleteSub = (void(__cdecl *)(ObjectMaster *))nullsub;

	}
}

void __cdecl Load_GRAS2(ObjectMaster *a1)
{
	{
		EntityData1 *v1;

		v1 = a1->Data1;
		v1->Object = &Object_OGRAS2;
		a1->MainSub = Basic_Main;
		a1->DisplaySub = Basic_Display;
		a1->DeleteSub = (void(__cdecl *)(ObjectMaster *))nullsub;

	}
}

void __cdecl Load_GRAS3(ObjectMaster *a1)
{
	{
		EntityData1 *v1;

		v1 = a1->Data1;
		v1->Object = &Object_OGRAS3;
		a1->MainSub = Basic_Main;
		a1->DisplaySub = Basic_Display;
		a1->DeleteSub = (void(__cdecl *)(ObjectMaster *))nullsub;

	}
}

ObjectListEntry Chaos0List_list[] = {
	{ 2, 3, 0, 0, 0, Ring_Main, "RING" } /* "RING" */,											//00
	{ 2, 3, 0, 0, 0, Load_GRAS0, "O GRAS0" } /* "Grass0" */,									//01
	{ 2, 3, 0, 0, 0, Load_GRAS1, "O GRAS1" } /* "Grass1" */,									//02
	{ 2, 3, 0, 0, 0, Load_GRAS2, "O GRAS2" } /* "Grass2" */,									//03
	{ 2, 3, 0, 0, 0, Load_GRAS3, "O GRAS3" } /* "Grass3" */,									//04
	{ 2, 6, 0, 0, 0, (ObjectFuncPtr)0x00549A30, "O HELI0" } /* "Helicopter" */,					//05
	{ 7, 3, 0, 0, 0, (ObjectFuncPtr)0x00549770, "O PATO0" } /* "PoliceCar" */,					//06
	{ 2, 3, 0, 0, 0, (ObjectFuncPtr)0x00548E70, "O_POLE" } /* "Pole" */,						//07	
	{ 2, 3, 1, 4000000, 0, (ObjectFuncPtr)0x00548D30, "O BCLOCK" } /* "Clock" */,				//08
};

ObjectList Chaos0List = { arraylengthandptrT(Chaos0List_list, int) };


PointerInfo pointers[] = {
	ptrdecl(0x974CD8, &Chaos0List),
};


void Init(const char *path, const HelperFunctions &helperFunctions)
{
	
}

extern "C"
{
	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer, &Init, NULL, 0, NULL, 0, NULL, 0, arrayptrandlength(pointers) };

	__declspec(dllexport) void cdecl Init()
	{
		
	}

	__declspec(dllexport) void __cdecl OnFrame()
	{
		
	}
}