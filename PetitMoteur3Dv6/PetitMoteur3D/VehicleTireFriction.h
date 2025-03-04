#pragma once

/*
* Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "PhysX/Include/PxPhysicsAPI.h"

using namespace physx;

//Drivable surface types.
enum
{
	SURFACE_TYPE_TARMAC,
	MAX_NUM_SURFACE_TYPES
};

//Tire types.
enum
{
	TIRE_TYPE_NORMAL = 0,
	TIRE_TYPE_WORN,
	MAX_NUM_TIRE_TYPES
};

PxVehicleDrivableSurfaceToTireFrictionPairs* createFrictionPairs(const PxMaterial* defaultMaterial);