#include "Coordinates.hpp"
#include <ostream>

namespace cj
{
	// TODO: inline?
	 relative_birds_eye_vector_2d absolute_birds_eye_position_vector_2d::to_relVec(const ISceneNode& refNode) const
	{
		return relVec::from_vector2df( to_vector2df() - absVec::from_position(refNode).to_vector2df() );
	}// to_absVec()

	 absAngle absVec::to_absAngle() const
	{	return absAngle(internalVec.getAngleTrig());	}// to_absAngle()

	 relAngle absVec::to_relAngle( const ISceneNode& refObj ) const
	{	return to_absAngle().to_relAngle(refObj);	}// to_relAngle()

	// Convert to angle (this is the whole point):
	 relAngle relAngleVec3d::to_relAngle() const
	{	return relAngle( to_vector3df().Y );	}// to_relAngle()

	 relAngle absAngle::to_relAngle(const ISceneNode& refNode) const
	{	return relAngle( (*this - absAngle::of_object(refNode)).to_float() );	}// to_relAngle()
}// cj
