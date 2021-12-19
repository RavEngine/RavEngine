#pragma once
#include "DataStructures.hpp"
#include <uuids.h>
#include <string>
#include "App.hpp"
#include "NetworkBase.hpp"

namespace RavEngine {

class SyncVar_base{
protected:
	static locked_hashmap<uuids::uuid, SyncVar_base*,SpinLock> all_syncvars;
	
	typedef ConcurrentQueue<std::string> queue_t;
	static queue_t queue_A, queue_B;
	static std::atomic<queue_t*> readingptr, writingptr;
	
public:
	static decltype(all_syncvars)& GetAllSyncvars() {
		return all_syncvars;
	}
	
	const uuids::uuid id = uuids::uuid::create();
	
	/* On the server:
	 *	invalid = the server has ownership
	 *	<any number> = the machine on the specified connection has ownership
	 *
	 * On the client:
	 *	invalid = this machine does __not__ have ownership
	 *	<any number> = this machine has ownership
	 */
	HSteamNetConnection owner = k_HSteamNetConnection_Invalid;
	
	inline bool IsOwner() const{
		if (GetApp()->networkManager.IsServer()){
			return owner == k_HSteamNetConnection_Invalid;
		}
		else if (GetApp()->networkManager.IsClient()){
			return owner != k_HSteamNetConnection_Invalid;
		}
	}
	
	virtual void NetSync(const std::string_view& data) = 0;
	
	/**
	Enqueue a message from the networking. For internal use only
	@param cmd the raw command from the networking
	*/
	static inline void EnqueueCmd(const std::string_view& cmd, HSteamNetConnection origin){
		writingptr.load()->enqueue(std::string(cmd));
	}
	
	/**
	Swap queues. For internal use only
	*/
	static inline void Swap(){
		queue_t *reading = readingptr.load(), *writing = writingptr.load();
		std::swap(reading,writing);
		readingptr.store(reading);
		writingptr.store(writing);
	}
	
	/**
	Consume all enqueued messages on the active queue. For internal use only.
	*/
	static inline void ProcessQueue(){
		std::string cmd;
		auto reading = readingptr.load();
		while(reading->try_dequeue(cmd)){
			//get the id this is for
			
			char idbytes[16];
			std::memcpy(idbytes,cmd.data()+1,sizeof(idbytes));
			
			uuids::uuid id(idbytes);
			all_syncvars.if_contains(id, [&](SyncVar_base* idptr) {
				idptr->NetSync(cmd);
			});
		}
	}
};

template<typename T>
class SyncVar : public SyncVar_base{
	T value, prev;
	int threshold = 0.1;
	
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
		if (!IsOwner()) {
			prev = value;
			std::memcpy(&value, data.data() + 16 + 1, sizeof(T));
		}
	}
	
	// T assignment
	inline void operator=(const T& other) {
		if (IsOwner()) {
			value = other;
			if (std::abs(value - prev) >= threshold) {
				//sync over network
				char data[sizeof(T) + 1 + 16];
				std::memcpy(data + 1 + 16, &value, sizeof(value));	// the data
				auto idbytes = id.raw();
				std::memcpy(data + 1, idbytes.data(), idbytes.size());
				data[0] = NetworkBase::CommandCode::SyncVar;

				GetApp()->networkManager.SyncVarUpdate(std::string_view(data, sizeof(data)));

				//update prev
				prev = value;
			}
		}
		else {
			Debug::Warning("Cannot set syncvar that is not locally owned");
		}
	}
	
	//convert to T
	inline operator T () const{
		return value;
	}
};

}
