#pragma once
//
//  PhysXDefines.h
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug.
//


//figure out defines for PhysX
#define PX_PHYSX_STATIC_LIB 1   // required for PhysX static link
#ifndef NDEBUG
    #ifndef _DEBUG
        #define _DEBUG
    #endif
    #undef NDEBUG
#else
    #ifndef NDEBUG
        #define NDEBUG
    #endif
    #undef _DEBUG
#endif
