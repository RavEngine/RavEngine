/*==============================================================================

 Copyright 2018 by Roland Rabien, Devin Lane
 For more information visit www.rabiensoftware.com

 ==============================================================================*/

#pragma once
#include <vector>

/** Cubic spline interpolation is a simple way to obtain a smooth curve from a set of
 discrete points. It has both C1 (first derivative) and C2 (second derivative) continuity,
 enabling it to produce a continuous piecewise function given a set of data points.

 Add points in increasing x order */
class Spline
{
public:
    Spline (const double *pointsX, const double *pointsY, int numPoints);

    double operator[] (double x) const noexcept { return interpolate(x); }
    double interpolate (double x) const noexcept;

    class Element
    {
    public:
        Element (double x_ = 0) : x (x_) {}

        Element (double x_, double a_, double b_, double c_, double d_)
          : x (x_), a (a_), b (b_), c (c_), d (d_) {}

        double eval (double xx) const
        {
            double xix (xx - x);
            return a + b * xix + c * (xix * xix) + d * (xix * xix * xix);
        }

        bool operator< (const Element& e) const { return x < e.x;   }
        bool operator< (const double xx) const  { return x < xx;    }

        double x = 0, a = 0, b = 0, c = 0, d = 0;
    };

private:
    std::vector<Element> elements;
};
