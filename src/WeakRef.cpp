//
//  WeakSharedObjectRef.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "WeakRef.hpp"
#include "SharedObject.hpp"

using namespace std;
using namespace RavEngine;

void WeakRefBase::Associate(RavEngine::SharedObject* obj){
	if (obj == nullptr){
		return;
	}
	obj->TrackWeak(this);
}

void WeakRefBase::Dissassociate(RavEngine::SharedObject* obj){
	if (obj == nullptr){
		return;
	}
	obj->UntrackWeak(this);
}
