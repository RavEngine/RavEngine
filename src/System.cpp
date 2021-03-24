//
//  System.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. 
//

#include "System.hpp"
#include "ScriptSystem.hpp"
#include "CTTI.hpp"
#include "PhysicsLinkSystem.hpp"
#include "SystemManager.hpp"

using namespace RavEngine;

const System::list_type System::empty = {};
const System::list_type ScriptSystem::queries = {CTTI<ScriptComponent>()};

const System::list_type PhysicsLinkSystemWrite::queries = {CTTI<PhysicsBodyComponent>()};

const System::list_type PhysicsLinkSystemRead::queries = {CTTI<RigidBodyDynamicComponent>()};
const System::list_type PhysicsLinkSystemRead::runbefore = {CTTI<ScriptSystem>()};

System::list_type empty;
