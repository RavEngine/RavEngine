#include <RavEngine/CTTI.hpp>
#include <RavEngine/World.hpp>
#include <RavEngine/Entity.hpp>
#include <RavEngine/ComponentHandle.hpp>
#include <RavEngine/App.hpp>
#include <unordered_map>
#include <iostream>
#include <functional>
#include <RavEngine/Uuid.hpp>
#include <string_view>
#include <RavEngine/Debug.hpp>
#include <cassert>
#include <span>

using namespace RavEngine;
using namespace std;

// needed for linker
const std::string_view RVE_VFS_get_name(){
    return "";
}
const std::span<const char> cmrc_get_file_data(const std::string_view& path) {
    return {};
}

#undef assert

#define assert(cond) \
{\
    Debug::Assert(cond, "Debug assertion failed! {}:{}",__FILE__,__LINE__);\
}

struct IntComponent {
    int value;
};

struct FloatComponent{
    float value;
};

struct MyPrototype : public Entity{
    void Create(){
        auto& comp = EmplaceComponent<IntComponent>();
        comp.value = 5;
    }
};

struct MyExtendedPrototype : public MyPrototype{
    void Create(){
        MyPrototype::Create();
        auto& comp = EmplaceComponent<FloatComponent>();
        comp.value = 7.5;
    }
};

int Test_CTTI(){
	
	auto t1 = CTTI<int>();
	auto t2 = CTTI<float>();
	auto t3 = CTTI<int>();
	
	assert(t1 == t3);
	assert(t1 != t2);
	assert(t2 != t3);
	
	return 0;
}

int Test_UUID(){
    
    //generate some random uuids
    for(int i = 0; i < 10; i++){
        auto id1 = uuids::uuid::create();
        auto data = id1.raw();
        uuids::uuid id2(data);
        assert(id1 == id2);
    }
    
    //copy constructor
    auto id1 = uuids::uuid::create();
    uuids::uuid id2(id1);
    assert(id1 == id2);
    return 0;
}

int Test_AddDel(){
    World w;
    auto e = w.Instantiate<Entity>();
    auto& ic = e.EmplaceComponent<IntComponent>();
    ic.value = 6;

    auto e2 = w.Instantiate<Entity>();
    e2.EmplaceComponent<FloatComponent>().value = 54.2;

    int count = 0;
    w.Filter([&](IntComponent& ic, FloatComponent& fc) {
        count++;
    });
    assert(count == 0);
    cout << "A 2-filter with 0 possibilities found " << count << " results\n";

    w.Filter([&](IntComponent& ic) {
        ic.value *= 2;
    });
    
    ComponentHandle<IntComponent> handle(e);
    
    assert(handle->value == 6 * 2);

    e.DestroyComponent<IntComponent>();
    assert(e.HasComponent<IntComponent>() == false);
    count = 0;
    w.Filter([&](FloatComponent& fc) {
        count++;
    });
    cout << "After deleting the only intcomponent, the floatcomponent count is " << count << "\n";
    assert(count == 1);

    count = 0;
    w.Filter([&](IntComponent& fc) {
        count++;
    });
    cout << "After deleting the only intcomponent, the intcomponent count is " << count << "\n";
    assert(count == 0);

    assert((e.GetWorld() == e2.GetWorld()));
    
    return 0;
}

int Test_SpawnDestroy(){
    
    World w;
   std::array<MyExtendedPrototype, 30> entities;
   for( auto& e : entities){
       e = w.Instantiate<MyExtendedPrototype>();
   }
   {
       int icount = 0;
       w.Filter([&](IntComponent& fc) {
           icount++;
       });
       int fcount = 0;
       w.Filter([&](FloatComponent& fc) {
           fcount++;
       });
       cout << "Spawning " << entities.size() << " 2-component entities yields " << icount << " intcomponents and " << fcount << " floatcomponents\n";
       assert(icount == entities.size());
       assert(fcount == entities.size());
   }
    constexpr int ibegin = 4;
    constexpr int iend = 20;
   for(int i = ibegin; i < iend; i++){
       entities[i].Destroy();
   }
   
   {
       int icount = 0;
       w.Filter([&](IntComponent& fc) {
           icount++;
       });
       int fcount = 0;
       w.Filter([&](FloatComponent& fc) {
           fcount++;
       });
       cout << "After destroying " << iend-ibegin << " 2-component entities, filter yields " << icount << " intcomponents and " << fcount << " floatcomponents\n";
       assert(icount == (entities.size() - (iend - ibegin )));
       assert(fcount == (entities.size() - (iend - ibegin)));
   }
    
    return 0;
}

int main(int argc, char** argv) {
    const unordered_map<std::string_view, std::function<int(void)>> tests{
		{"CTTI",&Test_CTTI},
        {"Test_UUID",&Test_UUID},
        {"Test_AddDel",&Test_AddDel},
        {"Test_SpawnDestroy",&Test_SpawnDestroy},
    };
	    
	if (argc < 2){
		cerr << "No test provided - use ctest" << endl;
		return -1;
	}
	
    auto test = argv[1];
    if (tests.find(test) != tests.end()) {
        RavEngine::App app;
        return tests.at(test)();
    }
    else {
        cerr << "No test with name: " << test << endl;
        return -1;
    }
    return 0;
}
