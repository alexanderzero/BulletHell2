#include "random.hpp"
#include <random>

static auto& prng_engine()
{
   static std::random_device rd{};
   static std::mt19937 engine{ rd() };
   
   return engine;
}

int randomInt(int min, int max)
{
   std::uniform_int_distribution<> dist{ min, max };
   return dist(prng_engine());
}
float randomFloat(float min, float max)
{
   std::uniform_real_distribution<float> dist{ min, max };
   return dist(prng_engine());
}