/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
OgreSegment.cpp  -  3D Line Segment class for intersection testing in Ogre3D
Some algorithms based off code from the Wild Magic library by Dave Eberly
-----------------------------------------------------------------------------
begin                : Mon Apr 02 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update    : 
-----------------------------------------------------------------------------
*/

#include "OgreSegment.h"
#include "OgreCapsule.h"

using namespace Ogre;

const Real PARALLEL_TOLERANCE = 0.0001;

//----------------------------------------------------------------------------
Segment::Segment ()
{
    // uninitialized
}
//----------------------------------------------------------------------------
Segment::Segment (const Vector3& origin,
                    const Vector3& direction, 
                    Real extent)
    :
    mOrigin(origin),
    mDirection(direction),
    mExtent(extent)
{
}
//----------------------------------------------------------------------------
void Segment::set(const Vector3& newOrigin, const Vector3& newEnd)
{
    mOrigin = newOrigin;
    // calc the direction vector
    mDirection = newEnd - mOrigin;
    mExtent = mDirection.normalise();
}
//----------------------------------------------------------------------------
void Segment::setOrigin(const Vector3& newOrigin)
{
    mOrigin = newOrigin;
}
//----------------------------------------------------------------------------
void Segment::setEndPoint(const Vector3& newEnd)
{
    // calc the direction vector
    mDirection = newEnd - mOrigin;
    mExtent = mDirection.normalise();
}
//----------------------------------------------------------------------------
Real Segment::distance(const Segment& otherSegment) const
{
    Real fSqrDist = squaredDistance(otherSegment);
    return Ogre::Math::Sqrt(fSqrDist);
}
//----------------------------------------------------------------------------
Real Segment::squaredDistance(const Segment& otherSegment) const
{
    Vector3 kDiff = mOrigin - otherSegment.mOrigin;
    Real fA01 = -mDirection.dotProduct(otherSegment.mDirection);
    Real fB0 = kDiff.dotProduct(mDirection);
    Real fB1 = -kDiff.dotProduct(otherSegment.mDirection);
    Real fC = kDiff.squaredLength();
    Real fDet = Math::Abs((Real)1.0 - fA01*fA01);
    Real fS0, fS1, fSqrDist, fExtDet0, fExtDet1, fTmpS0, fTmpS1;

    if (fDet >= PARALLEL_TOLERANCE)
    {
        // segments are not parallel
        fS0 = fA01*fB1-fB0;
        fS1 = fA01*fB0-fB1;
        fExtDet0 = mExtent*fDet;
        fExtDet1 = otherSegment.mExtent*fDet;

        if (fS0 >= -fExtDet0)
        {
            if (fS0 <= fExtDet0)
            {
                if (fS1 >= -fExtDet1)
                {
                    if (fS1 <= fExtDet1)  // region 0 (interior)
                    {
                        // minimum at two interior points of 3D lines
                        Real fInvDet = ((Real)1.0)/fDet;
                        fS0 *= fInvDet;
                        fS1 *= fInvDet;
                        fSqrDist = fS0*(fS0+fA01*fS1+((Real)2.0)*fB0) +
                            fS1*(fA01*fS0+fS1+((Real)2.0)*fB1)+fC;
                    }
                    else  // region 3 (side)
                    {
                        fS1 = otherSegment.mExtent;
                        fTmpS0 = -(fA01*fS1+fB0);
                        if (fTmpS0 < -mExtent)
                        {
                            fS0 = -mExtent;
                            fSqrDist = fS0*(fS0-((Real)2.0)*fTmpS0) +
                                fS1*(fS1+((Real)2.0)*fB1)+fC;
                        }
                        else if (fTmpS0 <= mExtent)
                        {
                            fS0 = fTmpS0;
                            fSqrDist = -fS0*fS0+fS1*(fS1+((Real)2.0)*fB1)+fC;
                        }
                        else
                        {
                            fS0 = mExtent;
                            fSqrDist = fS0*(fS0-((Real)2.0)*fTmpS0) +
                                fS1*(fS1+((Real)2.0)*fB1)+fC;
                        }
                    }
                }
                else  // region 7 (side)
                {
                    fS1 = -otherSegment.mExtent;
                    fTmpS0 = -(fA01*fS1+fB0);
                    if (fTmpS0 < -mExtent)
                    {
                        fS0 = -mExtent;
                        fSqrDist = fS0*(fS0-((Real)2.0)*fTmpS0) +
                            fS1*(fS1+((Real)2.0)*fB1)+fC;
                    }
                    else if (fTmpS0 <= mExtent)
                    {
                        fS0 = fTmpS0;
                        fSqrDist = -fS0*fS0+fS1*(fS1+((Real)2.0)*fB1)+fC;
                    }
                    else
                    {
                        fS0 = mExtent;
                        fSqrDist = fS0*(fS0-((Real)2.0)*fTmpS0) +
                            fS1*(fS1+((Real)2.0)*fB1)+fC;
                    }
                }
            }
            else
            {
                if (fS1 >= -fExtDet1)
                {
                    if (fS1 <= fExtDet1)  // region 1 (side)
                    {
                        fS0 = mExtent;
                        fTmpS1 = -(fA01*fS0+fB1);
                        if (fTmpS1 < -otherSegment.mExtent)
                        {
                            fS1 = -otherSegment.mExtent;
                            fSqrDist = fS1*(fS1-((Real)2.0)*fTmpS1) +
                                fS0*(fS0+((Real)2.0)*fB0)+fC;
                        }
                        else if (fTmpS1 <= otherSegment.mExtent)
                        {
                            fS1 = fTmpS1;
                            fSqrDist = -fS1*fS1+fS0*(fS0+((Real)2.0)*fB0)+fC;
                        }
                        else
                        {
                            fS1 = otherSegment.mExtent;
                            fSqrDist = fS1*(fS1-((Real)2.0)*fTmpS1) +
                                fS0*(fS0+((Real)2.0)*fB0)+fC;
                        }
                    }
                    else  // region 2 (corner)
                    {
                        fS1 = otherSegment.mExtent;
                        fTmpS0 = -(fA01*fS1+fB0);
                        if (fTmpS0 < -mExtent)
                        {
                            fS0 = -mExtent;
                            fSqrDist = fS0*(fS0-((Real)2.0)*fTmpS0) +
                                fS1*(fS1+((Real)2.0)*fB1)+fC;
                        }
                        else if (fTmpS0 <= mExtent)
                        {
                            fS0 = fTmpS0;
                            fSqrDist = -fS0*fS0+fS1*(fS1+((Real)2.0)*fB1)+fC;
                        }
                        else
                        {
                            fS0 = mExtent;
                            fTmpS1 = -(fA01*fS0+fB1);
                            if (fTmpS1 < -otherSegment.mExtent)
                            {
                                fS1 = -otherSegment.mExtent;
                                fSqrDist = fS1*(fS1-((Real)2.0)*fTmpS1) +
                                    fS0*(fS0+((Real)2.0)*fB0)+fC;
                            }
                            else if (fTmpS1 <= otherSegment.mExtent)
                            {
                                fS1 = fTmpS1;
                                fSqrDist = -fS1*fS1+fS0*(fS0+((Real)2.0)*fB0)
                                    + fC;
                            }
                            else
                            {
                                fS1 = otherSegment.mExtent;
                                fSqrDist = fS1*(fS1-((Real)2.0)*fTmpS1) +
                                    fS0*(fS0+((Real)2.0)*fB0)+fC;
                            }
                        }
                    }
                }
                else  // region 8 (corner)
                {
                    fS1 = -otherSegment.mExtent;
                    fTmpS0 = -(fA01*fS1+fB0);
                    if (fTmpS0 < -mExtent)
                    {
                        fS0 = -mExtent;
                        fSqrDist = fS0*(fS0-((Real)2.0)*fTmpS0) +
                            fS1*(fS1+((Real)2.0)*fB1)+fC;
                    }
                    else if (fTmpS0 <= mExtent)
                    {
                        fS0 = fTmpS0;
                        fSqrDist = -fS0*fS0+fS1*(fS1+((Real)2.0)*fB1)+fC;
                    }
                    else
                    {
                        fS0 = mExtent;
                        fTmpS1 = -(fA01*fS0+fB1);
                        if (fTmpS1 > otherSegment.mExtent)
                        {
                            fS1 = otherSegment.mExtent;
                            fSqrDist = fS1*(fS1-((Real)2.0)*fTmpS1) +
                                fS0*(fS0+((Real)2.0)*fB0)+fC;
                        }
                        else if (fTmpS1 >= -otherSegment.mExtent)
                        {
                            fS1 = fTmpS1;
                            fSqrDist = -fS1*fS1+fS0*(fS0+((Real)2.0)*fB0)
                                + fC;
                        }
                        else
                        {
                            fS1 = -otherSegment.mExtent;
                            fSqrDist = fS1*(fS1-((Real)2.0)*fTmpS1) +
                                fS0*(fS0+((Real)2.0)*fB0)+fC;
                        }
                    }
                }
            }
        }
        else 
        {
            if (fS1 >= -fExtDet1)
            {
                if (fS1 <= fExtDet1)  // region 5 (side)
                {
                    fS0 = -mExtent;
                    fTmpS1 = -(fA01*fS0+fB1);
                    if (fTmpS1 < -otherSegment.mExtent)
                    {
                        fS1 = -otherSegment.mExtent;
                        fSqrDist = fS1*(fS1-((Real)2.0)*fTmpS1) +
                            fS0*(fS0+((Real)2.0)*fB0)+fC;
                    }
                    else if (fTmpS1 <= otherSegment.mExtent)
                    {
                        fS1 = fTmpS1;
                        fSqrDist = -fS1*fS1+fS0*(fS0+((Real)2.0)*fB0)+fC;
                    }
                    else
                    {
                        fS1 = otherSegment.mExtent;
                        fSqrDist = fS1*(fS1-((Real)2.0)*fTmpS1) +
                            fS0*(fS0+((Real)2.0)*fB0)+fC;
                    }
                }
                else  // region 4 (corner)
                {
                    fS1 = otherSegment.mExtent;
                    fTmpS0 = -(fA01*fS1+fB0);
                    if (fTmpS0 > mExtent)
                    {
                        fS0 = mExtent;
                        fSqrDist = fS0*(fS0-((Real)2.0)*fTmpS0) +
                            fS1*(fS1+((Real)2.0)*fB1)+fC;
                    }
                    else if (fTmpS0 >= -mExtent)
                    {
                        fS0 = fTmpS0;
                        fSqrDist = -fS0*fS0+fS1*(fS1+((Real)2.0)*fB1)+fC;
                    }
                    else
                    {
                        fS0 = -mExtent;
                        fTmpS1 = -(fA01*fS0+fB1);
                        if (fTmpS1 < -otherSegment.mExtent)
                        {
                            fS1 = -otherSegment.mExtent;
                            fSqrDist = fS1*(fS1-((Real)2.0)*fTmpS1) +
                                fS0*(fS0+((Real)2.0)*fB0)+fC;
                        }
                        else if (fTmpS1 <= otherSegment.mExtent)
                        {
                            fS1 = fTmpS1;
                            fSqrDist = -fS1*fS1+fS0*(fS0+((Real)2.0)*fB0)
                                + fC;
                        }
                        else
                        {
                            fS1 = otherSegment.mExtent;
                            fSqrDist = fS1*(fS1-((Real)2.0)*fTmpS1) +
                                fS0*(fS0+((Real)2.0)*fB0)+fC;
                        }
                    }
                }
            }
            else   // region 6 (corner)
            {
                fS1 = -otherSegment.mExtent;
                fTmpS0 = -(fA01*fS1+fB0);
                if (fTmpS0 > mExtent)
                {
                    fS0 = mExtent;
                    fSqrDist = fS0*(fS0-((Real)2.0)*fTmpS0) +
                        fS1*(fS1+((Real)2.0)*fB1)+fC;
                }
                else if (fTmpS0 >= -mExtent)
                {
                    fS0 = fTmpS0;
                    fSqrDist = -fS0*fS0+fS1*(fS1+((Real)2.0)*fB1)+fC;
                }
                else
                {
                    fS0 = -mExtent;
                    fTmpS1 = -(fA01*fS0+fB1);
                    if (fTmpS1 < -otherSegment.mExtent)
                    {
                        fS1 = -otherSegment.mExtent;
                        fSqrDist = fS1*(fS1-((Real)2.0)*fTmpS1) +
                            fS0*(fS0+((Real)2.0)*fB0)+fC;
                    }
                    else if (fTmpS1 <= otherSegment.mExtent)
                    {
                        fS1 = fTmpS1;
                        fSqrDist = -fS1*fS1+fS0*(fS0+((Real)2.0)*fB0)
                            + fC;
                    }
                    else
                    {
                        fS1 = otherSegment.mExtent;
                        fSqrDist = fS1*(fS1-((Real)2.0)*fTmpS1) +
                            fS0*(fS0+((Real)2.0)*fB0)+fC;
                    }
                }
            }
        }
    }
    else
    {
        // The segments are parallel.  The average b0 term is designed to
        // ensure symmetry of the function.  That is, dist(seg0,seg1) and
        // dist(seg1,seg0) should produce the same number.
        Real fE0pE1 = mExtent + otherSegment.mExtent;
        Real fSign = (fA01 > (Real)0.0 ? (Real)-1.0 : (Real)1.0);
        Real fB0Avr = ((Real)0.5)*(fB0 - fSign*fB1);
        Real fLambda = -fB0Avr;
        if (fLambda < -fE0pE1)
        {
            fLambda = -fE0pE1;
        }
        else if (fLambda > fE0pE1)
        {
            fLambda = fE0pE1;
        }

//        fS1 = -fSign*fLambda*otherSegment.mExtent/fE0pE1;
//        fS0 = fLambda + fSign*fS1;
        fSqrDist = fLambda*(fLambda + ((Real)2.0)*fB0Avr) + fC;
    }
    // we don't need the following stuff - it's for calculating closest point
//    mClosestPoint0 = mOrigin + fS0*mDirection;
//    mClosestPoint1 = otherSegment.mOrigin + fS1*otherSegment.mDirection;
//    mSegment0Parameter = fS0;
//    mSegment1Parameter = fS1;
    return Math::Abs(fSqrDist);
}

//----------------------------------------------------------------------------
bool Segment::intersects(const Capsule &capsule) const
{
    Real fDist =  distance(capsule.mSegment);
    return fDist <= capsule.mRadius;
}
