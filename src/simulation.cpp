#include "simulation.h"

/*
 * // NOTE(robin): WTF BIRK?????
 * // 0 <= T <= m_currentTimestepSinceWrite
 * #define posAt(t, n) ((Vec3 *)(m_positions + (n) + ((t)*m_numParticles)))
 *
 * // T = {-1, 0}
 * #define magAt(t, n) \
 *     ((Vec3 *)(m_magnetization + (n) + \
 *               ((m_currentTimestepEven ? (t) + 1 : (t) * -1) * \
 *                m_numParticles)))
 *
 * #define phiAt(n) ((f64 *)(m_phi + (n)))
 *
 * #define forceAt(n) ((Vec3 *)(m_force + (n)))
 */

Vec3 * Simulation::posAt(int t, int n) {
    return &m_positions[n + t * m_numParticles];
}

Vec3 * Simulation::magAt(int t, int n) {
    return &m_positions[n + ((m_currentTimestepEven ? (t) + 1 : (t) * -1) *
                             m_numParticles)];
}

f64 * Simulation::phiAt(int n) { return &m_phi[n]; }

Vec3 * Simulation::forceAt(int n) { return &m_force[n]; }

Simulation::Simulation(size_t num_particles, size_t timesteps_ram,
                       long timestep_us, f64 width_basin, f64 viscosityOil,
                       f64 chi, f64 myR, f64 sigma, f64 radius_particle,
                       f64 radius_oil, f64 mass_oil, Vec3 mDipole,
                       long num_timesteps, std::string output,
                       std::string jarfile) {
    m_name                      = std::to_string(time(0));
    m_numParticles              = num_particles;
    m_timestepsRam              = timesteps_ram;
    m_timestepS                 = ((f64)timestep_us) / 1000000.0;
    m_numTimestep               = num_timesteps;
    m_currentTimestep           = 0;
    m_currentTimestepSinceWrite = 0;
    m_currentTimestepEven       = true;
    m_widthBasin                = width_basin;
    m_chi                       = chi;
    m_myR                       = myR;
    m_sigma                     = sigma;
    m_kSTension                 = (-2.0 * m_sigma) / (-4.0 + sqrt(6.0));
    m_radiusOil                 = radius_oil;
    m_areaOil                   = M_PI * pow(radius_oil, 2);
    m_volumeOil                 = (4.0 / 3.0) * M_PI * pow(radius_oil, 3);
    m_radiusParticle            = radius_particle;
    m_volumeParticle            = (4.0 / 3.0) * M_PI * pow(radius_particle, 3);
    m_massOil                   = mass_oil;
    m_mDipole                   = mDipole;
    m_viscosity                 = viscosityOil;
    m_jarfile                   = jarfile;

    if(output.size() > 0) {
        if(output[output.size() - 1] != '/') {
            m_output = output + "/";
        } else {
            m_output = output;
        }
    } else {
        m_output = output;
    }

    m_positions.reserve(num_particles * timesteps_ram);
    m_magnetization.reserve(2 * num_particles);
    m_phi.reserve(num_particles);
    m_force.reserve(num_particles);

    // Build initial Particles
    {
        size_t usedParticles = 0;
        f64    height        = 0.0f;
        while(usedParticles < m_numParticles) {
            *posAt(0, usedParticles) = Vec3(0.0f, 0.0f, height);
            *posAt(m_timestepsRam - 1, usedParticles++) =
                Vec3(0.0f, 0.0f, height);
            for(f64 width = m_radiusOil * 2; width < m_widthBasin;
                width += m_radiusOil * 2) {
                size_t toDistribute = (2 * M_PI * width) / (2 * m_radiusOil);
                if(toDistribute > (m_numParticles - usedParticles))
                    toDistribute = (m_numParticles - usedParticles);
                f64 alpha        = (2 * M_PI) / toDistribute;
                f64 currentAlpha = 0;
                for(size_t particle = 0; particle < toDistribute; particle++) {
                    *posAt(0, usedParticles) =
                        Vec3(cos(alpha * particle) * width,
                             sin(alpha * particle) * width, height);
                    *posAt(m_timestepsRam - 1, usedParticles++) =
                        Vec3(cos(alpha * particle) * width,
                             sin(alpha * particle) * width, height);
                    currentAlpha += alpha;
                }
                if(usedParticles == m_numParticles) break;
            }
            height += m_radiusOil * 2;
        }
    }

    m_currentTimestep++;
    m_currentTimestepEven = !m_currentTimestepEven;
    m_currentTimestepSinceWrite++;

    simulate();

    std::string metadata = std::string();
    metadata += "[SimulationData]\n";
    metadata += "Particles=" + std::to_string(m_numParticles) + "\n";
    metadata += "Timesteps=" + std::to_string(m_numTimestep) + "\n";
    metadata += "Timestamp=" + m_name + "\n";
    metadata += "[SimulationParameters]\n";
    metadata += "InitialWidth=" + std::to_string(m_widthBasin) + "\n";
    metadata += "RadiusOil=" + std::to_string(m_radiusOil) + "\n";
    metadata += "MassOil=" + std::to_string(m_massOil) + "\n";
    metadata += "PermeabilityOil=" + std::to_string(m_myR) + "\n";
    metadata += "Viscosity=" + std::to_string(m_viscosity) + "\n";
    metadata += "RadiusParticle=" + std::to_string(m_radiusParticle) + "\n";
    metadata += "MagneticSusceptibility=" + std::to_string(m_chi) + "\n";
    metadata += "SurfaceTension=" + std::to_string(m_sigma) + "\n";
    metadata += "MagneticDipoleX=" + std::to_string(m_mDipole.x) + "\n";
    metadata += "MagneticDipoleY=" + std::to_string(m_mDipole.y) + "\n";
    metadata += "MagneticDipoleZ=" + std::to_string(m_mDipole.z);

    std::cout << (m_output + m_name + ".sim.metadata").c_str() << std::endl;
    FILE * file = fopen((m_output + m_name + ".sim.metadata").c_str(), "w");
    fwrite(metadata.c_str(), sizeof(char), metadata.length(), file);
    fclose(file);

    system((std::string("java -jar ") + jarfile).c_str());
}

Vec3 Simulation::nablaKernel(Vec3 pos1, Vec3 pos2, f64 effectiveRadius) {
    Vec3 diff = pos1 - pos2;

    Vec3 ret = diff * -954.0 * sq(sq(effectiveRadius) + diff * -diff) /
               (32 * pow(effectiveRadius, 9) * M_PI);

    return ret;
}

Vec3 Simulation::hDipole(Vec3 atPos, Vec3 mVector, f64 my) {
    double tmp = atPos * atPos;
    Vec3   rhs = mVector / pow(tmp, 1.5) / (4 * M_PI * my);
    Vec3   lhs = atPos * -3.0 * (atPos * mVector) / pow(tmp, 2.5);

    return lhs + rhs;
}

f64 Simulation::nablaRijByRijCubed(Vec3 pos1, Vec3 pos2) {
    Vec3   diff = pos1 - pos2;
    double tmp  = diff * diff;
    Vec3   ret  = diff * 3 / pow(tmp, 2.5);
    return (ret.x + ret.y + ret.z - 3 / pow(tmp, 1.5));
}

size_t Simulation::getOneTimestepAgo() {
    size_t oneTimestepAgo = m_currentTimestepSinceWrite - 1;
    if(m_currentTimestepSinceWrite < 1) { oneTimestepAgo += m_timestepsRam; }

    return oneTimestepAgo;
}

f64 Simulation::nablaChiH(size_t numParticle) {
    size_t oneTimestepAgo = getOneTimestepAgo();

    f64  output  = 0.0;
    Vec3 nKernel = Vec3(0);
    for(size_t i = 0; i < m_numParticles; i++) {
        if(i == numParticle) continue;
        nKernel = nablaKernel(*posAt(oneTimestepAgo, numParticle),
                              *posAt(oneTimestepAgo, i), m_radiusParticle);
        output += nKernel * *magAt(-1, i);
    }
    output *= m_volumeParticle;
    output *= m_chi;
    return output;
}

Vec3 Simulation::H(size_t numParticle) {
    size_t oneTimestepAgo = getOneTimestepAgo();
    Vec3 pos = *posAt(oneTimestepAgo, numParticle);
    Vec3 currentPos(0);
    Vec3 diff(0);
    Vec3 result = hDipole(pos, m_mDipole, m_myR * m_my0);
    Vec3 sum(0);
    Vec3 mag(0);
    for(size_t i = 0; i < m_numParticles; i++) {
        if(i == numParticle) continue;
        currentPos = *posAt(oneTimestepAgo, i);
        diff = currentPos - pos;

        f64 factor1 = nablaChiH(i) / diff.norm();
        sum += diff * factor1;

        f64 factor2 = m_chi * nablaRijByRijCubed(currentPos, pos);
        mag         = *magAt(-1, i);
        sum += mag * factor2;
    }

    f64 factor = m_volumeParticle / (4 * M_PI * m_myR * m_my0);
    sum *= factor;

    result -= sum;

    return result;
}

void Simulation::computePhi() {
    Vec3 h(0, 0, 0);
    f64  phi = 0.0;
    for(size_t i = 0; i < m_numParticles; i++) {
        h = H(i);
        /* *magAt(0, i) = h;           */
        phi       = m_my0 * m_myR * sq(h.x) + sq(h.y) + sq(h.z) / 2.0;
        *phiAt(i) = phi < 0 ? -phi : phi;
    }
}

Vec3 Simulation::fMag(size_t numParticle) {
    size_t oneTimestepAgo =
        m_currentTimestepSinceWrite < 1
            ? m_currentTimestepSinceWrite - 1 + m_timestepsRam
            : m_currentTimestepSinceWrite - 1;
    Vec3 result(0, 0, 0);
    Vec3 sum(0, 0, 0);
    f64  phi = 0.0;
    for(size_t i = 0; i < m_numParticles; i++) {
        if(i == numParticle) continue;
        phi = *phiAt(i);
        sum = nablaKernel(*posAt(oneTimestepAgo, numParticle),
                          *posAt(oneTimestepAgo, i), m_radiusParticle);
        result.x += sum.x * phi;
        result.y += sum.y * phi;
        result.z += sum.z * phi;
    }
    result.x *= -m_volumeParticle;
    result.y *= -m_volumeParticle;
    result.z *= -m_volumeParticle;

    return result;
}

Vec3 Simulation::fOberflaecheDops(size_t numParticle) {
    size_t oneTimestepAgo =
        m_currentTimestepSinceWrite < 1
            ? m_currentTimestepSinceWrite - 1 + m_timestepsRam
            : m_currentTimestepSinceWrite - 1;
    f64    d       = 0.0;
    f64    force   = 0.0;
    f64    max     = 0.0;
    f64    maxDops = 0.0;
    Vec3 * particle;
    Vec3   currentParticle = *posAt(oneTimestepAgo, numParticle);
    Vec3   distance(0, 0, 0);

    Vec3 result(0, 0, 0);
    for(size_t i = 0; i < m_numParticles; i++) {
        if(i == numParticle) continue;

        particle   = posAt(oneTimestepAgo, i);
        distance.x = currentParticle.x - particle->x;
        distance.y = currentParticle.y - particle->y;
        distance.z = currentParticle.z - particle->z;

        d   = sqrt(sq(distance.x) + sq(distance.y) + sq(distance.z));
        max = (4.0 * m_radiusOil) - d;

        maxDops = ((-5.0 * m_gravity) * ((2 * m_radiusOil) / d + 0.1) +
                   (5.5 * m_gravity)) /
                  300;

        if(max < 0.0) continue;

        force = m_kSTension / d * max / 900.0;
        result.x -= force * distance.x;
        result.y -= force * distance.y;
        result.z -= force * distance.z;

        if(maxDops < 0.0) continue;

        result.x += maxDops * distance.x;
        result.y += maxDops * distance.y;
        result.z += maxDops * distance.z;
    }

    return result;
}

void Simulation::calculate() {
    computePhi();
    size_t oneTimestepAgo =
        m_currentTimestepSinceWrite < 1
            ? m_currentTimestepSinceWrite - 1 + m_timestepsRam
            : m_currentTimestepSinceWrite - 1;
    size_t twoTimestepAgo =
        m_currentTimestepSinceWrite < 2
            ? m_currentTimestepSinceWrite - 2 + m_timestepsRam
            : m_currentTimestepSinceWrite - 2;

    for(size_t i = 0; i < m_numParticles; i++) {
        Vec3 * f = forceAt(i);
        //*f = fMag(i);
        *f = Vec3(0, 0, 0);

        f->z -= m_gravity * m_massOil;

        f64 frictionCoefficient = 6 * M_PI * m_viscosity * m_radiusParticle;
        f->x -= frictionCoefficient * ((*posAt(twoTimestepAgo, i)).x -
                                       (*posAt(oneTimestepAgo, i)).x) *
                m_timestepS;
        f->y -= frictionCoefficient * ((*posAt(twoTimestepAgo, i)).y -
                                       (*posAt(oneTimestepAgo, i)).y) *
                m_timestepS;
        f->z -= frictionCoefficient * ((*posAt(twoTimestepAgo, i)).z -
                                       (*posAt(oneTimestepAgo, i)).z) *
                m_timestepS;

        Vec3 surfaceTensionDops = fOberflaecheDops(i);
        f->x += surfaceTensionDops.x;
        f->y += surfaceTensionDops.y;
        f->z += surfaceTensionDops.z;
    }

    Vec3   v(0, 0, 0);
    Vec3 * posN;
    Vec3 * posA;
    for(size_t i = 0; i < m_numParticles; i++) {
        v.x = ((*posAt(twoTimestepAgo, i)).x - (*posAt(oneTimestepAgo, i)).x);
        v.y = ((*posAt(twoTimestepAgo, i)).y - (*posAt(oneTimestepAgo, i)).y);
        v.z = ((*posAt(twoTimestepAgo, i)).z - (*posAt(oneTimestepAgo, i)).z);

        v.x += (*forceAt(i)).x / m_massOil;
        v.y += (*forceAt(i)).y / m_massOil;
        v.z += (*forceAt(i)).z / m_massOil;

        posN = posAt(m_currentTimestepSinceWrite, i);
        posA = posAt(oneTimestepAgo, i);

        posN->x = posA->x + (v.x * m_timestepS);
        posN->y = posA->y + (v.y * m_timestepS);
        posN->z = posA->z + (v.z * m_timestepS);
    }
}

void Simulation::simulate() {
    for(long ts = 0; ts < m_numTimestep; ts++) {
        printf("Timestep %li\r\n", ts);
        calculate();
        m_currentTimestep++;
        m_currentTimestepEven = !m_currentTimestepEven;
        m_currentTimestepSinceWrite++;

        if(m_currentTimestepSinceWrite > m_timestepsRam) {
            m_currentTimestepSinceWrite = 0;
            std::cout << (m_output + m_name + ".sim.metadata").c_str()
                      << std::endl;
            FILE * file = fopen((m_output + m_name + ".sim").c_str(), "ab");
            fwrite(m_positions.data(), sizeof(Vec3),
                   m_numParticles * m_timestepsRam, file);
            fclose(file);
        }
    }
    if(m_currentTimestepSinceWrite != 0) {
        std::cout << (m_output + m_name + ".sim.metadata").c_str() << std::endl;
        FILE * file = fopen((m_output + m_name + ".sim").c_str(), "ab");

        fwrite(m_positions.data(), sizeof(Vec3),
               m_numParticles * (m_currentTimestepSinceWrite - 1), file);
        fclose(file);
    }
}
