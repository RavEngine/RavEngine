#include <RavEngine/DataStructures.hpp>
#include <RavEngine/Debug.hpp>
#include <vector>
#include <chrono>
#include <typeinfo>
#include <RavEngine/AnimatorComponent.hpp>
#include <RavEngine/unordered_vector.hpp>
#include <boost/container/vector.hpp>

using namespace RavEngine;
using namespace std;


typedef std::chrono::high_resolution_clock clocktype;
static clocktype timer;

template<typename T>
static inline clocktype::duration time(const T& func){
	auto begin_time = timer.now();
	func();
	auto end_time = timer.now();
	return chrono::duration_cast<std::chrono::microseconds>(end_time - begin_time);
}

template<typename T, typename is_fn, typename es_fn>
static inline void do_test(const T& ds, const is_fn& insert_func, const es_fn& erase_func){
	
	auto dur = time([&]{
		// time to add 100K elements
		for(int i = 0; i < 100'000; i++){
			insert_func(i);
		}
	});
	Debug::Log("Time to add {} elements: {} µs",ds.size(), dur.count());
	
	// time to iterate 90*10 times (10 seconds worth of ticking on default)
	constexpr auto iter_count = 90*10;
	dur = time([&]{
		for(int i = 0; i < iter_count; i++){
			uint64_t sum = 0;
			for(const auto& elem : ds){
				sum += elem;				// calculate a sum so the compiler doesn't optimize this
			}
		}
	});
	Debug::Log("Time to iterate {} times: {} µs",iter_count, dur.count());
	
	// time to remove elements from the middle
	dur = time([&]{
		for (int i = 50'000; i < 51'000; i++){
			erase_func(i);
		}
	});
	Debug::Log("Time to remove {} elements: {} µs",1'000, dur.count());
}


int main(){
	
	// STL vector
	{
		Debug::Log("STL vector");
		std::vector<int> vec;
		
		do_test(vec,[&](int i){
			vec.push_back(i);
		},[&](int i){
			vec.erase(std::remove(vec.begin(),vec.end(),i),vec.end());
		});
	}
	
	// ozz vector
	{
		Debug::Log("ozz vector");
		ozz::vector<int> vec;
		
		do_test(vec,[&](int i){
			vec.push_back(i);
		},[&](int i){
			vec.erase(std::remove(vec.begin(),vec.end(),i),vec.end());
		});
	}
	
	// boost vector
	{
		Debug::Log("boost vector");
		boost::container::vector<int> vec;
		
		do_test(vec,[&](int i){
			vec.push_back(i);
		},[&](int i){
			vec.erase(std::remove(vec.begin(),vec.end(),i),vec.end());
		});
	}
	
	{
		Debug::Log("unordered_vector");
		unordered_vector<int> vec;
		do_test(vec,[&](int i){
			vec.insert(i);
		},[&](int i){
			vec.erase(i);
		});
	}
	
	{
		Debug::Log("unordered_vector with known iterators");
		unordered_vector<int> vec;
		do_test(vec,[&](int i){
			vec.insert(i);
		},[&](int i){
			vec.erase(vec.begin() + i);
		});
	}
	
	{
		Debug::Log("unordered_cached_vector");
		unordered_cached_vector<int> vec;
		do_test(vec,[&](int i){
			vec.insert(i);
		},[&](int i){
			vec.erase(i);
		});
	}
	
	{
		Debug::Log("locked_hashset std::mutex");
		locked_hashset<int> set;
		do_test(set, [&](int i){
			set.insert(i);
		}, [&](int i){
			set.erase(i);
		});
	}
	// flat hashset with lock
	{
		Debug::Log("locked_hashset Spinlock");
		locked_hashset<int,SpinLock> set;
		do_test(set, [&](int i){
			set.insert(i);
		}, [&](int i){
			set.erase(i);
		});
	}
	// flat hashset with no mutex
	{
		Debug::Log("phmap::flat_hashset");
		phmap::flat_hash_set<int> set;
		do_test(set, [&](int i){
			set.insert(i);
		}, [&](int i){
			set.erase(i);
		});
	}
	// node hashset
	{
		Debug::Log("locked_node_hashset spinlock");
		locked_node_hashset<int,SpinLock> set;
		do_test(set, [&](int i){
			set.insert(i);
		}, [&](int i){
			set.erase(i);
		});
	}
	// node hashset no lock
	{
		Debug::Log("locked_node_hashset no lock");
		phmap::node_hash_set<int> set;
		do_test(set, [&](int i){
			set.insert(i);
		}, [&](int i){
			set.erase(i);
		});
	}
	
	return 0;
}
