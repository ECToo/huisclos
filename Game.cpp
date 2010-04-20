#include "Game.hpp"

namespace cj
{

GameException::GameException(const char *msg)
   : description(msg)
{}

GameException::~GameException()
{}

const char *GameException::Message(void) const
{  return description;  }

bool Game::exists = false;

Game::Game() : wall(NULL)
{
   if(exists)
   {  throw GameException("ERROR: Only one cj::Game can exist!");  }

   srand(time(0));
   exists = true;
   Init();
   Run();
}

Game::~Game()
{
   if(!exists)
   {  throw GameException("ERROR: No cj::Game to destruct!");  }

   for(u32 i = 0; i < agents.size(); i++)
   {
      if(agents[i] != NULL)
      {
         agents[i]->GameOver();
         delete agents[i];
      }
   }

   if(wall)
   {  delete wall;  }

   exists = false;
}

void Game::Init(void)
{
   device = createDevice(EDT_OPENGL, dimension2d<u32>(680, 420), 16);

   if (device == 0)
   {  throw GameException("ERROR: Device creation failed!");  }

   driver = device->getVideoDriver();
   smgr = device->getSceneManager();
   guienv = device->getGUIEnvironment();
   device->setWindowCaption(L"Assignment 2 Demo");
   smgr->addCameraSceneNode(0, vector3df(0,200,0), vector3df(0,0,0));
   wall = new Wall(device, "t351sml.jpg");
   wall->makeWall(1,20,vector3df(0,0,20));
   wall->makeWall(1,20,vector3df(70,0,50));
   wall->makeWall(10,1,vector3df(0,0,-20));
   wall->makeWall(1,20,vector3df(-75,0,50));
   population = 6;
   gen_gap = 20;
   generation = 1;
   startvector = vector3df(40,0,0);
   // mark the target with a red circle
   IBillboardSceneNode *circle = smgr->addBillboardSceneNode(0, dimension2df(10,10), vector3df(-50,0,-50));
   circle->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
   circle->setMaterialFlag(EMF_LIGHTING, false);
   circle->setMaterialTexture(0, driver->getTexture("circle.png"));
   myfile.open("runoutput.txt");
   ifstream input;

   if(true)
   {
      //initial population
      for(u16 i = 0; i < population; i++)
      {
         agents.push_back(new cj::Agent(device, "faerie.md2", "Faerie5.BMP", "", startvector));
         //agents[i]->Seek(vector3df(-50,0,0), wall, false);
         totscores.push_back(0);
      }
      agents[0]->Seek(vector3df(-50,0,-50), wall, true);
      agents[1]->Seek(vector3df(-50,0,-50), wall, false);
      agents[2]->Seek(vector3df(-50,0,-50), wall, false);
   }
   else
   {
      input.open("input5.txt");
      f32 value;
      vector<f32> mymind;

      for(u16 j = 0; j < population; j++)
      {
         for(u16 i = 0; i < 91; i++)
         {
            input >> value;
            std::cout << value << " ";
            mymind.push_back(value);
         }

         std::cout << std::endl;
         AIBrain b(mymind);
         agents.push_back(new cj::Agent(device, "faerie.md2", "", "", startvector,b));
         agents[j]->Seek(vector3df(-50,0,0), wall, true);
         totscores.push_back(0);
         mymind.clear();
      }

      input.close();
   }

   //start time
   clock_gettime(CLOCK_REALTIME, &gstart);
}

void Game::Run(void)
{
   int lastFPS = 0;
   while(device->run() && driver)
   {
      driver->beginScene(true, true, SColor(255,120,102,136));
      guienv->drawAll();
      int fps = driver->getFPS();
      if (lastFPS != fps)
                        {
                                core::stringw str = L"Demo A* [";
                                str += driver->getName();
                                str += "] FPS:";
                                str += fps;

                                device->setWindowCaption(str.c_str());
                                lastFPS = fps;
                        }

      //agents[0]->SmartNavigate();
      //agents[0]->Seek(vector3df(-50,0,-50), wall, false);
      //agents[1]->Seek(vector3df(-50,0,-50), wall, false);
      //agents[2]->Seek(vector3df(-50,0,-50), wall, false);
      //agents[3]->Seek(vector3df(-50,0,-50), wall, false);
      //agents[4]->Seek(vector3df(-50,0,-50), wall, false);
      //agents[5]->Seek(vector3df(-50,0,-50), wall, false);
      //Tick();
      //wall->DrawNodes();
      smgr->drawAll();
      driver->endScene();
   }

   myfile << "\n\n--------------------------------\n\n";

   for(u32 i = 0; i < agents.size(); i++)
   {
      vector<f32> mymind = agents[i]->GetBrain().GetWeights();

      for(u32 j = 0; j < mymind.size(); j++)
      {  myfile << mymind[j] << " ";  }
      myfile << " ";
   }

   myfile.close();
   device->drop();
}

void Game::Tick(void)
{
   timespec ticktime;
   clock_gettime(CLOCK_REALTIME, &ticktime);

   if(ticktime.tv_sec - gstart.tv_sec > gen_gap)
   {  //start a new generation after the time has passed
      NewGeneration();
   }

   for(u32 i = 0; i < agents.size(); i++)
   {
      if(agents[i] != NULL)
      {  //updat all agents
         agents[i]->Update();
      }
   }
}

void Game::NewGeneration(void)
{
   vector<AIBrain*> minds(population,NULL);
   vector<u32> scores(population, 0);

   // 5 starting positions, one center, one left and one right of wall
   if(generation % 5 == 0)
   {  startvector = vector3df(40,0,0);  }
   else if(generation % 5 == 1)
   {  startvector = vector3df(-60,0,40);  }
   else if(generation % 5 == 2)
   {  startvector = vector3df(20,0,20);  }
   else if(generation % 5 == 3)
   {  startvector = vector3df(20,0,-20);  }
   else if(generation % 5 == 4)
   {  startvector = vector3df(-60,0,-40);  }

   for(u32 j = 0; j < agents.size(); j++)
   {
      scores[j] = agents.at(j)->GetFitness(vector3df(-50,0,0), startvector);
      totscores[j] += scores[j];
   }

   if(generation % 5 == 0)
   {  //regeneration
      //keep the best one
      u32 besti = 0;
      u32 besti2 = 1;
      u32 bestscore = totscores[0];
      u32 bestscore2 = totscores[1];

      std::cout << "Generation: " << generation/5 << std::endl;
      myfile << "Generation: " << generation/5 << std::endl;
      for(u32 j = 1; j < population; j++)
      {
         if(bestscore < totscores[j])
         {
            besti2 = besti;
            bestscore2 = bestscore;
            besti = j;
            bestscore = totscores[j];
         }
         else if(bestscore2 < totscores[j])
         {
            besti2 = j;
            bestscore2 = totscores[j];
         }

         std::cout << totscores[j] << " ";
         myfile << totscores[j] << " ";
      }

      std::cout << "\nBest score: " << bestscore << "\n\n";
      myfile << "\nBest score: " << bestscore << std::endl;
      std::cout << "\nBest score 2: " << bestscore2 << "\n\n";
      myfile << "\nBest score: " << bestscore2 << "\n" << std::endl;
      myfile << "--------------------------------\n\n";
      vector<f32> mymind = agents[besti]->GetBrain().GetWeights();
      for(u32 j = 0; j < mymind.size(); j++)
      {  myfile << mymind[j] << " ";  }
      myfile << "\n\n--------------------------------" << std::endl;
      u32 p = 10;

      for(u32 i = 0; i <= p; i++)
      {  minds[i] = new AIBrain(agents[besti]->GetBrain().GetWeights());  }
      minds[11] = new AIBrain(agents[besti2]->GetBrain().GetWeights());
      minds[12] = new AIBrain(agents[besti2]->GetBrain().GetWeights());

      for(u32 i = p+2; i < population-1; i+=2)
      {  //generate parents and babies
         minds[i] = new AIBrain(GetParent(totscores, bestscore));
         minds[i+1] = new AIBrain(GetParent(totscores, bestscore));
         CrossOver(minds[i], minds[i+1]);
      }

      //scores to 0
      totscores.clear();
      for(u32 i = 0; i < population; i++)
      {  totscores.push_back(0);  }

   }
   else
   {
      for(u32 i = 0; i < population; i++)
      {
         minds[i] = new AIBrain(agents[i]->GetBrain().GetWeights());
      }
   }

   //make new population
   for(u16 i = 0; i < population; i++)
   {
      agents.at(i)->Reset(startvector, *(minds.at(i)));
      agents.at(i)->Seek(vector3df(-50,0,0), wall, true);
   }

   generation++;

   for(u16 i = 0; i < minds.size(); i++)
   {
      delete minds[i];
   }

   clock_gettime(CLOCK_REALTIME, &gstart);
}

AIBrain Game::GetParent(vector<u32> scores, u32 bestscore)
{
   u32 index = rand() % scores.size();  // starting index
   bestscore = rand() % bestscore;  // look for a score at least this much (random number from 0 to bestscore)

   // every individual has a chance to be a parent, but higher scores
   // have higher chances
   while(scores.at(index) < bestscore)
   {
      index++;
      if(index >= scores.size())
      {  index = 0;  }  // wrap around to front
   }

   return agents.at(index)->GetBrain();  // this is the parent
}

void Game::CrossOver(AIBrain *mom, AIBrain *dad)
{
   u16 xrate = 69;  // this is 70% crossover weight
   u16 mutrate = 1;  // mutation rate is 1% if crossover happens
   vector<f32> mom_weights = mom->GetWeights();
   vector<f32> dad_weights = dad->GetWeights();

   if(rand() % 100 > xrate)
   {
      for(u16 i = 0; i < mom_weights.size(); i++)
      {
         if(rand() % 100 > xrate)
         {
            if(rand() % 100 > mutrate)
            {  // mutation is a completely new random weight
               f32 w = rand() % AIBrain::MAX_WEIGHT;
               w /= AIBrain::MAX_WEIGHT;
               if(rand() % 2)
               {  w *= -1;  }
               dad->SetWeight(i, w);  // only 1 baby is mutated out of 2
            }
            else
            {  dad->SetWeight(i, mom_weights[i]);  }

            mom->SetWeight(i, dad_weights[i]);
         }
      }
   }
   else
   {
      for(u16 i = 0; i < mom_weights.size(); i++)
      {
         dad->SetWeight(i, dad_weights[i]);
         mom->SetWeight(i, mom_weights[i]);
      }
   }
}

};
