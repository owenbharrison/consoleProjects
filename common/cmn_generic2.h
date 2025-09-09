#pragma once
#ifndef GENERIC2_STRUCT
#define GENERIC2_STRUCT

#include <cmath>

namespace cmn {
	template<typename T>
	struct generic2 {
		T x=0, y=0;

		generic2() {}
		generic2(T _x, T _y) : x(_x), y(_y) {}
		generic2(const generic2& v) : x(v.x), y(v.y) {}

		float& operator[](int i) {
			if(i==1) return y;
			return x;
		}

		const float& operator[](int i) const {
			if(i==1) return y;
			return x;
		}

		generic2& operator=(const generic2& v)=default;

		T dot(const generic2& v) const { return x*v.x+y*v.y; }
		T mag2() const { return this->dot(*this); }
		T mag() const { return std::sqrt(mag2()); }

		generic2 norm() const { 
			//dont divide by zero...
			T mag_sq=mag2();
			if(mag_sq==0) return {1, 0};
			return operator/(std::sqrt(mag_sq));
		}

		generic2 operator-() const { return {-x, -y}; }
		generic2 operator+(const generic2& v) const { return {x+v.x, y+v.y}; }
		generic2 operator+(const T& s) const { return operator+({s, s}); }
		generic2 operator-(const generic2& v) const { return operator+(-v); }
		generic2 operator-(const T& s) const { return operator+(-s); }
		generic2 operator*(const generic2& v) const { return {x*v.x, y*v.y}; }
		generic2 operator*(const T& s) const { return operator*({s, s}); }
		generic2 operator/(const generic2& v) const { return {x/v.x, y/v.y}; }
		generic2 operator/(const T& s) const { T r=1/s; return operator*({r, r}); }//opt?

		//i never really use these, but whatever
		generic2& operator+=(const generic2& v) { *this=*this+v; return*this; }
		generic2& operator+=(const T& v) { *this=*this+v; return*this; }
		generic2& operator-=(const generic2& v) { *this=*this-v; return*this; }
		generic2& operator-=(const T& v) { *this=*this-v; return*this; }
		generic2& operator*=(const generic2& v) { *this=*this*v; return*this; }
		generic2& operator*=(const T& v) { *this=*this*v; return*this; }
		generic2& operator/=(const generic2& v) { *this=*this/v; return*this; }
		generic2& operator/=(const T& v) { *this=*this/v; return*this; }
	};

	template<typename T> generic2<T> operator+(const T& s, const generic2<T>& v) { return v+s; }
	template<typename T> generic2<T> operator-(const T& s, const generic2<T>& v) { return -v+s; }
	template<typename T> generic2<T> operator*(const T& s, const generic2<T>& v) { return v*s; }
	template<typename T> generic2<T> operator/(const T& s, const generic2<T>& v) { return {s/v.x, s/v.y}; }

	typedef generic2<float> float2;
}
#endif//GENERIC2_STRUCT