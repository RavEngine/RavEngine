#define RVE_TESTING_ACCESS 1
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
    
    // test versioning
    auto gm = w.Instantiate<RavEngine::Entity>();
    gm.EmplaceComponent<IntComponent>(0);
    
    assert(w.CorrectVersion(gm.id));   // entity was not destroyed, so version is fine
    auto cpy = gm;
    gm.Destroy();   // gm's ID is set to invalid, but cpy's is not
    assert(not w.CorrectVersion(cpy.id)); // this handle is stale because the entity was destroyed
    
    gm =  w.Instantiate<RavEngine::Entity>();
    assert(w.CorrectVersion(gm.id));   // entity was recycled, so version is fine
    
    return 0;
}

int Test_CheckGraph() {
    struct Foo {};
    struct Bar {};
    struct C {};
    {
        World w;

        struct Test1System1 {
            void operator()(const Foo&, const Bar&, const C&) {

            }
        };

        struct Test1System2 {
            void operator()(const Foo&, const Bar&, const C&) {

            }
        };

        static_assert(CTTI<Test1System1>() != CTTI<Test1System2>(), "Different type names produce the same ID!");

        w.EmplaceSystem<Test1System1>();
        w.EmplaceSystem<Test1System2>();
        try {
            w.Tick(1);
        }
        catch (std::exception& s) {
            // this shouldn't hit, these 
            cout << "CheckGraph all-const errored when it should not have" << std::endl;
            return 1;
        }
    }
    {
        World w;
        struct Test3System1 {
            void operator()(const Bar&) {}
        };
        struct Test3System2 {
            void operator()(Bar&) {}
        };
        w.EmplaceSystem<Test3System1>();
        w.EmplaceSystem<Test3System2>();

        static_assert(CTTI<const Bar&>() == CTTI<Bar&>(), "Const ref and non-const ref have different IDs");
        static_assert(CTTI<const Bar>() != CTTI<Bar>(), "Const value and non-const value have the same IDs");
        static_assert(CTTI<Bar&>() != CTTI<Bar>(), "Reference and value have the same ID");

        const auto& tasks1 = w.getTypeToSystem().at(CTTI<Test3System1>());
        const auto& tasks2 = w.getTypeToSystem().at(CTTI<Test3System2>());

        if (tasks1.readDependencies[0] != tasks2.writeDependencies[0]) {
            cout << "Different IDs generated for the same type!" << std::endl;
            return 1;
        }
    }
    {
        World w;
        // these are unsafe A is wholly contained within B and there is a read-write conflict
        struct Test2System1{
            void operator()(const Foo&, Bar&) {             // 1 read, 1 write

            }
        };

        struct Test2System2 {
            void operator()(const Foo&, const Bar&, const C&) { // 3 reads

            }
        };
        static_assert(CTTI<Test2System1>() != CTTI<Test2System2>(), "Different type names produce the same ID!");

        w.EmplaceSystem<Test2System1>();
        w.EmplaceSystem<Test2System2>();

        auto type1 = CTTI<Test2System1>();
        auto type2 = CTTI<Test2System2>();
        
        bool caughtProblem = false;
        try {
            w.Tick(1);
        }
        catch (std::exception& e) {
            caughtProblem = true;
        }
        if (!caughtProblem) {
            cout << "CheckGraph write-read did not catch this problem when it should have" << std::endl;
            return 1;
        }
    }
    {
        World w;
        struct Test4System1 {
            void operator()(const Foo&) {}
            void before(World* w) const {}
        };
        struct Test4System2 {
            void operator()(const Bar&) {}
            void after(World* w) const {}
        };

        w.EmplaceSystem<Test4System1>();
        w.EmplaceSystem<Test4System2>();

        bool caughtProblem = false;
        try {
            w.Tick(1);
        }
        catch (std::exception& e) {
            caughtProblem = true;
        }
        if (!caughtProblem) {
            cout << "CheckGraph pre-post-hook did not catch this problem when it should have" << std::endl;
            return 1;
        }
    }

    return 0;
}

int main(int argc, char** argv) {
    const unordered_map<std::string_view, std::function<int(void)>> tests{
		{"CTTI",&Test_CTTI},
        {"Test_UUID",&Test_UUID},
        {"Test_AddDel",&Test_AddDel},
        {"Test_SpawnDestroy",&Test_SpawnDestroy},
        {"Test_CheckGraph",&Test_CheckGraph},
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
