//geometry.hpp

#pragma once

typedef float Degree;
typedef float Radian;

struct Vec2
{
	Vec2();
	Vec2(float x_in, float y_in);

	//operators
	Vec2& operator +=(Vec2 const& rhs);
	Vec2& operator -=(Vec2 const& rhs);
	Vec2& operator *=(float scalar);
	Vec2& operator /=(float scalar);

	//base data
	float x, y;
};

struct Circle
{
	Vec2 center;
	float radius;
};

struct Box
{
   static Box fromCenterExtents(Vec2 const& center, Vec2 const& extents);
   static Box fromCenterHalfExtents(Vec2 const& center, Vec2 const& halfExtents);
	Vec2 min, max;
};

struct Line
{
	Vec2 start, end;
};

struct OrientedBox
{
	Line center;
	float width;
};

//vector math

Vec2 normalized(Vec2 const& vec);
Vec2& normalize(Vec2&  vec);
float dot(Vec2 const& lhs, Vec2 const& rhs);
Vec2 perp(Vec2 const& vec);
float length(Vec2 const& vec);
float lengthSquared(Vec2 const& vec);
Vec2 polarToRect(Radian angle, float length = 1.0f);
Radian rectToPolar(Vec2 const& rect, float* outLen = nullptr);
Radian angleBetween(Vec2 const& pt1, Vec2 const& pt2);
bool isClockwise(Vec2 const& first, Vec2 const& second); //when rotating first to second, is it faster to clockwise?
Radian vectorAngleOffset(Vec2 const& first, Vec2 const& second); //travelling around

																 //collision checks

bool isColliding(Vec2 const& lhs, Box const& rhs);
bool isColliding(Box const& lhs, Box const& rhs);
bool isColliding(Box const& lhs, Circle const& rhs);
bool isColliding(Circle const& lhs, Circle const& rhs);
bool isColliding(Vec2 const& lhs, Circle const& rhs);

//distance checks

float distanceSquared(Vec2 const& lhs, Box const& rhs);

float distance(Vec2 const& lhs, Vec2 const& rhs);
float distance(Vec2 const& lhs, Box const& rhs);
float distance(Vec2 const& lhs, Circle const& rhs);
float distance(Circle const& lhs, Circle const& rhs);
float distance(Circle const& lhs, Box const& rhs);


//box math

//angles 


static const float PI = 3.1415926f;
static const float TAU = PI * 2;

static const Degree radToDegConvert = 180 / PI;
static const Radian degToRadConvert = PI / 180;


Radian degToRad(Degree degrees);
Degree radToDeg(Radian radians);


