#include "BulletHell2.hpp"

int main(int argc, char** argv)
{
   BulletHell2 game;
   game.startup();
   game.run();
   game.shutdown();
}