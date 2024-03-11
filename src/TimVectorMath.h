#ifndef TIM_MATH
#define TIM_MATH

#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 2D HEADER

typedef union
{
	struct
	{
		float x;
		float y;
	};
	float elements[2];
} Vec2;

typedef union
{
	float m[3][3];
	struct
	{
		float m00, m01, m02;
		float m10, m11, m12;
		float m20, m21, m22;
	};
} Mat3;

static inline Mat3 M3(
	float m00, float m10, float m20,
	float m01, float m11, float m21,
	float m02, float m12, float m22)
{
	Mat3 m;
	m.m[0][0] = m00;
	m.m[1][0] = m10;
	m.m[2][0] = m20;
	m.m[0][1] = m01;
	m.m[1][1] = m11;
	m.m[2][1] = m21;
	m.m[0][2] = m02;
	m.m[1][2] = m12;
	m.m[2][2] = m22;
	return m;
}

// 3D HEADER

typedef union
{
	struct
	{
		float x, y, z;
	};

	struct
	{
		float r, g, b;
	};

	struct
	{
		Vec2 xy;
		float _ignored0;
	};

	float elements[3];

} Vec3;

typedef union
{
	struct
	{
		float x, y, z, w;
	};

	struct
	{
		float r, g, b, a;
	};

	struct
	{
		Vec3 xyz;
		float _ignored0;
	};

	struct
	{
		Vec2 xy;
		Vec2 _ignored1;
	};

	float elements[4];

} Vec4;

typedef union
{
	float m[4][4];
	struct
	{
		float m00, m01, m02, m03;
		float m10, m11, m12, m13;
		float m20, m21, m22, m23;
		float m30, m31, m32, m33;
	};
} Mat4;

static inline Mat4 M4(
	float m00, float m10, float m20, float m30,
	float m01, float m11, float m21, float m31,
	float m02, float m12, float m22, float m32,
	float m03, float m13, float m23, float m33)
{
	Mat4 m;
	m.m[0][0] = m00;
	m.m[1][0] = m10;
	m.m[2][0] = m20;
	m.m[3][0] = m30;
	m.m[0][1] = m01;
	m.m[1][1] = m11;
	m.m[2][1] = m21;
	m.m[3][1] = m31;
	m.m[0][2] = m02;
	m.m[1][2] = m12;
	m.m[2][2] = m22;
	m.m[3][2] = m32;
	m.m[0][3] = m03;
	m.m[1][3] = m13;
	m.m[2][3] = m23;
	m.m[3][3] = m33;
	return m;
}

static inline Mat4 M4Identity();
static inline Mat4 M4Translation(Vec3 offset);
static inline Mat4 M4Scaling(Vec3 scale);
static inline Mat4 M4RotationX(float angle_in_rad);
static inline Mat4 M4RotationY(float angle_in_rad);
static inline Mat4 M4RotationZ(float angle_in_rad);

Mat4 M4Rotation(float angle_in_rad, Vec3 axis);

Mat4 M4Ortho(float left, float right, float bottom, float top, float back, float front);
Mat4 M4Perspective(float vertical_field_of_view_in_deg, float aspect_ratio, float near_view_distance, float far_view_distance);
Mat4 M4LookAt(Vec3 from, Vec3 to, Vec3 up);

static inline Mat4 M4Transpose(Mat4 matrix);
static inline Mat4 M4Mul(Mat4 a, Mat4 b);
Mat4 M4InvertAffine(Mat4 matrix);
Vec3 M4MulPos(Mat4 matrix, Vec3 position);
Vec3 M4MulDir(Mat4 matrix, Vec3 direction);

void M4Print(Mat4 matrix);
void M4PrintP(Mat4 matrix, int width, int precision);
void M4FPrint(FILE *stream, Mat4 matrix);
void M4FPrintP(FILE *stream, Mat4 matrix, int width, int precision);

// Mat3 functions

Mat3 M3Translation(Vec2 translation);
Mat3 M3TranslationAndScale(Vec2 translation, float scaleX, float scaleY);
Vec2 M3MulPos(Mat3 matrix, Vec2 position);
void M3FPrintP(FILE *stream, Mat3 matrix, int width, int precision);

// 2D Functions

static inline Vec2
V2(float x, float y)
{
	return {{x, y}};
}

static inline Vec2
V2Polar(float angle, float radius)
{
	float c = cosf(angle);
	float s = sinf(angle);
	return V2(c * radius, s * radius);
}

static inline Vec2
V2Add(Vec2 a, Vec2 b)
{
	return {{a.x + b.x, a.y + b.y}};
}

static inline Vec2
V2Sub(Vec2 a, Vec2 b)
{
	return {{a.x - b.x, a.y - b.y}};
}

static inline Vec2
V2Mul(Vec2 a, Vec2 b)
{
	return {{a.x * b.x, a.y * b.y}};
}

static inline Vec2
V2Div(Vec2 a, Vec2 b)
{
	return {{a.x / b.x, a.y / b.y}};
}


static inline Vec2
V2AddS(Vec2 a, float s)
{
	return {{a.x + s, a.y + s}};
}

static inline Vec2
V2SubS(Vec2 a, float s)
{
	return {{a.x - s, a.y - s}};
}

static inline Vec2
V2MulS(Vec2 a, float s)
{
	return {{a.x * s, a.y * s}};
}

static inline Vec2
V2DivS(Vec2 a, float s)
{
	return {{a.x / s, a.y / s}};
}

static inline float
V2Dot(Vec2 a, Vec2 b)
{
	return a.x * b.x + a.y * b.y;
}

static inline float
V2Len(Vec2 v)
{
	return sqrtf(v.x * v.x + v.y * v.y);
}

static inline float
V2Len2(Vec2 v)
{
	return v.x * v.x + v.y * v.y;
}

static inline float
V2Dist2(Vec2 a, Vec2 b)
{
	return V2Len2(V2Sub(b, a));
}

static inline float
V2Dist(Vec2 a, Vec2 b)
{
	return V2Len(V2Sub(b, a));
}


static inline Vec2
V2Norm(Vec2 v)
{
	return V2MulS(v, 1.0 / V2Len(v));
}

static inline Vec2
V2Rotate(Vec2 v, float t)
{
	float c = cosf(t);
	float s = sinf(t);
	return V2(c * v.x - s * v.y, s * v.x + c * v.y);
}

static inline Vec2
V2Lerp(Vec2 from, Vec2 to, float s)
{
	Vec2 diff = V2Sub(to, from);
	return V2Add(from, V2MulS(diff, s));
}

// 3D

static inline Vec3
V3(float x, float y, float z)
{
	return {{x, y, z}};
}

static inline Vec4
V4(float x, float y, float z, float w)
{
	return {{x, y, z, w}};
}

static inline Vec3
V3Add(Vec3 a, Vec3 b)
{
	return {{a.x + b.x, a.y + b.y, a.z + b.z}};
}

static inline Vec3
V3AddS(Vec3 a, float s)
{
	return {{a.x + s, a.y + s, a.z + s}};
}

static inline Vec3
V3Sub(Vec3 a, Vec3 b)
{
	return {{a.x - b.x, a.y - b.y, a.z - b.z}};
}

static inline Vec3
V3SubS(Vec3 a, float s)
{
	return {{a.x - s, a.y - s, a.z - s}};
}

static inline Vec3
V3Mul(Vec3 a, Vec3 b)
{
	return {{a.x * b.x, a.y * b.y, a.z * b.z}};
}

static inline Vec3
V3MulS(Vec3 a, float s)
{
	return {{a.x * s, a.y * s, a.z * s}};
}

static inline Vec3
V3Div(Vec3 a, Vec3 b)
{
	return {{a.x / b.x, a.y / b.y, a.z / b.z}};
}

static inline Vec3
V3DivS(Vec3 a, float s)
{
	return {{a.x / s, a.y / s, a.z / s}};
}

static inline float
V3Len(Vec3 v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline float
V3Dot(Vec3 a, Vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3
V3Norm(Vec3 v)
{
	float len = V3Len(v);
	if (len > 0)
		return {{v.x / len, v.y / len, v.z / len}};
	else
		return {{0, 0, 0}};
}

static inline Vec3
V3Proj(Vec3 v, Vec3 onto)
{
	return V3MulS(onto, V3Dot(v, onto) / V3Dot(onto, onto));
}

static inline Vec3
V3Cross(Vec3 a, Vec3 b)
{
	return {{a.y * b.z - a.z * b.y,
			 a.z * b.x - a.x * b.z,
			 a.x * b.y - a.y * b.x}};
}

static inline float
V3AngleBetween(Vec3 a, Vec3 b)
{
	return acosf(V3Dot(a, b) / (V3Len(a) * V3Len(b)));
}

static inline Mat4
M4Identity()
{
	return M4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
}

static inline Mat4
M4Translation(Vec3 offset)
{
	return M4(
		1, 0, 0, offset.x,
		0, 1, 0, offset.y,
		0, 0, 1, offset.z,
		0, 0, 0, 1);
}

static inline Mat4
M4Scaling(Vec3 scale)
{
	float x = scale.x, y = scale.y, z = scale.z;
	return M4(
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1);
}

static inline Mat4
M4RotationX(float angle_in_rad)
{
	float s = sinf(angle_in_rad), c = cosf(angle_in_rad);
	return M4(
		1, 0, 0, 0,
		0, c, -s, 0,
		0, s, c, 0,
		0, 0, 0, 1);
}

static inline Mat4
M4RotationY(float angle_in_rad)
{
	float s = sinf(angle_in_rad), c = cosf(angle_in_rad);
	return M4(
		c, 0, s, 0,
		0, 1, 0, 0,
		-s, 0, c, 0,
		0, 0, 0, 1);
}

static inline Mat4
M4RotationZ(float angle_in_rad)
{
	float s = sinf(angle_in_rad), c = cosf(angle_in_rad);
	return M4(
		c, -s, 0, 0,
		s, c, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
}

static inline Mat4
M4Transpose(Mat4 matrix)
{
	return M4(
		matrix.m00, matrix.m01, matrix.m02, matrix.m03,
		matrix.m10, matrix.m11, matrix.m12, matrix.m13,
		matrix.m20, matrix.m21, matrix.m22, matrix.m23,
		matrix.m30, matrix.m31, matrix.m32, matrix.m33);
}

/**
 * Multiplication of two 4x4 matrices.
 *
 * Implemented by following the row times column rule and illustrating it on a
 * whiteboard with the proper indices in mind.
 *
 * Further reading: https://en.wikipedia.org/wiki/Matrix_multiplication
 * But note that the article use the first index for rows and the second for
 * columns.
 */
static inline Mat4
M4Mul(Mat4 a, Mat4 b)
{
	Mat4 result;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			float sum = 0;
			for (int k = 0; k < 4; k++)
			{
				sum += a.m[k][j] * b.m[i][k];
			}
			result.m[i][j] = sum;
		}
	}

	return result;
}

#ifdef __cplusplus
static inline Vec2 operator-(Vec2 a)
{
	return V2(-a.x, -a.y);
}

static inline Vec2 operator+(Vec2 a, Vec2 b) { return V2Add(a, b); }
static inline Vec2 operator-(Vec2 a, Vec2 b) { return V2Sub(a, b); }

static inline Vec2 operator*(Vec2 a, Vec2 b) { return V2Mul(a, b); }
static inline Vec2 operator*(Vec2 a, float b) { return V2MulS(a, b); }
static inline Vec2 operator*(float a, Vec2 b) { return V2MulS(b, a); }

static inline Vec2 operator/(Vec2 a, Vec2 b) { return V2Div(a, b); }
static inline Vec2 operator/(Vec2 a, float b) { return V2DivS(a, b); }
static inline Vec2 operator/(float a, Vec2 b) { return V2DivS(b, 1.0f/a); }

static inline Vec2 operator+=(Vec2 &left, Vec2 right) { return left = V2Add(left, right); }
static inline Vec2 operator-=(Vec2 &left, Vec2 right) { return left = V2Sub(left, right); }

// Vec3
static inline Vec3 operator+(Vec3 a, Vec3 b) { return V3Add(a, b); }
static inline Vec3 operator-(Vec3 a, Vec3 b) { return V3Sub(a, b); }

static inline Vec3 operator*(Vec3 a, Vec3 b) { return V3Mul(a, b); }
static inline Vec3 operator*(Vec3 a, float b) { return V3MulS(a, b); }
static inline Vec3 operator*(float a, Vec3 b) { return V3MulS(b, a); }

static inline Vec3 operator+=(Vec3 &left, Vec3 right) { return left = V3Add(left, right); }
static inline Vec3 operator-=(Vec3 &left, Vec3 right) { return left = V3Sub(left, right); }

#endif

#endif // MATH_3D_HEADER
