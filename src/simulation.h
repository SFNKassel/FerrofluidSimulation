#pragma once

#include <cstring>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "math.h"

class Simulation {
public:
    Simulation(size_t num_particles, size_t timesteps_ram, long timestep_us,
               math width_basin, math viscosityOil, math chi, math myR,
               math sigma, math radius_particle, math radius_oil, math mass_oil,
               Vec3 mDipole, long num_timesteps, std::string output,
               std::string jarfile);

private:
    Vec3 nablaKernel(Vec3 pos1, Vec3 pos2, math effectiveRadius);
    Vec3 hDipole(Vec3 atPos, Vec3 mVector, math my);
    math nablaRijByRijCubed(Vec3 pos1, Vec3 pos2);
    math nablaChiH(size_t numParticle);
    Vec3 H(size_t numParticle);
    void computePhi();
    Vec3 fMag(size_t numParticle);
    Vec3 fOberflaecheDops(size_t numParticle);
    void calculate();
    void simulate();

    size_t m_numParticles;
    size_t m_timestepsRam;
    math   m_timestepS;
    long   m_numTimestep;
    long   m_currentTimestep;
    size_t m_currentTimestepSinceWrite;
    bool   m_currentTimestepEven;
    math   m_widthBasin;
    math   m_chi, m_myR, m_sigma;
    math   m_kSTension;
    math   m_radiusParticle, m_volumeParticle, m_volumeOil, m_radiusOil,
        m_areaOil;
    math        m_massOil;
    math        m_gravity = 9.81;
    math        m_viscosity;
    Vec3        m_mDipole{0, 0, 0};
    std::string m_name;
    math const  m_my0 = 0.0000004 * M_PI;

    Vec3 * m_positions;
    Vec3 * m_magnetization;
    math * m_phi;
    Vec3 * m_force;
     

    /*
     * std::vector<std::vector<Vec3>> m_positions;
     * std::vector<std::vector<Vec3>> m_magnetization;
     * std::vector<math>              m_phi;
     * std::vector<Vec3>              m_force;
     */
     

    std::string m_output;
    std::string m_jarfile;
};
