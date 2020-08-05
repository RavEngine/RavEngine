//
//  PhysXDefines.h
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once

//figure out defines for PhysX
#ifndef NDEBUG
    #define _DEBUG             
    #undef NDEBUG
#else
    #define NDEBUG
    #undef _DEBUG
#endif
