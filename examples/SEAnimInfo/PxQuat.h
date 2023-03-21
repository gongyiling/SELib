//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#ifndef PXFOUNDATION_PXQUAT_H
#define PXFOUNDATION_PXQUAT_H

/** \addtogroup foundation
@{
*/

#include "PxVec3.h"
#if !PX_DOXYGEN
namespace physx
{
#endif

/**
\brief This is a quaternion class. For more information on quaternion mathematics
consult a mathematics source on complex numbers.

*/

class PxQuat
{
  public:
	/**
	\brief Default constructor, does not do any initialization.
	*/
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat()
	{
	}

	//! identity constructor
	PX_CUDA_CALLABLE PX_INLINE PxQuat(PxIDENTITY r) : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
	{
		PX_UNUSED(r);
	}

	/**
	\brief Constructor from a scalar: sets the real part w to the scalar value, and the imaginary parts (x,y,z) to zero
	*/
	explicit PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat(float r) : x(0.0f), y(0.0f), z(0.0f), w(r)
	{
	}

	/**
	\brief Constructor.  Take note of the order of the elements!
	*/
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat(float nx, float ny, float nz, float nw) : x(nx), y(ny), z(nz), w(nw)
	{
	}

	/**
	\brief Creates from angle-axis representation.

	Axis must be normalized!

	Angle is in radians!

	<b>Unit:</b> Radians
	*/
	PX_CUDA_CALLABLE PX_INLINE PxQuat(float angleRadians, const PxVec3& unitAxis)
	{
		const float a = angleRadians * 0.5f;
		const float s = PxSin(a);
		w = PxCos(a);
		x = unitAxis.x * s;
		y = unitAxis.y * s;
		z = unitAxis.z * s;
	}

	/**
	\brief Copy ctor.
	*/
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat(const PxQuat& v) : x(v.x), y(v.y), z(v.z), w(v.w)
	{
	}

	/**
	\brief Creates from orientation matrix.

	\param[in] m Rotation matrix to extract quaternion from.
	*/
	PX_CUDA_CALLABLE PX_INLINE explicit PxQuat(const PxMat33& m); /* defined in PxMat33.h */

	/**
	\brief returns true if quat is identity
	*/
	PX_CUDA_CALLABLE PX_FORCE_INLINE bool isIdentity() const
	{
		return x==0.0f && y==0.0f && z==0.0f && w==1.0f;
	}

	/**
	\brief returns true if all elements are finite (not NAN or INF, etc.)
	*/
	PX_CUDA_CALLABLE bool isFinite() const
	{
		return PxIsFinite(x) && PxIsFinite(y) && PxIsFinite(z) && PxIsFinite(w);
	}

	/**
	\brief returns true if finite and magnitude is close to unit
	*/
	PX_CUDA_CALLABLE bool isUnit() const
	{
		const float unitTolerance = 1e-4f;
		return isFinite() && PxAbs(magnitude() - 1) < unitTolerance;
	}

	/**
	\brief returns true if finite and magnitude is reasonably close to unit to allow for some accumulation of error vs
	isValid
	*/
	PX_CUDA_CALLABLE bool isSane() const
	{
		const float unitTolerance = 1e-2f;
		return isFinite() && PxAbs(magnitude() - 1) < unitTolerance;
	}

	/**
	\brief returns true if the two quaternions are exactly equal
	*/
	PX_CUDA_CALLABLE PX_INLINE bool operator==(const PxQuat& q) const
	{
		return x == q.x && y == q.y && z == q.z && w == q.w;
	}

	/**
	\brief converts this quaternion to angle-axis representation
	*/
	PX_CUDA_CALLABLE PX_INLINE void toRadiansAndUnitAxis(float& angle, PxVec3& axis) const
	{
		const float quatEpsilon = 1.0e-8f;
		const float s2 = x * x + y * y + z * z;
		if(s2 < quatEpsilon * quatEpsilon) // can't extract a sensible axis
		{
			angle = 0.0f;
			axis = PxVec3(1.0f, 0.0f, 0.0f);
		}
		else
		{
			const float s = PxRecipSqrt(s2);
			axis = PxVec3(x, y, z) * s;
			angle = PxAbs(w) < quatEpsilon ? PxPi : PxAtan2(s2 * s, w) * 2.0f;
		}
	}

	/**
	\brief Gets the angle between this quat and the identity quaternion.

	<b>Unit:</b> Radians
	*/
	PX_CUDA_CALLABLE PX_INLINE float getAngle() const
	{
		return PxAcos(w) * 2.0f;
	}

	/**
	\brief Gets the angle between this quat and the argument

	<b>Unit:</b> Radians
	*/
	PX_CUDA_CALLABLE PX_INLINE float getAngle(const PxQuat& q) const
	{
		return PxAcos(dot(q)) * 2.0f;
	}

	/**
	\brief This is the squared 4D vector length, should be 1 for unit quaternions.
	*/
	PX_CUDA_CALLABLE PX_FORCE_INLINE float magnitudeSquared() const
	{
		return x * x + y * y + z * z + w * w;
	}

	/**
	\brief returns the scalar product of this and other.
	*/
	PX_CUDA_CALLABLE PX_FORCE_INLINE float dot(const PxQuat& v) const
	{
		return x * v.x + y * v.y + z * v.z + w * v.w;
	}

	PX_CUDA_CALLABLE PX_INLINE PxQuat getNormalized() const
	{
		const float s = 1.0f / magnitude();
		return PxQuat(x * s, y * s, z * s, w * s);
	}

	PX_CUDA_CALLABLE PX_INLINE float magnitude() const
	{
		return PxSqrt(magnitudeSquared());
	}

	// modifiers:
	/**
	\brief maps to the closest unit quaternion.
	*/
	PX_CUDA_CALLABLE PX_INLINE float normalize() // convert this PxQuat to a unit quaternion
	{
		const float mag = magnitude();
		if(mag != 0.0f)
		{
			const float imag = 1.0f / mag;

			x *= imag;
			y *= imag;
			z *= imag;
			w *= imag;
		}
		return mag;
	}

	/*
	\brief returns the conjugate.

	\note for unit quaternions, this is the inverse.
	*/
	PX_CUDA_CALLABLE PX_INLINE PxQuat getConjugate() const
	{
		return PxQuat(-x, -y, -z, w);
	}

	/*
	\brief returns imaginary part.
	*/
	PX_CUDA_CALLABLE PX_INLINE PxVec3 getImaginaryPart() const
	{
		return PxVec3(x, y, z);
	}

	/** brief computes rotation of x-axis */
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxVec3 getBasisVector0() const
	{
		const float x2 = x * 2.0f;
		const float w2 = w * 2.0f;
		return PxVec3((w * w2) - 1.0f + x * x2, (z * w2) + y * x2, (-y * w2) + z * x2);
	}

	/** brief computes rotation of y-axis */
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxVec3 getBasisVector1() const
	{
		const float y2 = y * 2.0f;
		const float w2 = w * 2.0f;
		return PxVec3((-z * w2) + x * y2, (w * w2) - 1.0f + y * y2, (x * w2) + z * y2);
	}

	/** brief computes rotation of z-axis */
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxVec3 getBasisVector2() const
	{
		const float z2 = z * 2.0f;
		const float w2 = w * 2.0f;
		return PxVec3((y * w2) + x * z2, (-x * w2) + y * z2, (w * w2) - 1.0f + z * z2);
	}

	/**
	rotates passed vec by this (assumed unitary)
	*/
	PX_CUDA_CALLABLE PX_FORCE_INLINE const PxVec3 rotate(const PxVec3& v) const
	{
		const float vx = 2.0f * v.x;
		const float vy = 2.0f * v.y;
		const float vz = 2.0f * v.z;
		const float w2 = w * w - 0.5f;
		const float dot2 = (x * vx + y * vy + z * vz);
		return PxVec3((vx * w2 + (y * vz - z * vy) * w + x * dot2), (vy * w2 + (z * vx - x * vz) * w + y * dot2),
		              (vz * w2 + (x * vy - y * vx) * w + z * dot2));
	}

	/**
	inverse rotates passed vec by this (assumed unitary)
	*/
	PX_CUDA_CALLABLE PX_FORCE_INLINE const PxVec3 rotateInv(const PxVec3& v) const
	{
		const float vx = 2.0f * v.x;
		const float vy = 2.0f * v.y;
		const float vz = 2.0f * v.z;
		const float w2 = w * w - 0.5f;
		const float dot2 = (x * vx + y * vy + z * vz);
		return PxVec3((vx * w2 - (y * vz - z * vy) * w + x * dot2), (vy * w2 - (z * vx - x * vz) * w + y * dot2),
		              (vz * w2 - (x * vy - y * vx) * w + z * dot2));
	}

	/**
	\brief Assignment operator
	*/
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat& operator=(const PxQuat& p)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		w = p.w;
		return *this;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat& operator*=(const PxQuat& q)
	{
		const float tx = w * q.x + q.w * x + y * q.z - q.y * z;
		const float ty = w * q.y + q.w * y + z * q.x - q.z * x;
		const float tz = w * q.z + q.w * z + x * q.y - q.x * y;

		w = w * q.w - q.x * x - y * q.y - q.z * z;
		x = tx;
		y = ty;
		z = tz;

		return *this;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat& operator+=(const PxQuat& q)
	{
		x += q.x;
		y += q.y;
		z += q.z;
		w += q.w;
		return *this;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat& operator-=(const PxQuat& q)
	{
		x -= q.x;
		y -= q.y;
		z -= q.z;
		w -= q.w;
		return *this;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat& operator*=(const float s)
	{
		x *= s;
		y *= s;
		z *= s;
		w *= s;
		return *this;
	}

	/** quaternion multiplication */
	PX_CUDA_CALLABLE PX_INLINE PxQuat operator*(const PxQuat& q) const
	{
		return PxQuat(w * q.x + q.w * x + y * q.z - q.y * z, w * q.y + q.w * y + z * q.x - q.z * x,
		              w * q.z + q.w * z + x * q.y - q.x * y, w * q.w - x * q.x - y * q.y - z * q.z);
	}

	/** quaternion addition */
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat operator+(const PxQuat& q) const
	{
		return PxQuat(x + q.x, y + q.y, z + q.z, w + q.w);
	}

	/** quaternion subtraction */
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat operator-() const
	{
		return PxQuat(-x, -y, -z, -w);
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat operator-(const PxQuat& q) const
	{
		return PxQuat(x - q.x, y - q.y, z - q.z, w - q.w);
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE PxQuat operator*(float r) const
	{
		return PxQuat(x * r, y * r, z * r, w * r);
	}

	/** the quaternion elements */
	float x, y, z, w;
};

PxQuat slerp(const float t, const PxQuat& left, const PxQuat& right)
{
	const float quatEpsilon = (float(1.0e-8f));

	float cosine = left.dot(right);
	float sign = float(1);
	if (cosine < 0)
	{
		cosine = -cosine;
		sign = float(-1);
	}

	float sine = float(1) - cosine * cosine;

	if (sine >= quatEpsilon * quatEpsilon)
	{
		sine = PxSqrt(sine);
		const float angle = PxAtan2(sine, cosine);
		const float i_sin_angle = float(1) / sine;

		const float leftw = PxSin(angle * (float(1) - t)) * i_sin_angle;
		const float rightw = PxSin(angle * t) * i_sin_angle * sign;

		return left * leftw + right * rightw;
	}

	return left;
}

PxQuat from_rotator(float pitch, float yaw, float roll)
{
	const float DEG_TO_RAD = PxPi / (180.f);
	const float RADS_DIVIDED_BY_2 = DEG_TO_RAD / 2.f;
	float SP, SY, SR;
	float CP, CY, CR;

	const float PitchNoWinding = pitch;
	const float YawNoWinding = yaw;
	const float RollNoWinding = roll;

	sincos(PitchNoWinding * RADS_DIVIDED_BY_2 , SP, CP);
	sincos(YawNoWinding * RADS_DIVIDED_BY_2 , SY, CY);
	sincos(RollNoWinding * RADS_DIVIDED_BY_2 , SR, CR);

	PxQuat RotationQuat;
	RotationQuat.x = CR * SP * SY - SR * CP * CY;
	RotationQuat.y = -CR * SP * CY - SR * CP * SY;
	RotationQuat.z = CR * CP * SY - SR * SP * CY;
	RotationQuat.w = CR * CP * CY + SR * SP * SY;
	return RotationQuat;
}

PxQuat from_rotator(const PxVec3& rotator)
{
	// pitch = y
	// yaw = z
	// roll = x
	return from_rotator(rotator.y, rotator.z, rotator.x);
}

float sqr(float x)
{
	return x * x;
}

float NormalizeAxis(float x)
{
	return x;
}

PxVec3 to_rotator(const PxQuat& q)
{
	const float SingularityTest = q.z * q.x - q.w * q.y;
	const float YawY = 2.f * (q.w * q.z + q.x * q.y);
	const float YawX = (1.f - 2.f * (sqr(q.y) + sqr(q.z)));

	// reference 
	// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/

	// this value was found from experience, the above websites recommend different values
	// but that isn't the case for us, so I went through different testing, and finally found the case 
	// where both of world lives happily. 
	const float SINGULARITY_THRESHOLD = 0.4999995f;
	const float RAD_TO_DEG = (180.f) / PxPi;
	float Pitch, Yaw, Roll;
	if (SingularityTest < -SINGULARITY_THRESHOLD)
	{
		Pitch = -90.f;
		Yaw = PxAtan2(YawY, YawX) * RAD_TO_DEG;
		Roll = NormalizeAxis(-Yaw - (2.f * PxAtan2(q.x, q.w) * RAD_TO_DEG));
	}
	else if (SingularityTest > SINGULARITY_THRESHOLD)
	{
		Pitch = 90.f;
		Yaw = PxAtan2(YawY, YawX) * RAD_TO_DEG;
		Roll = NormalizeAxis(Yaw - (2.f * PxAtan2(q.x, q.w) * RAD_TO_DEG));
	}
	else
	{
		Pitch = PxAsin(2.f * (SingularityTest)) * RAD_TO_DEG;
		Yaw = PxAtan2(YawY, YawX) * RAD_TO_DEG;
		Roll = PxAtan2(-2.f * (q.w * q.x + q.y * q.z), (1.f - 2.f * (sqr(q.x) +sqr(q.y)))) * RAD_TO_DEG;
	}
	return PxVec3(Roll, Pitch, Yaw);
}


#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif // #ifndef PXFOUNDATION_PXQUAT_H
