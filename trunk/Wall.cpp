#include "Wall.hpp"
#include "IO.hpp"
#include "assert_swig.hpp"

namespace cj
{

Wall::Wall(IrrlichtDevice* d, stringw t, u32 ds) : device(d), texture(t), dsize(ds)
{
    driver = device->getVideoDriver();
    smgr = device->getSceneManager();
    xpos = xneg = zpos = zneg = zrange = 0;
}

Wall::~Wall()
{
//dpr("Wall dtor.");
   vector<GraphNode*>::iterator it = paths.begin();
   for(;it != paths.end(); ++it)
   {  delete *it;  }
}

bool Wall::operator== (const Wall& rhs) const
{	return &rhs == this;	}// ==()

vector3df Wall::getRandomNodePosition()
{
dpr( "Picking random node." );
dpr( paths.size() );
	assert( !paths.empty() );
	u32 random = 0;/* TODO: : rand ∈ [0,paths.size()) */
	return (paths.front() + random)->point;
}// Wall::getRandomNodePosition()

void Wall::addNode(u32 size, vector3df position)
// creates a cube of size and places it at position
{
   bool exists;
   GraphNode* n = NULL;

   try{
      n = FindNode(position.X, position.Z);
      exists = (n->isWall == 2) && (n->point.X == position.X) && (n->point.Z == position.Z);
   }
   catch(out_of_range e)
   {  exists = false;  }

   if(!exists)
   {
      ExpandSpace(position);
      n = FindNode(position.X, position.Z);
      n->isWall = 2;

      for(s32 i = 0; i < 8; ++i)
      {
         if(n->connection[i] && n->connection[i]->isWall == 0)
         {  n->connection[i]->isWall = 1;  }
      }

      // add a wall node to the list
      frame.push_back(smgr->addCubeSceneNode(size, 0, -1, position));
      // set the texture
      frame[frame.size()-1]->setMaterialTexture(0, driver->getTexture(texture));
      // because we do not have a light source, lighting needs to be off or it will be black
      frame[frame.size()-1]->setMaterialFlag(EMF_LIGHTING, false);
      // create selector for collision detection
      ITriangleSelector* selector = smgr->createTriangleSelector(
        frame[frame.size()-1]->getMesh(), frame[frame.size()-1]);
      // attatch selector to wall node
      frame[frame.size()-1]->setTriangleSelector(selector);
      // discard the selector reference
      selector->drop();
   }
}

void Wall::makeWall(u32 length, u32 width, vector3df position)
{
   // snap to grid
   position.Y = 0;
   position.X = round(position.X/dsize) * dsize;
   position.Z = round(position.Z/dsize) * dsize;

   // add a cube for each section of the wall
   for(f32 l = 0; l < length; l++)
   {
      for(f32 w = 0; w < width; w++)
      {  addNode(dsize, position - vector3df(l*dsize,0,w*dsize));  }
   }
}

void Wall::ExpandSpace(vector3df a)
{
   if(xpos < a.X + 2*dsize)
   {
      s32 newxpos = a.X + 2*dsize;
      newxpos += newxpos % dsize;

      for(s32 i = xpos; i <= newxpos; i += dsize)
      {
         for(s32 j = zneg; j <= zpos; j+= dsize)
         {
            InsertPath(i,j);
         }
      }

      xpos = newxpos;
   }

   if(xneg > a.X - 2*dsize)
   {
      s32 newxneg = a.X - 2*dsize;
      newxneg -= abs(newxneg) % dsize;

      for(s32 i = xneg; i >= newxneg; i -= dsize)
      {
         for(s32 j = zneg; j <= zpos; j += dsize)
         {
            InsertPath(i,j);
         }
      }

      xneg = newxneg;
   }

   if(zpos < a.Z + 2*dsize)
   {
      s32 newzpos = a.Z + 2*dsize;
      newzpos += newzpos % dsize;

      for(s32 i = zpos; i <= newzpos; i += dsize)
      {
         for(s32 j = xneg; j <= xpos; j += dsize)
         {
            InsertPath(j,i);
         }
      }

      zpos = newzpos;
      zrange = abs((zneg - zpos)/dsize) + 1;
   }

   if(zneg > a.Z - 2*dsize)
   {
      s32 newzneg = a.Z - 2*dsize;
      newzneg -= abs(newzneg) % dsize;

      for(s32 i = zneg; i >= newzneg; i -= dsize)
      {
         for(s32 j = xneg; j <= xpos; j += dsize)
         {
            InsertPath(j,i);
         }
      }

      zneg = newzneg;
      zrange = abs((zneg - zpos)/dsize) + 1;
   }
}

void Wall::InsertPath(s32 x, s32 z)
{
dpr( "InsertPath()" );
   vector<GraphNode*>::iterator it = paths.begin();
   for(; it != paths.end(); ++it)
   {
      if(x < (*it)->point.X || (x == (*it)->point.X && z <= (*it)->point.Z))
      {  break;  }
   }

   if(it == paths.end() || x != (*it)->point.X || z != (*it)->point.Z)
   {
      GraphNode* newnode = new GraphNode;
      newnode->point = vector3df(x, 0, z);
      newnode->isWall = 0;
      newnode->back = NULL;
      newnode->backdir = -1;
      newnode->score = -1;
      newnode->closed = false;
      newnode->count = -1;
      s32 dx[] = {dsize, dsize, 0, -dsize, -dsize, -dsize, 0, dsize};
      s32 dz[] = {0, -dsize, -dsize, -dsize, 0, dsize, dsize, dsize};

      for(s32 i = 0; i < 8; i++)
      {
         GraphNode* n = NULL;
         s32 cx = x + dx[i];
         s32 cz = z + dz[i];

         for(u32 j = 0; j < paths.size(); j++)
         {
            if(paths.at(j) && paths.at(j)->point.X == cx && paths.at(j)->point.Z == cz)
            {
               n = paths.at(j);
               break;
            }
         }

         if(n)
         {  n->connection[(i+4)%8] = newnode;  }

         newnode->connection[i] = n;
      }

      paths.insert(it, newnode);
dpr( "Node count: " << paths.size() );
   }
}

void Wall::DrawNodes(void)
{
   vector<GraphNode*>::iterator it = paths.begin();
   SMaterial basic = SMaterial();
   basic.setFlag(EMF_LIGHTING, false);
   driver->setMaterial(basic);
   driver->setTransform(video::ETS_WORLD, matrix4());

   for(; it != paths.end(); ++it)
   {
      for(s32 i = 0; i < 8; i++)
      {
         if((*it)->connection[i])
         {
            line3d<f32> line;
            line.start = (*it)->point;
            line.end = (*it)->connection[i]->point;
            driver->draw3DLine(line.start, line.end, SColor(102,120,255,136));
         }
      }
   }
}

GraphNode* Wall::FindNode(s32 x, s32 z)
{
   s32 i = ((x-xneg)/dsize)*zrange + (z-zneg)/dsize;
   return paths.at(i);
}

GraphNode* Wall::FindCloseNode(s32 x, s32 z)
{
   x = round(x/dsize) * dsize;
   z = round(z/dsize) * dsize;
   if(x < xneg)
   {  x = xneg;  }
   if(x > xpos)
   {  x = xpos;  }
   if(z < zneg)
   {  z = zneg;  }
   if(z > zpos)
   {  z = zpos;  }
   s32 i = ((x-xneg)/dsize)*zrange + (z-zneg)/dsize;
   return paths.at(i);
}

std::list<vector3df> Wall::AStar(vector3df start, vector3df goal, s32 smooth, bool debug)
{
   std::list<vector3df> waypoints;

   if(paths.empty())
   {
      waypoints.push_back(goal);
      cerr << "ERROR: paths empty\n";
      return waypoints;
   }

   GraphNode* gstart = NotWall(start);
   GraphNode* ggoal = NotWall(goal);

   if(!gstart || !ggoal)
   {
      cerr << "ERROR: start or goal node invalid!";
      return waypoints;
   }

   gstart->back = NULL;
   gstart->count = 0;
   gstart->backdir = -1;
   gstart->score = fabs(ggoal->point.X - gstart->point.X + ggoal->point.Z - gstart->point.Z);
   gstart->closed = false;
   std::list<GraphNode*> open;
   open.push_front(gstart);
   std::list<GraphNode*> closed;
   GraphNode* current = NULL;

   while(!open.empty())
   {
      current = open.front();
      open.pop_front();
      if(current->closed)
      {  continue;  }

      for(s32 i = 0; i < 8; i++)
      {
         GraphNode* gnext = current->connection[i];

         if((gnext && gnext->score == -1) && (!(gnext->isWall) || (gnext->isWall == 1 && i % 2 == 0)))
         {
            gnext->back = current;
            gnext->backdir = i;
            if(i % 2 == 1)
            {  gnext->count = current->count + SQRT2 * dsize;  }
            else
            {  gnext->count = current->count + dsize;  }
            gnext->score = gnext->count + fabs(ggoal->point.X - gnext->point.X)
               + fabs(ggoal->point.Z - gnext->point.Z);
            InsertList(open, gnext);
         }
         else if(gnext && gnext->score > -1 && !(gnext->closed))
         {
            f32 temp_count = 0.0;
            if(i % 2)
            {  temp_count = current->count + SQRT2 * dsize;  }
            else
            {  temp_count = current->count + dsize;  }

            if(temp_count < gnext->count)
            {
               gnext->count = temp_count;
               gnext->score = gnext->count + fabs(ggoal->point.X - gnext->point.X)
                  + fabs(ggoal->point.Z - gnext->point.Z);
               gnext->back = current;
               gnext->backdir = i;
               InsertList(open, gnext, true);
            }
         }
      }

      closed.push_back(current);
      current-> closed = true;

      if(current->point.X == ggoal->point.X && current->point.Z == ggoal->point.Z)
      {
         GraphNode* previous = ggoal;
         previous->backdir = -2;
         waypoints.push_front(goal);

         while(current)
         {
            bool add = false;

            for(s32 i = 0; i < 8; i++)
            {
               if(!(current->connection[i]) || current->backdir != previous->backdir)
               {
                  add = true;
                  break;  // exits for
               }
            }

            if(add || smooth == 0)
            {  waypoints.push_front(current->point);  }

            previous = current;
            current = current->back;
         }

         break;  // exits while
      }  // end if(current->point.X == ggoal->point.X && current->point.Z == ggoal->point.Z)

   }  // end while(!open.empty())

   std::list<GraphNode*>::iterator it = closed.begin();

   for(; it != closed.end(); ++it)
   {
      (*it)->score = -1;
      (*it)->back = NULL;
      (*it)->backdir = -1;
      (*it)->count = -1;
      (*it)->closed = false;
   }

   for(it = open.begin(); it != open.end(); ++it)
   {
      (*it)->score = -1;
      (*it)->back = NULL;
      (*it)->backdir = -1;
      (*it)->count = -1;
      (*it)->closed = false;
   }

   if(smooth > 1)
   {
      std::list<vector3df>::iterator w1 = waypoints.begin();
      std::list<vector3df>::iterator w2 = waypoints.begin();
      if(w2 != waypoints.end())
      {  ++w2;  }
      vector3df last = start;

      while(w2 != waypoints.end())
      {
         if(PathIsWide(last, (*w2)))
         {
            waypoints.erase(w1);
            w1 = w2;
            ++w2;
         }
         else
         {
            last = (*w1);
            w1 = w2;
            ++w2;
         }
      }
   }

   if(debug)
   {
      std::list<vector3df>::iterator w = waypoints.begin();
      for(; w != waypoints.end(); w++)
      {
         IBillboardSceneNode *circle = smgr->addBillboardSceneNode(0, dimension2df(dsize,dsize), *w);
         circle->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
         circle->setMaterialFlag(EMF_LIGHTING, false);
         circle->setMaterialTexture(0, driver->getTexture("circle.png"));
      }
   }

   return waypoints;
}

bool Wall::PathIsWide(vector3df from, vector3df to)
{
   bool wide = true;
   vector3df point, pf1, pf2, pt1, pt2;
   pf1 = to - from;
   if(iszero(pf1.X) && iszero(pf1.Z))
   {  return wide;  }
   pf1.normalize();
   pf1.rotateXZBy(90);
   pf1 *= (dsize/2);
   pf2 = pf1 * -1;
   pf1 += from;
   pf2 += from;
   pt1 = from - to;
   pt1.normalize();
   pt1.rotateXZBy(90);
   pt1 *= (dsize/2);
   pt2 = pt1 * -1;
   pt1 += to;
   pt2 += to;
   triangle3df outtri;
   line3d<f32> line(pf1,pt1);

   if(smgr->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(line, point, outtri))
   {  wide = false;  }
   else
   {
      line.start = pf2;
      line.end = pt2;

      if(smgr->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(line, point, outtri))
      {  wide = false;  }
   }

   return wide;
}

GraphNode* Wall::NotWall(vector3df p)
{
   vector3df close(round(p.X/dsize)*dsize,0,round(p.Z/dsize)*dsize);
   GraphNode* g = NULL;
   if(close.X > xpos)
   {  close.X = xpos;  }
   else if(close.X < xneg)
   {  close.X = xneg;  }
   if(close.Z > zpos)
   {  close.Z = zpos;  }
   else if(close.Z < zneg)
   {  close.Z = zneg;  }
   try{
      g = FindNode(close.X, close.Z);
   }
   catch(out_of_range e)
   {  return g;  }
   if(g->point.X != close.X || g->point.Z != close.Z)
   {  return g;  }

   if(g->isWall > 1)
   {
      if(close.X < g->point.X)
      {
         if(close.Z < g->point.Z)
         {
            if(g->connection[2] && !(g->connection[2]->isWall))
            {  g = g->connection[2];  }
            else if(g->connection[3] && !(g->connection[3]->isWall))
            {  g = g->connection[3];  }
            else if(g->connection[4] && !(g->connection[4]->isWall))
            {  g = g->connection[4];  }
         }
         else
         {
            if(g->connection[4] && !(g->connection[4]->isWall))
            {  g = g->connection[4];  }
            else if(g->connection[5] && !(g->connection[5]->isWall))
            {  g = g->connection[5];  }
            else if(g->connection[6] && !(g->connection[6]->isWall))
            {  g = g->connection[6];  }
         }
      }
      else
      {
         if(close.Z < g->point.Z)
         {
            if(g->connection[0] && !(g->connection[0]->isWall))
            {  g = g->connection[0];  }
            else if(g->connection[1] && !(g->connection[1]->isWall))
            {  g = g->connection[1];  }
            else if(g->connection[2] && !(g->connection[2]->isWall))
            {  g = g->connection[2];  }
         }
         else
         {
            if(g->connection[6] && !(g->connection[6]->isWall))
            {  g = g->connection[6];  }
            else if(g->connection[7] && !(g->connection[7]->isWall))
            {  g = g->connection[7];  }
            else if(g->connection[0] && !(g->connection[0]->isWall))
            {  g = g->connection[0];  }
         }
      }
   }

   return g;
}

void Wall::InsertList(std::list<GraphNode*> &glist, GraphNode* node, bool remove)
{
   std::list<GraphNode*>::iterator it = glist.begin();
   std::list<GraphNode*>::iterator it2 = glist.begin();

   for(; it != glist.end(); ++it)
   {
      if(node->score < (*it)->score)
      {
         if(remove)
         {
            it2 = it;
            ++it2;
            for(; it2 != glist.end(); ++it2)
            {
               if((*it2) == node)
               {
                  glist.erase(it2);
                  break;
               }
            }
         }

         break;
      }
   }

   glist.insert(it, node);
}

}
