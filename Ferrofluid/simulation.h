#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <ctime>

#include "math.h"

void init(size_t num_particles, size_t timesteps_ram, long timestep_us, math width_basin, math viscosityOil, math chi, math myR, math sigma, math radius_particle, math radius_oil, math mass_oil, Vec3 mDipole, long num_timesteps = -1L);