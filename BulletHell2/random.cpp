#include "random.hpp"
#include <random>

//this commend was made by le epic troll brandon

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

Vec2 randomPointInCircle(Circle const& circle)
{
   float r = sqrtf(randomFloat(0.0f, 1.0f));
   float angle = randomFloat(0, TAU);
   Vec2 pos = circle.center;
   pos += polarToRect(angle, r * circle.radius);
   return pos;
}