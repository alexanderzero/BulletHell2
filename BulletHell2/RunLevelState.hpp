#pragma once

#include "BulletHell2.hpp"
#include "Level.hpp"

std::unique_ptr<BulletHellState> runLevelStateCreate(BulletHellContext* ctxt, Level level);
