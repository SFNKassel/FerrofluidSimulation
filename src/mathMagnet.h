#pragma once

#include "simulation.h"

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
	return -((3*sq(diff.x))/pow(sq(diff.x) + sq(diff.y) + sq(diff.z), 2.5))
		-((3 * sq(diff.y)) / pow(sq(diff.x) + sq(diff.y) + sq(diff.z), 2.5))
		-((3 * sq(diff.z)) / pow(sq(diff.x) + sq(diff.y) + sq(diff.z), 2.5))
		+(3 / pow(sq(diff.x) + sq(diff.y) + sq(diff.z), 1.5));
}

inline math nablaChiH(size_t numParticle) {
	math output = 0.0;
	Vec3 nKernel = Vec3(0, 0, 0);
	for (size_t i = 0; i < m_numParticles; i++) {
		if (i == numParticle)
			continue;
		nKernel = nablaKernel(*posAt(m_currentTimestepSinceWrite, numParticle), *posAt(m_currentTimestepSinceWrite, i), m_radiusParticle);
		output += nKernel.x * (magAt(-1, i))->x;
		output += nKernel.y * (magAt(-1, i))->y;
		output += nKernel.z * (magAt(-1, i))->z;
	}
	output *= m_volumeParticle;
	output *= m_chi;
	return output;
}

inline Vec3 H(size_t numParticle) {
	Vec3 pos = *posAt(m_currentTimestepSinceWrite, numParticle);
	Vec3 currentPos(0, 0, 0);
	Vec3 diff(0, 0, 0);
	Vec3 result = hDipole(pos, m_mDipole, m_myR * m_my0);
	Vec3 sum(0, 0, 0);
	Vec3 mag(0, 0, 0);
	for (size_t i = 0; i < m_numParticles; i++) {
		if (i == numParticle)
			continue;
		currentPos = *posAt(m_currentTimestepSinceWrite, i);
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
	Vec3 result(0, 0, 0);
	Vec3 sum(0, 0, 0);
	math phi = 0.0;
	for (size_t i = 0; i < m_numParticles; i++) {
		if (i == numParticle)
			continue;
		phi = *phiAt(i);
		sum = nablaKernel(*posAt(m_currentTimestepSinceWrite, numParticle), *posAt(m_currentTimestepSinceWrite, i), m_radiusParticle);
		result.x += sum.x * phi;
		result.y += sum.y * phi;
		result.z += sum.z * phi;
	}
	result.x *= -m_volumeParticle;
	result.y *= -m_volumeParticle;
	result.z *= -m_volumeParticle;

	return result;
}