#pragma once
#include "ComponentWithOwner.hpp"
#include "ComponentHandle.hpp"
#include "Queryable.hpp"

namespace RavEngine{

/**
 Constraints are bound to a ConstraintTarget component
 */
struct ConstraintTarget : public ComponentWithOwner, public Queryable<ConstraintTarget> {
	friend class Constraint;
	ConstraintTarget(entity_t id) : ComponentWithOwner(id){}
	void Destroy();
private:
	UnorderedSet<entity_t> targeters;
	void DeleteTargeter(decltype(targeters)::value_type source){
		targeters.erase(source);
	}
	void AddTargeter(decltype(targeters)::value_type source){
		targeters.insert(source);
	}
};
	
class Constraint : public ComponentWithOwner, public Queryable<Constraint>{
	friend class ConstraintTarget;
protected:
	ComponentHandle<ConstraintTarget> target;
	float influence = 1.0;
public:
	Constraint(entity_t, decltype(target));
	void Destroy();
	auto GetTarget() const { return target; }
	inline operator bool() const{
		return target;
	}
};

struct SocketConstraint : public Constraint, public QueryableDelta<Constraint,SocketConstraint>{
	using QueryableDelta<Constraint,SocketConstraint>::GetQueryTypes;
	std::string boneTarget;
	SocketConstraint(entity_t id, decltype(target), const decltype(boneTarget)& );
};

struct SocketSystem{
	void operator()(float, const SocketConstraint&, Transform&);
};
}
