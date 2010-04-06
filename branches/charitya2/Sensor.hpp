#ifndef __SENSOR_HPP__
#define __SENSOR_HPP__

#include "IO.hpp"

namespace cj
{
	namespace sensor
	{
		// ABC:
		class ISensors
		{
		public:
			virtual ~ISensors() {}

			virtual void setRangefinder( bool mode=true ) = 0;
			virtual void setRadar( bool mode=true ) = 0;
			virtual void setActivation( bool mode=true ) = 0;

			virtual void allSensorsOff()
			{
				setRangefinder(false);
				setRadar(false);
				setActivation(false);
			}// allSensorsOff()
			
			virtual bool getRangefinder() const = 0;
			virtual bool getRadar() const = 0;
			virtual bool getActivation() const = 0;
		};// ISensors

		// TODO: : Rename
		// FIXME: This should still be abstract...
		class SSensors : public ISensors
		{
		public:
			// Ctor
			SSensors()
			: rangefinderEnabled(false), radarEnabled(false), activationEnabled(false)
			{}// c

			// Dtor
			virtual ~SSensors() {}

			virtual void setRangefinder( bool mode=true )
			{	
dpr( "Setting rangefinder " << mode << " in SSensors" );
				rangefinderEnabled = mode;	
			}// 
			virtual void setRadar( bool mode=true )
			{	radarEnabled = mode;	}// 
			virtual void setActivation( bool mode=true )
			{	activationEnabled = mode;	}// 
		
			virtual bool getRangefinder() const
			{	return rangefinderEnabled;	}// 
			virtual bool getRadar() const
			{	return radarEnabled;	}// 
			// TODO: 'getActivation()' is misleading.  Rename.
			virtual bool getActivation() const
			{	return activationEnabled;	}// 

		protected:
			bool rangefinderEnabled;
			bool radarEnabled;
			bool activationEnabled;
		};// SSensors


	}// sensor
}// cj

#endif // inc

