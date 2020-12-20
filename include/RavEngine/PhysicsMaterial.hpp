#pragma once

#include "SharedObject.hpp"

namespace physx {
	class PxMaterial;
}

namespace RavEngine {
	enum class PhysicsMaterialFlag {
		DisableFriction = 1 << 0,
		DisableStrongFriction = 1 << 1
	};

	enum class PhysicsCombineMode {
		Average = 0,
		Min = 1,
		Multiply = 2,
		Max = 3,
		NValues = 4,
		Pad32 = 0x7fffffff
	};

	class PhysicsMaterial : public SharedObject {
	public:
		PhysicsMaterial(double sf=1, double df=1, double r	=1);
		virtual ~PhysicsMaterial();

		void setStaticFriction(double);
		void setDynamicFriction(double);
		void setRestitution(double);
		void setFrictionCombineMode(PhysicsCombineMode);
		void setRestitutionCombineMode(PhysicsCombineMode);

		double getStaticFriction();
		double getDynamicFriction();
		double getRestitution();
		PhysicsCombineMode getFrictionCombineMode();
		PhysicsCombineMode getRestitutionCombineMode();

		physx::PxMaterial* const getPhysXmat() {
			return mat;
		}

	protected:
		physx::PxMaterial* mat = nullptr;
	};
}
