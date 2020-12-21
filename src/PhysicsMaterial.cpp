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

void RavEngine::PhysicsMaterial::setStaticFriction(double sf)
{
	mat->setStaticFriction(sf);
}

void RavEngine::PhysicsMaterial::setDynamicFriction(double df)
{
	mat->setDynamicFriction(df);
}

void RavEngine::PhysicsMaterial::setRestitution(double r)
{
	mat->setRestitution(r);
}

void RavEngine::PhysicsMaterial::setFrictionCombineMode(PhysicsCombineMode mode)
{
	
	mat->setFrictionCombineMode(static_cast<physx::PxCombineMode::Enum>(static_cast<std::underlying_type<PhysicsCombineMode>::type>(mode)));
}

void RavEngine::PhysicsMaterial::setRestitutionCombineMode(PhysicsCombineMode mode)
{
	mat->setRestitutionCombineMode(static_cast<physx::PxCombineMode::Enum>(static_cast<std::underlying_type<PhysicsCombineMode>::type>(mode)));
}


double RavEngine::PhysicsMaterial::getStaticFriction() const
{
	return mat->getStaticFriction();
}

double RavEngine::PhysicsMaterial::getDynamicFriction() const
{
	return mat->getDynamicFriction();
}

double RavEngine::PhysicsMaterial::getRestitution() const
{
	return mat->getRestitution();
}

RavEngine::PhysicsCombineMode RavEngine::PhysicsMaterial::getFrictionCombineMode() const
{
	return PhysicsCombineMode(mat->getFrictionCombineMode());
}

RavEngine::PhysicsCombineMode RavEngine::PhysicsMaterial::getRestitutionCombineMode() const
{
	return PhysicsCombineMode(mat->getRestitutionCombineMode());
}
