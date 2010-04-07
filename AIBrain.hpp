#ifndef CJ_AIBRAIN_HPP
#define CJ_AIBRAIN_HPP

#include <vector>
#include <cmath>
#include <iostream>
#include <irrlicht.h>

using namespace irr;
using namespace irr::core;
using std::vector;

namespace cj
{

class AIBrain
{
    public:
        AIBrain();
        AIBrain(vector<f32> w);
        ~AIBrain();
        vector<f64> ChangeGoal(vector<f32> inputs);  //decide to create subgoals
        vector<f32> GetWeights(void);  //get neural network weights
        void SetWeight(u16 i, f32 w);  //adjust during crossover
        const static u16 MAX_WEIGHT;  //used for creating random floats

    private:
        vector<f32> weights;
        const static u16 HIDDEN;  //number of hidden nodes
        const static u16 INPUTS;  //number of inputs
        const static u16 OUTPUTS;  //number of outputs
};

}

#endif
