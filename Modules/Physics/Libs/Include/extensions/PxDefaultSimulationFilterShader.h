// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PX_PHYSICS_EXTENSIONS_DEFAULTSIMULATIONFILTERSHADER_H
#define PX_PHYSICS_EXTENSIONS_DEFAULTSIMULATIONFILTERSHADER_H
/** \addtogroup extensions
  @{
*/

#include "PxPhysXConfig.h"

#include "PxFiltering.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

class PxActor;

/**
\brief 64-bit mask used for collision filtering.

The collision filtering equation for 2 objects o0 and o1 is:

<pre> (G0 op0 K0) op2 (G1 op1 K1) == b </pre>

with

<ul>
<li> G0 = PxGroupsMask for object o0. See PxSetGroupsMask </li>
<li> G1 = PxGroupsMask for object o1. See PxSetGroupsMask </li>
<li> K0 = filtering constant 0. See PxSetFilterConstants </li>
<li> K1 = filtering constant 1. See PxSetFilterConstants </li>
<li> b = filtering boolean. See PxSetFilterBool </li>
<li> op0, op1, op2 = filtering operations. See PxSetFilterOps </li>
</ul>

If the filtering equation is true, collision detection is enabled.

@see PxSetFilterOps()
*/
class PxGroupsMask
{
public:
    PX_INLINE PxGroupsMask()
        : bits0(0)
        , bits1(0)
        , bits2(0)
        , bits3(0)
    {
    }
    PX_INLINE ~PxGroupsMask()
    {
    }

    PxU16 bits0, bits1, bits2, bits3;
};

/**
\brief Collision filtering operations.

@see PxGroupsMask
*/
struct PxFilterOp
{
    enum Enum
    {
        PX_FILTEROP_AND,
        PX_FILTEROP_OR,
        PX_FILTEROP_XOR,
        PX_FILTEROP_NAND,
        PX_FILTEROP_NOR,
        PX_FILTEROP_NXOR,
        PX_FILTEROP_SWAP_AND
    };
};

/**
\brief Implementation of a simple filter shader that emulates PhysX 2.8.x filtering

This shader provides the following logic:
\li If one of the two filter objects is a trigger, the pair is acccepted and #PxPairFlag::eTRIGGER_DEFAULT will be used for trigger reports
\li Else, if the filter mask logic (see further below) discards the pair it will be suppressed (#PxFilterFlag::eSUPPRESS)
\li Else, the pair gets accepted and collision response gets enabled (#PxPairFlag::eCONTACT_DEFAULT)

Filter mask logic:
Given the two #PxFilterData structures fd0 and fd1 of two collision objects, the pair passes the filter if the following
conditions are met:

	1) Collision groups of the pair are enabled
	2) Collision filtering equation is satisfied

@see PxSimulationFilterShader
*/

PxFilterFlags PxDefaultSimulationFilterShader(
PxFilterObjectAttributes attributes0,
PxFilterData filterData0,
PxFilterObjectAttributes attributes1,
PxFilterData filterData1,
PxPairFlags& pairFlags,
const void* constantBlock,
PxU32 constantBlockSize);

/**
	\brief Determines if collision detection is performed between a pair of groups

	\note Collision group is an integer between 0 and 31.

	\param[in] group1 First Group
	\param[in] group2 Second Group

	\return True if the groups could collide

	@see PxSetGroupCollisionFlag
*/
bool PxGetGroupCollisionFlag(const PxU16 group1, const PxU16 group2);

/**
	\brief Specifies if collision should be performed by a pair of groups

	\note Collision group is an integer between 0 and 31.

	\param[in] group1 First Group
	\param[in] group2 Second Group
	\param[in] enable True to enable collision between the groups

	@see PxGetGroupCollisionFlag
*/
void PxSetGroupCollisionFlag(const PxU16 group1, const PxU16 group2, const bool enable);

/**
	\brief Retrieves the value set with PxSetGroup()

	\note Collision group is an integer between 0 and 31.

	\param[in] actor The actor

	\return The collision group this actor belongs to

	@see PxSetGroup
*/
PxU16 PxGetGroup(const PxActor& actor);

/**
	\brief Sets which collision group this actor is part of

	\note Collision group is an integer between 0 and 31.

	\param[in] actor The actor
	\param[in] collisionGroup Collision group this actor belongs to

	@see PxGetGroup
*/
void PxSetGroup(PxActor& actor, const PxU16 collisionGroup);

/**
\brief Retrieves filtering operation. See comments for PxGroupsMask

\param[out] op0 First filter operator.
\param[out] op1 Second filter operator.
\param[out] op2 Third filter operator.

@see PxSetFilterOps PxSetFilterBool PxSetFilterConstants
*/
void PxGetFilterOps(PxFilterOp::Enum& op0, PxFilterOp::Enum& op1, PxFilterOp::Enum& op2);

/**
\brief Setups filtering operations. See comments for PxGroupsMask

\param[in] op0 Filter op 0.
\param[in] op1 Filter op 1.
\param[in] op2 Filter op 2.

@see PxSetFilterBool PxSetFilterConstants
*/
void PxSetFilterOps(const PxFilterOp::Enum& op0, const PxFilterOp::Enum& op1, const PxFilterOp::Enum& op2);

/**
\brief Retrieves filtering's boolean value. See comments for PxGroupsMask

\return flag Boolean value for filter.

@see PxSetFilterBool PxSetFilterConstants
*/
bool PxGetFilterBool();

/**
\brief Setups filtering's boolean value. See comments for PxGroupsMask

\param[in] enable Boolean value for filter.

@see PxSetFilterOps PxSsetFilterConstants
*/
void PxSetFilterBool(const bool enable);

/**
\brief Gets filtering constant K0 and K1. See comments for PxGroupsMask

\param[out] c0 the filtering constants, as a mask. See #PxGroupsMask.
\param[out] c1 the filtering constants, as a mask. See #PxGroupsMask.

@see PxSetFilterOps PxSetFilterBool PxSetFilterConstants
*/
void PxGetFilterConstants(PxGroupsMask& c0, PxGroupsMask& c1);

/**
\brief Setups filtering's K0 and K1 value. See comments for PxGroupsMask

\param[in] c0 The new group mask. See #PxGroupsMask.
\param[in] c1 The new group mask. See #PxGroupsMask.

@see PxSetFilterOps PxSetFilterBool PxGetFilterConstants
*/
void PxSetFilterConstants(const PxGroupsMask& c0, const PxGroupsMask& c1);

/**
\brief Gets 64-bit mask used for collision filtering. See comments for PxGroupsMask

\param[in] actor The actor

\return The group mask for the actor.

@see PxSetGroupsMask()
*/
PxGroupsMask PxGetGroupsMask(const PxActor& actor);

/**
\brief Sets 64-bit mask used for collision filtering. See comments for PxGroupsMask

\param[in] actor The actor
\param[in] mask The group mask to set for the actor.

@see PxGetGroupsMask()
*/
void PxSetGroupsMask(PxActor& actor, const PxGroupsMask& mask);

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif
