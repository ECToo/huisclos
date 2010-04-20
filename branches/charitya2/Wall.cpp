#include "Wall.hpp"

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
   vector<GraphNode*>::iterator it = paths.begin();
   for(;it != paths.end(); ++it)
   {  delete *it;  }
}

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

      for(s32 i = 0; i < 8; i++)
      {
         if(n->connection[i])
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
      newnode->score = -1;
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

std::list<vector3df> Wall::AStar(vector3df start, vector3df goal, bool debug)
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
   gstart->score = fabs(ggoal->point.X - gstart->point.X + ggoal->point.Z - gstart->point.Z);
   std::list<GraphNode*> open;
   open.push_front(gstart);
   std::list<GraphNode*> closed;
   GraphNode* current = NULL;

   while(!open.empty())
   {
      current = open.front();
      //cout << "Checking (" << current->point.X << " " << current->point.Z << ")\n";
      open.pop_front();

      for(s32 i = 0; i < 8; i++)
      {
         if(current->connection[i] && !(current->connection[i]->isWall) && current->connection[i]->score == -1)
         {
            current->connection[i]->back = current;
            if(i % 2)
            {  current->connection[i]->count = current->count + SQRT2 * dsize;  }
            else
            {  current->connection[i]->count = current->count + dsize;  }
            current->connection[i]->score = current->connection[i]->count +
               fabs(ggoal->point.X - current->connection[i]->point.X)
               + fabs(ggoal->point.Z - current->connection[i]->point.Z);
            InsertList(open,current->connection[i]);
            //cout << "Adding (" << current->connection[i]->point.X << " " << current->connection[i]->point.Z << ")\n";
         }
      }

      closed.push_back(current);
      if(current->point.X == ggoal->point.X && current->point.Z == ggoal->point.Z)
      {
         waypoints.push_front(goal);
         while(current)
         {
            waypoints.push_front(current->point);
            //cout << "(" << current->point.X << " " << current->point.Z << ")\n";

            if(debug)
            {
               IBillboardSceneNode *circle = smgr->addBillboardSceneNode(0, dimension2df(dsize,dsize), current->point);
               circle->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
               circle->setMaterialFlag(EMF_LIGHTING, false);
               circle->setMaterialTexture(0, driver->getTexture("circle.png"));
            }

            current = current->back;
         }

         break;
      }
   }

   std::list<GraphNode*>::iterator it = closed.begin();

   for(; it != closed.end(); ++it)
   {
      (*it)->score = -1;
      (*it)->back = NULL;
      (*it)->count = -1;
   }

   return waypoints;
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

   if(g->isWall)
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

void Wall::InsertList(std::list<GraphNode*> &glist, GraphNode* node)
{
   std::list<GraphNode*>::iterator it = glist.begin();

   for(; it != glist.end(); ++it)
   {
      if(node->score < (*it)->score)
      {  break;  }
   }

   glist.insert(it, node);
}

}
