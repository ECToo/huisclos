#ifndef __COORDINATES_HPP__
#define __COORDINATES_HPP__

#include <sstream>
#include <iostream>
#include <string>

#include <irrlicht.h>
using namespace irr;
using namespace irr::core;
using namespace irr::scene;

#include "IO.hpp"

// TODO: The aim is to, as much as possible, eliminate the difference in efficiency b/t Agent#setPosition() and Agent#setAbsolutePosition() and the like.
//** TODO: In order to eliminate the overhead incurred by an approach such as this, internal storage needs to be reworked.
namespace cj
{
	class mouse_vector_2d;
	class absolute_birds_eye_position_vector_2d;
	class relative_birds_eye_vector_2d;
	class absolute_irrlicht_position_vector_3d;
	class relative_irrlicht_vector_3d;		
	//class relative_angle_vector_2d;
	class relative_angle_vector_3d;
	class relAngle;
	class absAngle;

	typedef mouse_vector_2d 			mouseVec;
	typedef absolute_birds_eye_position_vector_2d	absVec; // TODO: absPos2d
	typedef relative_birds_eye_vector_2d		relVec; // TODO: relVec2d
	typedef absolute_irrlicht_position_vector_3d	absPos;
	// TODO: typedef relative_irrlicht_vector_3d		relVec;
	//typedef relative_angle_vector_2d		relAngleVec; // TODO, I suppose.
	typedef relative_angle_vector_3d		relAngleVec3d;

	// id=vec
	class I2dVector
	{
	public:
		// Dtor
		virtual ~I2dVector() {}

		// Getters:
		virtual f32 getX() const { return internalVec.X;	}
		virtual f32 getY() const { return internalVec.Y;	}
		// Idem, but shorter:
		virtual f32 x() const { return internalVec.X;	}
		virtual f32 y() const { return internalVec.Y;	}
		// Setters:
		virtual void setX( f32 x ) { internalVec.X = x;	}
		virtual void setY( f32 y ) { internalVec.Y = y;	}

		virtual f32 getLength() const
		{	return internalVec.getLength();	}// getLength()
		virtual void setLength( f32 newmag )
		{	internalVec.normalize() *= newmag;	}// getLength()
		// Faster & works just as well for some comparisons:
		virtual f32 getLengthSQ() const
		{	return internalVec.getLengthSQ();	}// getLengthSQ()

		// Simple output conversion to (absolute) 2D vector.
		virtual const vector2df& to_vector2df() const
		{	return internalVec;	}// to_vector2df()
		// Simple output conversion to (absolute) 3D vector in Cartesian space; i.e., no conversion Dammit():
		virtual vector3df to_vector3df() const
		{	return vector3df(x(),y(),0.0);	}// to_vector3df()
		// TODO: ?? Remove this, and replace with absVec::toIrr_vector3df_absolute() and relVec::toIrr_vector3df_relative(); for certainty ??
		// Convert to absolute Irrlicht 3D vector: goofy rearrangement.  (This is the one to use when passing to ISceneNode#setPosition() &c. calls.)
		virtual vector3df toIrr_vector3df() const
		{	return vector3df(y(),0.0,-x());	}// toIrr_vector3df()
	protected:
		// Ctor
		I2dVector(f32 x1 = 0.0, f32 y1 = 0.0): internalVec(x1,y1) {}

		vector2df internalVec;
	};// I2dVector

	/// 3D base class:
	class I3dVector
	{
	public:
		// Dtor
		virtual ~I3dVector() {}

		// Getters:
		virtual f32 getX() const { return internalVec.X;	}
		virtual f32 getY() const { return internalVec.Y;	}
		virtual f32 getZ() const { return internalVec.Z;	}
		// Idem, but shorter:
		virtual f32 x() const { return internalVec.X;	}
		virtual f32 y() const { return internalVec.Y;	}
		virtual f32 z() const { return internalVec.Z;	}
		// Setters:
		virtual void setX( f32 x ) { internalVec.X = x;	}
		virtual void setY( f32 y ) { internalVec.Y = y;	}
		virtual void setZ( f32 z ) { internalVec.Z = z;	}

		virtual f32 getLength() const
		{	return internalVec.getLength();	}// getLength()
		virtual void setLength( f32 newmag )
		{	internalVec.normalize() *= newmag;	}// getLength()

		// Simple output conversion to (absolute) 3d vector.
		virtual const vector3df& to_vector3df() const
		{	return internalVec;	}// to_vector3df()

		//virtual vector3df to_vector3df() const
		//{	return vector3df(x(),y(),0.0);	}// to_vector3df()
		// Convert to absolute Irrlicht 3D vector: goofy rearrangement.  (This is the one to use when passing to ISceneNode#setPosition() &c. calls.)
		virtual vector3df toIrr_vector3df() const
		{	return vector3df(y(),0.0,-x());	}// toIrr_vector3df()
	protected:
		// Ctor
		I3dVector(f32 x = 0.0, f32 y = 0.0, f32 z = 0.0): internalVec(x,y,z) {}

		vector3df internalVec;
	};// I3dVector



	// ABSOLUTE
	// id=absvec
	class absolute_birds_eye_position_vector_2d : public I2dVector
	{
	public:
		// CTORS
		// Default:
		absolute_birds_eye_position_vector_2d( f32 x1 = 0.0, f32 y1 = 0.0 ): I2dVector(x1,y1) {}// c
		// Dtor
		virtual ~absolute_birds_eye_position_vector_2d() {}

		// Explicit conversion: Naive.
		// TODO: It may be possible to do these conversions without construction by means of inheritance.
		static absolute_birds_eye_position_vector_2d from_vector2df(const vector2df& internalVec )
		{	return absolute_birds_eye_position_vector_2d( internalVec.X, internalVec.Y ); }// from_vector2df

		// From Irrlicht (transposed) vector:
		static absolute_birds_eye_position_vector_2d from_vector3df(const vector3df& internalVec )
		{	return absolute_birds_eye_position_vector_2d( -internalVec.Z, internalVec.X );	}// from_vector3df

		static absolute_birds_eye_position_vector_2d from_position(const ISceneNode& obj)
		{
			const vector3df pos = obj.getAbsolutePosition();
			return absolute_birds_eye_position_vector_2d( -pos.Z, pos.X );
		}// from_position

		virtual relative_birds_eye_vector_2d to_relVec(const ISceneNode& refNode) const;

		// Angle of vector: parallel to floor.
		absAngle to_absAngle() const;
		//{	return absAngle(internalVec.getAngleTrig());	}// to_absAngle()

		// Angle of vector with respect to some in-game object.  Basically a shortcut for a couple of other conversions.
		relAngle to_relAngle( const ISceneNode& refObj ) const;
		//{	return to_absAngle().to_relAngle(refObj);	}// to_relAngle()

		// Idem; for consistency with Irrlicht iface.
		//absAngle getAngleTrig() const
		//{	return to_absAngle();	}// getAngle()

		// Auto-convert to Irrlicht 3D vector:
		operator vector3df() const
		{	return toIrr_vector3df();	}// vector3df

		// TODO: Can I put this into the base class and not have polymorphism employed?
		bool operator== (const absolute_birds_eye_position_vector_2d& rhs) const
		{	return to_vector2df() == rhs.to_vector2df();	}// ==()

		absolute_birds_eye_position_vector_2d& operator+= (const absolute_birds_eye_position_vector_2d& rhs)
		{
			internalVec += rhs.internalVec;
			return *this;
		}// +=()

		// From regular Irrlicht vector: Naive version: Unused in order to force conversion.
		//absolute_birds_eye_position_vector_2d( const vector2df& internalVec ): x()(internalVec.x()), y()(internalVec.y()) {}// c

		// CCtor: Default
		// dtor: Default
		// =(): Default

		//// Convert to absolute 3D vector in Cartesian space; i.e., no conversion Dammit():
		//vector3df to_vector3df() const
		//{	return vector3df(x(),y(),0.0);	}// to_vector3df()
		//// Convert to absolute Irrlicht 3D vector: goofy rearrangement.
		//vector3df toIrr_vector3df() const
		//{	return vector3df(y(),0.0,-x());	}// toIrr_vector3df()
	};// absolute_birds_eye_position_vector_2d



	// RELATIVE
	// id=relvec
	class relative_birds_eye_vector_2d : public I2dVector
	{
	public:
		static relative_birds_eye_vector_2d from_vector2df(const vector2df& internalVec )
		{	return relative_birds_eye_vector_2d( internalVec.X, internalVec.Y ); }// from_vector2df

		static relative_birds_eye_vector_2d from_position(const ISceneNode& refNode)
		{
			const vector3df pos = refNode.getPosition();
			return relative_birds_eye_vector_2d( -pos.Z, pos.X );
		}// from_position

		static relative_birds_eye_vector_2d from_vector3df(const vector3df& internalVec )
		{	return relative_birds_eye_vector_2d( -internalVec.Z, internalVec.X );	}// from_vector3df

		// Convert to absolute Irrlicht 3D vector, such as can be checked against a getAbsolutePosition() call.
		//vector3df toIrr_vector3df(const ISceneNode& refNode) const
		//{	return to_absVec(refNode).toIrr_vector3df();	}// toIrr_vector3df()

		// CTOR
		relative_birds_eye_vector_2d( f32 x1 = 0.0, f32 y1 = 0.0 ): I2dVector(x1,y1) {}

		// DTOR
		virtual ~relative_birds_eye_vector_2d() {}

		virtual absolute_birds_eye_position_vector_2d to_absVec(const ISceneNode& refNode) const
		{
			absVec refPoint = absVec::from_position(refNode);
			refPoint += absVec::from_vector2df( to_vector2df() );
			return refPoint;
		}// to_absVec()


		bool operator== (const relVec& rhs) const
		{	return to_vector2df() == rhs.to_vector2df();	}// ==()

		relative_birds_eye_vector_2d  operator+ (const relative_birds_eye_vector_2d & rhs) const
		{	return relative_birds_eye_vector_2d::from_vector2df( internalVec + rhs.internalVec );	}// +()

		relative_birds_eye_vector_2d& operator+= (const relative_birds_eye_vector_2d& rhs)
		{
			internalVec += rhs.internalVec;
			return *this;
		}// +=


		relative_birds_eye_vector_2d& operator-= (const relative_birds_eye_vector_2d& rhs)
		{
			internalVec -= rhs.internalVec;
			return *this;
		}// -=()

		//virtual to_absVec(const ISceneNode& refPoint) const
		//{	return absVec( absVec::from_vector3df(refPoint.getAbsolutePosition()).X + x(), refPoint.getPosition().Y + y() );	}// to_absVec()
		//{	return absolute_birds_eye_position_vector_2d( refPoint.getPosition().X + getX(), refPoint.getPosition().Y + getY() );	}// to_absVec()
	};// relative_birds_eye_vector_2d
	//class relative_birds_eye_vector_2d : public I2dVector
	//{
	//public:
		//// CTOR
		//relative_birds_eye_vector_2d( const ISceneNode& ref, f32 x1 = 0.0, f32 y1 = 0.0 ): I2dVector(x1,y1), refPoint(ref) {}

		//virtual to_absVec() const
		//{	return absolute_birds_eye_position_vector_2d( refPoint.getPosition().X + getX(), refPoint.getPosition().Y + getY() );	}// to_absVec()
	//private:
		//const ISceneNode& refPoint;
	//};// relative_birds_eye_vector_2d


	// MOUSE
	// A vector relative to the upper-left corner of the screen.
	// id=mouse-vec, id=mousevec
	class mouse_vector_2d : public I2dVector
	{
	public:
		// Ctor
		mouse_vector_2d( const irr::video::IVideoDriver& d, f32 x1 = 0.0, f32 y1 = 0.0 ): I2dVector(x1,y1), driver(d) {}

		// Dtor
		virtual ~mouse_vector_2d() {}

		absVec to_absVec() const
		{	return absVec( x() - static_cast<float>(driver.getScreenSize().Width)/2.0, static_cast<float>(driver.getScreenSize().Height)/2.0 - y() );	}//
	protected:
		const irr::video::IVideoDriver& driver;
	};// mouse_vector_2d

	// ************************ 3D VECTORS ************************
	// id=3d
	// id=absPos
	class absolute_irrlicht_position_vector_3d : public irr::core::vector3d<f32>
	{ 
	public:
		// CTORS
		absolute_irrlicht_position_vector_3d(): vector3d<f32>() {}// default
		absolute_irrlicht_position_vector_3d( f32 X_trans, f32 Z_trans ): vector3d<f32>( Z_trans, 0.0, -X_trans ) {}// performs transpositions
		absolute_irrlicht_position_vector_3d( f32 X, f32 Y, f32 Z ): vector3d<f32>( X, Y, Z ) {}// the "straight man": no transpositions.

		// Default dtor
		~absolute_irrlicht_position_vector_3d() {}

		// Default copy c.
		//explicit absolute_irrlicht_position_vector_3d( const absolute_irrlicht_position_vector_3d& rhs ): vector3d<f32>(*this) {}

		// Default =().

		const vector3d<f32>& toIrr_vector3df() const
		{	return static_cast<const vector3d<f32>&>(*this);	}// 
		vector3d<f32>& toIrr_vector3df()
		{	return static_cast<vector3d<f32>&>(*this);	}// 


		// TODO: More efficient?
		static absPos from_vector3df(const vector3d<f32>& rhs )
		{	
			return absPos( rhs.X, rhs.Y, rhs.Z ); 
		}// from_vector3df
	};// absolute_vector_3d

	// id=relPos
	class relative_irrlicht_vector_3d : public vector3df
	{
	public:

	};// relPos

	//class relative_angle_vector_2d : public I2dVector
	//{
	//public:
		//// CTOR
		//relAngleVec( f32 x1 = 0.0, f32 y1 = 0.0 ): I2dVector(x1,y1) {}// c
		//// Dtor
		//virtual ~relAngleVec() {}
	//};// relative_angle_vector_2d


	// This is the type you should pass to ISceneNode#setRotation().
	class relative_angle_vector_3d : public I3dVector
	{
	public:
		// Use this to convert from e.g. ISceneNode#getRotation().
		static relAngleVec3d from_vector3df(const vector3df& anglevec)
		{	return relAngleVec3d( anglevec.X, anglevec.Y, anglevec.Z );	}//

		// CTOR
		relative_angle_vector_3d( f32 x = 0.0, f32 y = 0.0, f32 z = 0.0 ): I3dVector(x,y,z) {}// ctor
		// Dtor
		virtual ~relative_angle_vector_3d() {}

		// Convert to angle (this is the whole point):
		relAngle to_relAngle() const;
		//{	return relAngle( to_vector3df().Y );	}// to_relAngle()
	};// relative_angle_vector_3d

	////******************** ANGLES
	//id=angle
	class IAngle
	{
	public:
		// DTOR
		virtual ~IAngle() {}

		// Simple angle retrieval:
		virtual f32 to_float() const
		{	return internalAngle;	}// to_float()
	protected:
		// CTOR: Includes angle sanity check:
		IAngle( f32 ang ): internalAngle(rationalizeAngle(ang)) {}

		// Util accessors for use by children:
		f32 getValue() const
		{	return internalAngle;	}// getValue()
		void setValue( f32 ang )
		{	internalAngle = rationalizeAngle(ang);	}// setValue()
	private:
		f32 internalAngle;
	};// IAngle

	///************* ABSOLUTE ANGLE
	//id=absangle
	class absAngle : public IAngle
	{
	public:
		// Get heading of an in-game object.
		static absAngle of_object( const ISceneNode& refObj )
		{	return absAngle(refObj.getAbsoluteTransformation().getRotationDegrees().Y);	}// of_object()

		// CTOR: Simple value conversion.
		absAngle( f32 angle ): IAngle(angle) {}
		//static absAngle from_f32( f32 angle )
		//{	}

		// dtor
		virtual ~absAngle() {}

		relAngle to_relAngle(const ISceneNode& refNode) const;
		//{	return relAngle( (*this - absAngle::of_object(refNode)).to_float() );	}// to_relAngle()

		// OPERATORS
		absAngle operator+ (const absAngle& rhs) const
		{	return absAngle( this->to_float() + rhs.to_float() );	}// +()

		absAngle operator- (const absAngle& rhs) const
		{	return absAngle( this->to_float() - rhs.to_float() );	}// -()
	};// absAngle

	///************* RELATIVE ANGLE
	//id=relangle
	class relAngle : public IAngle
	{
	public:
		// Get heading of an in-game object (relative to parent).
		static relAngle of_object( const ISceneNode& refObj )
		{	return relAngleVec3d::from_vector3df(refObj.getRotation()).to_relAngle();	}// of_object()
		//{	return from_rotation3df(refObj.getRotation());	}// of_object()

		//// Convert from a rotation vector--that is, a yaw-pitch-roll tuple:
		//static relAngle from_rotation3df( const vector3df& rotation )
		//{	return rotation.Y	}// from_rotation3df

		// CTOR: Simple value conversion.
		relAngle( f32 angle ): IAngle(angle) {}

		// dtor
		virtual ~relAngle() {}

		absAngle to_absAngle(const ISceneNode& refNode) const
		{	return absAngle::of_object(refNode) + absAngle(to_float());	}// to_absAngle()

		// Convert to an Irrlicht angle-tuple.  Inverse of relAngleVec3d#to_relAngle().
		relAngleVec3d to_relAngleVec3d() const
		{	return relAngleVec3d(0.0, to_float(), 0.0);	}//

		// 0 Predicate:
		bool iszero() const
		{	return irr::core::iszero( to_float() );	}// iszero()

		// OPERATORS
		relAngle operator+ (const relAngle& rhs) const
		{	return relAngle( this->to_float() + rhs.to_float() );	}// +

		relAngle& operator+= (const relAngle& rhs)
		{
			setValue( to_float() + rhs.to_float() );
			return *this;
		}// +=()

		relAngle& operator-= (const relAngle& rhs)
		{
			setValue( to_float() - rhs.to_float() );
			return *this;
		}// -=()

		bool operator> (const relAngle& rhs) const
		{	return this->to_float() > rhs.to_float();	}// >

		bool operator== (const relAngle& rhs) const
		{	return this->to_float() == rhs.to_float();	}// ==
	};// relAngle


}// cj

// OUTPUT OPERATORS
inline std::wostream& operator<< (std::wostream& out, const cj::absVec& pos)
{
	return out << L'(' << pos.x() << L',' << pos.y() << L')';
}// <<()
inline std::wstring to_s( const cj::absVec& pos )
{
	std::wostringstream out;
	out << pos;
	return out.str();
}// to_s()
inline const wchar_t* to_cstr(  const cj::absVec& pos )
{
	std::wostringstream out;
	out << pos;
	return out.str().c_str();
}// to_cstr()

inline std::wostream& operator<< (std::wostream& out, const cj::mouseVec& pos)
{
	return out << pos.to_absVec();
}// <<()
inline std::wstring to_s( const cj::mouseVec& pos )
{
	std::wostringstream out;
	out << pos;
	return out.str();
}// to_s()

#endif// inc

