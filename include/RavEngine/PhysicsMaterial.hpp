#pragma once

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

	class PhysicsMaterial {
	public:
		PhysicsMaterial(double staticFriction=1, double dynamicFriction=1, double restitution = 1);
		virtual ~PhysicsMaterial();

		void SetStaticFriction(double);
		void SetDynamicFriction(double);
		void SetRestitution(double);
		void SetFrictionCombineMode(PhysicsCombineMode);
		void SetRestitutionCombineMode(PhysicsCombineMode);

		double GetStaticFriction() const;
		double GetDynamicFriction() const;
		double GetRestitution() const;
		PhysicsCombineMode GetFrictionCombineMode() const;
		PhysicsCombineMode GetRestitutionCombineMode() const;

        constexpr inline physx::PxMaterial* const GetPhysXmat() const {
			return mat;
		}

	protected:
		physx::PxMaterial* mat = nullptr;
	};
}
