//
//  main.cpp
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "WorldTest.hpp"
#include <iostream>

#ifdef _WIN32
	#include <Windows.h>
	#include <Xinput.h>
	#pragma comment(lib, "xinput")
	#pragma comment(lib,"Ws2_32")
#endif
#include "GameplayStatics.hpp"
#include "TestEntity.hpp"
#include "TestApp.h"

START_APP(TestApp)
