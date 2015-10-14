
//geometry.cpp


#include "geometry.hpp"
#include "math.h"

Vec2& Vec2::operator/=(float scalar)
{
	return (*this) *= (1.0f / scalar);
}

Vec2& Vec2::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	return *this;
}

Vec2& Vec2::operator-=(Vec2 const& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

Vec2& Vec2::operator+=(Vec2 const& rhs)
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}

Vec2::Vec2(float x_in, float y_in) : x(x_in), y(y_in)
{

}

Vec2::Vec2()
{
	x = y = 0.0f;
}

Vec2 normalized(Vec2 const& vec)
{
	Vec2 out(vec);
	return normalize(out);
}

Vec2& normalize(Vec2& vec)
{
	return vec /= length(vec);
}

float dot(Vec2 const& lhs, Vec2 const& rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y;
}
Vec2 perp(Vec2 const& vec)
{
	return Vec2(-vec.y, vec.x);
}
float length(Vec2 const& vec)
{
	return sqrtf(lengthSquared(vec));
}
float lengthSquared(Vec2 const& vec)
{
	return dot(vec, vec);
}


//collision checks



bool isColliding(Vec2 const& lhs, Box const& rhs)
{
	return (lhs.x >= rhs.min.x) &&
		(lhs.x <= rhs.max.x) &&
		(lhs.y >= rhs.min.y) &&
		(lhs.y <= rhs.max.y);
}
bool isColliding(Box const& lhs, Box const& rhs)
{
	if ((lhs.max.x < rhs.min.x) ||
		(lhs.min.x > rhs.max.x) ||
		(lhs.max.y < rhs.min.y) ||
		(lhs.min.y > rhs.max.y))
	{
		return false;
	}
	return true;
}
bool isColliding(Box const& lhs, Circle const& rhs)
{
	return distance(rhs, lhs) == 0.0f;
}
bool isColliding(Circle const& lhs, Circle const& rhs)
{
	return distance(lhs, rhs) == 0.0f; // you can make this use no sqrtf... lazy though
}
bool isColliding(Vec2 const& lhs, Circle const& rhs)
{
	return distance(lhs, rhs) == 0.0f; // you can make this use no sqrtf... lazy though
}



//distance checks

float distance(Vec2 const& lhs, Vec2 const& rhs)
{
	Vec2 temp(lhs);
	temp -= rhs;
	return length(temp);
}
float distance(Vec2 const& lhs, Box const& rhs)
{
	float xDist = 0.0f;
	float yDist = 0.0f;

	if (lhs.x < rhs.min.x)
	{
		xDist = rhs.min.x - lhs.x;
	}
	else if (lhs.x > rhs.max.x)
	{
		xDist = rhs.max.x - lhs.x;
	}

	if (lhs.y < rhs.min.y)
	{
		yDist = rhs.min.y - lhs.y;
	}
	else if (lhs.y > rhs.max.y)
	{
		yDist = rhs.max.y - lhs.y;
	}

	return sqrtf(xDist *xDist + yDist * yDist);
}
float distance(Vec2 const& lhs, Circle const& rhs)
{
	float dist = distance(lhs, rhs.center) - rhs.radius;
	return (dist < 0.0f) ? 0.0f : dist;
}
float distance(Circle const& lhs, Circle const& rhs)
{
	float dist = distance(lhs.center, rhs.center) - rhs.radius - lhs.radius;
	return (dist < 0.0f) ? 0.0f : dist;
}
float distance(Circle const& lhs, Box const& rhs)
{
	float dist = distance(lhs.center, rhs) - lhs.radius;
	return (dist < 0.0f) ? 0.0f : dist;
}


//angles

Radian degToRad(Degree degrees) { return degrees * degToRadConvert; }
Degree radToDeg(Radian radians) { return radians * radToDegConvert; }

Vec2 polarToRect(Radian angle, float length /*= 1.0f*/)
{
	Vec2 out(cos(angle), sin(angle));
	return out *= length;
}

Radian rectToPolar(Vec2 const& rect, float* outLen/*=nullptr*/)
{
	auto out = atan2f(rect.y, rect.x);
	if (outLen) *outLen = length(rect);
	return out;
}
Radian angleBetween(Vec2 const& pt1, Vec2 const& pt2)
{
	auto diff = pt2;
	diff -= pt1; //convert to a vector...
	return rectToPolar(diff); //angle in polar form.
}
bool isClockwise(Vec2 const& pt1, Vec2 const& pt2)
{
	return dot(pt1, perp(pt2)) > 0.0f;
}
Radian vectorAngleOffset(Vec2 const& first, Vec2 const& second)
{
	float dotp = dot(first, second);
	if (dotp > 1.0f) dotp = 1.0f;
	else if (dotp < -1.0f) dotp = -1.0f;
	Radian rawAngle = acosf(dotp);
	if (isClockwise(first, second))
	{
		return TAU - rawAngle;
	}
	return rawAngle;
}


Box Box::fromCenterExtents(Vec2 const& center, Vec2 const& extents)
{
   Vec2 halved = extents;
   halved *= 0.5f;
   return fromCenterHalfExtents(center, halved);
}
Box Box::fromCenterHalfExtents(Vec2 const& center, Vec2 const& halfExtents)
{
   Box out;
   out.min = out.max = center;
   out.min -= halfExtents;
   out.max += halfExtents;
   return out;
}