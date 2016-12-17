#pragma once

#include <cstring>
#include <ctime>
#include <iostream>
#include <cstdio>
/* #include <stdlib.h> */
#include <string>
#include <vector>

#include "math.h"

class Simulation {
public:
    Simulation(size_t num_particles, size_t timesteps_ram, long timestep_us,
               f64 width_basin, f64 viscosityOil, f64 chi, f64 myR,
               f64 sigma, f64 radius_particle, f64 radius_oil, f64 mass_oil,
               Vec3 mDipole, long num_timesteps, std::string output,
               std::string jarfile);

private:
    Vec3 nablaKernel(Vec3 pos1, Vec3 pos2, f64 effectiveRadius);
    Vec3 hDipole(Vec3 atPos, Vec3 mVector, f64 my);
    f64 nablaRijByRijCubed(Vec3 pos1, Vec3 pos2);
    f64 nablaChiH(size_t numParticle);
    Vec3 H(size_t numParticle);
    void computePhi();
    Vec3 fMag(size_t numParticle);
    Vec3 fOberflaecheDops(size_t numParticle);
    void calculate();
    void simulate();
    Vec3 * posAt(int t, int n);
    Vec3 * magAt(int t, int n);
    f64 * phiAt(int n);
    Vec3 * forceAt(int n);
    size_t getOneTimestepAgo();

    size_t m_numParticles;
    size_t m_timestepsRam;
    f64   m_timestepS;
    long   m_numTimestep;
    long   m_currentTimestep;
    size_t m_currentTimestepSinceWrite;
    bool   m_currentTimestepEven;
    f64   m_widthBasin;
    f64   m_chi, m_myR, m_sigma;
    f64   m_kSTension;
    f64   m_radiusParticle, m_volumeParticle, m_volumeOil, m_radiusOil,
        m_areaOil;
    f64        m_massOil;
    f64        m_gravity = 9.81;
    f64        m_viscosity;
    Vec3        m_mDipole{0, 0, 0};
    std::string m_name;
    f64 const  m_my0 = 0.0000004 * M_PI;

    /*
     * Vec3 * m_positions;
     * Vec3 * m_magnetization;
     * f64 * m_phi;
     * Vec3 * m_force;
     */

    std::vector<Vec3> m_positions;
    std::vector<Vec3> m_magnetization;
    std::vector<f64>              m_phi;
    std::vector<Vec3>              m_force;

    std::string m_output;
    std::string m_jarfile;
};
