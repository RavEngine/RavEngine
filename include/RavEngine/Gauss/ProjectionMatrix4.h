/*
 * ProjectionMatrix4.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_PROJECTION_MATRIX4_H
#define GS_PROJECTION_MATRIX4_H


#include "Decl.h"
#include "Details.h"
#include "Determinant.h"
#include "Inverse.h"
#include "Matrix.h"
#include "Vector4.h"


namespace Gs
{


//! Flags to generate a projection matrix.
struct ProjectionFlags
{
    enum
    {
        /**
        Interprets the field-of-view (FOV) as an horizontal view angle.
        Otherwise it is interpreted as a vertical view angle.
        */
        HorizontalFOV   = (1 << 0),

        /**
        Generates a right-handed coordinate system (Z+ points out of the screen).
        Otherwise a left-handed coordinate system is used (Z+ points into the screen).
        */
        RightHanded     = (1 << 1),

        /**
        Projects the Z coordinates into the range [-1.0, 1.0].
        Otherwise the range [0.0, 1.0] is used.
        */
        UnitCube        = (1 << 2),

        /**
        Flags preset for an OpenGL projection matrix: RightHanded | UnitCube.
        \see RightHanded
        \see UnitCube
        */
        OpenGLPreset    = (RightHanded | UnitCube),

        //! Flags preset for a Direct3D projection matrix: 0 (default setting).
        Direct3DPreset  = 0,
    };
};

//! Specifies where the coordinate system's origin of a planar projection is located.
enum class PlanarProjectionOrigin
{
    //! Left-top corner of the screen.
    LeftTop,

    //! Right-top corner of the screen.
    RightTop,

    //! Right-bottom corner of the screen.
    RightBottom,

    //! Left-bottom corner of the screen.
    LeftBottom,
};


/**
Sparse 4x4 projection matrix.
\remarks
\code
// / w 0 0 0 \
// | 0 h 0 0 |
// | 0 0 a c |
// \ 0 0 b d /
\endcode
*/
template <typename T>
class ProjectionMatrix4T
{

    public:

        static_assert(std::is_floating_point<T>::value, "projection matrices can only be used with floating point types");

        /* ----- Typenames ----- */

        //! Specifies the typename of the scalar components.
        using ScalarType        = T;

        //! Typename of this matrix type.
        using ThisType          = ProjectionMatrix4T<T>;

        //! Typename of the transposed of this matrix type.
        using TransposedType    = ProjectionMatrix4T<T>;

        /* ----- Functions ----- */

        ProjectionMatrix4T()
            #ifndef GS_DISABLE_AUTO_INIT
            :
            m00 { T(0) },
            m11 { T(0) },
            m22 { T(0) },
            m32 { T(0) },
            m23 { T(0) },
            m33 { T(0) }
            #endif
        {
        }

        ProjectionMatrix4T(const ThisType& rhs) :
            m00 { rhs.m00 },
            m11 { rhs.m11 },
            m22 { rhs.m22 },
            m32 { rhs.m32 },
            m23 { rhs.m23 },
            m33 { rhs.m33 }
        {
        }

        explicit ProjectionMatrix4T(UninitializeTag)
        {
            // do nothing
        }

        ThisType& operator += (const ThisType& rhs)
        {
            m00 += rhs.m00;
            m11 += rhs.m11;
            m22 += rhs.m22;
            m32 += rhs.m32;
            m23 += rhs.m23;
            m33 += rhs.m33;
            return *this;
        }

        ThisType& operator -= (const ThisType& rhs)
        {
            m00 -= rhs.m00;
            m11 -= rhs.m11;
            m22 -= rhs.m22;
            m32 -= rhs.m32;
            m23 -= rhs.m23;
            m33 -= rhs.m33;
            return *this;
        }

        ThisType& operator *= (const ThisType& rhs)
        {
            *this = (*this * rhs);
            return *this;
        }

        ThisType& operator *= (const T& rhs)
        {
            m00 *= rhs;
            m11 *= rhs;
            m22 *= rhs;
            m32 *= rhs;
            m23 *= rhs;
            m33 *= rhs;
            return *this;
        }

        ThisType& operator = (const ThisType& rhs)
        {
            m00 = rhs.m00;
            m11 = rhs.m11;
            m22 = rhs.m22;
            m32 = rhs.m32;
            m23 = rhs.m23;
            m33 = rhs.m33;
            return *this;
        }

        Vector4T<T> Project(const Vector4T<T>& v)
        {
            #ifdef GS_ROW_VECTORS
            auto p = v * (*this);
            #else
            auto p = (*this) * v;
            #endif
            p /= p.w;
            return p;
        }

        Vector4T<T> Unproject(const Vector4T<T>& v)
        {
            #ifdef GS_ROW_VECTORS
            auto p = v * Inverse();
            #else
            auto p = Inverse() * v;
            #endif
            p /= p.w;
            return p;
        }

        void ToMatrix4(Matrix<T, 4, 4>& m) const
        {
            m.At(0, 0) = m00;
            m.At(1, 0) = T(0);
            m.At(2, 0) = T(0);
            m.At(3, 0) = T(0);

            m.At(0, 1) = T(0);
            m.At(1, 1) = m11;
            m.At(2, 1) = T(0);
            m.At(3, 1) = T(0);

            m.At(0, 2) = T(0);
            m.At(1, 2) = T(0);
            m.At(2, 2) = m22;
            m.At(3, 2) = m32;

            m.At(0, 3) = T(0);
            m.At(1, 3) = T(0);
            m.At(2, 3) = m23;
            m.At(3, 3) = m33;
        }

        Matrix<T, 4, 4> ToMatrix4() const
        {
            Matrix<T, 4, 4> result;
            ToMatrix4(result);
            return result;
        }

        ProjectionMatrix4T<T> Inverse() const
        {
            ProjectionMatrix4T<T> inv { *this };
            inv.MakeInverse();
            return inv;
        }

        bool MakeInverse()
        {
            ProjectionMatrix4T<T> in { *this };
            return Gs::Inverse(*this, in);
        }

        /**
        Returns a type casted instance of this projection matrix.
        \tparam C Specifies the static cast type.
        */
        template <typename C> ProjectionMatrix4T<C> Cast() const
        {
            ProjectionMatrix4T<C> result { UninitializeTag{} };

            result.m00 = static_cast<C>(m00);
            result.m11 = static_cast<C>(m11);
            result.m22 = static_cast<C>(m22);
            result.m32 = static_cast<C>(m32);
            result.m23 = static_cast<C>(m23);
            result.m33 = static_cast<C>(m33);

            return result;
        }

        /**
        Generates a perspective projection.
        \param[out] m Specifies the resulting projection matrix.
        \param[in] aspect Specifies the aspect ratio. This is commonly width/height (e.g. 1920/1080).
        \param[in] nearPlane Specifies the distance of the near clipping plane. This is must be in the range (0, far). Common values are in the range [0.1, 1].
        \param[in] farPlane Specifies the distance of the far clipping plane. This is must be in the range (near, +inf). Common values are in the range [10, 1000].
        \param[in] fov Specifies the field-of-view (FOV) angle (in radians). This must be in the range (0, 180). Common values are in the range [45, 90].
        \param[in] flags Optional bit mask with projection generation flags (see ProjectionFlags).
        By default 0, which generates a left-handed projection matrix where the Z values are projected to the range [0, 1].
        \see ProjectionFlags
        */
        static void Perspective(ProjectionMatrix4T<T>& m, const T& aspect, const T& nearPlane, const T& farPlane, const T& fov, int flags = 0)
        {
            T w, h;

            bool horzFOV        = (( flags & ProjectionFlags::HorizontalFOV ) != 0);
            bool rightHanded    = (( flags & ProjectionFlags::RightHanded   ) != 0);
            bool unitCube       = (( flags & ProjectionFlags::UnitCube      ) != 0);

            if (horzFOV)
            {
                w = T(1) / std::tan(fov / T(2));
                h = w * aspect;
            }
            else
            {
                h = T(1) / std::tan(fov / T(2));
                w = h / aspect;
            }

            m.m00 = w;
            m.m11 = h;

            m.m22 = (unitCube ? (farPlane + nearPlane)/(farPlane - nearPlane) : farPlane/(farPlane - nearPlane));

            #ifdef GS_ROW_VECTORS
            m.m23 = T(1);
            #else
            m.m32 = T(1);
            #endif

            #ifdef GS_ROW_VECTORS
            m.m32 =
            #else
            m.m23 =
            #endif
            (unitCube ? -(T(2)*farPlane*nearPlane)/(farPlane - nearPlane) : -(farPlane*nearPlane)/(farPlane - nearPlane));

            m.m33 = T(0);

            if (rightHanded)
            {
                m.m22 = -m.m22;
                #ifdef GS_ROW_VECTORS
                m.m23 = -m.m23;
                #else
                m.m32 = -m.m32;
                #endif
            }
        }

        //! \see Perspective(ProjectionMatrix4T<T>&, const T&, const T&, const T&, const T&, int)
        static ProjectionMatrix4T<T> Perspective(const T& aspect, const T& nearPlane, const T& farPlane, const T& fov, int flags = 0)
        {
            ProjectionMatrix4T<T> m { UninitializeTag{} };
            Perspective(m, aspect, nearPlane, farPlane, fov, flags);
            return m;
        }

        /**
        Generates an orthogonal projection.
        \param[out] m Specifies the resulting projection matrix.
        \param[in] width Specifies the view width.
        \param[in] height Specifies the view height.
        \param[in] nearPlane Specifies the distance of the near clipping plane. This is must be in the range (0, far). Common values are in the range [0.1, 1].
        \param[in] farPlane Specifies the distance of the far clipping plane. This is must be in the range (near, +inf). Common values are in the range [10, 1000].
        \param[in] flags Optional bit mask with projection generation flags (see ProjectionFlags).
        By default 0, which generates a left-handed projection matrix where the Z values are projected to the range [0, 1].
        \see ProjectionFlags
        */
        static void Orthogonal(ProjectionMatrix4T<T>& m, const T& width, const T& height, const T& nearPlane, const T& farPlane, int flags = 0)
        {
            bool rightHanded    = (( flags & ProjectionFlags::RightHanded ) != 0);
            bool unitCube       = (( flags & ProjectionFlags::UnitCube    ) != 0);

            m.m00 = T(2)/width;
            m.m11 = T(2)/height;

            m.m22 = (unitCube ? T(2)/(farPlane - nearPlane) : T(1)/(farPlane - nearPlane));

            #ifdef GS_ROW_VECTORS
            m.m23 = T(0);
            #else
            m.m32 = T(0);
            #endif

            #ifdef GS_ROW_VECTORS
            m.m32 =
            #else
            m.m23 =
            #endif
            (unitCube ? -(farPlane + nearPlane)/(farPlane - nearPlane) : -nearPlane/(farPlane - nearPlane));

            m.m33 = T(1);

            if (rightHanded)
                m.m22 = -m.m22;
        }

        //! \see Orthogonal(ProjectionMatrix4T<T>&, const T&, const T&, const T&, const T&, int
        static ProjectionMatrix4T<T> Orthogonal(const T& width, const T& height, const T& nearPlane, const T& farPlane, int flags = 0)
        {
            ProjectionMatrix4T<T> m { UninitializeTag{} };
            Orthogonal(m, width, height, nearPlane, farPlane, flags);
            return m;
        }

        /**
        Generates a 2D planar projection.
        \param[out] m Specifies the resulting projection matrix. This is from type Matrix<T, 4, 4> and not ProjectionMatrix4T<T> since it is a special case.
        \param[in] width Specifies the resolution width. This should be a screen resolution.
        \param[in] height Specifies the resolution height. This should be a screen resolution.
        \param[in] origin Specifies where the coordinate system's origin of this planar projection is located. By default at the left-top corner of the screen.
        \code
        // Example of the point distribution for the respective origin:
        //
        //     LeftTop         RightTop        RightBottom      LeftBottom
        // (0,0)-----(W,0)  (W,0)-----(0,0)  (W,H)-----(0,H)  (0,H)-----(W,H)
        //   |         |      |         |      |         |      |         |
        //   |         |      |         |      |         |      |         |
        // (0,H)-----(W,H)  (W,H)-----(0,H)  (W,0)-----(0,0)  (0,0)-----(W,0)
        \endcode
        \remarks Division by W after a multiplication with a vector is not necessary, since its value will be always 1 with this matrix.
        */
        static void Planar(Matrix<T, 4, 4>& m, const T& width, const T& height, const PlanarProjectionOrigin origin = PlanarProjectionOrigin::LeftTop)
        {
            m.At(0, 0) = T(2)/width;
            m.At(1, 0) = T(0);
            m.At(2, 0) = T(0);
            m.At(3, 0) = T(0);

            m.At(0, 1) = T(0);
            m.At(1, 1) = T(2)/height;
            m.At(2, 1) = T(0);
            m.At(3, 1) = T(0);

            m.At(0, 2) = T(0);
            m.At(1, 2) = T(0);
            m.At(2, 2) = T(1);
            m.At(3, 2) = T(0);

            m.At(0, 3) = T(-1);
            m.At(1, 3) = T(-1);
            m.At(2, 3) = T(0);
            m.At(3, 3) = T(1);

            if (origin == PlanarProjectionOrigin::LeftTop || origin == PlanarProjectionOrigin::RightTop)
            {
                m.At(1, 1) = -m.At(1, 1);
                m.At(1, 3) = -m.At(1, 3);
            }
            if (origin == PlanarProjectionOrigin::RightTop || origin == PlanarProjectionOrigin::RightBottom)
            {
                m.At(0, 0) = -m.At(0, 0);
                m.At(0, 3) = -m.At(0, 3);
            }
        }

        //! \see Planar(Matrix<T, 4, 4>&, const T&, const T&, const PlanarProjectionOrigin)
        static Matrix<T, 4, 4> Planar(const T& width, const T& height, const PlanarProjectionOrigin origin = PlanarProjectionOrigin::LeftTop)
        {
            Matrix<T, 4, 4> m { UninitializeTag{} };
            Planar(m, width, height, origin);
            return m;
        }

        T m00;
        T     m11;
        T         m22, m32;
        T         m23, m33;

};


/* --- Global Operators --- */

#ifdef GS_ROW_VECTORS

template <typename T>
Vector4T<T> operator * (const Vector4T<T>& v, const ProjectionMatrix4T<T>& m)
{
    return Vector4T<T>(
        m.m00*v.x,
        m.m11*v.y,
        m.m22*v.z + m.m32*v.w,
        m.m23*v.z + m.m33*v.w
    );
}

#else

template <typename T>
Vector4T<T> operator * (const ProjectionMatrix4T<T>& m, const Vector4T<T>& v)
{
    return Vector4T<T>(
        m.m00*v.x,
        m.m11*v.y,
        m.m22*v.z + m.m23*v.w,
        m.m32*v.z + m.m33*v.w
    );
}

#endif

template <typename T>
ProjectionMatrix4T<T> operator * (const ProjectionMatrix4T<T>& lhs, const ProjectionMatrix4T<T>& rhs)
{
    ProjectionMatrix4T<T> result { UninitializeTag{} };

    #ifdef GS_ROW_VECTORS

    result.m00 = lhs.m00*rhs.m00;
    result.m11 = lhs.m11*rhs.m11;
    result.m22 = lhs.m22*rhs.m22 + lhs.m32*rhs.m23;
    result.m32 = lhs.m22*rhs.m32 + lhs.m32*rhs.m33;
    result.m23 = lhs.m23*rhs.m22 + lhs.m33*rhs.m23;
    result.m33 = lhs.m23*rhs.m32 + lhs.m33*rhs.m33;

    #else

    result.m00 = lhs.m00*rhs.m00;
    result.m11 = lhs.m11*rhs.m11;
    result.m22 = lhs.m22*rhs.m22 + lhs.m23*rhs.m32;
    result.m32 = lhs.m32*rhs.m22 + lhs.m33*rhs.m32;
    result.m23 = lhs.m22*rhs.m23 + lhs.m23*rhs.m33;
    result.m33 = lhs.m32*rhs.m23 + lhs.m33*rhs.m33;

    #endif

    return result;
}


/* --- Global Functions --- */

namespace Details
{

template <class M, typename T>
void ExtractClippingPlanes4x4(const M& m, T& nearPlane, T& farPlane, int flags = 0)
{
    bool rightHanded    = (( flags & ProjectionFlags::RightHanded ) != 0);
    bool unitCube       = (( flags & ProjectionFlags::UnitCube    ) != 0);

    /* Get inverse projection matrix */
    auto inv = m;
    Inverse(inv, m);

    /* Get near/far vectors */
    auto nearVec = Vector4T<T> { T(0), T(0), (unitCube ? T(-1) : T(0)), T(1) };
    auto farVec = Vector4T<T> { T(0), T(0), T(1), T(1) };

    /* Extract near/far clipping planes */
    #ifdef GS_ROW_VECTORS
    nearVec = nearVec * inv;
    farVec = farVec * inv;
    #else
    nearVec = inv * nearVec;
    farVec = inv * farVec;
    #endif

    nearPlane = nearVec.z / nearVec.w;
    farPlane = farVec.z / farVec.w;

    if (rightHanded)
    {
        nearPlane = -nearPlane;
        farPlane = -farPlane;
    }
}

} // /namespace Details

template <typename T>
void ExtractClippingPlanes(const Matrix<T, 4, 4>& m, T& nearPlane, T& farPlane, int flags = 0)
{
    Details::ExtractClippingPlanes4x4(m, nearPlane, farPlane, flags);
}

template <typename T>
void ExtractClippingPlanes(const ProjectionMatrix4T<T>& m, T& nearPlane, T& farPlane, int flags = 0)
{
    Details::ExtractClippingPlanes4x4(m, nearPlane, farPlane, flags);
}


/* --- Type Alias --- */

using ProjectionMatrix4     = ProjectionMatrix4T<Real>;
using ProjectionMatrix4f    = ProjectionMatrix4T<float>;
using ProjectionMatrix4d    = ProjectionMatrix4T<double>;
using ProjectionMatrix4i    = ProjectionMatrix4T<int>;


} // /namespace Gs


#endif



// ================================================================================
