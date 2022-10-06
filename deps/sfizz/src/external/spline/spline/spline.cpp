/*==============================================================================

 Copyright 2018 by Roland Rabien, Devin Lane
 For more information visit www.rabiensoftware.com

 ==============================================================================*/

/* "THE BEER-WARE LICENSE" (Revision 42): Devin Lane wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return. */

#include "spline.h"
#include <cassert>

Spline::Spline (const double *pointsX, const double *pointsY, int numPoints)
{
    assert (numPoints >= 3); // "Must have at least three points for interpolation"

    int n = numPoints - 1;

    std::vector<double> b, d, a, c, l, u, z, h;

    a.resize (n);
    b.resize (n);
    c.resize (n + 1);
    d.resize (n);
    h.resize (n + 1);
    l.resize (n + 1);
    u.resize (n + 1);
    z.resize (n + 1);

    l[0] = 1.0;
    u[0] = 0.0;
    z[0] = 0.0;
    h[0] = pointsX[1] - pointsX[0];

    for (int i = 1; i < n; i++)
    {
        h[i] = pointsX[i+1] - pointsX[i];
        l[i] = (2 * (pointsX[i+1] - pointsX[i-1])) - (h[i-1]) * u[i-1];
        u[i] = (h[i]) / l[i];
        a[i] = (3.0 / (h[i])) * (pointsY[i+1] - pointsY[i]) - (3.0 / (h[i-1])) * (pointsY[i] - pointsY[i-1]);
        z[i] = (a[i] - (h[i-1]) * z[i-1]) / l[i];
    }

    l[n] = 1.0;
    z[n] = 0.0;
    c[n] = 0.0;

    for (int j = n - 1; j >= 0; j--)
    {
        c[j] = z[j] - u[j] * c[j+1];
        b[j] = (pointsY[j+1] - pointsY[j]) / (h[j]) - ((h[j]) * (c[j+1] + 2.0 * c[j])) / 3.0;
        d[j] = (c[j+1] - c[j]) / (3.0 * h[j]);
    }

    elements.reserve (n);
    for (int i = 0; i < n; i++)
        elements.emplace_back (pointsX[i], pointsY[i], b[i], c[i], d[i]);
}

double Spline::interpolate (double x) const noexcept
{
    int n = static_cast<int>(elements.size());
    if (n == 0)
        return 0.0;

    int i;
    for (i = 0; i < n; i++)
    {
        if (! (elements[i] < x))
            break;
    }
    if (i != 0) i--;

    return elements[i].eval (x);
}
