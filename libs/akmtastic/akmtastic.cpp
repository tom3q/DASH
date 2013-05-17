/*
 * libAKMtastic - free open source replacement for proprietary AKM binaries
 *
 * Copyright (C) 2012 Tomasz Figa <tomasz.figa@gmail.com>
 *
 * Heavily based on akmd-free by Antti S. Lankila et al.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

extern "C" {
#include "SEMC_APIs.h"
};

#include <cmath>

/*
 * Akmtastic implementation
 */

class Vector {
	float v[3];

public:
	Vector()
	{
		v[0] = v[1] = v[2] = 0.0f;
	}

	Vector(float x, float y, float z)
	{
		set(x, y, z);
	}

	static Vector multiply(const Vector &a, const Vector &b)
	{
		Vector res = a;
		res.multiply(b);
		return res;
	}

	static Vector multiply(const Vector &a, float b)
	{
		Vector res = a;
		res.multiply(b);
		return res;
	}

	static Vector divide(const Vector &a, float b)
	{
		Vector res = a;
		res.divide(b);
		return res;
	}

	void set(float x, float y, float z)
	{
		v[0] = x;
		v[1] = y;
		v[2] = z;
	}

	void multiply(const Vector &m)
	{
		v[0] *= m.v[0];
		v[1] *= m.v[1];
		v[2] *= m.v[2];
	}

	void multiply(float m)
	{
		v[0] *= m;
		v[1] *= m;
		v[2] *= m;
	}

	void divide(float d)
	{
		v[0] /= d;
		v[1] /= d;
		v[2] /= d;
	}

	void add(const Vector &m)
	{
		v[0] += m.v[0];
		v[1] += m.v[1];
		v[2] += m.v[2];
	}

	float length(void) const
	{
		return sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	}

	float dot(const Vector &o) const
	{
		return v[0]*o.v[0] + v[1]*o.v[1] + v[2]*o.v[2];
	}

	float x(void) const
	{
		return v[0];
	}

	float y(void) const
	{
		return v[1];
	}

	float z(void) const
	{
		return v[2];
	}
};

class Matrix {

public:
	Matrix()
	{

	}
};

class Average {
public:
	const Vector &push(const Vector &v)
	{
		return v;
	}
};

class Calibrator {
	Vector scale;
	Vector translation;

public:
	Calibrator()
	{
	}

	virtual ~Calibrator()
	{
	}

	virtual void push(const Vector &val)
	{

	}

	virtual void fix(Vector &val)
	{
		val.add(translation);
		val.multiply(scale);
	}

	virtual void reset(void)
	{
	}
};

class AccCalibrator : public Calibrator {
	static const int REFRESH = 10;

	/*
	 * Demand length to match with the long-term average before the vector
	 * is trusted to represent gravity.
	 */
	static const float ERROR = 0.05f;

	/* Exponential average applied on acceleration to estimate gravity. */
	static const float GRAVITY_SMOOTH = 0.8f;

	Vector g;

public:
	AccCalibrator() :
		g()
	{
	}

	virtual void push(const Vector &val)
	{
		g.multiply(GRAVITY_SMOOTH);
		g.add(Vector::multiply(val, 1.0f - GRAVITY_SMOOTH));

		/*
		 * val and g must have about the same length and point to
		 * about same direction before we trust the value accumulated
		 * to g
		 */
		float al = val.length();
		float gl = g.length();

		if (al == 0 || gl == 0) {
			return;
		}

		Vector an = Vector::divide(val, al);
		Vector gn = Vector::divide(g, gl);

		if (fabsf(al - gl) < ERROR && an.dot(gn) > 1.0f - ERROR)
			Calibrator::push(g);
	}

	virtual void fix(Vector &val)
	{
		Calibrator::fix(val);
	}

	virtual void reset(void)
	{
		g.set(0.0f, 0.0f, 0.0f);
		Calibrator::reset();
	}
};

class MagCalibrator : public Calibrator {
public:
	MagCalibrator()
	{
	}

	virtual ~MagCalibrator()
	{
	}

	virtual void push(const Vector &val)
	{
	}

	virtual void fix(Vector &val)
	{
	}

	virtual void reset(void)
	{
		Calibrator::reset();
	}
};

class Akmtastic {
	AccCalibrator accCalibrator;
	Average accAverage;
	Vector accVal;

	MagCalibrator magCalibrator;
	Average magAverage;
	Vector magVal;

	mutable Vector oriVal;
	mutable bool oriCalculated;

	bool started;
	int fd;

public:
	Akmtastic(int maxFormNumber, const register_map_ak897x *regs,
						const int16_t magLayout[3][3]) :
		accCalibrator(),
		magCalibrator(),
		started(false),
		fd(-1)
	{
	}

	~Akmtastic(void)
	{
		if (started)
			stop();
	}

	int start(const char *path)
	{
		if (started)
			return -1;

		started = true;

		return 0;
	}

	int stop(void)
	{
		if (!started)
			return -1;

		started = false;

		return 0;
	}

	int pushAcceleration(int x, int y, int z, int sensitivity)
	{
		accVal = Vector(x, y, z);
		accVal.multiply(720.f / sensitivity);
		accVal = accAverage.push(accVal);
		accCalibrator.push(accVal);
		accCalibrator.fix(accVal);
		oriCalculated = false;
		return 0;
	}

	int pushMagnetic(int x, int y, int z, int status, const int period)
	{
		magVal = Vector(x, y, z);
		magVal = magAverage.push(magVal);
		magCalibrator.push(magVal);
		magCalibrator.fix(magVal);
		oriCalculated = false;
		return 0;
	}

	const Vector &getOrientation(void) const
	{
		if (!oriCalculated) {
			oriCalculated = true;
		}

		return oriVal;
	}

	const Vector &getMagnetic(void) const
	{
		return magVal;
	}

	unsigned int getCalibrationGoodness(void)
	{
		return 0;
	}

	void recalibrate(void)
	{
		accCalibrator.reset();
	}

	int changeFormFactor(int formFactor)
	{
		return 0;
	}
};

/*
 * SEMC_API wrappers
 */

static Akmtastic *self = NULL;

int AKM_Init(int maxFormNumber, const register_map_ak897x *regs,
					const int16_t mag_layout[3][3])
{
	self = new Akmtastic(maxFormNumber, regs, mag_layout);
	if (!self)
		return -1;

	return 0;
}

void AKM_Release(void)
{
	delete self;
	self = 0;
}

int AKM_Start(const char *path)
{
	if (!self)
		return -1;

	return self->start(path);
}

int AKM_Stop(const char *path)
{
	if (!self)
		return -1;

	return self->stop();
}

int AKM_SaveAcc(int acc_x, int acc_y, int acc_z, int acc_sensitivity)
{
	if (!self)
		return -1;

	self->pushAcceleration(acc_x, acc_y, acc_z, acc_sensitivity);
	return 0;
}

int AKM_SaveMag(int mag_x, int mag_y, int mag_z,
					int mag_status, const int period)
{
	if (!self)
		return -1;

	self->pushMagnetic(mag_x, mag_y, mag_z, mag_status, period);
	return 0;
}

int AKM_GetOrientationValues(sensors_event_t *data)
{
	if (!self)
		return -1;

	const Vector &val = self->getOrientation();
	data->orientation.azimuth = val.x();
	data->orientation.pitch = val.y();
	data->orientation.roll = val.z();

	return 0;
}

int AKM_GetMagneticValues(sensors_event_t *data)
{
	if (!self)
		return -1;

	const Vector &val = self->getMagnetic();
	data->magnetic.x = val.x();
	data->magnetic.y = val.y();
	data->magnetic.z = val.z();

	return 0;
}

unsigned int AKM_GetCalibrationGoodness(void)
{
	if (!self)
		return 0;

	return self->getCalibrationGoodness();
}

void AKM_ForceReCalibration(void)
{
	if (self)
		self->recalibrate();
}

int AKM_ChangeFormFactor(int formFactorNumber)
{
	if (!self)
		return -1;

	return self->changeFormFactor(formFactorNumber);
}
