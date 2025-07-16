#pragma once
#include "ComponentWithOwner.hpp"
#include "ComponentHandle.hpp"
#include "Queryable.hpp"
#include "DataProvider.hpp"

namespace RavEngine{
	struct AnimatorComponent;
/**
 Constraints are bound to a ConstraintTarget component
 */
struct ConstraintTarget : public ComponentWithOwner, public Queryable<ConstraintTarget> {
	friend class Constraint;
	ConstraintTarget(Entity id) : ComponentWithOwner(id){}
	void Destroy();
private:
	// stores the list of entities that are targeting the owner of this component
	UnorderedSet<Entity> targeters;
	
	/**
	 Entities that are targeting this owner call this method when they break connection
	 @param source the entity ID being constrained
	 */
	void DeleteTargeter(decltype(targeters)::value_type source){
		targeters.erase(source);
	}
	
	/**
	 Entities that are targeting this owner call this method when they establish connection
	 @param source the entity ID being constrained
	 */
	void AddTargeter(decltype(targeters)::value_type source){
		targeters.insert(source);
	}
};

/**
 Base class of all Constraints
 */
class Constraint : public ComponentWithOwner, public Queryable<Constraint>{
	friend class ConstraintTarget;
protected:
	// the target the owner is bound to
	ComponentHandle<ConstraintTarget> target;
	float influence = 1.0;
public:
	Constraint(Entity, decltype(target));
	// invoked by the world on component removal or owner destruction, do not invoke manually
	void Destroy();
	auto GetTarget() const { return target; }
	inline operator bool() const{
		return target;
	}
};

struct SocketConstraint : public Constraint, public QueryableDelta<Constraint,SocketConstraint>{
	using QueryableDelta<Constraint,SocketConstraint>::GetQueryTypes;
	std::string boneTarget;
	SocketConstraint(Entity id, decltype(target), const decltype(boneTarget)& );
};

/**
 Executes all Socket Constraints
 */
struct SocketSystem{
	struct DataProvider : public RavEngine::ValidatorProvider<RavEngine::AnimatorComponent> {};
	void operator()(DataProvider&, const SocketConstraint&, Transform&);
};
}
