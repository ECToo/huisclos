// Hopefully (parts of) this file won't always be needed.

#ifndef __IO_HPP__
#define __IO_HPP__

// Hack macro to reorient coords:
#define TRANSPOSE_2D_COORDS_DAMMIT(X,Y) vector3df(Y,0,-X)

#include <sstream>
#include <string>
#include <irrlicht.h>

#ifndef NDEBUG
#include <iostream>
#define dpr( val ) (std::wcout << val << std::endl);
#else
#define dpr( val )
#endif

//template <typename T>
//inline void pr( const T& obj )
//{	std::cout << obj;	}// pr()

// "Normalize," or "rationalize," or whatever, an angle:
inline irr::f32 rationalizeAngle( irr::f32 theta )
{
	if( theta >= 360.0f ) {	theta -= 360.0f;	}// if
	else if( theta < 0.0f ) {	theta += 360.0f;	}// eif

	return theta;
}// rationalizeAngle()

inline std::wostream& operator<< (std::wostream& out, const irr::core::vector3df& vec)
{
	// TODO: Round down a few digits.
	return out << L'(' << vec.X << L',' << vec.Y << L',' << vec.Z << L')';
}// <<()
//inline std::string to_s( const irr::core::vector3df& vec )
//{
	//std::wostringstream out;
	//return (out << vec << std::endl).str();
//}// to_s()


inline std::wostream& operator<< (std::wostream& out, const irr::core::position2di& pos )
{
	return out << L'(' << pos.X << L',' << pos.Y << L')';
}// <<()
//inline std::string to_s( const irr::core::position2di& pos )
//{
	//std::wostringstream out;
	//return std::string( (out << pos).str() );
//}// to_s()

inline std::wostream& operator<< (std::wostream& out, const irr::core::vector2df& pos )
{
	// TODO: Round down a few digits.
	return out << L'(' << pos.X << L',' << pos.Y << L')';
}// <<()
//inline std::string to_s( const irr::core::vector2df& pos )
//{
	//std::wostringstream out;
	//return (out << pos).str();
//}// to_s()

inline irr::core::stringw transposeVectorCoordsDammit( const irr::core::vector3df& v )
{
	irr::core::stringw out;
	out +=  L'(';
	out += -(v.Z);
       	out += L',';
       	out += v.X;
	out += L')';
	return out;
}// transposeVectorCoordsDammit()

inline irr::f32 reorientGlobalAngleDammit( const irr::f32 theta )
{
	return 90 - theta;
}// reorientGlobalAngleDammit()


#endif

