//
//  PhysXDefines.h
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once

//figure out defines for PhysX
#define PX_PHYSX_STATIC_LIB 1   // required for PhysX static link
#ifndef NDEBUG
    #define _DEBUG             
    #undef NDEBUG
#else
    #define NDEBUG
    #undef _DEBUG
#endif
