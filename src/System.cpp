//
//  System.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. 
//

#include "System.hpp"
#include "ScriptSystem.hpp"
#include "CTTI.hpp"
#include "SystemManager.hpp"
#include "RPCSystem.hpp"
#include "AnimatorSystem.hpp"

using namespace RavEngine;

const System::list_type System::empty = {};
const System::list_type ScriptSystem::queries = {CTTI<ScriptComponent>()};

const System::list_type RPCSystem::queries = {CTTI<RPCComponent>()};

const decltype(AnimatorSystem::queries) AnimatorSystem::queries = {CTTI<AnimatorComponent>()};

System::list_type empty;
