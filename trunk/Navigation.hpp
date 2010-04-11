#ifndef __NAVIGATION_HPP__
#define __NAVIGATION_HPP__


#include "assert_swig.hpp"
#include "Coordinates.hpp"

//#include <vector>
#include "boost/utility.hpp" // noncopyable
#include "boost/ptr_container/ptr_vector.hpp"

#include "irrlicht.h"

namespace cj
{
	namespace nav
	{
		using namespace irr::core;
		using namespace irr::video;
		//using namespace std;
		//

		// fwd dec:
		class PathNode;

		typedef boost::ptr_vector<PathNode> PathNodeList;
		//typedef vector<PathNode*> PathNodeList;

		class IPathElement : public boost::noncopyable
		{
		public:
			IPathElement() {}
			virtual ~IPathElement() {}

			virtual void setVisible( bool vis=true ) = 0;
			virtual bool getVisible() = 0;
		};// IPathElement

		/// ******* id=PathNode *********
		//id=node
		class PathNode : public IPathElement
		{
		public:
			//static f32
			//static const wchar_t displayGraphicName[] = L"./blue.jpg";

			// id=CTOR
			//PathNode( ISceneManager& s, IVideoDriver& d, const vector3df& p, u32 i ): smgr(s), driver(d), position(p), circle(smgr.addBillboardSceneNode( NULL, dimension2df(10.0,10.0), position )), index(i), graphic("./blue.jpg")
			PathNode( ISceneManager& s, IVideoDriver& d, const absVec& p, u32 i ): smgr(s), driver(d), position(p), circle(smgr.addBillboardSceneNode( NULL, dimension2df(10.0,10.0), position.toIrr_vector3df() )), index(i), graphic("./blue.jpg")
			{
				assert(circle);

				// id=circle
				circle->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
				circle->setMaterialFlag(EMF_LIGHTING, false);
				circle->setMaterialTexture(0, driver.getTexture(graphic));

				circle->setVisible(false);
			}// c

			// id=DTOR
			virtual ~PathNode()
			{
				circle->remove();
			}// dtor

			bool getFancyGraphic() const
			{	return graphic == "./circle.png";	}
			void setFancyGraphic( bool b )
			{
				graphic = (b ? "./circle.png" : "./blue.jpg");
				circle->setMaterialTexture(0, driver.getTexture(graphic)); // FIXME not hard-coded
			}//

			/// ACCESSORS
			// Note: Not bothering with relative coords here.
			const absVec& getPosition() const
			{	return position;	}// getAbsolutePosition()
			void setPosition( const absVec& dest )
			{
				position = dest;
				circle->setPosition( position.toIrr_vector3df() );
				circle->updateAbsolutePosition();
				assert( getPosition() == dest );
			}// setPosition()


			// Index
			u32 getIndex() const
			{	return index;	}// getIndex()
			void setIndex( u32 i )
			{	index = i;	}// setIndex()

			virtual void setVisible( bool vis=true )
			{	circle->setVisible(vis);	}//
			virtual bool getVisible()
			{	return circle->isVisible();	}//

			virtual bool operator==( const PathNode& rhs) const
			{	return this == &rhs;	}// ==()
		private:
			ISceneManager& smgr;
			IVideoDriver& driver;
			absVec position;
			IBillboardSceneNode* const circle;
			u32 index;
			stringw graphic;
		};// PathNode

		// id=PATHEDGE
		class PathEdge :  public IPathElement
		{
		public:
			// ctor
			PathEdge( IVideoDriver& d, f32 weight, PathNode* end1, PathNode* end2 ) : driver(d), edgeWeight(weight), visible(false), endpoint1(end1), endpoint2(end2)
			{}// c

			// ACCESSORS
			virtual const PathNode* const getEndpoint1() const { return endpoint1;	}//
			virtual PathNode* const getEndpoint1() { return endpoint1;	}//
			virtual const PathNode* const getEndpoint2() const { return endpoint2;	}//
			virtual PathNode* const getEndpoint2() { return endpoint2;	}//

			//virtual f32 getLength() const
			//{ return (getEndpoint1()->getPosition().toIrr_vector3df() - getEndpoint2()->getPosition().toIrr_vector3df()).getLength();	}// getLength()

			virtual f32 weight() const
			{	return edgeWeight;	}// weight()

			// Draws a line b/t the two endpoint nodes, if they exist.
			virtual void setVisible( bool vis=true )
			{
				// FIXME: Does this need to be run on every tick?
				if( getEndpoint1() && getEndpoint2() )
				{
					visible = vis;
					driver.draw3DLine( getEndpoint1()->getPosition().toIrr_vector3df(), getEndpoint2()->getPosition().toIrr_vector3df(), SColor(102,120,255,136));
				}// if
			}// setVisible()
			virtual bool getVisible()
			{	return visible;	}//


			virtual bool operator==( const PathEdge& rhs) const
			{
				return (*endpoint1 == *(rhs.endpoint1) &&  *endpoint2 == *(rhs.endpoint2)) || (*endpoint1 == *(rhs.endpoint2) && ( *endpoint2 == *(rhs.endpoint1)) );
			}// ==()
		private:
			IVideoDriver& driver;
			const f32 edgeWeight;
			bool visible;
			PathNode* endpoint1;
			PathNode* endpoint2;
		};// PathEdge

		// TODO: Pull decls to an ABC.  Rename this to DenseNavGraph.
		class NavGraph : public boost::noncopyable
		{
		public:
			// CTOR
			NavGraph(ISceneManager& s, IVideoDriver& d)
			: smgr(s), driver(d), highestIndex(0) {}//ctor


			u32 nodeCount() const {	return nodeList.size();	}// nodeCount()
			// TODO: u32 edgeCount() const {}
			const PathNodeList& nodes() const
			{	return nodeList;	}//

			// TODO: const EdgesList& edges() const;


			// TODO: Private.  friend of Game::addNode.
			//PathNode& addNode( f32 x, f32 y )// TODO: integers?
			PathNode& addNode( const vector3df& absPosition )
			{
				PathNode* newNode = new PathNode(smgr, driver, absVec::from_vector3df(absPosition), highestIndex++);
				assert(newNode);
				nodeList.push_back(newNode);
				return *newNode;
			}// addNode()
			// TODO: removeNode()

			PathEdge& addEdge( PathNode& start, PathNode& end, f32 weight)
			{
				PathEdge* newEdge = new PathEdge(driver, weight, &start, &end );
				// TODO
				return *newEdge;
			}// addEdge()
		private:
			ISceneManager& smgr;
			IVideoDriver& driver;
			PathNodeList nodeList;// master list of all nodes created.
			// TODO: edges
			u32 highestIndex;// incremented as nodes are added, to give each a unique index.
		};// NavGraph
	}// nav
}// cj

#endif// inc

