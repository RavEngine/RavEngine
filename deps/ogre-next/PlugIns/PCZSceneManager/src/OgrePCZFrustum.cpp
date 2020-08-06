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
OgrePCZFrustum.cpp  -  PCZ Supplemental culling Frustum 

This isn't really a traditional "frustum", but more a collection of
extra culling planes used by the PCZ Scene Manager for supplementing
the camera culling and light zone culling by creating extra culling
planes from visible portals.  Since portals are 4 sided, the extra 
culling planes tend to form frustums (pyramids) but nothing in the 
code really assumes that the culling planes are frustums.  They are
just treated as planes.  

The "originPlane" is a culling plane which passes through the origin
point specified.  It is used to cull portals which are close to, but
behind the camera view.  (the nature of the culling routine doesn't
give correct results if you just use the "near" plane of the standard
camera frustum (unless that near plane distance is 0.0, but that is
highly not recommended for other reasons having to do with having a 
legal view frustum).

-----------------------------------------------------------------------------
begin                : Tue May 29 2007
author               : Eric Cha
email                : ericc@xenopi.com
-----------------------------------------------------------------------------
*/

#include "OgrePCZFrustum.h"
#include "OgrePCPlane.h"
#include "OgreAntiPortal.h"
#include "OgrePortal.h"

namespace Ogre
{

    PCZFrustum::PCZFrustum() :
    mUseOriginPlane(false), mProjType(PT_PERSPECTIVE)
    { }

    PCZFrustum::~PCZFrustum()
    {
        removeAllCullingPlanes();
        // clear out the culling plane reservoir
        PCPlaneList::iterator pit = mCullingPlaneReservoir.begin();
        while ( pit != mCullingPlaneReservoir.end() )
        {
            PCPlane * plane = *pit;
            // go to next entry
            pit++;
            //delete the entry in the list
            OGRE_DELETE_T(plane, PCPlane, MEMCATEGORY_SCENE_CONTROL);
        }
        mCullingPlaneReservoir.clear();
    }

    bool PCZFrustum::isVisible( const AxisAlignedBox & bound) const
    {
        // Null boxes are always invisible
        if (bound.isNull()) return false;

        // Infinite boxes are always visible
        if (bound.isInfinite()) return true;

        // Get centre of the box
        Vector3 centre = bound.getCenter();
        // Get the half-size of the box
        Vector3 halfSize = bound.getHalfSize();

        // Check originplane if told to
        if (mUseOriginPlane)
        {
            Plane::Side side = mOriginPlane.getSide(centre, halfSize);
            if (side == Plane::NEGATIVE_SIDE)
            {
                return false;
            }
        }

        // For each extra active culling plane, see if the entire aabb is on the negative side
        // If so, object is not visible
        PCPlaneList::const_iterator pit = mActiveCullingPlanes.begin();
        while ( pit != mActiveCullingPlanes.end() )
        {
            PCPlane * plane = *pit;
            Plane::Side xside = plane->getSide(centre, halfSize);
            if (xside == Plane::NEGATIVE_SIDE)
            {
                return false;
            }
            pit++;
        }
        return true;
    }

    bool PCZFrustum::isVisible( const Sphere & bound) const
    {
        // Check originplane if told to
        if (mUseOriginPlane)
        {
            Plane::Side side = mOriginPlane.getSide(bound.getCenter());
            if (side == Plane::NEGATIVE_SIDE)
            {
                Real dist = mOriginPlane.getDistance(bound.getCenter());
                if (dist > bound.getRadius())
                {
                    return false;
                }
            }
        }

        // For each extra active culling plane, see if the entire sphere is on the negative side
        // If so, object is not visible
        PCPlaneList::const_iterator pit = mActiveCullingPlanes.begin();
        while ( pit != mActiveCullingPlanes.end() )
        {
            PCPlane * plane = *pit;
            Plane::Side xside = plane->getSide(bound.getCenter());
            if (xside == Plane::NEGATIVE_SIDE)
            {
                Real dist = plane->getDistance(bound.getCenter());
                if (dist > bound.getRadius())
                {
                    return false;
                }
            }
            pit++;
        }
        return true;
    }

    /* isVisible() function for portals */
    // NOTE: Everything needs to be updated spatially before this function is
    //       called including portal corners, frustum planes, etc.
    bool PCZFrustum::isVisible(const PortalBase* portal) const
    {
        // if portal isn't enabled, it's not visible
        if (!portal->getEnabled()) return false;

        // if the frustum has no planes, just return true
        if (mActiveCullingPlanes.empty())
        {
            return true;
        }
        // check if this portal is already in the list of active culling planes (avoid
        // infinite recursion case)
        PCPlaneList::const_iterator pit = mActiveCullingPlanes.begin();
        while ( pit != mActiveCullingPlanes.end() )
        {
            PCPlane * plane = *pit;
            if (plane->getPortal() == portal)
            {
                return false;
            }
            pit++;
        }
        // if portal is of type AABB or Sphere, then use simple bound check against planes
        if (portal->getType() == PortalBase::PORTAL_TYPE_AABB)
        {
            AxisAlignedBox aabb;
            aabb.setExtents(portal->getDerivedCorner(0), portal->getDerivedCorner(1));
            return isVisible(aabb);
        }
        else if (portal->getType() == PortalBase::PORTAL_TYPE_SPHERE)
        {
            return isVisible(portal->getDerivedSphere());
        }

        // only do this check if it's a portal. (anti portal doesn't care about facing)
        if (portal->getTypeFlags() == PortalFactory::FACTORY_TYPE_FLAG)
        {
            // check if the portal norm is facing the frustum
            Vector3 frustumToPortal = portal->getDerivedCP() - mOrigin;
            Vector3 portalDirection = portal->getDerivedDirection();
            Real dotProduct = frustumToPortal.dotProduct(portalDirection);
            if ( dotProduct > 0 )
            {
                // portal is faced away from Frustum 
                return false;
            }
        }

        // check against frustum culling planes
        bool visible_flag;

        // Check originPlane if told to
        if (mUseOriginPlane)
        {
            // set the visible flag to false
            visible_flag = false;
            // we have to check each corner of the portal
            for (int corner = 0; corner < 4; corner++)
            {
                Plane::Side side = mOriginPlane.getSide(portal->getDerivedCorner(corner));
                if (side != Plane::NEGATIVE_SIDE)
                {
                    visible_flag = true;
                    break;
                }
            }
            // if the visible_flag is still false, then the origin plane
            // culled all the portal points
            if (visible_flag == false)
            {
                // ALL corners on negative side therefore out of view
                return false;
            }
        }

        // For each active culling plane, see if all portal points are on the negative 
        // side. If so, the portal is not visible
        pit = mActiveCullingPlanes.begin();
        while ( pit != mActiveCullingPlanes.end() )
        {
            PCPlane * plane = *pit;
            // set the visible flag to false
            visible_flag = false;
            // we have to check each corner of the portal
            for (int corner = 0; corner < 4; corner++)
            {
                Plane::Side side =plane->getSide(portal->getDerivedCorner(corner));
                if (side != Plane::NEGATIVE_SIDE)
                {
                    visible_flag = true;
                    break;
                }
            }
            // if the visible_flag is still false, then this plane
            // culled all the portal points
            if (visible_flag == false)
            {
                // ALL corners on negative side therefore out of view
                return false;
            }
            pit++;
        }
        // no plane culled all the portal points and the norm
        // was facing the frustum, so this portal is visible
        return true;

    }

    /* special function that returns true only when aabb fully fits inside the frustum. */
    bool PCZFrustum::isFullyVisible(const AxisAlignedBox& bound) const
    {
        // Null boxes are always invisible
        if (bound.isNull()) return false;

        // Infinite boxes are never fully visible
        if (bound.isInfinite()) return false;

        // Get centre of the box
        Vector3 centre = bound.getCenter();
        // Get the half-size of the box
        Vector3 halfSize = bound.getHalfSize();

        // Check originplane if told to
        if (mUseOriginPlane)
        {
            Plane::Side side = mOriginPlane.getSide(centre, halfSize);
            if (side != Plane::POSITIVE_SIDE) return false;
        }

        // For each extra active culling plane,
        // see if the aabb is not on the positive side
        // If so, object is not fully visible
        PCPlaneList::const_iterator pit = mActiveCullingPlanes.begin();
        while ( pit != mActiveCullingPlanes.end() )
        {
            PCPlane * plane = *pit;
            Plane::Side xside = plane->getSide(centre, halfSize);
            if (xside != Plane::POSITIVE_SIDE)
            {
                return false;
            }
            pit++;
        }
        return true;
    }

    /* special function that returns true only when sphere fully fits inside the frustum. */
    bool PCZFrustum::isFullyVisible(const Sphere& bound) const
    {
        // Check originplane if told to
        if (mUseOriginPlane)
        {
            if (mOriginPlane.getDistance(bound.getCenter()) <= bound.getRadius() ||
                mOriginPlane.getSide(bound.getCenter()) != Plane::POSITIVE_SIDE)
            {
                return false;
            }
        }

        // For each extra active culling plane,
        // see if the sphere is not on the positive side
        // If so, object is not fully visible
        PCPlaneList::const_iterator pit = mActiveCullingPlanes.begin();
        while ( pit != mActiveCullingPlanes.end() )
        {
            PCPlane* plane = *pit;

            if (plane->getDistance(bound.getCenter()) <= bound.getRadius() ||
                plane->getSide(bound.getCenter()) != Plane::POSITIVE_SIDE)
            {
                return false;
            }

            pit++;
        }
        return true;
    }

    /* special function that returns true only when portal fully fits inside the frustum. */
    bool PCZFrustum::isFullyVisible(const PortalBase* portal) const
    {
        // if portal isn't enabled, it's not visible
        if (!portal->getEnabled()) return false;

        // if the frustum has no planes, just return true
        if (mActiveCullingPlanes.empty())
        {
            return true;
        }
        // check if this portal is already in the list of active culling planes (avoid
        // infinite recursion case)
        PCPlaneList::const_iterator pit = mActiveCullingPlanes.begin();
        while ( pit != mActiveCullingPlanes.end() )
        {
            PCPlane * plane = *pit;
            if (plane->getPortal() == portal)
            {
                return false;
            }
            pit++;
        }
        // if portal is of type AABB or Sphere, then use simple bound check against planes
        if (portal->getType() == PortalBase::PORTAL_TYPE_AABB)
        {
            AxisAlignedBox aabb;
            aabb.setExtents(portal->getDerivedCorner(0), portal->getDerivedCorner(1));
            return isFullyVisible(aabb);
        }
        else if (portal->getType() == PortalBase::PORTAL_TYPE_SPHERE)
        {
            return isFullyVisible(portal->getDerivedSphere());
        }

        // only do this check if it's a portal. (anti portal doesn't care about facing)
        if (portal->getTypeFlags() == PortalFactory::FACTORY_TYPE_FLAG)
        {
            // check if the portal norm is facing the frustum
            Vector3 frustumToPortal = portal->getDerivedCP() - mOrigin;
            Vector3 portalDirection = portal->getDerivedDirection();
            Real dotProduct = frustumToPortal.dotProduct(portalDirection);
            if ( dotProduct > 0 )
            {
                // portal is faced away from Frustum 
                return false;
            }
        }

        // Check originPlane if told to
        if (mUseOriginPlane)
        {
            // we have to check each corner of the portal
            for (int corner = 0; corner < 4; corner++)
            {
                Plane::Side side = mOriginPlane.getSide(portal->getDerivedCorner(corner));
                if (side == Plane::NEGATIVE_SIDE) return false;
            }
        }

        // For each active culling plane, see if any portal points are on the negative 
        // side. If so, the portal is not fully visible
        pit = mActiveCullingPlanes.begin();
        while ( pit != mActiveCullingPlanes.end() )
        {
            PCPlane * plane = *pit;
            // we have to check each corner of the portal
            for (int corner = 0; corner < 4; corner++)
            {
                Plane::Side side =plane->getSide(portal->getDerivedCorner(corner));
                if (side == Plane::NEGATIVE_SIDE) return false;
            }
            pit++;
        }
        // no plane culled all the portal points and the norm
        // was facing the frustum, so this portal is fully visible
        return true;
    }

    /* A 'more detailed' check for visibility of an AAB.  This function returns
      none, partial, or full for visibility of the box.  This is useful for 
      stuff like Octree leaf culling */
    PCZFrustum::Visibility PCZFrustum::getVisibility( const AxisAlignedBox &bound )
    {

        // Null boxes always invisible
        if ( bound.isNull() )
            return NONE;

        // Get centre of the box
        Vector3 centre = bound.getCenter();
        // Get the half-size of the box
        Vector3 halfSize = bound.getHalfSize();

        bool all_inside = true;

        // Check originplane if told to
        if (mUseOriginPlane)
        {
            Plane::Side side = mOriginPlane.getSide(centre, halfSize);
            if (side == Plane::NEGATIVE_SIDE)
            {
                return NONE;
            }
            // We can't return now as the box could be later on the negative side of another plane.
            if(side == Plane::BOTH_SIDE) 
            {
                all_inside = false;
            }
        }

        // For each active culling plane, see if the entire aabb is on the negative side
        // If so, object is not visible
        PCPlaneList::iterator pit = mActiveCullingPlanes.begin();
        while ( pit != mActiveCullingPlanes.end() )
        {
            PCPlane * plane = *pit;
            Plane::Side xside = plane->getSide(centre, halfSize);
            if(xside == Plane::NEGATIVE_SIDE) 
            {
                return NONE;
            }
            // We can't return now as the box could be later on the negative side of a plane.
            if(xside == Plane::BOTH_SIDE) 
            {
                all_inside = false;
                break;
            }
            pit++;
        }

        if ( all_inside )
            return FULL;
        else
            return PARTIAL;

    }

    // calculate  culling planes from portal and frustum 
    // origin and add to list of  culling planes
    // NOTE: returns 0 if portal was completely culled by existing planes
    //       returns > 0 if culling planes are added (# is planes added)
    int PCZFrustum::addPortalCullingPlanes(PortalBase* portal)
    {
        int addedcullingplanes = 0;

        // If portal is of type aabb or sphere, add a plane which is same as frustum
        // origin plane (ie. redundant).  We do this because we need the plane as a flag
        // to prevent infinite recursion 
        if (portal->getType() == PortalBase::PORTAL_TYPE_AABB ||
            portal->getType() == PortalBase::PORTAL_TYPE_SPHERE)
        {
            PCPlane * newPlane = getUnusedCullingPlane();
            newPlane->setFromOgrePlane(mOriginPlane);
            newPlane->setPortal(portal);
            mActiveCullingPlanes.push_front(newPlane);
            addedcullingplanes++;
            return addedcullingplanes;
        }

        // only do this check if it's an anti portal since it's double facing.
        bool flipPlane = false;
        if (portal->getTypeFlags() == AntiPortalFactory::FACTORY_TYPE_FLAG)
        {
            // check if the portal norm is facing the frustum
            Vector3 frustumToPortal = portal->getDerivedCP() - mOrigin;
            Vector3 portalDirection = portal->getDerivedDirection();
            Real dotProduct = frustumToPortal.dotProduct(portalDirection);

            // it's facing away from the frustum. Flip the planes.
            if (dotProduct > 0) flipPlane = true;
        }

        // For portal Quads: Up to 4 planes can be added by the sides of a portal quad.
        // Each plane is created from 2 corners (world space) of the portal and the
        // frustum origin (world space).
        int i,j;
        Plane::Side pt0_side, pt1_side;
        bool visible;
        PCPlaneList::iterator pit;
        for (i=0;i<4;i++)
        {
            // first check if both corners are outside of one of the existing planes
            j = i+1;
            if (j > 3)
            {
                j = 0;
            }
            visible = true;
            pit = mActiveCullingPlanes.begin();
            while ( pit != mActiveCullingPlanes.end() )
            {
                PCPlane * plane = *pit;
                pt0_side = plane->getSide(portal->getDerivedCorner(i));
                pt1_side = plane->getSide(portal->getDerivedCorner(j));
                if (pt0_side == Plane::NEGATIVE_SIDE &&
                    pt1_side == Plane::NEGATIVE_SIDE)
                {
                    // the portal edge was actually completely culled by one of  culling planes
                    visible = false;
                    break;
                }
                pit++;
            }
            if (visible)
            {
                // add the plane created from the two portal corner points and the frustum location
                // to the  culling plane
                PCPlane * newPlane = getUnusedCullingPlane();
                if (mProjType == PT_ORTHOGRAPHIC) // use camera direction if projection is orthographic.
                {
                    if (flipPlane)
                    {
                        newPlane->redefine(portal->getDerivedCorner(j) + mOriginPlane.normal,
                            portal->getDerivedCorner(i), portal->getDerivedCorner(j));
                    }
                    else
                    {
                        newPlane->redefine(portal->getDerivedCorner(j) + mOriginPlane.normal,
                            portal->getDerivedCorner(j), portal->getDerivedCorner(i));
                    }
                }
                else
                {
                    if (flipPlane)
                    {
                        newPlane->redefine(mOrigin,
                            portal->getDerivedCorner(i),
                            portal->getDerivedCorner(j));
                    }
                    else
                    {
                        newPlane->redefine(mOrigin,
                            portal->getDerivedCorner(j),
                            portal->getDerivedCorner(i));
                    }
                }
                newPlane->setPortal(portal);
                mActiveCullingPlanes.push_front(newPlane);
                addedcullingplanes++;
            }
        }
        // if we added ANY planes from the quad portal, we should add the plane of the
        // portal itself as an additional culling plane.
        if (addedcullingplanes > 0)
        {
            PCPlane * newPlane = getUnusedCullingPlane();

            if (flipPlane)
            {
                newPlane->redefine(
                    portal->getDerivedCorner(2),
                    portal->getDerivedCorner(0),
                    portal->getDerivedCorner(1));
            }
            else
            {
                newPlane->redefine(
                    portal->getDerivedCorner(2),
                    portal->getDerivedCorner(1),
                    portal->getDerivedCorner(0));
            }

            newPlane->setPortal(portal);
            mActiveCullingPlanes.push_back(newPlane);
            addedcullingplanes++;
        }
        return addedcullingplanes;
    }

    // remove culling planes created from the given portal
    void PCZFrustum::removePortalCullingPlanes(PortalBase* portal)
    {
        PCPlaneList::iterator pit = mActiveCullingPlanes.begin();
        while ( pit != mActiveCullingPlanes.end() )
        {
            PCPlane * plane = *pit;
            if (plane->getPortal() == portal)
            {
                // put the plane back in the reservoir
                mCullingPlaneReservoir.push_front(plane);
                // erase the entry from the active culling plane list
                pit = mActiveCullingPlanes.erase(pit);
            }
            else
            {
                pit++;
            }
        }

    }

    // remove all active extra culling planes
    // NOTE: Does not change the use of the originPlane!
    void PCZFrustum::removeAllCullingPlanes(void)
    {
        PCPlaneList::iterator pit = mActiveCullingPlanes.begin();
        while ( pit != mActiveCullingPlanes.end() )
        {
            PCPlane * plane = *pit;
            // put the plane back in the reservoir
            mCullingPlaneReservoir.push_front(plane);
            // go to next entry
            pit++;
        }
        mActiveCullingPlanes.clear();
    }

    // set the origin plane
    void PCZFrustum::setOriginPlane(const Vector3 &rkNormal, const Vector3 &rkPoint)
    {
        mOriginPlane.redefine(rkNormal, rkPoint);
    }

    // get an unused PCPlane from the CullingPlane Reservoir
    // note that this removes the PCPlane from the reservoir!
    PCPlane * PCZFrustum::getUnusedCullingPlane(void)
    {
        PCPlane * plane = 0;
        if (mCullingPlaneReservoir.size() > 0)
        {
            PCPlaneList::iterator pit = mCullingPlaneReservoir.begin();
            plane = *pit;
            mCullingPlaneReservoir.erase(pit);
            return plane;
        }
        // no available planes! create one
        plane = OGRE_NEW_T(PCPlane, MEMCATEGORY_SCENE_CONTROL);
        return plane;
    }

}
