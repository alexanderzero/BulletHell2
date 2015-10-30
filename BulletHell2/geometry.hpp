//geometry.hpp

#pragma once

#include <math.h>

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

struct Vec3
{
   Vec3() {}

   Vec3(float x, float y, float z)
   {
      this->x = x;
      this->y = y;
      this->z = z;
   }

   Vec3& operator-=(const Vec3& right)
   {
      x -= right.x;
      y -= right.y;
      z -= right.z;
      return *this;
   }

   Vec3& operator*=(float scalar)
   {
      x *= scalar;
      y *= scalar;
      z *= scalar;
      return *this;
   }

   //base data
   float x, y, z;
};

//Vec3 functions
inline float dot(const Vec3& a, const Vec3& b)
{
   return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline float length(const Vec3& vec)
{
   return sqrtf(dot(vec, vec));
}

inline Vec3 normalized(const Vec3& in)
{
   Vec3 out;
   out = in;
   out *= 1.0 / length(in);
   return out;
}

inline Vec3 cross(const Vec3& a, const Vec3& b)
{
   Vec3 out;
   out.x = a.y*b.z - a.z*b.y;
   out.y = a.z*b.x - a.x*b.z;
   out.z = a.x*b.y - a.y*b.x;
   return out;
}

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

struct Mat4
{
   float data[16];
   float& operator[](int i)
   {
      return data[i];
   }

   static Mat4 identity()
   {
      Mat4 out;
      for (size_t i = 0; i < 16; i++)
         out[i] = 0.0;
      out[0] = 1.0;
      out[5] = 1.0;
      out[10] = 1.0;
      out[15] = 1.0;
      return out;
   }

   static Mat4 ortho(float left, float right, float bottom, float top, float near, float far)
   {
      Mat4 out;
      float tx = -(right + left) / (right - left);
      float ty = -(top + bottom) / (top - bottom);
      float tz = -(far + near) / (far - near);
      out[0] = 2 / (right - left);
      out[1] = 0.0;
      out[2] = 0.0;
      out[3] = 0.0;
      out[4] = 0.0;
      out[5] = 2 / (top - bottom);
      out[6] = 0.0;
      out[7] = 0.0;
      out[8] = 0.0;
      out[9] = 0.0;
      out[10] = -2 / (far - near);
      out[11] = 0.0;
      out[12] = tx;
      out[13] = ty;
      out[14] = tz;
      out[15] = 1.0;
      return out;
   }

   static Mat4 camera(Vec3 position, Vec3 target, Vec3 up)
   {
      Vec3 lookdir = target;
      lookdir -= position;
      lookdir = normalized(lookdir);

      Vec3 right = normalized(cross(lookdir, up));

      Vec3 rotated_up = normalized(cross(right, lookdir));

      Mat4 out;
      out[0] = right.x; out[1] = right.y; out[2] = right.z; out[3] = 0.0;
      out[4] = rotated_up.x; out[5] = rotated_up.y; out[6] = rotated_up.z; out[7] = 0.0;
      out[8] = -lookdir.x; out[9] = -lookdir.y; out[10] = -lookdir.z; out[11] = 0.0;
      out[12] = -position.x; out[13] = -position.y; out[14] = -position.z; out[15] = 1.0;
      return out;
   }
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
float distance(Line const& seg, Vec2 const& point);

//box math

//angles 


static const float PI = 3.1415926f;
static const float TAU = PI * 2;

static const Degree radToDegConvert = 180 / PI;
static const Radian degToRadConvert = PI / 180;


Radian degToRad(Degree degrees);
Degree radToDeg(Radian radians);


//interpolation

float lerp(float v1, float v2, float t);
float cosInterp(float v1, float v2, float t);
Vec2 lerp(Vec2 const& v1, Vec2 const& v2, float t);
Vec2 cosInterp(Vec2 const& v1, Vec2 const& v2, float t);