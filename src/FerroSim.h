#pragma once

#include "constants.h"
#include "math.h"
#include <functional>
#include <iostream>
#include <vector>
#include <string>
#include <time.h>

using namespace std;
using namespace constant;

struct sim_data {
    sim_data(int sim_size)
        : new_pos(sim_size, 0), pos(sim_size, 0), old_pos(sim_size, 0),
          mag(sim_size, 0), new_mag(sim_size, 0), force(sim_size, 0),
          phi(sim_size, 0) {}

    vector<Vec3> new_pos;
    vector<Vec3> pos;
    vector<Vec3> old_pos;
    vector<Vec3> mag;
    vector<Vec3> new_mag;
    vector<Vec3> force;
    vector<f64>  phi;
};

#define rfloat ((float)rand()/(float)(RAND_MAX))

struct sim : sim_data {
    int                            size;
    int                            timesteps;
    f64                            delta_t;
    int                            current_timestep = 1;
    function<void(vector<Vec3> &)> save_function;

    sim(int size, int timesteps, long delta_t,
        function<void(vector<Vec3> &)> save_function)
        : sim_data(size), size(size), timesteps(timesteps),
          delta_t((f64)delta_t / 1000000.0), save_function(save_function) {
        init_particles();
    }

    void init_particles() {
		srand(time(NULL));

        int used   = 0;
        f64 height = 0.0f;
        while(used < size) {
            pos[used++] = Vec3((rfloat * radius_oil * 0.05), (rfloat * radius_oil * 0.05), height + (rfloat * radius_oil * 0.05));
            for(f64 width = radius_oil * 2; width < width_basin;
                width += radius_oil * 2) {
                int toDistribute = (2 * M_PI * width) / (2 * radius_oil);
                if(toDistribute > (size - used)) toDistribute = (size - used);
                f64 alpha        = (2 * M_PI) / toDistribute;
                f64 currentAlpha = 0;
                for(int particle = 0; particle < toDistribute; particle++) {
                    pos[used++] = Vec3(cos(alpha * particle) * width + (rfloat * radius_oil * 0.05),
                                       sin(alpha * particle) * width + (rfloat * radius_oil * 0.05), height + (rfloat * radius_oil * 0.05));
                    currentAlpha += alpha;
                }
                if(used == size) break;
            }
            height += radius_oil * 2;
        }

        old_pos = pos;
    }

    Vec3 nablaKernel(Vec3 pos1, Vec3 pos2, f64 effectiveRadius) {
        Vec3 diff = pos1 - pos2;

        Vec3 ret = diff * -954.0 * sq(sq(effectiveRadius) + diff * -diff) /
                   (32 * pow(effectiveRadius, 9) * M_PI);

        return ret;
    }

    Vec3 hDipole(Vec3 atPos, Vec3 mVector, f64 my) {
        double tmp = atPos * atPos;
        Vec3   rhs = mVector / pow(tmp, 1.5) / (4 * M_PI * my);
        Vec3   lhs = atPos * -3.0 * (atPos * mVector) / pow(tmp, 2.5);

        return lhs + rhs;
    }

    f64 nablaRijByRijCubed(Vec3 pos1, Vec3 pos2) {
        Vec3   diff = pos1 - pos2;
        double tmp  = diff * diff;
        Vec3   ret  = diff * 3 / pow(tmp, 2.5);
        return (ret.x + ret.y + ret.z - 3 / pow(tmp, 1.5));
    }

    f64 nablaChiH(int numParticle) {
        f64  output  = 0.0;
        Vec3 nKernel = Vec3(0);
        for(int i = 0; i < size; i++) {
            if(i == numParticle) continue;
            nKernel = nablaKernel(pos[numParticle], pos[i], radius_particle);
            output += nKernel * mag[i];
        }
        output *= volume_particle;
        output *= chi;
        return output;
    }

    Vec3 H(int num) {
        Vec3 position = pos[num];
        Vec3 currentPos(0);
        Vec3 diff(0);
        Vec3 result = hDipole(position, m_dipole, myR * my0);
        Vec3 sum(0);
        for(int i = 0; i < size; i++) {
            if(i == num) continue;
            currentPos = pos[i];
            diff       = currentPos - position;

            f64 factor1 = nablaChiH(i) / diff.norm();
            sum += diff * factor1;

            f64 factor2 = chi * nablaRijByRijCubed(currentPos, position);
            sum += mag[i] * factor2;
        }

        f64 factor = volume_particle / (4 * M_PI * myR * my0);
        sum *= factor;

        result -= sum;

        return result;
    }

    void computePhi() {
        Vec3 h(0, 0, 0);
        f64  local_phi;
        for(int i = 0; i < size; i++) {
            h          = H(i);
            new_mag[i] = h;
            local_phi  = my0 * myR * (h * h) / 2.0;
            phi[i]     = abs(local_phi);
        }
    }

    Vec3 fMag(int num) {
        Vec3 result(0);
        Vec3 sum(0);
        f64  local_phi = 0.0;
        for(int i = 0; i < size; i++) {
            if(i == num) continue;
            local_phi = phi[i];
            sum       = nablaKernel(pos[num], pos[i], radius_particle);
            result += sum * local_phi;
        }
        result *= -volume_particle;

        return result;
    }

    Vec3 fOberflaecheDops(int num) {
        f64  d               = 0.0;
        f64  max             = 0.0;
        f64  maxDops         = 0.0;
        Vec3 currentParticle = pos[num];
        Vec3 distance(0);
        Vec3 result(0);

        for(int i = 0; i < size; i++) {
            if(i == num) continue;

            Vec3 & particle = pos[i];
            distance        = currentParticle - particle;
            d               = distance.norm();

            max     = (4.0 * radius_oil) - d;
			maxDops = (0.5 / pow(2 * radius_oil, 15)) * pow(d - (4 * radius_oil), 16);

            if(max < 0.0) continue;

            result -= distance / d * (kSTension * max);

            if(maxDops < 0.0) continue;

            result += distance / d * (kSTension * maxDops);
			//printf("Fd %f ", (kSTension * maxDops));
			//printf("Fo %f ", (kSTension * max));
			//printf("D %f\t", d);
        }

        return result;
    }

    void calculate() {
       // computePhi();

        for(int i = 0; i < size; i++) {
            Vec3 & f = force[i];
            //*f = fMag(i);
            f = Vec3(0);

            f.z -= gravity * mass_oil;

            f64 frictionCoefficient = 6 * M_PI * viscosity * radius_particle;

            Vec3 dist = old_pos[i] - pos[i];
            f -= dist * frictionCoefficient * delta_t;

            Vec3 surfaceTensionDops = fOberflaecheDops(i);
            f += surfaceTensionDops;
        }

        for(int i = 0; i < size; i++) {
			Vec3 v(0);
            v = old_pos[i] - pos[i];

            // NOTE(robin): fix for birk fail :D
            v += force[i] / mass_oil * delta_t;

            /* v += force[i] / mass_oil; */

            Vec3 & posA = pos[i];
            new_pos[i]  = posA + (v * delta_t);
			if (new_pos[i].z < 0) new_pos[i].z = 0; //Force Bottom Cutoff
        }

        // swap the vectors, to save the awkward calculations of oneTimeStepAgo,
        // swap the pointers internally
        swap(mag, new_mag);

        swap(pos, old_pos);
        swap(pos, new_pos);
    }

    void simulate() {
        for(long ts = 0; ts < timesteps; ts++) {
            printf("Timestep %li\r\n", ts);
            save_function(pos);
            calculate();
            current_timestep++;
        }
    }

    void write_metadata(string filename, string timestamp) {
        string metadata = string();
        metadata += "[SimulationData]\n";
        metadata += "Particles=" + to_string(size) + "\n";
        metadata += "Timesteps=" + to_string(timesteps) + "\n";
        metadata += "Timestamp=" + timestamp + "\n";
        metadata += "[SimulationParameters]\n";
        metadata += "InitialWidth=" + to_string(width_basin) + "\n";
        metadata += "RadiusOil=" + to_string(radius_oil) + "\n";
        metadata += "MassOil=" + to_string(mass_oil) + "\n";
        metadata += "PermeabilityOil=" + to_string(myR) + "\n";
        metadata += "Viscosity=" + to_string(viscosity) + "\n";
        metadata += "RadiusParticle=" + to_string(radius_particle) + "\n";
        metadata += "MagneticSusceptibility=" + to_string(chi) + "\n";
        metadata += "SurfaceTension=" + to_string(sigma) + "\n";
        metadata += "MagneticDipoleX=" + to_string(m_dipole.x) + "\n";
        metadata += "MagneticDipoleY=" + to_string(m_dipole.y) + "\n";
        metadata += "MagneticDipoleZ=" + to_string(m_dipole.z);

        FILE * file = fopen(filename.c_str(), "w");
        fwrite(metadata.c_str(), sizeof(char), metadata.length(), file);
        fclose(file);
    }
};
