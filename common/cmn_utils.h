#pragma once
#ifndef CMN_UTILS_H
#define CMN_UTILS_H

#include <random>

namespace cmn {
	//clever default param placement
	static float random(float a=1, float b=0) {
		static std::mt19937 rng{std::random_device{}()};
		static std::uniform_real_distribution<float> dist(0, 1);
		float t=dist(rng);
		return a+t*(b-a);
	}

	//clamp x in between a and b.
	template<typename T>
	const T& clamp(const T& x, const T& a, const T& b) {
		if(x<a) return a;
		if(x>b) return b;
		return x;
	}

	//map value from one range into another
	float map(float x, float a, float b, float c, float d) {
		float t=(x-a)/(b-a);
		return c+t*(d-c);
	}
}

#endif