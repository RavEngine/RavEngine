// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2025 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef EXT_CONSTRAINT_HELPER_H
#define EXT_CONSTRAINT_HELPER_H

#include "foundation/PxAssert.h"
#include "foundation/PxTransform.h"
#include "foundation/PxMat33.h"
#include "foundation/PxSIMDHelpers.h"
#include "extensions/PxD6Joint.h"
#include "ExtJointData.h"
#include "foundation/PxVecMath.h"

namespace physx
{
namespace Ext
{
	namespace joint
	{
		PX_FORCE_INLINE void applyNeighborhoodOperator(const PxTransform32& cA2w, PxTransform32& cB2w)
		{
			if(cA2w.q.dot(cB2w.q)<0.0f)	// minimum dist quat (equiv to flipping cB2bB.q, which we don't use anywhere)
				cB2w.q = -cB2w.q;
		}

		/*
		\brief Transform the two joint frames into the world frame using the global poses of the associated actors.

		\param[out]	cA2w	joint frame associated with body 0 expressed in the world frame
							ie if g0 is the global pose of actor 0 then cA2w = g0 * jointFrame_0.
		\param[out]	cB2w	joint frame associated with body 1 expressed in the world frame
							ie if g1 is the global pose of actor 1 then cB2w = g1 * jointFrame_1.
		\param[in]	data	contains cmLocalPose^-1 * jointFrame for each body.
		\param[in]	bA2w	pose of the centre of mass of body 0 expressed in the world frame.
		\param[in]	bB2w	pose of the centre of mass of body 1 expressed in the world frame.

		\note b2w = g*cmLocalPose so we have g = b2w*cmLocalPose^-1. 
		We therefore have g * jointFrame = b2w * cmLocalPose^-1 * jointFrame = b2w * data.c2b
		*/
		PX_INLINE void computeJointFrames(PxTransform32& cA2w, PxTransform32& cB2w, const JointData& data, const PxTransform& bA2w, const PxTransform& bB2w)
		{
			PX_ASSERT(bA2w.isValid() && bB2w.isValid());

			//cA2w = bA2w * (cMassLocalPose0^-1 * jointFrame0)
			//cB2w = bB2w * (cMassLocalPose1^-1 * jointFrame1)
			aos::transformMultiply<false, true>(cA2w, bA2w, data.c2b[0]);
			aos::transformMultiply<false, true>(cB2w, bB2w, data.c2b[1]);

			PX_ASSERT(cA2w.isValid() && cB2w.isValid());
		}

		PX_INLINE void computeJacobianAxes(PxVec3 row[3], const PxQuat& qa, const PxQuat& qb)
		{
			// Compute jacobian matrix for (qa* qb)  [[* means conjugate in this expr]]
			// d/dt (qa* qb) = 1/2 L(qa*) R(qb) (omega_b - omega_a)
			// result is L(qa*) R(qb), where L(q) and R(q) are left/right q multiply matrix

			const PxReal wa = qa.w, wb = qb.w;
			const PxVec3 va(qa.x,qa.y,qa.z), vb(qb.x,qb.y,qb.z);

			const PxVec3 c = vb*wa + va*wb;
			const PxReal d0 = wa*wb;
			const PxReal d1 = va.dot(vb);
			const PxReal d = d0 - d1;

			row[0] = (va * vb.x + vb * va.x + PxVec3(d,     c.z, -c.y)) * 0.5f;
			row[1] = (va * vb.y + vb * va.y + PxVec3(-c.z,  d,    c.x)) * 0.5f;
			row[2] = (va * vb.z + vb * va.z + PxVec3(c.y,   -c.x,   d)) * 0.5f;

			if((d0 + d1) != 0.0f)  // check if relative rotation is 180 degrees which can lead to singular matrix
				return;
			else
			{
				row[0].x += PX_EPS_F32;
				row[1].y += PX_EPS_F32;
				row[2].z += PX_EPS_F32;
			}
		}

		PX_FORCE_INLINE Px1DConstraint* _linear(const PxVec3& axis, const PxVec3& ra, const PxVec3& rb, PxReal posErr, PxConstraintSolveHint::Enum hint, Px1DConstraint* c)
		{
			c->solveHint		= PxU16(hint);
			c->linear0			= axis;
			c->angular0			= ra.cross(axis);
			c->linear1			= axis;
			c->angular1			= rb.cross(axis);
			c->geometricError	= posErr;
			PX_ASSERT(c->linear0.isFinite());
			PX_ASSERT(c->linear1.isFinite());
			PX_ASSERT(c->angular0.isFinite());
			PX_ASSERT(c->angular1.isFinite());
			return c;
		}

		PX_FORCE_INLINE Px1DConstraint* _angular(const PxVec3& axis, PxReal posErr, PxConstraintSolveHint::Enum hint, Px1DConstraint* c)
		{
			c->solveHint		= PxU16(hint);
			c->linear0			= PxVec3(0.0f);
			c->angular0			= axis;
			c->linear1			= PxVec3(0.0f);
			c->angular1			= axis;
			c->geometricError	= posErr;
			c->flags			|= Px1DConstraintFlag::eANGULAR_CONSTRAINT;
			return c;
		}

		class ConstraintHelper
		{
			Px1DConstraint* mConstraints;
			Px1DConstraint* mCurrent;
			PX_ALIGN(16, PxVec3p	mRa);
			PX_ALIGN(16, PxVec3p	mRb);
			PX_ALIGN(16, PxVec3p	mCA2w);
			PX_ALIGN(16, PxVec3p	mCB2w);

		public:
			ConstraintHelper(Px1DConstraint* c, const PxVec3& ra, const PxVec3& rb)
				: mConstraints(c), mCurrent(c), mRa(ra), mRb(rb)	{}

			/*PX_NOINLINE*/	ConstraintHelper(Px1DConstraint* c, PxConstraintInvMassScale& invMassScale,
					PxTransform32& cA2w, PxTransform32& cB2w, PxVec3p& body0WorldOffset,
					const JointData& data, const PxTransform& bA2w, const PxTransform& bB2w)
				: mConstraints(c), mCurrent(c)
			{
				using namespace aos;

				V4StoreA(V4LoadA(&data.invMassScale.linear0), &invMassScale.linear0);	//invMassScale = data.invMassScale;

				computeJointFrames(cA2w, cB2w, data, bA2w, bB2w);

				if(1)
				{
					const Vec4V cB2wV = V4LoadA(&cB2w.p.x);
					const Vec4V raV = V4Sub(cB2wV, V4LoadU(&bA2w.p.x));	// const PxVec3 ra = cB2w.p - bA2w.p;

					V4StoreU(raV, &body0WorldOffset.x);					// body0WorldOffset = ra;

					V4StoreA(raV, &mRa.x);								// mRa = ra;

					V4StoreA(V4Sub(cB2wV, V4LoadU(&bB2w.p.x)), &mRb.x);	// mRb = cB2w.p - bB2w.p;

					V4StoreA(V4LoadA(&cA2w.p.x), &mCA2w.x);				// mCA2w = cA2w.p;
					V4StoreA(cB2wV, &mCB2w.x);							// mCB2w = cB2w.p;
				}
				else
				{
					const PxVec3 ra = cB2w.p - bA2w.p;

					body0WorldOffset = ra;

					mRa = ra;

					mRb = cB2w.p - bB2w.p;

					mCA2w = cA2w.p;
					mCB2w = cB2w.p;
				}
			}

			PX_FORCE_INLINE const PxVec3& getRa()	const	{ return mRa; }
			PX_FORCE_INLINE const PxVec3& getRb()	const	{ return mRb; }

			// hard linear & angular
			PX_FORCE_INLINE void linearHard(const PxVec3& axis, PxReal posErr)
			{
				Px1DConstraint* c = linear(axis, posErr, PxConstraintSolveHint::eEQUALITY);
				c->flags |= Px1DConstraintFlag::eOUTPUT_FORCE;
			}

			PX_FORCE_INLINE void angularHard(const PxVec3& axis, PxReal posErr)
			{
				Px1DConstraint* c = angular(axis, posErr, PxConstraintSolveHint::eEQUALITY);
				c->flags |= Px1DConstraintFlag::eOUTPUT_FORCE;
			}

			// limited linear & angular
			PX_FORCE_INLINE void linearLimit(const PxVec3& axis, PxReal ordinate, PxReal limitValue, const PxJointLimitParameters& limit)
			{
				if(!limit.isSoft() || ordinate > limitValue)
					addLimit(linear(axis, limitValue - ordinate, PxConstraintSolveHint::eNONE), limit);
			}

			PX_FORCE_INLINE void angularLimit(const PxVec3& axis, PxReal ordinate, PxReal limitValue, const PxJointLimitParameters& limit)
			{
				if(!limit.isSoft() || ordinate > limitValue)
					addLimit(angular(axis, limitValue - ordinate, PxConstraintSolveHint::eNONE), limit);
			}

			PX_FORCE_INLINE void angularLimit(const PxVec3& axis, PxReal error, const PxJointLimitParameters& limit)
			{
				addLimit(angular(axis, error, PxConstraintSolveHint::eNONE), limit);
			}

			PX_FORCE_INLINE void anglePair(PxReal angle, PxReal lower, PxReal upper, const PxVec3& axis, const PxJointLimitParameters& limit)
			{
				PX_ASSERT(lower<upper);
				const bool softLimit = limit.isSoft();

				if(!softLimit || angle < lower)
					angularLimit(-axis, -(lower - angle), limit);
				if(!softLimit || angle > upper)
					angularLimit(axis, (upper - angle), limit);
			}

			// driven linear & angular

			PX_FORCE_INLINE void linear(const PxVec3& axis, PxReal velTarget, PxReal error, const PxD6JointDrive& drive)
			{
				addDrive(linear(axis, error, PxConstraintSolveHint::eNONE), velTarget, drive);
			}

			PX_FORCE_INLINE void angular(const PxVec3& axis, PxReal velTarget, PxReal error, const PxD6JointDrive& drive, PxConstraintSolveHint::Enum hint = PxConstraintSolveHint::eNONE)
			{
				addDrive(angular(axis, error, hint), velTarget, drive);
			}

			PX_FORCE_INLINE PxU32 getCount()	const	{ return PxU32(mCurrent - mConstraints); }

			void prepareLockedAxes(const PxQuat& qA, const PxQuat& qB, const PxVec3& cB2cAp, PxU32 lin, PxU32 ang, PxVec3& raOut, PxVec3& rbOut, PxVec3* axis=NULL)
			{
				Px1DConstraint* current = mCurrent;
				
				PxVec3 errorVector(0.0f);

				PxVec3 ra = mRa;
				PxVec3 rb = mRb;
				if(lin)
				{
					const PxMat33Padded axes(qA);
					if(axis)
						*axis = axes.column0;
					
					if(lin&1) errorVector -= axes.column0 * cB2cAp.x;
					if(lin&2) errorVector -= axes.column1 * cB2cAp.y;
					if(lin&4) errorVector -= axes.column2 * cB2cAp.z;

					ra += errorVector;

					//Note that our convention is that C(s) = geometricError = (xA + rA) - (xB + rB)
					//where xA, xB are the positions of the two bodies in the world frame and rA, rB
					//are the vectors in the world frame from each body to the joint anchor.
					//We solve Jv + C(s)/dt = Jv + geometricError/dt = 0.
					//With GA, GB denoting the actor poses in world frame and LA, LB denoting the 
					//associated joint frames we have: cB2cAp = [(GA*LA)^-1 * (GB*LB)].p
					//But cB2cAp = (GA*LA).q.getConjugate() * ((xB + rB) - (xA + rA))
					//To match our convention we want geometricError = (GA*LA).q.getConjugate() * ((xA + rA) - (xB + rB))
					//cB2cAp therefore has the wrong sign to be used directly as the geometric error.
					//We need to negate cB2cAp to ensure that we set geometricError with the correct sign.				
					if(lin&1) _linear(axes.column0, ra, rb, -cB2cAp.x, PxConstraintSolveHint::eEQUALITY, current++);
					if(lin&2) _linear(axes.column1, ra, rb, -cB2cAp.y, PxConstraintSolveHint::eEQUALITY, current++);
					if(lin&4) _linear(axes.column2, ra, rb, -cB2cAp.z, PxConstraintSolveHint::eEQUALITY, current++);
				}

				if (ang)
				{
					const PxQuat qB2qA = qA.getConjugate() * qB;

					PxVec3 row[3];
					computeJacobianAxes(row, qA, qB);
					if (ang & 1) _angular(row[0], -qB2qA.x, PxConstraintSolveHint::eEQUALITY, current++);
					if (ang & 2) _angular(row[1], -qB2qA.y, PxConstraintSolveHint::eEQUALITY, current++);
					if (ang & 4) _angular(row[2], -qB2qA.z, PxConstraintSolveHint::eEQUALITY, current++);
				}

				raOut = ra;
				rbOut = rb;

				for(Px1DConstraint* front = mCurrent; front < current; front++)
					front->flags |= Px1DConstraintFlag::eOUTPUT_FORCE;

				mCurrent = current;
			}

			PX_FORCE_INLINE	Px1DConstraint* getConstraintRow()
			{
				return mCurrent++;
			}

		private:
			PX_FORCE_INLINE Px1DConstraint* linear(const PxVec3& axis, PxReal posErr, PxConstraintSolveHint::Enum hint)
			{
				return _linear(axis, mRa, mRb, posErr, hint, mCurrent++);
			}

			PX_FORCE_INLINE Px1DConstraint* angular(const PxVec3& axis, PxReal posErr, PxConstraintSolveHint::Enum hint)
			{
				return _angular(axis, posErr, hint, mCurrent++);
			}

			void addLimit(Px1DConstraint* c, const PxJointLimitParameters& limit)
			{
				PxU16 flags = PxU16(c->flags | Px1DConstraintFlag::eOUTPUT_FORCE);

				if(limit.isSoft())
				{
					flags |= Px1DConstraintFlag::eSPRING;
					c->mods.spring.stiffness = limit.stiffness;
					c->mods.spring.damping = limit.damping;
				}
				else
				{
					c->solveHint = PxConstraintSolveHint::eINEQUALITY;
					c->mods.bounce.restitution = limit.restitution;
					c->mods.bounce.velocityThreshold = limit.bounceThreshold;

					if (c->geometricError > 0.0f)
					{
						flags |= Px1DConstraintFlag::eKEEPBIAS;
						// note: positive error is the scenario where the limit is not hit yet. It reflects the
						// distance to the limit. Using eKEEPBIAS feels unintuitive in general but what seems to
						// be solved with this is:
						//
						// imagine the following scenario: object o moving towards a limit with velocity v
						// 
						//                  |
						// o---> v          |
						//                  |
						//
						// and let's denote the following distances
						// 
						// |<-------->|  |v|*dt  (travel distance assuming time step dt)
						// |<-------------->|  |ge| (distance to limit = geometric error)
						// 
						// furthermore, the sign convention is that v as drawn here is negative and ge is
						// positive. Since -v*dt is smaller than ge, the limit will not get hit in the dt time
						// step range. This means, the velocity after the sim step should not change and remain v.
						// For the solver this means no impulse should get applied.
						// The impulse applied by the solver is of the form:
						// 
						// impulse = -r * ((v - vT) + ge/dt)  (r is a positive scalar value)
						// 
						// for this example, let's assume the target velocity vT is zero, so:
						// 
						// impulse = -r * (v + ge/dt)  (1)
						// 
						// Without Px1DConstraintFlag::eKEEPBIAS, the part related to the geometric error is ignored
						// during velocity iterations:
						//
						// impulse = -r * v
						// 
						// The solver will apply the resulting (positive) impulse and this will change the velocity
						// of the object. That would be wrong though because the object does not hit the limit yet
						// and the velocity should stay the same.
						// 
						// Why does Px1DConstraintFlag::eKEEPBIAS prevent this from happening? In this case, equation
						// (1) applies and since -v*dt < ge, the resulting impulse will be negative ((v + ge/dt) is
						// positive). Limit constraints are inequality constraints and clamp the impulse in the range
						// [0, maxImpulse], thus the negative impulse will get clamped to zero and the velocity will
						// not change (as desired).
						// 
						// Why then create this constraint at all? Imagine the same scenario but with a velocity
						// magnitude such that the limit gets hit in the dt time step range:
						// 
						// |<--------------------->|  |v|*dt
						// |<-------------->|  |ge|
						// 
						// (v + ge/dt) will be negative and the impulse positive. The impulse will get applied and
						// will make sure that the velocity is reduced by the right amount such that the object
						// stops at the limit (and does not breach it).
					}

					if(limit.restitution>0.0f)
						flags |= Px1DConstraintFlag::eRESTITUTION;
				}

				c->flags = flags;
				c->minImpulse = 0.0f;
			}

			void addDrive(Px1DConstraint* c, PxReal velTarget, const PxD6JointDrive& drive)
			{
				c->velocityTarget = velTarget;

				PxU16 flags = PxU16(c->flags | Px1DConstraintFlag::eSPRING | Px1DConstraintFlag::eHAS_DRIVE_LIMIT);
				
				if(drive.flags & PxD6JointDriveFlag::eACCELERATION)
					flags |= Px1DConstraintFlag::eACCELERATION_SPRING;
				
				if (drive.flags & PxD6JointDriveFlag::eOUTPUT_FORCE)
					flags |= Px1DConstraintFlag::eOUTPUT_FORCE;

				c->flags = flags;
				c->mods.spring.stiffness = drive.stiffness;
				c->mods.spring.damping = drive.damping;
				
				c->minImpulse = -drive.forceLimit;
				c->maxImpulse = drive.forceLimit;

				PX_ASSERT(c->linear0.isFinite());
				PX_ASSERT(c->angular0.isFinite());
			}
		};
	}
} // namespace

}

#endif
