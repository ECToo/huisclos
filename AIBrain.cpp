#include "AIBrain.hpp"

namespace cj
{

const u16 AIBrain::HIDDEN = 10;
const u16 AIBrain::INPUTS = 10;
const u16 AIBrain::OUTPUTS = 3;
const u16 AIBrain::MAX_WEIGHT = 100;

AIBrain::AIBrain()
{  //initial random numbers
   u32 total = INPUTS * HIDDEN + HIDDEN * OUTPUTS;

   //range for weights is -1 to 1
   for(u32 i = 0; i < total; i++)
   {
      f32 w = rand() % MAX_WEIGHT;
      w /= MAX_WEIGHT;
      if(rand() % 2)
      {  w *= -1;  }
      weights.push_back(w);
   }
}

AIBrain::AIBrain(vector<f32> w) : weights(w)
{}

AIBrain::~AIBrain()
{}

vector<f64> AIBrain::ChangeGoal(vector<f32> inputs)
{
   vector<f64> hidden_sum(HIDDEN,0.00);
   vector<f64> output_sum(OUTPUTS,0.00);

   for(u16 i = 0; i < inputs.size(); i++)
   {  //sum weights * input for hidden nodes
      for(u16 j = 0; j < HIDDEN; j++)
      {  hidden_sum.at(j) += weights.at(i+j) * inputs.at(i);  }
   }

   u16 bias = HIDDEN + inputs.size();

   for(u16 k = 0; k < HIDDEN; k++)
   {  //do the same for the bias
      hidden_sum.at(k) -= weights.at(bias+k);

      for(u16 m = 0; m < OUTPUTS; m++)
      {  //weights * hidden summation is the output node
         output_sum.at(m) += hidden_sum.at(k) * weights.at(bias+m+k);
      }
   }

   //Use sigmoid function for each output node
   for(u16 n = 0; n < OUTPUTS; n++)
   {  output_sum.at(n) = 1/(1 + exp(-output_sum.at(n)));  }

   return output_sum;
}

vector<f32> AIBrain::GetWeights(void)
{  return weights;  }

void AIBrain::SetWeight(u16 i, f32 w)
{
   weights[i] = w;
}

}
