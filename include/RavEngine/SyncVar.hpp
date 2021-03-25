#pragma once
#include "DataStructures.hpp"
#include <uuids.h>
#include <string>
#include "App.hpp"
#include "NetworkBase.hpp"

namespace RavEngine {

class SyncVar_base{
protected:
	static locked_hashmap<uuids::uuid, SyncVar_base*> all_syncvars;
	
	typedef ConcurrentQueue<std::string> queue_t;
	static queue_t queue_A, queue_B;
	static std::atomic<queue_t*> readingptr, writingptr;
	
public:
	virtual void NetSync(const std::string_view& data) = 0;
	
	static inline void EnqueueCmd(const std::string_view& cmd){
		writingptr.load()->enqueue(std::string(cmd));
	}
	
	static inline void Swap(){
		queue_t *reading = readingptr.load(), *writing = writingptr.load();
		std::swap(reading,writing);
		readingptr.store(reading);
		writingptr.store(writing);
	}
	
	static inline void ProcessQueue(){
		std::string cmd;
		auto reading = readingptr.load();
		while(reading->try_dequeue(cmd)){
			//get the id this is for
			
			char idbytes[16];
			std::memcpy(idbytes,cmd.data()+1,sizeof(idbytes));
			
			uuids::uuid id(idbytes);
			
			if (all_syncvars.contains(id)){
				auto idptr = all_syncvars.at(id);
				idptr->NetSync(cmd);
			}
		}
	}
};

template<typename T>
class SyncVar : public SyncVar_base{
	T value, prev;
	int threshold = 0.1;
	uuids::uuid id = uuids::uuid::create();
	
public:
	SyncVar(const T& input ) : value(input){
		all_syncvars[id] = this;
	}
	SyncVar(){
		all_syncvars[id] = this;
	}
	
	~SyncVar(){
		all_syncvars.erase(id);
	}
	
	void NetSync(const std::string_view& data) override{
		prev = value;
		std::memcpy(&value, data.data() + 16 + 1, sizeof(T));
	}
	
	// T assignment
	inline void operator=(const T& other) {
		value = other;
		if (std::abs(value - prev) >= threshold){
			//sync over network
			const char data[sizeof(T) + 1 + 16];
			std::memcpy(data + 1 + 16, &value, sizeof(value));	// the data
			auto idbytes = id.raw();
			std::memcpy(data + 1, idbytes.data(),idbytes.size());
			data[0] = NetworkBase::CommandCode::SyncVar;
			
			App::networkManager.SyncVarUpdate(std::string_view(data,sizeof(data)));
			
			//update prev
			prev = value;

		}
	}
	
	//convert to T
	inline operator T () const{
		return value;
	}
};

}
