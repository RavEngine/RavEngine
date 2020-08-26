/*
 * Spherical.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_SPHERICAL_H
#define GS_SPHERICAL_H


#include "Decl.h"
#include "Real.h"
#include "Tags.h"

#include <cmath>


namespace Gs
{


/**
\brief Spherical coordinate class with components: radius, theta, phi.
\tparam T Specifies the data type of the vector components.
This should be a primitive data type such as float, double, int etc.
\remarks To use operators such as +, - etc. convert it to a Vector3.
A spherical coordinate with a radius of 1, and both theta and phi equal to 0 will result in a Vector3 (0, 0, 1),
i.e. pointing towards the Z coordinate.
\see Vector3
*/
template <typename T>
class SphericalT
{

    public:

        #ifndef GS_DISABLE_AUTO_INIT
        SphericalT() :
            radius { T(0) },
            theta  { T(0) },
            phi    { T(0) }
        {
        }
        #else
        SphericalT() = default;
        #endif

        SphericalT(const SphericalT<T>& rhs) :
            radius { rhs.radius },
            theta  { rhs.theta  },
            phi    { rhs.phi    }
        {
        }

        SphericalT(const T& radius, const T& theta, const T& phi) :
            radius { radius },
            theta  { theta  },
            phi    { phi    }
        {
        }

        /**
        \brief Converts the specified cartesian coordinate into spherical coordinate.
        \remarks The implementation of this constructor is included in the "Appendix.h" file.
        */
        explicit SphericalT(const Vector<T, 3>& cartesianCoord)
        {
            radius = cartesianCoord.Length();
            if (radius > T(0))
            {
                theta   = std::acos(cartesianCoord.z / radius); //std::atan2(cartesianCoord.z, cartesianCoord.x);
                phi     = std::atan2(cartesianCoord.y, cartesianCoord.x);
            }
            else
            {
                theta   = T(0);
                phi     = T(0);
            }
        }

        explicit SphericalT(UninitializeTag)
        {
            // do nothing
        }

        //! Returns the squared length of this spherical coordinate. This is simply radius*radius.
        T LengthSq() const
        {
            return radius*radius;
        }

        //! Returns the length of this sphercial coordinate. This is simply radius.
        T Length() const
        {
            return radius;
        }

        /**
        \breif Normalizes this spherical coordiante to the unit length of 1. This is simply 'radius = 1'.
        \see Normalized
        \see Length
        */
        void Normalize()
        {
            radius = T(1);
        }

        /**
        \breif Returns a normalized instance of this spherical coordinate.
        \see Normalize
        */
        SphericalT<T> Normalized() const
        {
            return SphericalT<T>(T(1), theta, phi);
        }

        /**
        \breif Resizes this spherical coordinate to the specified length. This is simply 'radius = length'.
        \see Normalize
        \see Length
        */
        void Resize(const T& length)
        {
            radius = length;
        }

        /**
        \breif Returns a type casted instance of this spherical coordinate.
        \tparam C Specifies the static cast type.
        */
        template <typename C>
        SphericalT<C> Cast() const
        {
            return SphericalT<C>(
                static_cast<C>(radius),
                static_cast<C>(theta),
                static_cast<C>(phi)
            );
        }

        //! Returns a pointer to the first element of this spherical coordinate.
        T* Ptr()
        {
            return &radius;
        }

        //! Returns a constant pointer to the first element of this spherical coordinate.
        const T* Ptr() const
        {
            return &radius;
        }

        T radius, theta, phi;

};


/* --- Type Alias --- */

using Spherical     = SphericalT<Real>;
using Sphericalf    = SphericalT<float>;
using Sphericald    = SphericalT<double>;
using Sphericali    = SphericalT<std::int32_t>;
using Sphericalui   = SphericalT<std::uint32_t>;
using Sphericalb    = SphericalT<std::int8_t>;
using Sphericalub   = SphericalT<std::uint8_t>;


} // /namespace Gs


#endif



// ================================================================================
