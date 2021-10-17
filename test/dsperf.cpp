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

static std::chrono::system_clock timer;

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
	cout << StrFormat("Time to add {} elements: {} µs\n",ds.size(), dur.count()/100);
	
	// time to remove elements from the middle
	dur = time([&]{
		for (int i = 50'000; i < 51'000; i++){
			erase_func(i);
		}
	});
	cout << StrFormat("Time to remove {} elements: {} µs\n",1'000, dur.count()/100);
    
    // time to iterate 90*10 times (10 seconds worth of ticking on default)
    constexpr auto iter_count = 90*10;
    uint64_t sum = 0;
    dur = time([&]{
        for(int i = 0; i < iter_count; i++){
            for(const auto& elem : ds){
                sum += elem;                // calculate a sum so the compiler doesn't optimize this
            }
        }
    });
    cout << StrFormat("Time to iterate {} times: {} µs (sum = {})\n",iter_count, dur.count()/100,sum);
    
}


int main(int argc, const char** argv){
	
	// STL vector
	{
		cout<<"STL vector\n";
		std::vector<int> vec;
		
		do_test(vec,[&](int i){
			vec.push_back(i);
		},[&](int i){
			vec.erase(std::remove(vec.begin(),vec.end(),i),vec.end());
		});
	}
	
	// ozz vector
	{
		cout << "\nozz vector\n";
		ozz::vector<int> vec;
		
		do_test(vec,[&](int i){
			vec.push_back(i);
		},[&](int i){
			vec.erase(std::remove(vec.begin(),vec.end(),i),vec.end());
		});
	}
	
	// boost vector
	{
        cout << "\nboost vector\n";
		boost::container::vector<int> vec;
		
		do_test(vec,[&](int i){
			vec.push_back(i);
		},[&](int i){
			vec.erase(std::remove(vec.begin(),vec.end(),i),vec.end());
		});
	}
	
	{
		cout << "\nunordered_vector\n";
		unordered_vector<int> vec;
		do_test(vec,[&](int i){
			vec.insert(i);
		},[&](int i){
			vec.erase(i);
		});
	}
	
	{
		cout << ("\nunordered_vector with known iterators\n");
		unordered_vector<int> vec;
		do_test(vec,[&](int i){
			vec.insert(i);
		},[&](int i){
			vec.erase(vec.begin() + i);
		});
	}
	
	{
		cout << ("\nunordered_contiguous_set\n");
		unordered_contiguous_set<int> vec;
		do_test(vec,[&](int i){
			vec.insert(i);
		},[&](int i){
			vec.erase(i);
		});
	}
    
    {
        cout << ("\nstd::unordered_set\n");
        std::unordered_set<int> vec;
        do_test(vec,[&](int i){
            vec.insert(i);
        },[&](int i){
            vec.erase(i);
        });
    }
	
	{
		cout << ("\nlocked_hashset std::mutex\n");
		locked_hashset<int> set;
		do_test(set, [&](int i){
			set.insert(i);
		}, [&](int i){
			set.erase(i);
		});
	}
	// flat hashset with lock
	{
		cout << ("\nlocked_hashset Spinlock\n");
		locked_hashset<int,SpinLock> set;
		do_test(set, [&](int i){
			set.insert(i);
		}, [&](int i){
			set.erase(i);
		});
	}
	// flat hashset with no mutex
	{
		cout << ("\nphmap::flat_hashset\n");
		phmap::flat_hash_set<int> set;
		do_test(set, [&](int i){
			set.insert(i);
		}, [&](int i){
			set.erase(i);
		});
	}
	// node hashset
	{
		cout << ("\nlocked_node_hashset spinlock\n");
		locked_node_hashset<int,SpinLock> set;
		do_test(set, [&](int i){
			set.insert(i);
		}, [&](int i){
			set.erase(i);
		});
	}
	// node hashset no lock
	{
		cout << ("\nlocked_node_hashset no lock\n");
		phmap::node_hash_set<int> set;
		do_test(set, [&](int i){
			set.insert(i);
		}, [&](int i){
			set.erase(i);
		});
	}
	
	return 0;
}
