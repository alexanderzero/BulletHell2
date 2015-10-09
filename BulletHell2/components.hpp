#pragma once

//components are all small...  it's easy to just define/ include all in this one file.
#include <string>
#include <vector>
#include <unordered_map>
#include "geometry.hpp"

struct NameComponent
{
	NameComponent() {}
	NameComponent(std::string name_in) : name(std::move(name_in)) {}
	std::string name;
};

struct PositionComponent
{
	PositionComponent() {}
	PositionComponent(Vec2 pos_in) : pos(pos_in) {}
	PositionComponent(float x, float y) : pos(Vec2(x,y)) {}
	Vec2 pos;
};

