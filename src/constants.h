#pragma once

#include "math.h"

#ifdef M_PI
#undef M_PI
#endif

namespace constant {
    constexpr f64 M_PI   = 3.14159265358979323;
    constexpr f64 width_basin = 0.05;
    constexpr f64 viscosity   = 1.0;
    constexpr f64 chi         = 0;
    constexpr f64 myR         = 0.0;
    constexpr f64 sigma       = 72.75;
    constexpr f64 kSTension   = 1.2898979485566355 * sigma;
    constexpr f64 radius_oil  = 0.01;
    constexpr f64 area_oil    = M_PI * radius_oil * radius_oil;
    constexpr f64 volume_oil =
        (4.0 / 3.0) * M_PI * radius_oil * radius_oil * radius_oil;
    constexpr f64 radius_particle = 0.0001;
    constexpr f64 volume_particle = (4.0 / 3.0) * M_PI * radius_particle *
                                    radius_particle * radius_particle;
    constexpr f64  mass_oil = 0.001;
    constexpr Vec3 m_dipole(0);
    constexpr f64  gravity = 9.81;
    constexpr f64  my0   = 0.0000004 * M_PI;
}
