#include "Coordinates.hpp"
#include <ostream>

namespace cj
{
absVec absVec::from_vector2df(const vector2df& internalVec )
{	return absVec( internalVec.X, internalVec.Y ); }// from_vector2df

absVec absVec::from_vector3df(const vector3df& internalVec )
{	return absVec( -internalVec.Z, internalVec.X );	}// from_vector3df

absVec absVec::from_position(const ISceneNode& obj)
{
	const vector3df pos = obj.getAbsolutePosition();
	return absVec( -pos.Z, pos.X );
}// from_position

bool absVec::operator== (const absolute_birds_eye_position_vector_2d& rhs) const
{	return to_vector2df() == rhs.to_vector2df();	}// ==()

absVec& absVec::operator+= (const absVec& rhs)
{
	internalVec += rhs.internalVec;
	return *this;
}// +=()

absVec absVec::operator- (const relVec& rhs) const
{
	return( absVec::from_vector2df( internalVec + rhs.to_vector2df() ) );
}// -()

// TODO: inline?
relVec absVec::to_relVec(const ISceneNode& refNode) const
{
	return relVec::from_vector2df( to_vector2df() - absVec::from_position(refNode).to_vector2df() );
}// to_absVec()

 absAngle absVec::to_absAngle() const
{	return absAngle(internalVec.getAngleTrig());	}// to_absAngle()

 relAngle absVec::to_relAngle( const ISceneNode& refObj ) const
{	return to_absAngle().to_relAngle(refObj);	}// to_relAngle()

relVec relVec::from_position(const ISceneNode& refNode)
{
	const vector3df pos = refNode.getPosition();
	return relative_birds_eye_vector_2d( -pos.Z, pos.X );
}// from_position

absVec relVec::to_absVec(const ISceneNode& refNode) const
{
	absVec refPoint = absVec::from_position(refNode);
	refPoint += absVec::from_vector2df( to_vector2df() );
	return refPoint;
}// to_absVec()

bool relVec::operator== (const relVec& rhs) const
{	return to_vector2df() == rhs.to_vector2df();	}// ==()
relVec relVec::operator+ (const relative_birds_eye_vector_2d & rhs) const
{	return relative_birds_eye_vector_2d::from_vector2df( internalVec + rhs.internalVec );	}// +()

relVec& relVec::operator+= (const relative_birds_eye_vector_2d& rhs)
{
	internalVec += rhs.internalVec;
	return *this;
}// +=

relVec& relVec::operator-= (const relative_birds_eye_vector_2d& rhs)
{
	internalVec -= rhs.internalVec;
	return *this;
}// -=()



// TODO: More efficient?
absPos absPos::from_vector3df(const vector3d<f32>& rhs )
{
	return absPos( rhs.X, rhs.Y, rhs.Z );
}// from_vector3df

bool absPos::operator== (const absVec& rhs) const
{	return( toIrr_vector3df() == rhs.toIrr_vector3df() );	}// ==()

// Convert to angle (this is the whole point):
 relAngle relAngleVec3d::to_relAngle() const
{	return relAngle( to_vector3df().Y );	}// to_relAngle()

 relAngle absAngle::to_relAngle(const ISceneNode& refNode) const
{	return relAngle( (*this - absAngle::of_object(refNode)).to_float() );	}// to_relAngle()
}// cj


std::wostream& operator<< (std::wostream& out, const cj::absVec& pos)
{
	return out << L'(' << pos.x() << L',' << pos.y() << L')';
}// <<()

std::wstring to_s( const cj::absVec& pos )
{
	std::wostringstream out;
	out << pos;
	return out.str();
}// to_s()

const wchar_t* to_cstr(  const cj::absVec& pos )
{
	std::wostringstream out;
	out << pos;
	return out.str().c_str();
}// to_cstr()

std::wostream& operator<< (std::wostream& out, const cj::relVec& pos)
{
	return out << L'(' << pos.x() << L',' << pos.y() << L')';
}// <<()

std::wostream& operator<< (std::wostream& out, const cj::mouseVec& pos)
{
	return out << pos.to_absVec();
}// <<()

std::wstring to_s( const cj::mouseVec& pos )
{
	std::wostringstream out;
	out << pos;
	return out.str();
}// to_s()

