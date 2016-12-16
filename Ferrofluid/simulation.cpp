#include "simulation.h"

size_t m_numParticles;
size_t m_timestepsRam;
math m_timestepS;
long m_numTimestep;
long m_currentTimestep;
size_t m_currentTimestepSinceWrite;
bool m_currentTimestepEven;
math m_widthBasin;
math m_chi, m_myR, m_sigma;
math m_kSTension;
math m_radiusParticle, m_volumeParticle, m_volumeOil, m_radiusOil, m_areaOil;
math m_massOil;
math m_gravity = 9.81;
math m_viscosity;
Vec3 m_mDipole(0, 0, 0);
std::string m_name;
math constexpr m_my0 = 0.0000004 * M_PI;

//0 <= T <= m_currentTimestepSinceWrite
#define posAt(t, n) ((Vec3*) (m_positions + (n) + ((t) * m_numParticles)))
Vec3 *m_positions;

// T = {-1, 0}
#define magAt(t, n) ((Vec3*) (m_magnetization + (n) + ((m_currentTimestepEven ? (t) + 1 : (t) * -1) * m_numParticles)))
Vec3 *m_magnetization;

#define phiAt(n) ((math*) (m_phi + (n)))
math *m_phi;

#define forceAt(n) ((Vec3*) (m_force + (n)))
Vec3 *m_force;

inline Vec3 nablaKernel(Vec3 pos1, Vec3 pos2, math effectiveRadius) {
	Vec3 diff(pos1.x - pos2.x, pos1.y - pos2.y, pos1.z - pos2.z);
	return Vec3(-(945.0*diff.x*sq(sq(effectiveRadius) - sq(diff.x) - sq(diff.y) - sq(diff.z))) / (32 * pow(effectiveRadius, 9)*M_PI),
		-(945.0*diff.y*sq(sq(effectiveRadius) - sq(diff.x) - sq(diff.y) - sq(diff.z))) / (32 * pow(effectiveRadius, 9)*M_PI),
		-(945.0*diff.z*sq(sq(effectiveRadius) - sq(diff.x) - sq(diff.y) - sq(diff.z))) / (32 * pow(effectiveRadius, 9)*M_PI));
}

inline Vec3 hDipole(Vec3 atPos, Vec3 mVector, math my) {
	return Vec3((((-3 * atPos.x*(mVector.x*atPos.x + mVector.y*atPos.y + mVector.z*atPos.z)) / pow(sq(atPos.x) + sq(atPos.y) + sq(atPos.z), 2.5)) + mVector.x / pow(sq(atPos.x) + sq(atPos.y) + sq(atPos.z), 1.5)) / (4 * M_PI*my),
		(((-3 * atPos.y*(mVector.x*atPos.x + mVector.y*atPos.y + mVector.z*atPos.z)) / pow(sq(atPos.x) + sq(atPos.y) + sq(atPos.z), 2.5)) + mVector.y / pow(sq(atPos.x) + sq(atPos.y) + sq(atPos.z), 1.5)) / (4 * M_PI*my),
		(((-3 * atPos.z*(mVector.x*atPos.x + mVector.y*atPos.y + mVector.z*atPos.z)) / pow(sq(atPos.x) + sq(atPos.y) + sq(atPos.z), 2.5)) + mVector.z / pow(sq(atPos.x) + sq(atPos.y) + sq(atPos.z), 1.5)) / (4 * M_PI*my));
}

inline math nablaRijByRijCubed(Vec3 pos1, Vec3 pos2) {
	Vec3 diff(pos1.x - pos2.x, pos1.y - pos2.y, pos1.z - pos2.z);
	return -((3 * sq(diff.x)) / pow(sq(diff.x) + sq(diff.y) + sq(diff.z), 2.5))
		- ((3 * sq(diff.y)) / pow(sq(diff.x) + sq(diff.y) + sq(diff.z), 2.5))
		- ((3 * sq(diff.z)) / pow(sq(diff.x) + sq(diff.y) + sq(diff.z), 2.5))
		+ (3 / pow(sq(diff.x) + sq(diff.y) + sq(diff.z), 1.5));
}

inline math nablaChiH(size_t numParticle) {
	size_t oneTimestepAgo = m_currentTimestepSinceWrite < 1 ? m_currentTimestepSinceWrite - 1 + m_timestepsRam : m_currentTimestepSinceWrite - 1;
	math output = 0.0;
	Vec3 nKernel = Vec3(0, 0, 0);
	for (size_t i = 0; i < m_numParticles; i++) {
		if (i == numParticle)
			continue;
		nKernel = nablaKernel(*posAt(oneTimestepAgo, numParticle), *posAt(oneTimestepAgo, i), m_radiusParticle);
		output += nKernel.x * (magAt(-1, i))->x;
		output += nKernel.y * (magAt(-1, i))->y;
		output += nKernel.z * (magAt(-1, i))->z;
	}
	output *= m_volumeParticle;
	output *= m_chi;
	return output;
}

inline Vec3 H(size_t numParticle) {
	size_t oneTimestepAgo = m_currentTimestepSinceWrite < 1 ? m_currentTimestepSinceWrite - 1 + m_timestepsRam : m_currentTimestepSinceWrite - 1;
	Vec3 pos = *posAt(oneTimestepAgo, numParticle);
	Vec3 currentPos(0, 0, 0);
	Vec3 diff(0, 0, 0);
	Vec3 result = hDipole(pos, m_mDipole, m_myR * m_my0);
	Vec3 sum(0, 0, 0);
	Vec3 mag(0, 0, 0); 
	for (size_t i = 0; i < m_numParticles; i++) {
		if (i == numParticle)
			continue;
		currentPos = *posAt(oneTimestepAgo, i);
		diff.x = currentPos.x - pos.x;
		diff.y = currentPos.y - pos.y;
		diff.z = currentPos.z - pos.z;

		math factor1 = nablaChiH(i) / sqrt(sq(diff.x) + sq(diff.y) + sq(diff.z));
		sum.x += diff.x * factor1;
		sum.y += diff.y * factor1;
		sum.z += diff.z * factor1;

		math factor2 = m_chi * nablaRijByRijCubed(currentPos, pos);
		mag = *magAt(-1, i);
		sum.x += mag.x * factor2;
		sum.y += mag.y * factor2;
		sum.z += mag.z * factor2;
	}

	math factor = m_volumeParticle / (4 * M_PI * m_myR * m_my0);
	sum.x *= factor;
	sum.y *= factor;
	sum.z *= factor;

	result.x -= sum.x;
	result.y -= sum.y;
	result.z -= sum.z;

	return result;
}

inline void computePhi() {
	Vec3 h(0, 0, 0);
	math phi = 0.0;
	for (size_t i = 0; i < m_numParticles; i++) {
		h = H(i);
		phi = m_my0 * m_myR * sq(h.x) + sq(h.y) + sq(h.z) / 2.0;
		*phiAt(i) = phi < 0 ? -phi : phi;
	}
}

inline Vec3 fMag(size_t numParticle) {
	size_t oneTimestepAgo = m_currentTimestepSinceWrite < 1 ? m_currentTimestepSinceWrite - 1 + m_timestepsRam : m_currentTimestepSinceWrite - 1;
	Vec3 result(0, 0, 0);
	Vec3 sum(0, 0, 0);
	math phi = 0.0;
	for (size_t i = 0; i < m_numParticles; i++) {
		if (i == numParticle)
			continue;
		phi = *phiAt(i);
		sum = nablaKernel(*posAt(oneTimestepAgo, numParticle), *posAt(oneTimestepAgo, i), m_radiusParticle);
		result.x += sum.x * phi;
		result.y += sum.y * phi;
		result.z += sum.z * phi;
	}
	result.x *= -m_volumeParticle;
	result.y *= -m_volumeParticle;
	result.z *= -m_volumeParticle;

	return result;
}

inline Vec3 fOberflaecheDops(size_t numParticle) {
	size_t oneTimestepAgo = m_currentTimestepSinceWrite < 1 ? m_currentTimestepSinceWrite - 1 + m_timestepsRam : m_currentTimestepSinceWrite - 1;
	math d = 0.0;
	math force = 0.0;
	math max = 0.0;
	math maxDops = 0.0;
	Vec3 *particle;
	Vec3 currentParticle = *posAt(oneTimestepAgo, numParticle);
	Vec3 distance(0,0,0);

	Vec3 result(0, 0, 0);
	for (size_t i = 0; i < m_numParticles; i++)
	{
		if (i == numParticle) 
			continue;

		particle = posAt(oneTimestepAgo, i);
		distance.x = currentParticle.x - particle->x;
		distance.y = currentParticle.y - particle->y;
		distance.z = currentParticle.z - particle->z;

		d = sqrt(sq(distance.x) + sq(distance.y) + sq(distance.z));
		max = (4.0 * m_radiusOil) - d;

		maxDops = ((-5.0 * m_gravity) * ((2 * m_radiusOil) / d + 0.1) + (5.5 * m_gravity))/ 300;

		if (max < 0.0)
			continue;

		force = m_kSTension / d * max / 900.0;
		result.x -= force * distance.x;
		result.y -= force * distance.y;
		result.z -= force * distance.z;

		if (maxDops < 0.0)
			continue;

		result.x += maxDops * distance.x;
		result.y += maxDops * distance.y;
		result.z += maxDops * distance.z;
	}

	return result;
}

inline void calculate() {
	computePhi();
	size_t oneTimestepAgo = m_currentTimestepSinceWrite < 1 ? m_currentTimestepSinceWrite - 1 + m_timestepsRam : m_currentTimestepSinceWrite - 1;
	size_t twoTimestepAgo = m_currentTimestepSinceWrite < 2 ? m_currentTimestepSinceWrite - 2 + m_timestepsRam : m_currentTimestepSinceWrite - 2;

	for (size_t i = 0; i < m_numParticles; i++) {
		Vec3 *f = forceAt(i);
		//*f = fMag(i);
		*f = Vec3(0, 0, 0);

		f->z -= m_gravity * m_massOil;

		math frictionCoefficient = 6 * M_PI * m_viscosity * m_radiusParticle;
		f->x -= frictionCoefficient * ((*posAt(twoTimestepAgo, i)).x - (*posAt(oneTimestepAgo, i)).x) * m_timestepS;
		f->y -= frictionCoefficient * ((*posAt(twoTimestepAgo, i)).y - (*posAt(oneTimestepAgo, i)).y) * m_timestepS;
		f->z -= frictionCoefficient * ((*posAt(twoTimestepAgo, i)).z - (*posAt(oneTimestepAgo, i)).z) * m_timestepS;
		
		Vec3 surfaceTensionDops = fOberflaecheDops(i);
		f->x += surfaceTensionDops.x;
		f->y += surfaceTensionDops.y;
		f->z += surfaceTensionDops.z;
	}

	Vec3 v(0, 0, 0);
	Vec3 *posN;
	Vec3 *posA;
	for (size_t i = 0; i < m_numParticles; i++) {
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

inline void simulate() {
	for (long ts = 0; ts < m_numTimestep; ts++) {
		printf("Timestep %i\r\n", ts);
		calculate();
		m_currentTimestep++;
		m_currentTimestepEven = !m_currentTimestepEven;
		m_currentTimestepSinceWrite++;

		if (m_currentTimestepSinceWrite > m_timestepsRam) {
			m_currentTimestepSinceWrite = 0;
			FILE* file = fopen((m_name + ".sim").c_str(), "ab");
     		fwrite(((math*)m_positions), sizeof(math), (m_numParticles) * (m_timestepsRam) * 3, file);
			fclose(file);
		}
	}
	if (m_currentTimestepSinceWrite = !0) {
		FILE* file = fopen((m_name + ".sim").c_str(), "ab");
		fwrite(((math*)m_positions), sizeof(math), (m_numParticles)* (m_currentTimestepSinceWrite - 1) * 3, file);
		fclose(file);
	}
}

void init(size_t num_particles, size_t timesteps_ram, long timestep_us, math width_basin, math viscosityOil, math chi, math myR, math sigma, math radius_particle, math radius_oil, math mass_oil, Vec3 mDipole, long num_timesteps) {
	m_name = std::to_string(time(0));
	m_numParticles = num_particles;
	m_timestepsRam = timesteps_ram;
	m_timestepS = ((math)timestep_us) / 1000000.0;
	m_numTimestep = num_timesteps;
	m_currentTimestep = 0;
	m_currentTimestepSinceWrite = 0;
	m_currentTimestepEven = true;
	m_widthBasin = width_basin;
	m_chi = chi;
	m_myR = myR;
	m_sigma = sigma;
	m_kSTension = (-2.0 * m_sigma) / (-4.0 + sqrt(6.0));
	m_radiusOil = radius_oil;
	m_areaOil = M_PI * pow(radius_oil, 2);
	m_volumeOil = (4.0 / 3.0) * M_PI * pow(radius_oil, 3);
	m_radiusParticle = radius_particle;
	m_volumeParticle = (4.0 / 3.0) * M_PI * pow(radius_particle, 3);
	m_massOil = mass_oil;
	m_mDipole = mDipole;
	m_viscosity = viscosityOil;

	m_positions = (Vec3*)malloc(sizeof(Vec3) * timesteps_ram * num_particles);
	m_magnetization = (Vec3*)malloc(sizeof(Vec3) * 2 * num_particles);
	m_phi = (math*)malloc(sizeof(math) * num_particles);
	m_force = (Vec3*)malloc(sizeof(Vec3) * num_particles);

	//Build initial Particles
	{
		size_t usedParticles = 0;
		math height = 0.0f;
		while (usedParticles < m_numParticles) {
			*posAt(0, usedParticles) = Vec3(0.0f, 0.0f, height);
			*posAt(m_timestepsRam - 1, usedParticles++) = Vec3(0.0f, 0.0f, height);
			for (math width = m_radiusOil * 2; width < m_widthBasin; width += m_radiusOil * 2) {
				size_t toDistribute = (2 * M_PI * width) / (2 * m_radiusOil);
				if (toDistribute > (m_numParticles - usedParticles)) toDistribute = (m_numParticles - usedParticles);
				math alpha = (2 * M_PI) / toDistribute;
				math currentAlpha = 0;
				for (size_t particle = 0; particle < toDistribute; particle++) {
					*posAt(0, usedParticles) = Vec3(cos(alpha*particle) * width, sin(alpha*particle) * width, height);
					*posAt(m_timestepsRam - 1, usedParticles++) = Vec3(cos(alpha*particle) * width, sin(alpha*particle) * width, height);
					currentAlpha += alpha;
				}
				if (usedParticles == m_numParticles) break;
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
	
	FILE* file = fopen((m_name + ".sim.metadata").c_str(), "w");
	fwrite(metadata.c_str(), sizeof(char), metadata.length(), file);
	fclose(file);

	system("java -jar BinToHRF.jar");

	free(m_positions);
	free(m_phi);
	free(m_magnetization);
	free(m_force);
}