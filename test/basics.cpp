#include <RavEngine/CTTI.hpp>
#include <unordered_map>
#include <iostream>
#include <cassert>
#include <functional>
#include <uuids.h>

using namespace RavEngine;
using namespace std;

int Test_CTTI(){
	
	auto t1 = CTTI<int>;
	auto t2 = CTTI<float>;
	auto t3 = CTTI<int>;
	
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
        uuids::uuid id2(data.data());
        assert(id1 == id2);
    }
    
    //copy constructor
    auto id1 = uuids::uuid::create();
    uuids::uuid id2(id1);
    assert(id1 == id2);
    return 0;
}

int main(int argc, const char** argv) {
    const unordered_map <string, std::function<int(void)>> tests{
		{"CTTI",&Test_CTTI},
        {"Test_UUID",&Test_UUID}
    };
	    
	if (argc < 2){
		cerr << "No test provided - use ctest" << endl;
		return -1;
	}
	
    const std::string test = argv[1];
    if (tests.find(test) != tests.end()) {
       return tests.at(test)();
    }
    else {
        cerr << "No test with name: " << test << endl;
        return -1;
    }
    return 0;
}
