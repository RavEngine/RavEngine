#include "PhysicsMaterial.hpp"
#include "PhysXDefines.h"
#include <PxMaterial.h>
#include "PhysicsSolver.hpp"

RavEngine::PhysicsMaterial::PhysicsMaterial(double sf, double df, double r)
{
	mat = PhysicsSolver::phys->createMaterial(sf, df, r);
}

RavEngine::PhysicsMaterial::~PhysicsMaterial()
{
	mat->release();
}

void RavEngine::PhysicsMaterial::SetStaticFriction(double sf)
{
	mat->setStaticFriction(sf);
}

void RavEngine::PhysicsMaterial::SetDynamicFriction(double df)
{
	mat->setDynamicFriction(df);
}

void RavEngine::PhysicsMaterial::SetRestitution(double r)
{
	mat->setRestitution(r);
}

void RavEngine::PhysicsMaterial::SetFrictionCombineMode(PhysicsCombineMode mode)
{
	
	mat->setFrictionCombineMode(static_cast<physx::PxCombineMode::Enum>(static_cast<std::underlying_type<PhysicsCombineMode>::type>(mode)));
}

void RavEngine::PhysicsMaterial::SetRestitutionCombineMode(PhysicsCombineMode mode)
{
	mat->setRestitutionCombineMode(static_cast<physx::PxCombineMode::Enum>(static_cast<std::underlying_type<PhysicsCombineMode>::type>(mode)));
}


double RavEngine::PhysicsMaterial::GetStaticFriction() const
{
	return mat->getStaticFriction();
}

double RavEngine::PhysicsMaterial::GetDynamicFriction() const
{
	return mat->getDynamicFriction();
}

double RavEngine::PhysicsMaterial::GetRestitution() const
{
	return mat->getRestitution();
}

RavEngine::PhysicsCombineMode RavEngine::PhysicsMaterial::GetFrictionCombineMode() const
{
	return PhysicsCombineMode(mat->getFrictionCombineMode());
}

RavEngine::PhysicsCombineMode RavEngine::PhysicsMaterial::GetRestitutionCombineMode() const
{
	return PhysicsCombineMode(mat->getRestitutionCombineMode());
}
