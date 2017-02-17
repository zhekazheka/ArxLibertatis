/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_MATH_RANDOM_H
#define ARX_MATH_RANDOM_H

#include <limits>

#include <boost/type_traits.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>

#include <glm/gtc/constants.hpp>

#include "math/Types.h"
#include "platform/Platform.h"


#define ARX_RANDOM_DEBUG

#ifdef ARX_RANDOM_DEBUG
	#include "platform/Thread.h"
#endif

/*!
 * Random number generator.
 */
class Random {
	
public:
	
	//! Generates a random integer value in the range [intMin, intMax].
	template <typename IntType> IntType get();
	template <typename IntType> IntType get(IntType min, IntType max);
	int get(int min = 0, int max = std::numeric_limits<int>::max());
	unsigned int getu(unsigned int min, unsigned int max);
	
	//! Generates a random floating point value in the range [realMin, realMax).
	template <class RealType> RealType getf();
	template <class RealType> RealType getf(RealType realMin, RealType realMax);
	float getf(float realMin = 0.0f, float realMax = 1.0f);
	
	//! Return a random iterator pointing in the range [begin, end).
	template <class Iterator>
	Iterator getIterator(Iterator begin, Iterator end);
	
	//! Return a random iterator in the given container.
	template <class Container>
	typename Container::iterator getIterator(Container& container);
	
	//! Return a random const_iterator in the given container.
	template <class Container>
	typename Container::const_iterator getIterator(const Container& container);
	
	inline Vec3f randomOffsetXZ(float range);
	
	/*!
	 * Generate a random vertor with independently unform distributed components.
	 *
	 * \param min minimum value for all components (default: 0.f)
	 * \param max maximum value for all components (default: 1.f)
	 */
	inline Vec3f randomVec(float min = 0.f, float max = 1.f);
	
	inline Vec3f randomVec3f();
	
	inline Vec2f linearRand(Vec2f const & min, Vec2f const & max);
	
	inline Vec3f linearRand(Vec3f const & min, Vec3f const & max);
	
	//! Generate a random 2D vector which coordinates are regulary distributed within the area of a disk of a given radius
	inline Vec2f diskRand(float radius);
	
	//! Generate a random 2D vector which coordinates areregulary distributed on a circle of a given radius
	inline Vec2f circularRand(float radius);
	
	//! Generate a random 3D vector which coordinates are regulary distributed on a sphere of a given radius
	inline Vec3f sphericalRand(float radius);
	
	
	//! Seed the random number generator using the current time.
	void seed();
	
	//! Seed the random number generator with the given value.
	void seed(unsigned int seedVal);
	
private:
	
	typedef boost::random::mt19937 Generator;
	
	Generator rng;
	
#ifdef ARX_RANDOM_DEBUG
	thread_id_type m_dbg_threadId;
#endif
};

extern Random g_rand;

///////////////////////////////////////////////////////////////////////////////

template <class IntType>
IntType Random::get(IntType min, IntType max) {
	ARX_STATIC_ASSERT(boost::is_integral<IntType>::value, "get must be called with ints");
	
#ifdef ARX_RANDOM_DEBUG
	arx_assert(m_dbg_threadId == Thread::getCurrentThreadId());
#endif
	
	return typename boost::random::uniform_int_distribution<IntType>(min, max)(rng);
}

template <class IntType>
IntType Random::get() {
	return Random::get<IntType>(0, std::numeric_limits<IntType>::max());
}

inline int Random::get(int min, int max) {
	return Random::get<int>(min, max);
}

inline unsigned int Random::getu(unsigned int min, unsigned int max) {
	return Random::get<unsigned int>(min, max);
}

template <class RealType>
RealType Random::getf(RealType min, RealType max) {
	ARX_STATIC_ASSERT(boost::is_float<RealType>::value, "getf must be called with floats");
	
#ifdef ARX_RANDOM_DEBUG
	arx_assert(m_dbg_threadId == Thread::getCurrentThreadId());
#endif
	
	return typename boost::random::uniform_real_distribution<RealType>(min, max)(rng);
}

template <class RealType>
RealType Random::getf() {
	return Random::getf<RealType>(RealType(0.0), RealType(1.0));
}

inline float Random::getf(float min, float max) {
	return Random::getf<float>(min, max);
}

template <class Iterator>
Iterator Random::getIterator(Iterator begin, Iterator end) {
	typedef typename std::iterator_traits<Iterator>::difference_type diff_t;
	
	diff_t dist = std::distance(begin, end);
	diff_t toAdvance = Random::get<diff_t>(0, dist-1);
	
	std::advance(begin, toAdvance);
	
	return begin;
}

template <class Container>
typename Container::iterator Random::getIterator(Container& container) {
	return getIterator(container.begin(), container.end());
}

template <class Container>
typename Container::const_iterator Random::getIterator(const Container& container) {
	return getIterator(container.begin(), container.end());
}


inline Vec3f Random::randomOffsetXZ(float range) {
	return Vec3f(getf(-range, range), 0.f, getf(-range, range));
}

/*!
 * Generate a random vertor with independently unform distributed components.
 *
 * \param min minimum value for all components (default: 0.f)
 * \param max maximum value for all components (default: 1.f)
 */
inline Vec3f Random::randomVec(float min, float max) {
	float range = max - min;
	return Vec3f(getf() * range + min, getf() * range + min, getf() * range + min);
}

inline Vec3f Random::randomVec3f() {
	return Vec3f(getf(), getf(), getf());
}

inline Vec2f Random::linearRand(Vec2f const & min, Vec2f const & max) {
	return Vec2f(getf(min.x, max.x), getf(min.y, max.y));
}

inline Vec3f Random::linearRand(Vec3f const & min, Vec3f const & max) {
	return Vec3f(getf(min.x, max.x), getf(min.y, max.y), getf(min.z, max.z));
}

//! Generate a random 2D vector which coordinates are regulary distributed within the area of a disk of a given radius
inline Vec2f Random::diskRand(float radius) {
	Vec2f result(0);
	float lenRadius(0);
	
	do {
		result = linearRand(Vec2f(-radius), Vec2f(radius));
		lenRadius = glm::length(result);
	} while(lenRadius > radius);
	
	return result;
}

//! Generate a random 2D vector which coordinates areregulary distributed on a circle of a given radius
inline Vec2f Random::circularRand(float radius) {
	float a = getf(float(0), glm::pi<float>() * 2);
	return Vec2f(std::cos(a), std::sin(a)) * radius;
}

//! Generate a random 3D vector which coordinates are regulary distributed on a sphere of a given radius
inline Vec3f Random::sphericalRand(float radius) {
	float z = getf(float(-1), float(1));
	float a = getf(float(0), glm::pi<float>() * 2);
	
	float r = std::sqrt(float(1) - z * z);
	
	float x = r * std::cos(a);
	float y = r * std::sin(a);
	
	return Vec3f(x, y, z) * radius;
}


#endif // ARX_MATH_RANDOM_H
