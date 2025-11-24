/*
Title: "Vector Math library"
File: VectorMath.h
Author(s): Eduardo Mart�nez Vidal

Abstract:
Vector, Quaternion and Matrix definitions

Update Log:

*/

#pragma once
#ifdef _MSC_VER
#pragma warning( push )
//4201: Non standard nameless struct. 
#pragma warning(disable: 4201) 
#endif

#include <cstdint>
#include <memory>
#include <cstring>
#include "BasicMath.h"
#include "weave/system/memory/DataType.h"

namespace weave {

//Helper utility classes
class Transform; //Forward declaration for Matrix4x4 to Transform operations. Implemented in Transform.
struct TransformData; //Forward declaration for Matrix4x4 to TransformData operations. Implemented in Transform.
//Forward declaration of vector templates
template<typename Type>
struct Vector2T;
template<typename Type>
struct Vector3T;
template<typename Type>
struct Vector4T;

//Forward declaration of matrix templates
template<typename Type>
struct Matrix4x4T;
template<typename Type>
struct Matrix3x3T;
template<typename Type>
struct Matrix2x2T;

//Forward declarion of quaternion template
template<typename Type>
struct QuaternionT;

//Forward declaration of curve template
template<typename Type>
class CurveT;

//Forward declaration of spline template
template<uint32_t, typename Type>
class SplineT;

template<typename Type>
class DynamicSplineT;

//Long name defines
using Vector2 = Vector2T<float>;
using Vector3 = Vector3T<float>;
using Vector4 = Vector4T<float>;
using DVector2 = Vector2T<double>;
using DVector3 = Vector3T<double>;
using DVector4 = Vector4T<double>;
using BVector2 = Vector2T<bool>;
using BVector3 = Vector3T<bool>;
using BVector4 = Vector4T<bool>;
using IVector2 = Vector2T<int>;
using IVector3 = Vector3T<int>;
using IVector4 = Vector4T<int>;
using UVector2 = Vector2T<unsigned int>;
using UVector3 = Vector3T<unsigned int>;
using UVector4 = Vector4T<unsigned int>;

using Matrix2x2 = Matrix2x2T<float>;
using Matrix3x3 = Matrix3x3T<float>;
using Matrix4x4 = Matrix4x4T<float>;
using DMatrix2x2 = Matrix2x2T<double>;
using DMatrix3x3 = Matrix3x3T<double>;
using DMatrix4x4 = Matrix4x4T<double>;

using Quaternion = QuaternionT<float>;
using DQuaternion = QuaternionT<double>;

using Curve = CurveT<float>;

template<uint32_t curveCount>
using Spline = SplineT<curveCount, float>;

using DynamicSpline = DynamicSplineT<float>;

//A plane is simple a rename of Vec4 with the normal being the xyz part
template<typename Type>
using PlaneT = Vector4T<Type>;
using Plane = Vector4;
using DPlane = DVector4;

//GLSL style defines
using vec2 = Vector2T<float>;
using vec3 = Vector3T<float>;
using vec4 = Vector4T<float>;
using dvec2 = Vector2T<double>;
using dvec3 = Vector3T<double>;
using dvec4 = Vector4T<double>;
using bvec2 = Vector2T<bool>;
using bvec3 = Vector3T<bool>;
using bvec4 = Vector4T<bool>;
using ivec2 = Vector2T<int>;
using ivec3 = Vector3T<int>;
using ivec4 = Vector4T<int>;
using uvec2 = Vector2T<unsigned int>;
using uvec3 = Vector3T<unsigned int>;
using uvec4 = Vector4T<unsigned int>;

template<typename Type>
using vec2t = Vector2T<Type>;
template<typename Type>
using vec3t = Vector3T<Type>;
template<typename Type>
using vec4t = Vector4T<Type>;

using mat2 = Matrix2x2T<float>;
using mat3 = Matrix3x3T<float>;
using mat4 = Matrix4x4T<float>;
using dmat2 = Matrix2x2T<double>;
using dmat3 = Matrix3x3T<double>;
using dmat4 = Matrix4x4T<double>;

template<typename Type>
using mat2t = Matrix2x2T<Type>;
template<typename Type>
using mat3t = Matrix3x3T<Type>;
template<typename Type>
using mat4t = Matrix4x4T<Type>;

using quat = QuaternionT<float>;
using dquat = QuaternionT<double>;
template<typename Type>
using quatt = QuaternionT<Type>;

/*
//Vector Math conversion functions from string
template<typename T>
T string_to(std::string const& str_in) {
	std::string str = str_in;
	std::replace(str.begin(), str.end(), ',', ' ');
	std::stringstream ss(str);

	T val;
	for (uint32_t n = 0; n < T::NumComps(); ++n) {
		ss >> val[n];
	}

}
*/

/*
//String convertion functions for Math types
namespace std {

template<typename T>
std::string to_string(weave::Vector2T<T> const &v) {
	return std::to_string(v.x) + " " + std::to_string(v.y);
}
template<typename T>
std::string to_string(weave::Vector3T<T> const &v) {
	return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z);
}
template<typename T>
std::string to_string(weave::Vector4T<T> const &v) {
	return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z) + " " + std::to_string(v.w);
}

template<typename T>
std::string to_string(weave::Matrix2x2T<T> const &v) {
	return std::to_string(v.X) + " " + std::to_string(v.Y);
}

template<typename T>
std::string to_string(weave::Matrix3x3T<T> const &v) {
	return std::to_string(v.X) + " " + std::to_string(v.Y) + " " + std::to_string(v.Z);
}

template<typename T>
std::string to_string(weave::Matrix4x4T<T> const &v) {
	return std::to_string(v.X) + " " + std::to_string(v.Y) + " " + std::to_string(v.Z) + " " + std::to_string(v.W);
}

template<typename T>
std::string to_string(weave::QuaternionT<T> const &v) {
	return std::to_string(v.ijks);
}

}
*/

//Template declarations
//Vec2
template <typename Type>
struct Vector2T {
	using BaseType = Type;

	union {
		Type data[2] = {};
		struct { Type x, y; };
		struct { Type r, g; };
	};

	constexpr Vector2T();
	Vector2T(Vector2T const &vIn) = default;
	Vector2T(Vector2T &&vIn) = default;
	constexpr explicit Vector2T(Type val);
	Vector2T(std::initializer_list<Type> list);

	constexpr explicit Vector2T(Type x, Type y);
	explicit Vector2T(Type const *vIn);

	//Assignment
	constexpr void Set(Type x, Type y);
	void Set(Type const *vIn);
	constexpr void Set(Vector2T const &vIn);

	//Information about the vector
	constexpr unsigned int Size() const;

	//Bool checkers. Will not compile given some Types
	bool Any() const;
	bool All() const;

	//Operators: Access
	template<typename Index>
	constexpr Type operator [] (Index i) const;
	template<typename Index>
	constexpr Type& operator[] (Index i);

	//Operators: Assignment
	Vector2T& operator = (Vector2T const &vIn);
	Vector2T& operator = (Vector2T &&vIn);
	Vector2T& operator = (std::initializer_list<Type> list);
	Vector2T& operator = (Type const *vIn);

	constexpr Vector2T& operator += (Vector2T const &vIn);
	constexpr Vector2T& operator -= (Vector2T const &vIn);
	constexpr Vector2T& operator *= (Vector2T const &vIn);
	constexpr Vector2T& operator *= (Type c);
	constexpr Vector2T& operator /= (Vector2T const &vIn);
	constexpr Vector2T& operator /= (Type c);

	operator bool() const { return All(); }

	constexpr static inline uint32_t NumComps();
};


//Vec3
template <typename Type>
struct Vector3T {
	using BaseType = Type;

	union {
		Type data[3] = {};
		struct { Type x, y, z; };
		struct { Type r, g, b; };
		struct { Vector2T<Type> xy; Type z_alt; };
		struct { Vector2T<Type> rg; Type b_alt; };
		struct { Type x_alt; Vector2T<Type> yz; };
		struct { Type r_alt; Vector2T<Type> gb; };
	};

	constexpr Vector3T();
	Vector3T(Vector3T const &vIn) = default;
	Vector3T(Vector3T &&vIn) = default;
	constexpr explicit Vector3T(Type val);
	Vector3T(std::initializer_list<Type> list);

	constexpr explicit Vector3T(Type x, Type y, Type z);
	explicit Vector3T(Type const *vIn);
	constexpr explicit Vector3T(Vector2T<Type> const &vIn, Type z);
	constexpr explicit Vector3T(Type x, Vector2T<Type> const &vIn);
	//explicit Vector3T(Quaternion const &q);

	//Assignment
	constexpr void Set(Type x, Type y, Type z);
	void Set(Type const *vIn);
	constexpr void Set(Vector3T const &vIn);
	constexpr void Set(Vector2T<Type> const &vIn, Type z);
	constexpr void Set(Type x, Vector2T<Type> const &vIn);

	//Information about the vector
	constexpr unsigned int Size() const;

	//Bool checkers. Will not compile given some Types
	bool Any() const;
	bool All() const;

	//Operators: Access
	template<typename Index>
	constexpr Type operator [] (Index i) const;
	template<typename Index>
	constexpr Type& operator[] (Index i);

	//Operators: Assignment
	Vector3T& operator = (Vector3T const &vIn);
	Vector3T& operator = (Vector3T &&vIn);
	Vector3T& operator = (std::initializer_list<Type> list);
	Vector3T& operator = (Type const *vIn);
	//Vector3T& operator = (Quaternion const &q);

	constexpr Vector3T& operator += (Vector3T const &vIn);
	constexpr Vector3T& operator -= (Vector3T const &vIn);
	constexpr Vector3T& operator *= (Vector3T const &vIn);
	constexpr Vector3T& operator *= (Type c);
	constexpr Vector3T& operator /= (Vector3T const &vIn);
	constexpr Vector3T& operator /= (Type c);

	operator bool() const { return All(); }

	constexpr static inline uint32_t NumComps();
};

//Vec4
template <typename Type>
struct Vector4T {
	using BaseType = Type; 
	
	union {
		Type data[4] = {};
		struct { Type x, y, z, w; };
		struct { Type r, g, b, a; };
		struct { Vector2T<Type> xy, zw; };
		struct { Vector2T<Type> rg, ba; };
		struct { Type x_alt; Vector2T<Type> yz; Type w_alt0; };
		struct { Type r_alt; Vector2T<Type> gb; Type a_alt0; };
		struct { Type x_alt2; Vector3T<Type> yzw; };
		struct { Type r_alt2; Vector3T<Type> gba; };
		struct { Vector3T<Type> xyz; Type w_alt1; };
		struct { Vector3T<Type> rgb; Type a_alt1; };
	};

	constexpr Vector4T();
	Vector4T(Vector4T const &vIn) = default;
	Vector4T(Vector4T &&vIn) = default;
	constexpr explicit Vector4T(Type val);
	Vector4T(std::initializer_list<Type> list);

	constexpr explicit Vector4T(Type x, Type y, Type z, Type w);
	explicit Vector4T(Type const *vIn);
	constexpr Vector4T(Vector3T<Type> const &vIn);
	constexpr explicit Vector4T(Vector3T<Type> const &vIn, Type w);
	constexpr explicit Vector4T(Type x, Vector3T<Type> const &vIn);
	constexpr explicit Vector4T(Type x, Type y, Vector2T<Type> const &vIn);
	constexpr explicit Vector4T(Type x, Vector2T<Type> const &vIn, Type w);
	constexpr explicit Vector4T(Vector2T<Type> const &vIn, Type z, Type w);
	constexpr explicit Vector4T(Vector2T<Type> const &vInXY, Vector2T<Type> const &vInZW);
	explicit Vector4T(QuaternionT<Type> const &q) : Vector4T(q.ijks) {}

	//Assignment
	constexpr void Set(Type x, Type y, Type z, Type w);
	void Set(Type const *vIn);
	constexpr void Set(Vector4T const &vIn);
	constexpr void Set(Vector3T<Type> const &vIn);
	constexpr void Set(Vector3T<Type> const &vIn, Type w);
	constexpr void Set(Type x, Vector3T<Type> const &vIn);
	constexpr void Set(Type x, Type y, Vector2T<Type> const &vIn);
	constexpr void Set(Type x, Vector2T<Type> const &vIn, Type w);
	constexpr void Set(Vector2T<Type> const &vIn, Type z, Type w);


	//Information about the vector
	constexpr unsigned int Size() const;

	//Bool checkers. Will not compile given some Types
	bool Any() const;
	bool All() const;

	//Operators: Access
	template<typename Index>
	constexpr Type operator [] (Index i) const;
	template<typename Index>
	constexpr Type& operator[] (Index i);

	//Operators: Assignment
	Vector4T& operator = (Vector4T const &vIn);
	Vector4T& operator = (Vector4T &&vIn);
	Vector4T& operator = (std::initializer_list<Type> list);
	Vector4T& operator = (Type const *vIn);
	Vector4T& operator = (Vector3T<Type> const &vIn);
	Vector4T& operator = (QuaternionT<Type> const &q);
	
	constexpr Vector4T& operator += (Vector4T const &vIn);
	constexpr Vector4T& operator -= (Vector4T const &vIn);
	constexpr Vector4T& operator *= (Vector4T const &vIn);
	constexpr Vector4T& operator *= (Type c);
	constexpr Vector4T& operator /= (Vector4T const &vIn);
	constexpr Vector4T& operator /= (Type c);

	operator bool() const { return All(); }

	constexpr static inline uint32_t NumComps();
};

/*
Matrix4x4

This matrix class is based on column vectors organized in memory as column major; that is, the colums are stored sequentialy. This is the OpenGL style.

Conceptualy, the matrix looks like this:

|Xx Yx Zx Tx|			The X, Y and Z vectors of the base space are defined as columns followed by the Translation vector
|Xy Yy Zy Ty|
|Xz Yz Zz Tz|
|0  0  0  1 |

Vectors thus are multiplied as columns:

|Xx Yx Zx Tx|   |x|   |x'|
|Xy Yy Zy Ty|   |y|	  |y'|		Where if W = 1 the T vector defines a translation
|Xz Yz Zz Tz| * |z| = |z'|
|0  0  0  1 |   |w|   |w'|

However, in memory they are stored in a way that may aparently be transposed. This storage method should be visualized as consecutive vectors instead:

m[] = (X,Y,Z,T)     Where X,Y,Z and T are vectors. Thus we store each colum in a sequence, which linearly would look like this:

m[] = ([Xx,Xy,Xz,Xw],[Yx,Yy,Yz,Yw],[Zx,Zy,Zz,Zw],[Tx,Ty,Tz,1])

So, even though conceptualy we operate with vectors stored as columns, internaly they are stored linearly, a column vector after another.

The locations then are:

X = (m[0], m[1], m[2], m[3])
Y = (m[4], m[5], m[6], m[7])
Z = (m[8], m[9], m[10],m[11])
T = (m[12],m[13],m[14],m[15])

Or viewed on a more paper friendly way:
X    Y     Z     W
m[0] m[4] m[8 ] m[12]
m[1] m[5] m[9 ] m[13]
m[2] m[6] m[10] m[14]
m[3] m[7] m[11] m[15]

Multiplication via the interface functions is equivalent to post multiplying the matrix. Example:

A.Mult(B) is equivalent to A = A*B
A.Mult(C,D) is equivalent to A = C*D
A.Transform(v) is equivalent to v' = A*v

*/

template<typename Type>
struct Matrix4x4T {
	using BaseType = Type;
	using VectorType = Vector4T<Type>;

	//Memory layout as consecutive vectors
	union {
		Type data[16] = {};
		//struct { Type Xx, Xy, Xz, Xw, Yx, Yy, Yz, Yw, Zx, Zy, Zz, Zw, Wx, Wy, Wz, Ww; };
		struct { Vector4T<Type> X, Y, Z, W; };
		Vector4T<Type> columns[4];
	};

	Matrix4x4T();
	Matrix4x4T(Matrix4x4T const &) = default;
	Matrix4x4T(Matrix4x4T &&) = default;

	Matrix4x4T(Type Xx, Type Xy, Type Xz, Type Xw,
			   Type Yx, Type Yy, Type Yz, Type Yw,
			   Type Zx, Type Zy, Type Zz, Type Zw,
			   Type Wx, Type Wy, Type Wz, Type Ww,
			   bool transpose = false
	);

	Matrix4x4T(Type const *mIn);

	Matrix4x4T(Vector4T<Type> const &X, Vector4T<Type> const &Y, Vector4T<Type> const &Z, Vector4T<Type> const &W);
	Matrix4x4T(Vector3T<Type> const &X, Vector3T<Type> const &Y, Vector3T<Type> const &Z);

	Matrix4x4T(Matrix3x3T<Type> const &m);

	Matrix4x4T(QuaternionT<Type> const &q);
	
	Matrix4x4T(QuaternionT<Type> const &q, Vector3T<Type> const &position);
	Matrix4x4T(QuaternionT<Type> const &q, Vector3T<Type> const &position, Vector3T<Type> const &scale);

	//Implemented in Transform.h
	Matrix4x4T(Transform const &tIn);
	Matrix4x4T(TransformData const &tIn);

	~Matrix4x4T() = default;

	void Identity();

	void Set(Type Xx, Type Xy, Type Xz, Type Xw,
			 Type Yx, Type Yy, Type Yz, Type Yw,
			 Type Zx, Type Zy, Type Zz, Type Zw,
			 Type Wx, Type Wy, Type Wz, Type Ww,
			 bool transpose = false);

	//Sets the matrix based on separated RTS
	void Set(QuaternionT<Type> const &q, Vector3T<Type> const &pos);
	void Set(QuaternionT<Type> const &q, Vector3T<Type> const &pos, Vector3T<Type> const &scale);

	//Row setters
	void SetRows(Vector3T<Type> const &a, Vector3T<Type> const &b, Vector3T<Type> const &c);
	void SetRows(Vector3T<Type> const &a, Vector3T<Type> const &b, Vector3T<Type> const &c, Vector3T<Type> const &d);
	void SetRows(Vector4T<Type> const &a, Vector4T<Type> const &b, Vector4T<Type> const &c, Vector4T<Type> const &d);

	template<typename Index>
	void SetRow(Index num, Vector3T<Type> const &a);
	template<typename Index>
	void SetRow(Index num, Vector4T<Type> const &a);
	
	//Column and row accessors
	void GetRows(Vector3T<Type> &a, Vector3T<Type> &b, Vector3T<Type> &c) const;
	void GetRows(Vector4T<Type> &a, Vector4T<Type> &b, Vector4T<Type> &c, Vector4T<Type> &d) const;
	template<typename Index>
	Vector4T<Type> GetRow(Index num) const;

	//Sets the rotational part of the matrix
	void SetNormalTransform(Matrix3x3T<Type> const &inmatrix);
	//Sets the rotational part of the matrix
	void SetNormalTransform(Matrix4x4T const &inmatrix);

	//Removes the translation part of the matrix
	void MakeNormalTransform();

	//Returns a 3x3 matrix with the rotational part 
	Matrix3x3T<Type> GetNormalTransform() const;
	Matrix4x4T GetNormalTransform4x4() const;

	//Returns the scale applied to the matrix as a vector3.
	Vector3T<Type> GetScalingFactor() const;
	//Removes the scaling factor from the matrix by normalizing the axes and returns the removed scaling factor
	Vector3T<Type> RemoveScalingFactor();


	//Returns the diagonal for the matrix
	Vector4T<Type> GetDiagonal() const;

	//Matrix operations
	Vector3T<Type> TransformNormal(Vector3T<Type> const &n) const;

	//Returns the transposed matrix
	Matrix4x4T Transposed() const;
	//Returns the inverse of the matrix
	Matrix4x4T Inverted() const;
	//Returns the inverse of the matrix assuming the matrix is orthogonal
	Matrix4x4T InvertedOrtho() const;

	//Transposes the matrix
	void Transpose();
	//Inverts the matrix
	int Invert();
	//Inverts the matrix assuming the matrix is orthogonal
	void InvertOrtho();

	//Returns the matrix trace
	Type Trace() const;
	//Returns the determinant of the matrix
	Type Determinant() const;

	//Interpolate between two matrices and store the result
	//Note that SRT matrix interpolation must be done by separating the scale, rotation and translation components, otherwise the result will be incorrect within the 3D gfx context
	//This interpolation is thus generating wrong values if the matrix represents a SRT. Use Transforms and interpolate them.
	void Interpolate(Matrix4x4T const &m1, Matrix4x4T const &m2, Type p);

	//Weights two matrices and stores the result
	void Weight(Matrix4x4T const &m1, Matrix4x4T const &m2, Type w1, Type w2);
	//Weights multiple matrices with an array of indices and weights
	void Weight(Matrix4x4T *ms, int const *indices, Type const *weights, unsigned int count);

	//Applies a translation transformation to the current matrix
	void Translate(Type x, Type y, Type z);
	void Translate(weave::Vector3T<Type> const &vec);
	void Translate(weave::Vector4T<Type> const &vec);
	//Applies a scaling transformation to the current matrix
	void Scale(Type x, Type y, Type z);
	void Scale(weave::Vector3T<Type> const &vec);
	void Scale(weave::Vector4T<Type> const &vec);
	//Applies a rotation transformation to the current matrix. Angles are in radians
	void Rotate(Type radians, Type  x, Type  y, Type  z);
	void Rotate(Type radians, weave::Vector3T<Type> const &vec);
	//Applies a rotation transformation to the current matrix using euler angles, in radians
	void Rotate(Type pitch, Type yaw, Type roll);
	void Rotate(weave::Vector3T<Type> const &pitchYawRoll);
	//Applies a reflection matrix to the current matrix given a reflection plane
	void Reflect(Plane const &plane);

	//Creates a translation matrix. Replaces the current matrix.
	void Translation(Type x, Type y, Type z);
	void Translation(weave::Vector3T<Type> const &vec);
	void Translation(weave::Vector4T<Type> const &vec);
	//Creates a scaling matrix. Replaces the current matrix.
	void Scaling(Type x, Type y, Type z);
	void Scaling(weave::Vector3T<Type> const &vec);
	void Scaling(weave::Vector4T<Type> const &vec);
	//Creates a rotational matrix. Replaces the current matrix. Angles are in radians.
	void Rotation(Type radians, Type  x, Type  y, Type  z);
	void Rotation(Type radians, weave::Vector3T<Type> const &vec);
	//Creates a rotational matrix using euler angles. Replaces the current matrix. Angles are in radians.
	void Rotation(Type pitch, Type yaw, Type roll);
	void Rotation(weave::Vector3T<Type> const &pitchYawRoll);
	//Creates a reflection matrix given a reflection plane
	void Reflection(Plane const &plane);

	//Loads an orthographic projection matrix
	void Ortho(Type left, Type right, Type bottom, Type top, Type zNear, Type zFar);
	//Loads a perspective matrix. Angle is supplied in degrees.
	void PerspectiveDeg(Type fovyDeg, Type ratio, Type zNear, Type zFar);
	//Loads a perspective matrix. Angle is supplied in radians.
	void PerspectiveRad(Type fovyRad, Type ratio, Type zNear, Type zFar);
	//Loads a perspective matrix in right hand zero notation. Angle is supplied in degrees.
	void PerspectiveRhzDeg(Type fovyDeg, Type ratio, Type zNear, Type zFar);
	//Loads a perspective matrix in right hand zero notation. Angle is supplied in radians.
	void PerspectiveRhzRad(Type fovyRad, Type ratio, Type zNear, Type zFar);
	//Applies a look at transformation to the matrix with legacy opengl style
	void LookAt(Type eyex, Type eyey, Type eyez, Type centerx, Type centery, Type centerz, Type upx, Type upy, Type upz);

	//Static Ctors, used to build specific matrices on the fly
	//Constructs a translation matrix and returns it
	static Matrix4x4T TranslationCtor(Type x, Type y, Type z);
	static Matrix4x4T TranslationCtor(weave::Vector3T<Type> const &vec);
	static Matrix4x4T TranslationCtor(weave::Vector4T<Type> const &vec);
	//Constructs a scaling matrix and returns it
	static Matrix4x4T ScalingCtor(Type x, Type y, Type z);
	static Matrix4x4T ScalingCtor(weave::Vector3T<Type> const &vec);
	static Matrix4x4T ScalingCtor(weave::Vector4T<Type> const &vec);
	//Constructs a rotational matrix and returns it. Angles are in radians.
	static Matrix4x4T RotationCtor(Type radians, Type  x, Type  y, Type  z);
	static Matrix4x4T RotationCtor(Type radians, weave::Vector3T<Type> const &vec);
	//Constructs a rotational matrix using euler angles and returns it. Angles are in radians.
	static Matrix4x4T RotationCtor(Type pitch, Type yaw, Type roll);
	static Matrix4x4T RotationCtor(weave::Vector3T<Type> const &pitchYawRoll);
	//Constructs a reflection matrix given a reflection plane
	static Matrix4x4T ReflectionCtor(Plane const &plane);

	//Constructs an orthographic projection matrix
	static Matrix4x4T OrthoCtor(Type left, Type right, Type bottom, Type top, Type zNear, Type zFar);
	//Constructs a perspective matrix. Angle is supplied in degrees.
	static Matrix4x4T PerspectiveDegCtor(Type fovyDeg, Type ratio, Type zNear, Type zFar);
	//Constructs a perspective matrix. Angle is supplied in radians.
	static Matrix4x4T PerspectiveRadCtor(Type fovyRad, Type ratio, Type zNear, Type zFar);
	//Constructs a perspective matrix in right hand zero notation. Angle is supplied in degrees.
	static Matrix4x4T PerspectiveRhzDegCtor(Type fovyDeg, Type ratio, Type zNear, Type zFar);
	//Constructs a perspective matrix in right hand zero notation. Angle is supplied in radians.
	static Matrix4x4T PerspectiveRhzRadCtor(Type fovyRad, Type ratio, Type zNear, Type zFar);
	
	//Constructs a look at transformation to the matrix
	static Matrix4x4T LookAtCtor(Type eyex, Type eyey, Type eyez, Type centerx, Type centery, Type centerz, Type upx, Type upy, Type upz);


	//Operators
	Matrix4x4T& operator = (Matrix3x3T<Type> const &mIn);
	Matrix4x4T& operator = (Matrix4x4T const &mIn);
	Matrix4x4T& operator = (Matrix4x4T &&mIn);
	Matrix4x4T& operator = (Type const *mIn);
	Matrix4x4T& operator = (weave::Transform const &tIn);
	Matrix4x4T& operator = (weave::TransformData const &tIn);
	Matrix4x4T& operator = (QuaternionT<Type> const &q);

	Matrix4x4T operator + (Matrix4x4T const &mIn) const;
	Matrix4x4T operator - (Matrix4x4T const &mIn) const;
	Matrix4x4T operator * (Matrix4x4T const &mIn) const;
	Matrix4x4T operator * (Type s) const;
	Matrix4x4T operator / (Type s) const;
	Vector3T<Type> operator * (Vector3T<Type> const &vIn) const;
	Vector4T<Type> operator * (Vector4T<Type> const &vIn) const;
	//Normal transform
	Vector3T<Type> operator % (Vector3T<Type> const &vIn) const;
	Vector4T<Type> operator % (Vector4T<Type> const &vIn) const;

	Matrix4x4T& operator += (Matrix4x4T const &mIn);
	Matrix4x4T& operator -= (Matrix4x4T const &mIn);
	Matrix4x4T& operator *= (Matrix4x4T const &mIn);
	Matrix4x4T& operator *= (Type s);
	Matrix4x4T& operator /= (Type s);

	template<typename Index>
	Type& operator [] (Index i);
	template<typename Index>
	Type operator [] (Index i) const;
	template<typename IndexR, typename IndexC>
	Type& operator() (IndexR row, IndexC col);
	template<typename IndexR, typename IndexC>
	Type  operator() (IndexR row, IndexC col) const;

	bool operator== (Matrix4x4T const &mCmp) const;
	bool operator!= (Matrix4x4T const &mCmp) const;

	constexpr static inline uint32_t NumComps();
};


//Matrix3x3
//See Matrix4x4 notes.
template<typename Type>
struct Matrix3x3T {
	using BaseType = Type;
	using VectorType = Vector3T<Type>;

	//Memory layout as consecutive vectors
	union {
		Type data[9] = {};
		//struct { Type Xx, Xy, Xz, Yx, Yy, Yz, Zx, Zy, Zz; };
		struct { Vector3T<Type> X, Y, Z; };
		Vector3T<Type> columns[3];
	};

	Matrix3x3T();
	Matrix3x3T(Matrix3x3T const &) = default;
	Matrix3x3T(Matrix3x3T &&) = default;


	Matrix3x3T(Type Xx, Type Xy, Type Xz,
			   Type Yx, Type Yy, Type Yz,
			   Type Zx, Type Zy, Type Zz,
			   bool transpose = false
	);

	Matrix3x3T(Type const *mIn);
	Matrix3x3T(Matrix4x4T<Type> const &m4x4);
	Matrix3x3T(Vector3T<Type> const &X, Vector3T<Type> const &Y, Vector3T<Type> const &Z);
	Matrix3x3T(QuaternionT<Type> const &q);

	~Matrix3x3T() = default;

	void Identity();

	void Set(Type Xx, Type Xy, Type Xz,
			 Type Yx, Type Yy, Type Yz,
			 Type Zx, Type Zy, Type Zz,
			 bool transpose = false
	);

	//Returns the scale applies to the matrix as a vector3.
	Vector3T<Type> GetScalingFactor() const;
	//Removes the scaling factor from the matrix by normalizing the axes and returns the removed scaling factor
	Vector3T<Type> RemoveScalingFactor();

	void SetRows(Vector3T<Type> const &a, Vector3T<Type> const &b, Vector3T<Type> const &c);
	template<typename Index>
	void SetRow(Index num, Vector3T<Type> const &a);
	
	void GetRows(Vector3T<Type> &a, Vector3T<Type> &b, Vector3T<Type> &c) const;
	template<typename Index>
	Vector3T<Type> GetRow(Index num) const;

	Vector3T<Type> GetDiagonal() const;

	Matrix3x3T Transposed() const;
	Matrix3x3T Inverted() const;
	Matrix3x3T InvertedOrtho() const;

	void Transpose();
	int Invert();
	void InvertOrtho();

	void SkewSymmetric(Vector3T<Type> const &v);

	Type Trace() const;

	//Returns the determinant of the matrix
	Type Determinant() const;

	void Interpolate(Matrix3x3T const &m1, Matrix3x3T const &m2, Type p);

	void Weight(Matrix3x3T const &m1, Matrix3x3T const &m2, Type w1, Type w2);
	void Weight(Matrix3x3T *ms, int const *indices, Type const *weights, unsigned int count);

	//Rotates the matrix. The current matrix is transformed by the specified rotation. Angles are in radians.
	void Rotate(Type radians, Type  x, Type  y, Type  z);
	void Rotate(Type radians, weave::Vector3T<Type> const &vec);
	void Rotate(Type pitch, Type yaw, Type roll);
	void Rotate(weave::Vector3T<Type> const &pitchYawRoll);

	//Scales the matrix. The current matrix is transformed by the specified scaling
	void Scale(Type x, Type y, Type z);
	void Scale(weave::Vector3T<Type> const &vec);

	//Creates a rotational transformation matrix instead of multiplying the transformation over. The current matrix is replaced. Angles are in radians.
	void Rotation(Type radians, Type  x, Type  y, Type  z);
	void Rotation(Type radians, weave::Vector3T<Type> const &vec);
	void Rotation(Type pitch, Type yaw, Type roll);
	void Rotation(weave::Vector3T<Type> const &pitchYawRoll);

	//Creates a scaling transformation matrix instead of multiplying the transformation over. The current matrix is replaced.
	void Scaling(Type x, Type y, Type z);
	void Scaling(weave::Vector3T<Type> const &vec);

	//Inline operators
	Matrix3x3T& operator = (Matrix3x3T<Type> const &mIn);
	Matrix3x3T& operator = (Matrix3x3T<Type> &&mIn);
	Matrix3x3T& operator = (Type const *mIn);
	Matrix3x3T& operator = (Matrix4x4T<Type> const &mIn);
	Matrix3x3T& operator = (QuaternionT<Type> const &q);

	Matrix3x3T operator + (Matrix3x3T const &mIn) const;
	Matrix3x3T operator - (Matrix3x3T const &mIn) const;
	Matrix3x3T operator * (Matrix3x3T const &mIn) const;
	Matrix3x3T operator * (Type s) const;
	Matrix3x3T operator / (Type s) const;
	Vector3T<Type> operator * (Vector3T<Type> const &vIn) const;

	Matrix3x3T& operator += (Matrix3x3T const &mIn);
	Matrix3x3T& operator -= (Matrix3x3T const &mIn);
	Matrix3x3T& operator *= (Matrix3x3T const &mIn);
	Matrix3x3T& operator *= (Type s);
	Matrix3x3T& operator /= (Type s);

	template<typename Index>
	Type& operator [] (Index i);
	template<typename Index>
	Type operator [] (Index i) const;

	template<typename IndexR, typename IndexC>
	Type& operator() (IndexR row, IndexC col);
	template<typename IndexR, typename IndexC>
	Type  operator() (IndexR row, IndexC col) const;

	bool operator== (Matrix3x3T const &mCmp) const;
	bool operator!= (Matrix3x3T const &mCmp) const;

	constexpr static inline uint32_t NumComps();
};


//Matrix2x2
//See Matrix4x4 notes.
template<typename Type>
struct Matrix2x2T {
	using BaseType = Type;
	using VectorType = Vector2T<Type>;

	union {
		Type data[4] = {};
		//struct { Type Xx, Xy, Yx, Yy; };
		struct { Vector2T<Type> X, Y; };
		Vector2T<Type> columns[2];
	};

	Matrix2x2T();
	Matrix2x2T(Matrix2x2T const &) = default;
	Matrix2x2T(Matrix2x2T &&) = default;

	Matrix2x2T(Type Xx, Type Xy,
			   Type Yx, Type Yy,
			   bool transpose = false
	);

	Matrix2x2T(Type const *mIn);

	Matrix2x2T(Vector2T<Type> const &X, Vector2T<Type> const &Y);

	~Matrix2x2T();

	void Identity();

	void Set(Type Xx, Type Xy,
			 Type Yx, Type Yy,
			 bool transpose = false
	);

	void SetRows(Vector2T<Type> const &a, Vector2T<Type> const &b);
	template<typename Index>
	void SetRow(Index index, Vector2T<Type> const &a);

	void GetRows(Vector2T<Type> &a, Vector2T<Type> &b) const;

	template<typename Index>
	Vector2T<Type> GetRow(Index num) const;

	//Returns the scale applies to the matrix as a vector2.
	Vector2T<Type> GetScalingFactor() const;
	//Removes the scaling factor from the matrix by normalizing the axes and returns the removed scaling factor
	Vector2T<Type> RemoveScalingFactor();

	Vector2T<Type> GetDiagonal() const;

	Matrix2x2T Transposed() const;
	Matrix2x2T Inverted() const;
	Matrix2x2T InvertedOrtho() const;

	void Transpose();
	int Invert();
	void InvertOrtho();

	Type Trace() const;

	Type Determinant() const;

	void Interpolate(Matrix2x2T const &m1, Matrix2x2T const &m2, Type p);
	void Weight(Matrix2x2T const &m1, Matrix2x2T const &m2, Type w1, Type w2);
	void Weight(Matrix2x2T *ms, int const *indices, Type const *weights, unsigned int count);

	void Scale(Type x, Type y);
	void Rotate(Type radians, Type  x, Type  y);

	//Creates a scaling matrix. Replaces the current matrix
	void Scaling(Type x, Type y);
	//Creates a rotational matrix. Replaces the current matrix. Angles in radians.
	void Rotation(Type radians, Type x, Type y);

	//Inline operators
	Matrix2x2T& operator = (Matrix2x2T const &mIn);
	Matrix2x2T& operator = (Matrix2x2T &&mIn);
	Matrix2x2T& operator = (Type const *mIn);

	Matrix2x2T operator + (Matrix2x2T const &mIn) const;
	Matrix2x2T operator - (Matrix2x2T const &mIn) const;
	Matrix2x2T operator * (Matrix2x2T const &mIn) const;
	Matrix2x2T operator * (Type s) const;
	Matrix2x2T operator / (Type s) const;
	Vector2T<Type> operator * (Vector2T<Type> const &vIn) const;

	Matrix2x2T& operator += (Matrix2x2T const &mIn);
	Matrix2x2T& operator -= (Matrix2x2T const &mIn);
	Matrix2x2T& operator *= (Matrix2x2T const &mIn);
	Matrix2x2T& operator *= (Type s);
	Matrix2x2T& operator /= (Type s);

	template<typename Index>
	Type& operator [] (Index i);
	template<typename Index>
	Type operator [] (Index i) const;

	template<typename IndexR, typename IndexC>
	Type& operator() (IndexR row, IndexC col);
	template<typename IndexR, typename IndexC>
	Type  operator() (IndexR row, IndexC col) const;

	bool operator== (Matrix2x2T const &mCmp) const;
	bool operator!= (Matrix2x2T const &mCmp) const;

	constexpr static inline uint32_t NumComps();

};


//Quaternion:
template<typename Type>
struct QuaternionT {
	using BaseType = Type;
	
	//Components are ordered to match a vector4 representation
	union {
		Type data[4] = {};
		struct { Type i, j, k, s; };
		struct { Vector2T<Type> ij, ks; };
		struct { Type i_alt; Vector2T<Type> jk; Type s_alt0; };
		struct { Type i_alt2; Vector3T<Type> jks; };
		struct { Vector3T<Type> ijk; Type s_alt1; };
		Vector4T<Type> ijks;
	};

	constexpr QuaternionT();
	QuaternionT(QuaternionT const &) = default;
	QuaternionT(QuaternionT &&) = default;

	//Initializes the quaternion by setting its direct values
	QuaternionT(Type i, Type j, Type k, Type s);
	QuaternionT(Type const *vIn, Type s);
	QuaternionT(Vector3T<Type> const &vIn, Type s);
	QuaternionT(Type const *vIn);
	explicit QuaternionT(Vector3T<Type> const &euler);
	QuaternionT(Vector4T<Type> const &v4);
	//Initializes the quaternion by converting a matrix
	QuaternionT(Matrix3x3T<Type> const &r);
	QuaternionT(Matrix4x4T<Type> const &r);
	//Initializes the quaterion with an orientation determined by a rotation in radians and an axis
	QuaternionT(Type radians, Type x, Type y, Type z, bool normalize);
	QuaternionT(Type radians, weave::Vector3T<Type> const &axis, bool normalize = true);

	void Set(Type i, Type j, Type k, Type s);
	void Set(Type s, Type const *vIn);

	//QuaternionT conjugate
	void Conjugate();

	//Normalization
	void Normalize();

	//Inversion
	void Invert();
	QuaternionT Inverted() const;

	//Sets the quaternion identity (0,0,0,1)
	constexpr void Identity();

	//Sets the orientation of a quaternion given an angle in radians and a rotation vector
	void Orientation(Type radians, Type x, Type y, Type z, bool normalize = true);
	void Orientation(Type radians, weave::Vector3T<Type> const &axis, bool normalize = true);
	//Sets the orientation of a quaternion given euler angles in radians
	void Orientation(Type pitch, Type yaw, Type roll);
	void Orientation(weave::Vector3T<Type> const &pitchYawRoll);
	//Rotates the quaternion given a scaled vector that contains the axis and angle in radians
	void RotateScaled(weave::Vector3T<Type> const &scaledVector);
	//Rotates the quaternion by the defined axis and angle in radians
	void Rotate(Type radians, Type x, Type y, Type z, bool normalize = true);
	void Rotate(Type radians, weave::Vector3T<Type> const &axis, bool normalize = true);
	//Rotates the quaternion by the defined euler angles in radians
	void Rotate(Type pitch, Type yaw, Type roll);
	void Rotate(weave::Vector3T<Type> const &pitchYawRoll);
	//Rotates applying only a pitch rotation
	void RotatePitch(Type pitch);
	//Rotates applying only a yaw rotation
	void RotateYaw(Type yaw);
	//Rotates applying only a roll rotation
	void RotateRoll(Type roll);

	//Converts the quaternion to euler pitch, yaw, roll
	void ToEuler(Type &pitch, Type &yaw, Type &roll) const;
	//Converts the quaternion to euler pitch, yaw, roll
	weave::Vector3T<Type> ToEuler() const;

	//Convert quaternion to 3x3 rotational matrix
	Matrix3x3T<Type> ToMatrix() const;
	//Convert quaternion to 4x4 rotational matrix
	Matrix4x4T<Type> ToMatrix4() const;

	//Convert a 3x3 matrix to quaternion
	void FromMatrix(Matrix3x3T<Type> const &m);
	//Convert a 4x4 matrix to quaternion (ignoring translation)
	void FromMatrix(Matrix4x4T<Type> const &m);

	//Transforms a vector and returns the result 
	Vector3T<Type> Transform(Vector3T<Type> const &v) const;

	QuaternionT& operator = (QuaternionT const &q);
	QuaternionT& operator = (QuaternionT &&q);
	QuaternionT& operator = (Type const *vIn);
	QuaternionT& operator = (Vector4T<Type> const &vIn);
	QuaternionT& operator += (QuaternionT const &q);
	QuaternionT& operator -= (QuaternionT const &q);
	QuaternionT& operator *= (Type sc);
	QuaternionT& operator *= (QuaternionT const &q);
	QuaternionT& operator /= (Vector3T<Type> const &vIn);
	QuaternionT& operator /= (Type x);
	QuaternionT operator + (QuaternionT const &qIn) const;
	QuaternionT operator - (QuaternionT const &qIn) const;
	QuaternionT operator - () const;
	QuaternionT operator ~ () const;
	QuaternionT operator + (Type x) const;
	QuaternionT operator - (Type x) const;
	QuaternionT operator * (QuaternionT const &vIn) const;
	QuaternionT operator * (Type x) const;
	Vector3T<Type> operator * (Vector3T<Type> const &vIn) const;
	Vector3T<Type> operator * (Vector4T<Type> const &vIn) const;
	QuaternionT operator / (Type x)  const;
	bool operator == (QuaternionT const &vIn) const;
	bool operator != (QuaternionT const &vIn) const;
	operator bool() const { return ijks.All(); }

	template<typename Index>
	Type operator [] (Index i) const;
	template<typename Index>
	Type& operator[] (Index i);

	QuaternionT& operator = (Matrix3x3T<Type> const &m);
	QuaternionT& operator = (Matrix4x4T<Type> const &m);

	constexpr static inline uint32_t NumComps();
};

//Inline utilities
namespace algebra {

//The following global items can be used on methods that must return a const reference of an object but require returning a constant value
const Matrix2x2 Identity2x2;
const Matrix3x3 Identity3x3;
const Matrix4x4 Identity4x4;
const Vector2 IdentityVec2;
const Vector3 IdentityVec3;
const Vector4 IdentityVec4;
const Matrix4x4 BiasMatrix(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0);
const Matrix4x4 PerspectiveRhzCorrectionMatrix(
	1.0, 0.0, 0.0, 0.0,
	0.0,-1.0, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.0, 0.0, 0.5, 1.0);

//Equivalent compare (epsilon based floating point comparision)
template<typename Type>
bool equivalent(Type a, Type b, Type precision = Type(0.001)) { return std::abs(a - b) < precision; }
template<typename Type>
bool equivalent(vec2t<Type> const& a, vec2t<Type> const& b, Type precision = Type(0.001)) { return (std::abs(a.x - b.x) < precision) && (std::abs(a.y - b.y) < precision); }
template<typename Type>
bool equivalent(vec3t<Type> const& a, vec3t<Type> const& b, Type precision = Type(0.001)) { return (std::abs(a.x - b.x) < precision) && (std::abs(a.y - b.y) < precision) && (std::abs(a.z - b.z) < precision); }
template<typename Type>
bool equivalent(vec4t<Type> const& a, vec4t<Type> const& b, Type precision = Type(0.001)) { return (std::abs(a.x - b.x) < precision) && (std::abs(a.y - b.y) < precision) && (std::abs(a.z - b.z) < precision) && (std::abs(a.w - b.w) < precision); }

//Abs
template<typename Type>
inline vec2t<Type> abs(vec2t<Type> const &v) { return vec2t<Type>(std::abs(v.x), std::abs(v.y)); }
template<typename Type>
inline vec3t<Type> abs(vec3t<Type> const &v) { return vec3t<Type>(std::abs(v.x), std::abs(v.y), std::abs(v.z)); }
template<typename Type>
inline vec4t<Type> abs(vec4t<Type> const &v) { return vec4t<Type>(std::abs(v.x), std::abs(v.y), std::abs(v.z), std::abs(v.w)); }

//Dot product
template<typename Type>
inline Type dot(vec2t<Type> const& v1, vec2t<Type> const &v2) { return v1[0] * v2[0] + v1[1] * v2[1]; }
template<typename Type>
inline Type dot(vec3t<Type> const& v1, vec3t<Type> const &v2) { return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]; }
template<typename Type>
inline Type dot(vec4t<Type> const& v1, vec4t<Type> const &v2) { return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] + v1[3] * v2[3]; }
template<typename Type>
inline Type dot(quatt<Type> const &q1, quatt<Type> const &q2) { return q1.i * q2.i + q1.j * q2.j + q1.k * q2.k + q1.s * q2.s; }

//Length
template<typename Type>
inline Type lengthSqr(Vector2T<Type> const &v) { return v.x*v.x + v.y*v.y; }
template<typename Type>
inline Type lengthSqr(Vector3T<Type> const &v) { return v.x*v.x + v.y*v.y + v.z*v.z; }
template<typename Type>
inline Type lengthSqr(Vector4T<Type> const &v) { return v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w; }
template<typename Type>
inline Type lengthSqr(QuaternionT<Type> const &v) { return v.i*v.i + v.j*v.j + v.k*v.k + v.s*v.s; }

template<class vecType>
inline auto length(vecType const &v) { return std::sqrt(weave::algebra::lengthSqr(v)); }

//Normalization
template<class vecType>
inline vecType normalize(vecType const &v1) { return v1 / length(v1); }
template<typename Type>
inline quatt<Type> normalize(quatt<Type> const &q1) { quatt<Type> qret = q1; qret.Normalize(); return qret; }
template<class vecType>
inline vecType normalizesafe(vecType const &v1) { return v1 / (length(v1) + 0.0001f); }
template<class vecType>
inline bool normalizecond(vecType &v1) {
	auto len = length(v1);
	if (len <= 0.000001f) {
		return false;
	}
	v1 /= len;
	return true;
}

//Cross product
template<typename Type>
inline vec3t<Type> cross(vec3t<Type> const& v1, vec3t<Type> const& v2) { return vec3t<Type>(v1[1] * v2[2] - v1[2] * v2[1], v1[2] * v2[0] - v1[0] * v2[2], v1[0] * v2[1] - v1[1] * v2[0]); }

//Other utilities
//Normal vector of the surface defined by three points
template<typename vecType>
inline vecType normalvector(vecType const &p1, vecType const &p2, vecType const &p3) { return normalize(cross(p3 - p2, p1 - p2)); }

//Clamps the given value between min and max
template<typename Type>
inline Type clamp(Type value, Type min, Type max) { return std::max<Type>(min, std::min<Type>(max, value)); }

//Returns the shortest angle between two vectors (using dot product)
//Vectors are normalized by the function
template<class vecType>
inline typename vecType::BaseType angle(vecType const &v1, vecType const &v2) { return std::acos(clamp<typename vecType::BaseType>(dot(v1, v2) / (length(v1)*length(v2)), typename vecType::BaseType (-1.0), typename vecType::BaseType(1.0))); }

//Returns the shortest angle between two vectors (using dot product)
//Vectors are assumed normalized
template<class vecType>
inline typename vecType::BaseType anglenormal(vecType const &v1, vecType const &v2) { return std::acos(clamp<typename vecType::BaseType>(dot(v1, v2), typename vecType::BaseType(-1.0f), typename vecType::BaseType(1.0f))); }
template<class Type>
inline Type anglenormal(quatt<Type> const &q, quatt<Type> const &r) { return Type(2.0) * std::acos(clamp<Type>((q.i*r.i + q.j*r.j + q.k*r.k + q.s*r.s), Type(-1.0), Type(1.0f))); }

//Returns the angle between two 2D vectors (using tan2)
//Vectors are normalized by the function
template<class vecType>
inline typename vecType::BaseType angle2D(vecType const &v1, vecType const &v2) { 
	vecType a = algebra::normalize(v1);
	vecType b = algebra::normalize(v2);

	auto diff = std::atan2(b.y, b.x) - std::atan2(a.y, a.x);
	if(diff > algebra::F_PI) diff -= algebra::F_2PI;
	else if(diff < -algebra::F_PI) diff += algebra::F_2PI;
	return diff;
}

//Returns the angle between two 2D vectors (using tan2)
//Vector are assumed normalized
template<class vecType>
inline typename vecType::BaseType anglenormal2D(vecType const &v1, vecType const &v2) { 
	auto diff = std::atan2(v2.y, v2.x) - std::atan2(v1.y, v1.x);
	if(diff > algebra::F_PI) diff -= algebra::F_2PI;
	else if(diff < -algebra::F_PI) diff += algebra::F_2PI;
	return diff;
}

//Returns a 32bit RGBA value of a Vector4. The values of the vector are clamped between 0 and 1
unsigned int vec2rgba(Vector4 const &v);
//Returns a 32bit RGBA value of a Vector3. The values of the vector are clamped between 0 and 1
unsigned int vec2rgba(Vector3 const &v);
//Converts a 32bit RGBA value to a Vector4. The values of the vector are clamped between 0 and 1
Vector4 rgba2vec(unsigned int rgba);

//Component wise min and max between two vectors
template<class vecType>
inline vecType maxcomps(vecType const &v1, vecType const &v2) {
	vecType out;
	for(uint32_t i = 0; i < vecType::NumComps(); ++i) {
		out[i] = std::max(v1[i], v2[i]);
	}

	return out;
}

template<class vecType>
inline vecType mincomps(vecType const &v1, vecType const &v2) {
	vecType out;
	for(uint32_t i = 0; i < vecType::NumComps(); ++i) {
		out[i] = std::min(v1[i], v2[i]);
	}

	return out;
}

//Min and max component of a vector
template<class vecType>
inline auto maxof(vecType const &v1) -> auto {
	auto x = v1[0];
	for(uint32_t i = 1; i < vecType::NumComps(); ++i) {
		if(v1[i] > x) x = v1[i];
	}
	return x;
}

template<class vecType>
inline auto minof(vecType const &v1) -> auto {
	auto x = v1[0];
	for(uint32_t i = 1; i < vecType::NumComps(); ++i) {
		if(v1[i] < x) x = v1[i];
	}
	return x;
}

//Returns the bitangent (orthonormal vector between normal and tangent) and modifies normal and tangent so that normal, tangent and bitagent form an orthonormal base
template<typename Type>
inline vec3t<Type> orthonormalize(vec3t<Type> &normal, vec3t<Type> &tangent) {
	normal = weave::algebra::normalize(normal);
	vec3t<Type> bitangent = weave::algebra::normalize(cross(tangent, normal));
	tangent = weave::algebra::normalize(cross(normal, bitangent));

	return bitangent;
}

//Returns the bitangent (orthonormal vector between normal and tangent) and modifies tangent so that normal, tangent and bitagent form an orthonormal base. Assumes normal is normalized already
template<typename Type>
inline vec3t<Type> orthogonalize(vec3t<Type> const &normal, vec3t<Type> &tangent) {
	vec3t<Type> bitangent = weave::algebra::normalize(cross(normal, tangent));
	tangent = weave::algebra::normalize(cross(bitangent, normal));
	return bitangent;
}

//Projects a vector on a target vector without normalizing the target
template<class vecType>
inline vecType project(vecType const &vector, vecType const &on) {
	return on * dot(on, vector);
}

//Projects a vector on a target vector. Normalizes the target vector before projection
template<class vecType>
inline vecType projectnormalize(vecType const &vector, vecType const &on) { 
	//Normalize the on vector
	vecType normal = normalize(on);
	//Project the vector onto the normal
	return normal * dot(normal, vector);
}

//Projects a vector on a plane defined by the plane normal
template<class vecType>
inline vecType projectplane(vecType const &vector, vecType const &planeNormal) { 
	return vector - planeNormal * dot(planeNormal, vector); 
}

//Reflects a vector on a surface defined by the given normal
template<class vecType>
inline vecType reflect(vecType const &incidentv, vecType const &normal) { 
	return incidentv - ((normal)* dot(normal, incidentv) * typename vecType::BaseType(2.0)); 
}

//Calculates refraction on a surface defined by the given normal and index of refraction (IOR)
template<class vecType>
inline vecType refract(vecType const &incidentv, vecType const &normal, typename vecType::BaseType ior) {
	auto idotn = dot(normal, incidentv);
	auto k = typename vecType::BaseType(1.0) - ior*ior*(typename vecType::BaseType(1.0) - idotn*idotn);
	if(k < 0.0)
		return vecType();
	else
		return ior * incidentv - (ior*idotn + std::sqrt(k)) * normal;
}

//Transposes a matrix
template <class MatrixClass>
inline MatrixClass transpose(MatrixClass const &m) { return m.Transposed(); }

//Inverts a matrix (slow, but assumes no preconditions)
template <class MatrixClass>
inline MatrixClass invert(MatrixClass const &m) { return m.Inverted(); }

//Inverts an orthogonal matrix. If the matrix is not orthogonal, results will not be valid
template <class MatrixClass>
inline MatrixClass orthoinvert(MatrixClass const &m) { return m.InvertedOrtho(); }

//Swaps the handness of a Matrix4x4 from right to left or viceversa (tranposes the rotation 3x3 section and negates de z componente of the position)
template <typename Type>
inline Matrix4x4T<Type> swaphandness(Matrix4x4T<Type> const &m) { 
	Matrix4x4T<Type> mout = transpose(m.GetNormalTransform4x4());
	Vector4T<Type> col = m.W;
	Vector4T<Type> row = m.GetRow(3);
	col.z = -col.z;
	row.z = -row.z;
	mout.W = col;
	mout.SetRow(3, row);
	
	return mout;
}

//Generates a plane (vec4) based on three points
template <typename Type>
inline PlaneT<Type> makeplane(vec3t<Type> const &p1, vec3t<Type> const &p2, vec3t<Type> const &p3) {
	PlaneT<Type> plane(normalvector(p1, p2, p3));
	plane.w = -dot(plane.xyz, p1); 
	return plane;
}

//Generates a plane (vec4) based on a normal and a point on the plane
template <typename Type>
inline PlaneT<Type> makeplane(vec3t<Type> const &normal, vec3t<Type> const &point) {
	PlaneT<Type> plane(normalize(normal));
	plane.w = -dot(plane.xyz, point);
	return plane;
}

}

//************************
//Template implementations
//************************

//---------
//Vec2 impl

template<typename Type>
inline constexpr Vector2T<Type>::Vector2T() {}

template<typename Type>
inline constexpr Vector2T<Type>::Vector2T(Type val) : x(val), y(val) {}

template<typename Type>
inline Vector2T<Type>::Vector2T(std::initializer_list<Type> list) {
	int i = 0;
	for(; i<list.size() && i<2; ++i)
		data[i] = list.begin()[i];
}

template<typename Type>
inline constexpr Vector2T<Type>::Vector2T(Type x, Type y) : x(x), y(y) {}

template<typename Type>
inline Vector2T<Type>::Vector2T(Type const * vIn) : x(vIn[0]), y(vIn[1]) {}

//Assignment
template<typename Type>
inline constexpr void Vector2T<Type>::Set(Type ix, Type iy) { x = ix; y = iy; }

template<typename Type>
inline void Vector2T<Type>::Set(Type const * vIn) { x = vIn[0]; y = vIn[1]; }

template<typename Type>
inline constexpr void Vector2T<Type>::Set(Vector2T const & vIn) { x = vIn.x; y = vIn.y; }

//Information about the vector
template<typename Type>
inline constexpr unsigned int Vector2T<Type>::Size() const { return 2; }

//Bool checkers. Will not compile given some Types
template<typename Type>
inline bool Vector2T<Type>::Any() const {
	for(int i = 0; i < 2; ++i) {
		if(data[i]) {
			return true;
		}
	}

	return false;
}

template<typename Type>
inline bool Vector2T<Type>::All() const {
	for(int i = 0; i < 2; ++i) {
		if(!data[i]) {
			return false;
		}
	}

	return true;
}

template<typename Type>
inline constexpr uint32_t Vector2T<Type>::NumComps() { return 2; }

//Operators: Access
template<typename Type>
template<typename Index>
inline constexpr Type Vector2T<Type>::operator [] (Index i) const { return data[i]; }
template<typename Type>
template<typename Index>
inline constexpr Type& Vector2T<Type>::operator[] (Index i) { return data[i]; }



//Operators: Assignment
template<typename Type>
inline Vector2T<Type>& Vector2T<Type>::operator = (std::initializer_list<Type> list) {
	int i = 0;
	for(; i<list.size() && i<2; ++i)
		data[i] = list.begin()[i];
	for(; i < 2; ++i)
		data[i] = {};

	return *this;
}

template<typename Type>
inline Vector2T<Type>& Vector2T<Type>::operator = (Vector2T const &) = default;
template<typename Type>
inline Vector2T<Type>& Vector2T<Type>::operator = (Vector2T &&) = default;

template<typename Type>
inline Vector2T<Type>& Vector2T<Type>::operator = (Type const *vIn) { x = vIn[0]; y = vIn[1]; return *this; }
template<typename Type>
inline constexpr Vector2T<Type>& Vector2T<Type>::operator += (Vector2T const &vIn) { x += vIn.x; y += vIn.y; return *this; }
template<typename Type>
inline constexpr Vector2T<Type>& Vector2T<Type>::operator -= (Vector2T const &vIn) { x -= vIn.x; y -= vIn.y; return *this; }
template<typename Type>
inline constexpr Vector2T<Type>& Vector2T<Type>::operator *= (Vector2T const &vIn) { x *= vIn.x; y *= vIn.y; return *this; }
template<typename Type>
inline constexpr Vector2T<Type>& Vector2T<Type>::operator *= (Type c) { x *= c; y *= c; return *this; }
template<typename Type>
inline constexpr Vector2T<Type>& Vector2T<Type>::operator /= (Vector2T const &vIn) { x /= vIn.x; y /= vIn.y; return *this; }
template<typename Type>
inline constexpr Vector2T<Type>& Vector2T<Type>::operator /= (Type c) { x /= c; y /= c; return *this; }

//Unary -
template<typename T, std::enable_if_t<std::is_unsigned<T>::value, int> = 0>
inline Vector2T<T> operator - (Vector2T<T> const &v1) {
	return v1;
}

template<typename T, std::enable_if_t<std::is_signed<T>::value, int> = 0>
inline Vector2T<T> operator - (Vector2T<T> const &v1) {
	return Vector2T<T>(-v1.x, -v1.y);
}

//Artihmetic operations
template<typename T>
inline Vector2T<T> operator + (Vector2T<T> const &v1, Vector2T<T> const &v2) {
	return Vector2T<T>(v1.x + v2.x, v1.y + v2.y);
}

template<typename T>
inline Vector2T<T> operator - (Vector2T<T> const &v1, Vector2T<T> const &v2) {
	return Vector2T<T>(v1.x - v2.x, v1.y - v2.y);
}

template<typename T>
inline Vector2T<T> operator * (Vector2T<T> const &v1, Vector2T<T> const &v2) {
	return Vector2T<T>(v1.x * v2.x, v1.y * v2.y);
}

template<typename T>
inline Vector2T<T> operator / (Vector2T<T> const &v1, Vector2T<T> const &v2) {
	return Vector2T<T>(v1.x / v2.x, v1.y / v2.y);
}

template<typename T>
inline Vector2T<T> operator + (T c, Vector2T<T> const &v1) {
	return Vector2T<T>(c + v1.x, c + v1.y);
}

template<typename T>
inline Vector2T<T> operator + (Vector2T<T> const &v1, T c) {
	return Vector2T<T>(v1.x + c, v1.y + c);
}

template<typename T>
inline Vector2T<T> operator - (T c, Vector2T<T> const &v1) {
	return Vector2T<T>(c - v1.x, c - v1.y);
}

template<typename T>
inline Vector2T<T> operator - (Vector2T<T> const &v1, T c) {
	return Vector2T<T>(v1.x - c, v1.y - c);
}

template<typename T>
inline Vector2T<T> operator * (T c, Vector2T<T> const &v1) {
	return Vector2T<T>(c * v1.x, c * v1.y);
}

template<typename T>
inline Vector2T<T> operator * (Vector2T<T> const &v1, T c) {
	return Vector2T<T>(v1.x * c, v1.y * c);
}

template<typename T>
inline Vector2T<T> operator / (T c, Vector2T<T> const &v1) {
	return Vector2T<T>(c / v1.x, c / v1.y);
}

template<typename T>
inline Vector2T<T> operator / (Vector2T<T> const &v1, T c) {
	return Vector2T<T>(v1.x / c, v1.y / c);
}

//Equality checks
template<typename T>
inline Vector2T<bool> operator == (Vector2T<T> const &v1, Vector2T<T> const &v2) {
	return Vector2T<bool>(v1.x == v2.x, v1.y == v2.y);
}

template<typename T>
inline Vector2T<bool> operator != (Vector2T<T> const &v1, Vector2T<T> const &v2) {
	return Vector2T<bool>(v1.x != v2.x, v1.y != v2.y);
}

//Comparision checks
template<typename T>
inline Vector2T<bool> operator < (Vector2T<T> const &v1, Vector2T<T> const &v2) {
	return Vector2T<bool>(v1.x < v2.x, v1.y < v2.y);
}

template<typename T>
inline Vector2T<bool> operator <= (Vector2T<T> const &v1, Vector2T<T> const &v2) {
	return Vector2T<bool>(v1.x <= v2.x, v1.y <= v2.y);
}

template<typename T>
inline Vector2T<bool> operator > (Vector2T<T> const &v1, Vector2T<T> const &v2) {
	return Vector2T<bool>(v1.x > v2.x, v1.y > v2.y);
}

template<typename T>
inline Vector2T<bool> operator >= (Vector2T<T> const &v1, Vector2T<T> const &v2) {
	return Vector2T<bool>(v1.x >= v2.x, v1.y >= v2.y);
}

//---------
//Vec3 impl

template<typename Type>
inline constexpr Vector3T<Type>::Vector3T() {}

template<typename Type>
inline constexpr Vector3T<Type>::Vector3T(Type val) : x(val), y(val), z(val) {}

template<typename Type>
inline Vector3T<Type>::Vector3T(std::initializer_list<Type> list) {
	size_t i = 0;
	for(; i<list.size() && i<3; ++i)
		data[i] = list.begin()[i];
}

template<typename Type>
inline constexpr Vector3T<Type>::Vector3T(Type x, Type y, Type z) : x(x), y(y), z(z) {}

template<typename Type>
inline Vector3T<Type>::Vector3T(Type const * vIn) : x(vIn[0]), y(vIn[1]), z(vIn[2]) {}

template<typename Type>
inline constexpr Vector3T<Type>::Vector3T(Vector2T<Type> const & vIn, Type z) : x(vIn.x), y(vIn.y), z(z) {}

template<typename Type>
inline constexpr Vector3T<Type>::Vector3T(Type x, Vector2T<Type> const & vIn) : x(x), y(vIn.x), z(vIn.y) {}

//Assignment
template<typename Type>
inline constexpr void Vector3T<Type>::Set(Type ix, Type iy, Type iz) { x = ix; y = iy; z = iz; }

template<typename Type>
inline void Vector3T<Type>::Set(Type const * vIn) { x = vIn[0]; y = vIn[1]; z = vIn[2]; }

template<typename Type>
inline constexpr void Vector3T<Type>::Set(Vector3T const & vIn) { x = vIn.x; y = vIn.y; z = vIn.z; }

template<typename Type>
inline constexpr void Vector3T<Type>::Set(Vector2T<Type> const & vIn, Type iz) { x = vIn.x; y = vIn.y; z = iz; }

template<typename Type>
inline constexpr void Vector3T<Type>::Set(Type ix, Vector2T<Type> const & vIn) { x = ix; y = vIn.x; z = vIn.y; }

//Information about the vector
template<typename Type>
inline constexpr unsigned int Vector3T<Type>::Size() const { return 3; }

//Bool checkers. Will not compile given some Types
template<typename Type>
inline bool Vector3T<Type>::Any() const {
	for(int i = 0; i < 3; ++i) {
		if(data[i]) {
			return true;
		}
	}

	return false;
}

template<typename Type>
inline bool Vector3T<Type>::All() const {
	for(int i = 0; i < 3; ++i) {
		if(!data[i]) {
			return false;
		}
	}

	return true;
}

template<typename Type>
inline constexpr uint32_t Vector3T<Type>::NumComps() { return 3; }

//Operators: Access
template<typename Type>
template<typename Index>
inline constexpr Type Vector3T<Type>::operator [] (Index i) const { return data[i]; }
template<typename Type>
template<typename Index>
inline constexpr Type& Vector3T<Type>::operator[] (Index i) { return data[i]; }

//Operators: Assignment
template<typename Type>
inline Vector3T<Type>& Vector3T<Type>::operator = (Vector3T const &vIn) = default;
template<typename Type>
inline Vector3T<Type>& Vector3T<Type>::operator = (Vector3T &&vIn) = default;

template<typename Type>
inline Vector3T<Type>& Vector3T<Type>::operator = (std::initializer_list<Type> list) {
	size_t i = 0;
	for(; i<list.size() && i<3; ++i)
		data[i] = list.begin()[i];
	for(; i < 3; ++i)
		data[i] = {};

	return *this;
}

template<typename Type>
inline Vector3T<Type>& Vector3T<Type>::operator = (Type const *vIn) { x = vIn[0]; y = vIn[1]; z = vIn[2]; return *this; }
//void Vector3T<Type>::operator = (Quaternion const &q);

template<typename Type>
inline constexpr Vector3T<Type>& Vector3T<Type>::operator += (Vector3T const &vIn) { x += vIn.x; y += vIn.y; z += vIn.z; return *this; }
template<typename Type>
inline constexpr Vector3T<Type>& Vector3T<Type>::operator -= (Vector3T const &vIn) { x -= vIn.x; y -= vIn.y; z -= vIn.z; return *this; }
template<typename Type>
inline constexpr Vector3T<Type>& Vector3T<Type>::operator *= (Vector3T const &vIn) { x *= vIn.x; y *= vIn.y; z *= vIn.z; return *this; }
template<typename Type>
inline constexpr Vector3T<Type>& Vector3T<Type>::operator *= (Type c) { x *= c; y *= c; z *= c; return *this; }
template<typename Type>
inline constexpr Vector3T<Type>& Vector3T<Type>::operator /= (Vector3T const &vIn) { x /= vIn.x; y /= vIn.y; z /= vIn.z; return *this; }
template<typename Type>
inline constexpr Vector3T<Type>& Vector3T<Type>::operator /= (Type c) { x /= c; y /= c; z /= c; return *this; }

//Unary -
template<typename T, std::enable_if_t<std::is_unsigned<T>::value, int> = 0>
inline Vector3T<T> operator - (Vector3T<T> const &v1) {
	return v1;
}

template<typename T, std::enable_if_t<std::is_signed<T>::value, int> = 0>
inline Vector3T<T> operator - (Vector3T<T> const &v1) {
	return Vector3T<T>(-v1.x, -v1.y, -v1.z);
}

//Artihmetic operations
template<typename T>
inline Vector3T<T> operator + (Vector3T<T> const &v1, Vector3T<T> const &v2) {
	return Vector3T<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

template<typename T>
inline Vector3T<T> operator - (Vector3T<T> const &v1, Vector3T<T> const &v2) {
	return Vector3T<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

template<typename T>
inline Vector3T<T> operator * (Vector3T<T> const &v1, Vector3T<T> const &v2) {
	return Vector3T<T>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

template<typename T>
inline Vector3T<T> operator / (Vector3T<T> const &v1, Vector3T<T> const &v2) {
	return Vector3T<T>(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}

template<typename T>
inline Vector3T<T> operator + (T c, Vector3T<T> const &v1) {
	return Vector3T<T>(c + v1.x, c + v1.y, c + v1.z);
}

template<typename T>
inline Vector3T<T> operator + (Vector3T<T> const &v1, T c) {
	return Vector3T<T>(v1.x + c, v1.y + c, v1.z + c);
}

template<typename T>
inline Vector3T<T> operator - (T c, Vector3T<T> const &v1) {
	return Vector3T<T>(c - v1.x, c - v1.y, c - v1.z);
}

template<typename T>
inline Vector3T<T> operator - (Vector3T<T> const &v1, T c) {
	return Vector3T<T>(v1.x - c, v1.y - c, v1.z - c);
}

template<typename T>
inline Vector3T<T> operator * (T c, Vector3T<T> const &v1) {
	return Vector3T<T>(c * v1.x, c * v1.y, c * v1.z);
}

template<typename T>
inline Vector3T<T> operator * (Vector3T<T> const &v1, T c) {
	return Vector3T<T>(v1.x * c, v1.y * c, v1.z * c);
}


template<typename T>
inline Vector3T<T> operator / (T c, Vector3T<T> const &v1) {
	return Vector3T<T>(c / v1.x, c / v1.y, c / v1.z);
}

template<typename T>
inline Vector3T<T> operator / (Vector3T<T> const &v1, T c) {
	return Vector3T<T>(v1.x / c, v1.y / c, v1.z / c);
}

//Equality checks
template<typename T>
inline Vector3T<bool> operator == (Vector3T<T> const &v1, Vector3T<T> const &v2) {
	return Vector3T<bool>(v1.x == v2.x, v1.y == v2.y, v1.z == v2.z);
}

template<typename T>
inline Vector3T<bool> operator != (Vector3T<T> const &v1, Vector3T<T> const &v2) {
	return Vector3T<bool>(v1.x != v2.x, v1.y != v2.y, v1.z != v2.z);
}

//Comparision checks
template<typename T>
inline Vector3T<bool> operator < (Vector3T<T> const &v1, Vector3T<T> const &v2) {
	return Vector3T<bool>(v1.x < v2.x, v1.y < v2.y, v1.z < v2.z);
}

template<typename T>
inline Vector3T<bool> operator <= (Vector3T<T> const &v1, Vector3T<T> const &v2) {
	return Vector3T<bool>(v1.x <= v2.x, v1.y <= v2.y, v1.z <= v2.z);
}

template<typename T>
inline Vector3T<bool> operator > (Vector3T<T> const &v1, Vector3T<T> const &v2) {
	return Vector3T<bool>(v1.x > v2.x, v1.y > v2.y, v1.z > v2.z);
}

template<typename T>
inline Vector3T<bool> operator >= (Vector3T<T> const &v1, Vector3T<T> const &v2) {
	return Vector3T<bool>(v1.x >= v2.x, v1.y >= v2.y, v1.z >= v2.z);
}



//---------
//Vec4 impl

template<typename Type>
inline constexpr Vector4T<Type>::Vector4T() {}
template<typename Type>
inline constexpr Vector4T<Type>::Vector4T(Type val) : x(val), y(val), z(val), w(val) {}
template<typename Type>
inline Vector4T<Type>::Vector4T(std::initializer_list<Type> list) {
	int i = 0;
	for(; i<list.size() && i<4; ++i)
		data[i] = list.begin()[i];
}
template<typename Type>
inline constexpr Vector4T<Type>::Vector4T(Type x, Type y, Type z, Type w) : x(x), y(y), z(z), w(w) {}
template<typename Type>
inline Vector4T<Type>::Vector4T(Type const * vIn) : x(vIn[0]), y(vIn[1]), z(vIn[2]), w(vIn[3]) {}
template<typename Type>
inline constexpr Vector4T<Type>::Vector4T(Vector3T<Type> const & vIn) : x(vIn.x), y(vIn.y), z(vIn.z), w(Type(1)) {}
template<typename Type>
inline constexpr Vector4T<Type>::Vector4T(Vector3T<Type> const & vIn, Type w) : x(vIn.x), y(vIn.y), z(vIn.z), w(w) {}
template<typename Type>
inline constexpr Vector4T<Type>::Vector4T(Type x, Vector3T<Type> const & vIn) : x(x), y(vIn.x), z(vIn.y), w(vIn.z) {}
template<typename Type>
inline constexpr Vector4T<Type>::Vector4T(Type x, Type y, Vector2T<Type> const & vIn) : x(x), y(y), z(vIn.x), w(vIn.y) {}
template<typename Type>
inline constexpr Vector4T<Type>::Vector4T(Type x, Vector2T<Type> const & vIn, Type w) : x(x), y(vIn.x), z(vIn.y), w(w) {}
template<typename Type>
inline constexpr Vector4T<Type>::Vector4T(Vector2T<Type> const & vIn, Type z, Type w) : x(vIn.x), y(vIn.y), z(z), w(w) {}
template<typename Type>
inline constexpr Vector4T<Type>::Vector4T(Vector2T<Type> const & vInXY, Vector2T<Type> const & vInZW) : xy(vInXY), zw(vInZW) {}

//Assignment
template<typename Type>
inline constexpr void Vector4T<Type>::Set(Type ix, Type iy, Type iz, Type iw) { x = ix; y = iy; z = iz; w = iw; }
template<typename Type>
inline void Vector4T<Type>::Set(Type const * vIn) { x = vIn[0]; y = vIn[1]; z = vIn[2]; w = vIn[3]; }
template<typename Type>
inline constexpr void Vector4T<Type>::Set(Vector4T const & vIn) { x = vIn.x; y = vIn.y; z = vIn.z; w = vIn.w; }
template<typename Type>
inline constexpr void Vector4T<Type>::Set(Vector3T<Type> const & vIn) { x = vIn.x; y = vIn.y; z = vIn.z; w = Type(1); }
template<typename Type>
inline constexpr void Vector4T<Type>::Set(Vector3T<Type> const & vIn, Type iw) { x = vIn.x; y = vIn.y; z = vIn.z; w = iw; }
template<typename Type>
inline constexpr void Vector4T<Type>::Set(Type ix, Vector3T<Type> const & vIn) { x = ix; y = vIn.x; z = vIn.y; w = vIn.z; }
template<typename Type>
inline constexpr void Vector4T<Type>::Set(Type ix, Type iy, Vector2T<Type> const & vIn) { x = ix; y = iy; z = vIn.x; w = vIn.y; }
template<typename Type>
inline constexpr void Vector4T<Type>::Set(Type ix, Vector2T<Type> const & vIn, Type iw) { x = ix; y = vIn.x; z = vIn.y; w = iw; }
template<typename Type>
inline constexpr void Vector4T<Type>::Set(Vector2T<Type> const & vIn, Type iz, Type iw) { x = vIn.x; y = vIn.y; z = iz; w = iw; }

//Information about the vector
template<typename Type>
inline constexpr unsigned int Vector4T<Type>::Size() const { return 4; }

//Bool checkers. Will not compile given some Types
template<typename Type>
inline bool Vector4T<Type>::Any() const {
	for(int i = 0; i < 4; ++i) {
		if(data[i]) {
			return true;
		}
	}

	return false;
}
template<typename Type>
inline bool Vector4T<Type>::All() const {
	for(int i = 0; i < 4; ++i) {
		if(!data[i]) {
			return false;
		}
	}

	return true;
}

template<typename Type>
inline constexpr uint32_t Vector4T<Type>::NumComps() { return 4; }

//Operators: Access
template<typename Type>
template<typename Index>
inline constexpr Type Vector4T<Type>::operator [] (Index i) const { return data[i]; }
template<typename Type>
template<typename Index>
inline constexpr Type& Vector4T<Type>::operator[] (Index i) { return data[i]; }



//Operators: Assignment
template<typename Type>
inline Vector4T<Type>& Vector4T<Type>::operator = (Vector4T<Type> const&) = default;
template<typename Type>
inline Vector4T<Type>& Vector4T<Type>::operator = (Vector4T<Type> &&) = default;

template<typename Type>
inline Vector4T<Type>& Vector4T<Type>::operator = (std::initializer_list<Type> list) {
	int i = 0;
	for(; i<list.size() && i<4; ++i)
		data[i] = list.begin()[i];
	for(; i < 4; ++i)
		data[i] = {};
	
	return *this;
}

template<typename Type>
inline Vector4T<Type>& Vector4T<Type>::operator = (Type const *vIn) { x = vIn[0]; y = vIn[1]; z = vIn[2]; w = vIn[3]; return *this; }
template<typename Type>
inline Vector4T<Type>& Vector4T<Type>::operator = (Vector3T<Type> const &vIn) { x = vIn.x; y = vIn.y; z = vIn.z; w = Type(1); return *this; }
template<typename Type>
inline Vector4T<Type>& Vector4T<Type>::operator = (QuaternionT<Type> const &q) { *this = q.ijks; return *this; }

template<typename Type>
inline constexpr Vector4T<Type>& Vector4T<Type>::operator += (Vector4T const &vIn) { x += vIn.x; y += vIn.y; z += vIn.z; w += vIn.w; return *this; }
template<typename Type>
inline constexpr Vector4T<Type>& Vector4T<Type>::operator -= (Vector4T const &vIn) { x -= vIn.x; y -= vIn.y; z -= vIn.z; w -= vIn.w; return *this; }
template<typename Type>
inline constexpr Vector4T<Type>& Vector4T<Type>::operator *= (Vector4T const &vIn) { x *= vIn.x; y *= vIn.y; z *= vIn.z; w *= vIn.w; return *this; }
template<typename Type>
inline constexpr Vector4T<Type>& Vector4T<Type>::operator *= (Type c) { x *= c; y *= c; z *= c; w *= c; return *this; }
template<typename Type>
inline constexpr Vector4T<Type>& Vector4T<Type>::operator /= (Vector4T const &vIn) { x /= vIn.x; y /= vIn.y; z /= vIn.z; w /= vIn.w; return *this; }
template<typename Type>
inline constexpr Vector4T<Type>& Vector4T<Type>::operator /= (Type c) { x /= c; y /= c; z /= c; w /= c; return *this; }

//Unary -
template<typename T, std::enable_if_t<std::is_unsigned<T>::value, int> = 0>
inline Vector4T<T> operator - (Vector4T<T> const &v1) {
	return v1;
}

template<typename T, std::enable_if_t<std::is_signed<T>::value, int> = 0>
inline Vector4T<T> operator - (Vector4T<T> const &v1) {
	return Vector4T<T>(-v1.x, -v1.y, -v1.z, -v1.w);
}

//Artihmetic operations
template<typename T>
inline Vector4T<T> operator + (Vector4T<T> const &v1, Vector4T<T> const &v2) {
	return Vector4T<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

template<typename T>
inline Vector4T<T> operator - (Vector4T<T> const &v1, Vector4T<T> const &v2) {
	return Vector4T<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

template<typename T>
inline Vector4T<T> operator * (Vector4T<T> const &v1, Vector4T<T> const &v2) {
	return Vector4T<T>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

template<typename T>
inline Vector4T<T> operator / (Vector4T<T> const &v1, Vector4T<T> const &v2) {
	return Vector4T<T>(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
}

template<typename T>
inline Vector4T<T> operator + (T c, Vector4T<T> const &v1) {
	return Vector4T<T>(c + v1.x, c + v1.y, c + v1.z, c + v1.w);
}

template<typename T>
inline Vector4T<T> operator + (Vector4T<T> const &v1, T c) {
	return Vector4T<T>(v1.x + c, v1.y + c, v1.z + c, v1.w + c);
}

template<typename T>
inline Vector4T<T> operator - (T c, Vector4T<T> const &v1) {
	return Vector4T<T>(c - v1.x, c - v1.y, c - v1.z, c - v1.w);
}

template<typename T>
inline Vector4T<T> operator - (Vector4T<T> const &v1, T c) {
	return Vector4T<T>(v1.x - c, v1.y - c, v1.z - c, v1.w - c);
}

template<typename T>
inline Vector4T<T> operator * (T c, Vector4T<T> const &v1) {
	return Vector4T<T>(c * v1.x, c * v1.y, c * v1.z, c * v1.w);
}

template<typename T>
inline Vector4T<T> operator * (Vector4T<T> const &v1, T c) {
	return Vector4T<T>(v1.x * c, v1.y * c, v1.z * c, v1.w * c);
}


template<typename T>
inline Vector4T<T> operator / (T c, Vector4T<T> const &v1) {
	return Vector4T<T>(c / v1.x, c / v1.y, c / v1.z, c / v1.w);
}

template<typename T>
inline Vector4T<T> operator / (Vector4T<T> const &v1, T c) {
	return Vector4T<T>(v1.x / c, v1.y / c, v1.z / c, v1.w / c);
}

//Equality checks
template<typename T>
inline Vector4T<bool> operator == (Vector4T<T> const &v1, Vector4T<T> const &v2) {
	return Vector4T<bool>(v1.x == v2.x, v1.y == v2.y, v1.z == v2.z, v1.w == v2.w);
}

template<typename T>
inline Vector4T<bool> operator != (Vector4T<T> const &v1, Vector4T<T> const &v2) {
	return Vector4T<bool>(v1.x != v2.x, v1.y != v2.y, v1.z != v2.z, v1.w != v2.w);
}

//Comparision checks
template<typename T>
inline Vector4T<bool> operator < (Vector4T<T> const &v1, Vector4T<T> const &v2) {
	return Vector4T<bool>(v1.x < v2.x, v1.y < v2.y, v1.z < v2.z, v1.w < v2.w);
}

template<typename T>
inline Vector4T<bool> operator <= (Vector4T<T> const &v1, Vector4T<T> const &v2) {
	return Vector4T<bool>(v1.x <= v2.x, v1.y <= v2.y, v1.z <= v2.z, v1.w <= v2.w);
}

template<typename T>
inline Vector4T<bool> operator > (Vector4T<T> const &v1, Vector4T<T> const &v2) {
	return Vector4T<bool>(v1.x > v2.x, v1.y > v2.y, v1.z > v2.z, v1.w > v2.w);
}

template<typename T>
inline Vector4T<bool> operator >= (Vector4T<T> const &v1, Vector4T<T> const &v2) {
	return Vector4T<bool>(v1.x >= v2.x, v1.y >= v2.y, v1.z >= v2.z, v1.w >= v2.w);
}


//---------
//Mat4 impl

template<typename Type>
inline Matrix4x4T<Type>::Matrix4x4T() 
	: X(1,0,0,0), Y(0,1,0,0), Z(0,0,1,0), W(0,0,0,1) {
}
template<typename Type>
inline Matrix4x4T<Type>::Matrix4x4T(Type Xx, Type Xy, Type Xz, Type Xw, Type Yx, Type Yy, Type Yz, Type Yw, Type Zx, Type Zy, Type Zz, Type Zw, Type Wx, Type Wy, Type Wz, Type Ww, bool transpose) {
	Set(Xx, Xy, Xz, Xw, Yx, Yy, Yz, Yw, Zx, Zy, Zz, Zw, Wx, Wy, Wz, Ww, transpose);
}
template<typename Type>
inline Matrix4x4T<Type>::Matrix4x4T(Type const * mIn) {
	std::memcpy(data, mIn, sizeof(data));
}
template<typename Type>
inline Matrix4x4T<Type>::Matrix4x4T(Vector4T<Type> const & X, Vector4T<Type> const & Y, Vector4T<Type> const & Z, Vector4T<Type> const & W) 
	: X(X), Y(Y), Z(Z), W(W) {
}
template<typename Type>
inline Matrix4x4T<Type>::Matrix4x4T(Vector3T<Type> const & X, Vector3T<Type> const & Y, Vector3T<Type> const & Z)
	: X(X), Y(Y), Z(Z), W(0,0,0,1) {

}

template<typename Type>
inline Matrix4x4T<Type>::Matrix4x4T(Matrix3x3T<Type> const & m) {
	*this = m;
}

template<typename Type>
inline Matrix4x4T<Type>::Matrix4x4T(QuaternionT<Type> const & q) {
	*this = q;
	X.w = 0;
	Y.w = 0;
	Z.w = 0;
	W.Set(0, 0, 0, 1);
}

template<typename Type>
inline Matrix4x4T<Type>::Matrix4x4T(QuaternionT<Type> const & q, Vector3T<Type> const & position) {
	Set(q, position);
}

template<typename Type>
inline Matrix4x4T<Type>::Matrix4x4T(QuaternionT<Type> const & q, Vector3T<Type> const & position, Vector3T<Type> const & scale) {
	Set(q, position, scale);
}

template<typename Type>
inline void Matrix4x4T<Type>::Identity() {
	X.Set(1, 0, 0, 0);
	Y.Set(0, 1, 0, 0);
	Z.Set(0, 0, 1, 0);
	W.Set(0, 0, 0, 1);
}

template<typename Type>
inline void Matrix4x4T<Type>::Set(Type Xx, Type Xy, Type Xz, Type Xw, Type Yx, Type Yy, Type Yz, Type Yw, Type Zx, Type Zy, Type Zz, Type Zw, Type Wx, Type Wy, Type Wz, Type Ww, bool transpose) {

	//Set matrix on column major (X, Y, Z, W)
	if(!transpose) {
		data[0] = Xx; data[4] = Yx; data[8] = Zx; data[12] = Wx;
		data[1] = Xy; data[5] = Yy; data[9] = Zy; data[13] = Wy;
		data[2] = Xz; data[6] = Yz; data[10] = Zz; data[14] = Wz;
		data[3] = Xw; data[7] = Yw; data[11] = Zw; data[15] = Ww;
	}
	else {
		//unless they asked us to transpose it
		data[0] = Xx; data[4] = Xy; data[8] = Xz; data[12] = Xw;
		data[1] = Yx; data[5] = Yy; data[9] = Yz; data[13] = Yw;
		data[2] = Zx; data[6] = Zy; data[10] = Zz; data[14] = Zw;
		data[3] = Wx; data[7] = Wy; data[11] = Wz; data[15] = Ww;
	}

}
template<typename Type>
inline void Matrix4x4T<Type>::Set(QuaternionT<Type> const & q, Vector3T<Type> const & pos) {
	*this = q;
	W.Set(pos);
	this->SetRow(3, Vector4T<Type>(0, 0, 0, 1));
}

template<typename Type>
inline void Matrix4x4T<Type>::Set(QuaternionT<Type> const & q, Vector3T<Type> const & pos, Vector3T<Type> const &scale) {
	*this = q;
	Scale(scale);
	W.Set(pos);
	this->SetRow(3, Vector4T<Type>(0, 0, 0, 1));
}

//Column and row setters
template<typename Type>
inline void Matrix4x4T<Type>::SetRows(Vector3T<Type> const & a, Vector3T<Type> const & b, Vector3T<Type> const & c) {
	//Assign as row major
	data[0] = a[0]; data[4] = a[1]; data[8] = a[2]; data[12] = 0;
	data[1] = b[0]; data[5] = b[1]; data[9] = b[2]; data[13] = 0;
	data[2] = c[0]; data[6] = c[1]; data[10] = c[2]; data[14] = 0;
}
template<typename Type>
inline void Matrix4x4T<Type>::SetRows(Vector3T<Type> const & a, Vector3T<Type> const & b, Vector3T<Type> const & c, Vector3T<Type> const & d) {
	//Assign as row major
	data[0] = a[0]; data[4] = a[1]; data[8] = a[2]; data[12] = 0;
	data[1] = b[0]; data[5] = b[1]; data[9] = b[2]; data[13] = 0;
	data[2] = c[0]; data[6] = c[1]; data[10] = c[2]; data[14] = 0;
	data[3] = d[0]; data[7] = d[1]; data[11] = d[2]; data[15] = 1;
}
template<typename Type>
inline void Matrix4x4T<Type>::SetRows(Vector4T<Type> const & a, Vector4T<Type> const & b, Vector4T<Type> const & c, Vector4T<Type> const & d) {
	//Assign as row major
	data[0] = a[0]; data[4] = a[1]; data[8] = a[2]; data[12] = a[3];
	data[1] = b[0]; data[5] = b[1]; data[9] = b[2]; data[13] = b[3];
	data[2] = c[0]; data[6] = c[1]; data[10] = c[2]; data[14] = c[3];
	data[3] = d[0]; data[7] = d[1]; data[11] = d[2]; data[15] = d[3];
}
template<typename Type>
template<typename Index>
inline void Matrix4x4T<Type>::SetRow(Index num, Vector3T<Type> const & a) {
	data[0 + num] = a[0]; data[4 + num] = a[1]; data[8 + num] = a[2];
}
template<typename Type>
template<typename Index>
inline void Matrix4x4T<Type>::SetRow(Index num, Vector4T<Type> const & a) {
	data[0 + num] = a[0]; data[4 + num] = a[1]; data[8 + num] = a[2]; data[12 + num] = a[3];
}

//Sets the rotational part of the matrix
template<typename Type>
inline void Matrix4x4T<Type>::SetNormalTransform(Matrix3x3T<Type> const & inmatrix) {
	//Assign as column major (X,Y,Z,W)
	X.xyz = inmatrix.X;
	Y.xyz = inmatrix.Y;
	Z.xyz = inmatrix.Z;
}

//Sets the rotational part of the matrix
template<typename Type>
inline void Matrix4x4T<Type>::SetNormalTransform(Matrix4x4T const & inmatrix) {
	//Assign as column major (X,Y,Z)
	X.xyz = inmatrix.X.xyz;
	Y.xyz = inmatrix.Y.xyz;
	Z.xyz = inmatrix.Z.xyz;
}

//Removes the translation part of the matrix
template<typename Type>
inline void Matrix4x4T<Type>::MakeNormalTransform() {
	data[3] = data[7] = data[11] = 0.0f;
	W.Set(0, 0, 0, 1);
}

//Returns a 3x3 matrix with the rotational part 
template<typename Type>
inline Matrix3x3T<Type> Matrix4x4T<Type>::GetNormalTransform() const {
	return Matrix3x3T<Type>{X.xyz, Y.xyz, Z.xyz};
}
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::GetNormalTransform4x4() const {
	return Matrix4x4T{X.xyz, Y.xyz, Z.xyz};
}

//Returns the scale applied to the matrix as a vector3.
template<typename Type>
inline Vector3T<Type> Matrix4x4T<Type>::GetScalingFactor() const {
	//The scaling factor for each vector is found as the magnitud of each column on the 3x3 portion of the matrix
	return Vector3T<Type>(
		weave::algebra::length(X.xyz),
		weave::algebra::length(Y.xyz),
		weave::algebra::length(Z.xyz)
		);
}

//Removes the scaling factor from the matrix by normalizing the axes and returns the removed scaling factor
template<typename Type>
inline Vector3T<Type> Matrix4x4T<Type>::RemoveScalingFactor() {
	auto factor = GetScalingFactor();
	X /= factor.x;
	Y /= factor.y;
	Z /= factor.z;
		
	return factor;
}

//Row accessors
template<typename Type>
inline void Matrix4x4T<Type>::GetRows(Vector3T<Type>& a, Vector3T<Type>& b, Vector3T<Type>& c) const {
	a[0] = data[0]; a[1] = data[4]; a[2] = data[8];
	b[0] = data[1]; b[1] = data[5]; b[2] = data[9];
	c[0] = data[2]; c[1] = data[6]; c[2] = data[10];
}
template<typename Type>
inline void Matrix4x4T<Type>::GetRows(Vector4T<Type>& a, Vector4T<Type>& b, Vector4T<Type>& c, Vector4T<Type>& d) const {
	a[0] = data[0]; a[1] = data[4]; a[2] = data[8]; a[3] = data[12];
	b[0] = data[1]; b[1] = data[5]; b[2] = data[9]; b[3] = data[13];
	c[0] = data[2]; c[1] = data[6]; c[2] = data[10]; c[3] = data[14];
	d[0] = data[3]; d[1] = data[7]; d[2] = data[11]; d[3] = data[15];
}
template<typename Type>
template<typename Index>
inline Vector4T<Type> Matrix4x4T<Type>::GetRow(Index num) const {
	return Vector4T<Type>(data[0 + num], data[4 + num], data[8 + num], data[12 + num]);
}

//Returns the diagonal for the matrix
template<typename Type>
inline Vector4T<Type> Matrix4x4T<Type>::GetDiagonal() const {
	return Vector4T<Type>(data[0], data[5], data[10], data[15]);
}

//Matrix operations
template<typename Type>
inline Vector3T<Type> Matrix4x4T<Type>::TransformNormal(Vector3T<Type> const & n) const {
	return Vector3T<Type>(
		data[0] * n[0] + data[4] * n[1] + data[8] * n[2],
		data[1] * n[0] + data[5] * n[1] + data[9] * n[2],
		data[2] * n[0] + data[6] * n[1] + data[10] * n[2]);
}

//Returns the transposed matrix
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::Transposed() const {
	return Matrix4x4T(
		data[0], data[1], data[2], data[3],
		data[4], data[5], data[6], data[7],
		data[8], data[9], data[10], data[11],
		data[12], data[13], data[14], data[15],
		true
	);
}

//Transposes the matrix
template<typename Type>
inline void Matrix4x4T<Type>::Transpose() {
	*this = Matrix4x4T(
		data[0], data[1], data[2], data[3],
		data[4], data[5], data[6], data[7],
		data[8], data[9], data[10], data[11],
		data[12], data[13], data[14], data[15],
		true
	);
}

//Returns the inverse of the matrix
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::Inverted() const {
	auto ret = *this;
	ret.Invert();
	return ret;
}

//Inverts the matrix
template<typename Type>
inline int Matrix4x4T<Type>::Invert() {
	Type fA0 = data[0] * data[5] - data[1] * data[4];
	Type fA1 = data[0] * data[6] - data[2] * data[4];
	Type fA2 = data[0] * data[7] - data[3] * data[4];
	Type fA3 = data[1] * data[6] - data[2] * data[5];
	Type fA4 = data[1] * data[7] - data[3] * data[5];
	Type fA5 = data[2] * data[7] - data[3] * data[6];
	Type fB0 = data[8] * data[13] - data[9] * data[12];
	Type fB1 = data[8] * data[14] - data[10] * data[12];
	Type fB2 = data[8] * data[15] - data[11] * data[12];
	Type fB3 = data[9] * data[14] - data[10] * data[13];
	Type fB4 = data[9] * data[15] - data[11] * data[13];
	Type fB5 = data[10] * data[15] - data[11] * data[14];

	Type fDet = fA0*fB5 - fA1*fB4 + fA2*fB3 + fA3*fB2 - fA4*fB1 + fA5*fB0;
	if(std::abs(fDet) <= 0.0f)
		return 0;

	Matrix4x4T kInv;
	kInv(0, 0) = +data[5] * fB5 - data[6] * fB4 + data[7] * fB3;
	kInv(0, 1) = -data[4] * fB5 + data[6] * fB2 - data[7] * fB1;
	kInv(0, 2) = +data[4] * fB4 - data[5] * fB2 + data[7] * fB0;
	kInv(0, 3) = -data[4] * fB3 + data[5] * fB1 - data[6] * fB0;
	kInv(1, 0) = -data[1] * fB5 + data[2] * fB4 - data[3] * fB3;
	kInv(1, 1) = +data[0] * fB5 - data[2] * fB2 + data[3] * fB1;
	kInv(1, 2) = -data[0] * fB4 + data[1] * fB2 - data[3] * fB0;
	kInv(1, 3) = +data[0] * fB3 - data[1] * fB1 + data[2] * fB0;
	kInv(2, 0) = +data[13] * fA5 - data[14] * fA4 + data[15] * fA3;
	kInv(2, 1) = -data[12] * fA5 + data[14] * fA2 - data[15] * fA1;
	kInv(2, 2) = +data[12] * fA4 - data[13] * fA2 + data[15] * fA0;
	kInv(2, 3) = -data[12] * fA3 + data[13] * fA1 - data[14] * fA0;
	kInv(3, 0) = -data[9] * fA5 + data[10] * fA4 - data[11] * fA3;
	kInv(3, 1) = +data[8] * fA5 - data[10] * fA2 + data[11] * fA1;
	kInv(3, 2) = -data[8] * fA4 + data[9] * fA2 - data[11] * fA0;
	kInv(3, 3) = +data[8] * fA3 - data[9] * fA1 + data[10] * fA0;


	Type fInvDet = 1.0f / fDet;

	*this = kInv * fInvDet;

	return 1;
}

//Inverts the matrix assuming the matrix is orthogonal
template<typename Type>
inline void Matrix4x4T<Type>::InvertOrtho() {
	//This operation is only a valid inverse if the matrix is othogonal
	Type res[16];
	//Transpose the rotational part of the matrix
	res[0] = data[0];
	res[1] = data[4];
	res[2] = data[8];
	res[3] = 0.0;

	res[4] = data[1];
	res[5] = data[5];
	res[6] = data[9];
	res[7] = 0.0;

	res[8] = data[2];
	res[9] = data[6];
	res[10] = data[10];
	res[11] = 0.0;

	//Apply the translation negated (inverted)
	res[12] = -data[12] * res[0] - data[13] * res[4] - data[14] * res[8];
	res[13] = -data[12] * res[1] - data[13] * res[5] - data[14] * res[9];
	res[14] = -data[12] * res[2] - data[13] * res[6] - data[14] * res[10];
	res[15] = 1.0;

	std::memcpy(data, res, sizeof(data));
}

//Returns the inverse of the matrix assuming the matrix is orthogonal
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::InvertedOrtho() const {
	auto m = *this;
	m.InvertOrtho();
	return m;
}

//Returns the matrix trace
template<typename Type>
inline Type Matrix4x4T<Type>::Trace() const { return data[0] + data[5] + data[10] + data[15]; }

//Returns the determinant
template<typename Type>
inline Type Matrix4x4T<Type>::Determinant() const {
	Type fA0 = data[0] * data[5] - data[1] * data[4];
	Type fA1 = data[0] * data[6] - data[2] * data[4];
	Type fA2 = data[0] * data[7] - data[3] * data[4];
	Type fA3 = data[1] * data[6] - data[2] * data[5];
	Type fA4 = data[1] * data[7] - data[3] * data[5];
	Type fA5 = data[2] * data[7] - data[3] * data[6];
	Type fB0 = data[8] * data[13] - data[9] * data[12];
	Type fB1 = data[8] * data[14] - data[10] * data[12];
	Type fB2 = data[8] * data[15] - data[11] * data[12];
	Type fB3 = data[9] * data[14] - data[10] * data[13];
	Type fB4 = data[9] * data[15] - data[11] * data[13];
	Type fB5 = data[10] * data[15] - data[11] * data[14];

	Type fDet = fA0*fB5 - fA1*fB4 + fA2*fB3 + fA3*fB2 - fA4*fB1 + fA5*fB0;
	return fDet;
}

//Weights multiple matrices with an array of indices and weights
template<typename Type>
inline void Matrix4x4T<Type>::Weight(Matrix4x4T * ms, int const * indices, Type const * weights, unsigned int count) {
	Type *wmatrix = ms[indices[0]].m;
	//Initialize to the first weighted matrix
	for(unsigned int j = 0; j<16; ++j) {
		this->data[j] = wmatrix[j] * weights[0];
	}
	//Weight the rest
	for(unsigned int i = 1; i<count; ++i) {
		wmatrix = ms[indices[i]].m;
		for(unsigned int j = 0; j<16; ++j) {
			this->data[j] += wmatrix[j] * weights[i];
		}
	}
}

//Applies a translation transformation to the current matrix
template<typename Type>
inline void Matrix4x4T<Type>::Translate(Type x, Type y, Type z) {
	//Translation matrix
	data[12] += data[0] * x + data[4] * y + data[8] * z;
	data[13] += data[1] * x + data[5] * y + data[9] * z;
	data[14] += data[2] * x + data[6] * y + data[10] * z;
}
template<typename Type>
inline void Matrix4x4T<Type>::Translate(weave::Vector3T<Type> const & vec) { Translate(vec.x, vec.y, vec.z); }
template<typename Type>
inline void Matrix4x4T<Type>::Translate(weave::Vector4T<Type> const & vec) { Translate(vec.x, vec.y, vec.z); }

//Applies a scaling transformation to the current matrix
template<typename Type>
inline void Matrix4x4T<Type>::Scale(Type x, Type y, Type z) {
	//Sets the scaling coeficients on X, Y and Z vectors
	data[0] *= x;
	data[1] *= x;
	data[2] *= x;

	data[4] *= y;
	data[5] *= y;
	data[6] *= y;
	
	data[8] *= z;
	data[9] *= z;
	data[10] *= z;
}
template<typename Type>
inline void Matrix4x4T<Type>::Scale(weave::Vector3T<Type> const & vec) { Scale(vec.x, vec.y, vec.z); }
template<typename Type>
inline void Matrix4x4T<Type>::Scale(weave::Vector4T<Type> const & vec) { Scale(vec.x, vec.y, vec.z); }

//Applies a rotation transformation to the current matrix. Angles are in radians
template<typename Type>
inline void Matrix4x4T<Type>::Rotate(Type radians, Type x, Type y, Type z) {
	*this *= (RotationCtor(radians, x, y, z));
}
template<typename Type>
inline void Matrix4x4T<Type>::Rotate(Type radians, weave::Vector3T<Type> const & vec) { Rotate(radians, vec.x, vec.y, vec.z); }

//Applies a rotation transformation to the current matrix using euler angles, in radians
template<typename Type>
inline void Matrix4x4T<Type>::Rotate(Type pitch, Type yaw, Type roll) {
	Type A = std::cos(pitch);
	Type B = std::sin(pitch);
	Type C = std::cos(yaw);
	Type D = std::sin(yaw);
	Type E = std::cos(roll);
	Type F = std::sin(roll);
	Type AD = A * D;
	Type BD = B * D;

	*this *= Matrix4x4T(
		C*E, -C*F, D, 0,
		BD*E + A*F, -BD*F + A*E, -B*C, 0,
		-AD*E + B*F, AD*F + B*E, A*C, 0,
		0, 0, 0, 1,
		true
	);
}
template<typename Type>
inline void Matrix4x4T<Type>::Rotate(weave::Vector3T<Type> const & pitchYawRoll) { Rotate(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z); }

//Applies a reflection matrix to the current matrix given a reflection plane
template<typename Type>
inline void Matrix4x4T<Type>::Reflect(Plane const & plane) {
	*this *= (ReflectionCtor(plane));
}

//Creates a translation matrix. Replaces the current matrix.
template<typename Type>
inline void Matrix4x4T<Type>::Translation(Type x, Type y, Type z) {
	//Translation matrix sets the W vector. 
	//Note that the matrix is in column major (X,Y,Z,W).
	X.Set(1, 0, 0, 0);
	Y.Set(0, 1, 0, 0);
	Z.Set(0, 0, 1, 0);
	W.Set(x, y, z, 1);
}
template<typename Type>
inline void Matrix4x4T<Type>::Translation(weave::Vector3T<Type> const & vec) { Translation(vec.x, vec.y, vec.z); }
template<typename Type>
inline void Matrix4x4T<Type>::Translation(weave::Vector4T<Type> const & vec) { Translation(vec.x, vec.y, vec.z); }

//Creates a scaling matrix. Replaces the current matrix.
template<typename Type>
inline void Matrix4x4T<Type>::Scaling(Type x, Type y, Type z) {
	X.Set(x, 0, 0, 0);
	Y.Set(0, y, 0, 0);
	Z.Set(0, 0, z, 0);
	W.Set(0, 0, 0, 1);
}
template<typename Type>
inline void Matrix4x4T<Type>::Scaling(weave::Vector3T<Type> const & vec) { Scaling(vec.x, vec.y, vec.z); }
template<typename Type>
inline void Matrix4x4T<Type>::Scaling(weave::Vector4T<Type> const & vec) { Scaling(vec.x, vec.y, vec.z); }

//Creates a rotational matrix. Replaces the current matrix. Angles are in radians.
template<typename Type>
inline void Matrix4x4T<Type>::Rotation(Type radians, Type x, Type y, Type z) {
	Type mag = std::sqrt(x*x + y*y + z*z);
	x /= mag; y /= mag; z /= mag;
	Type c = std::cos(radians);
	Type s = std::sin(radians);
	Type umc = 1 - c;

	Set(
		x*x*umc + c, x*y*umc - z*s, x*z*umc + y*s, 0,
		y*x*umc + z*s, y*y*umc + c, y*z*umc - x*s, 0,
		z*x*umc - y*s, z*y*umc + x*s, z*z*umc + c, 0,
		0, 0, 0, 1,
		true //Indicates the matrix is being supplied in "paper written" form instead of each vector linearly
	);
}
template<typename Type>
inline void Matrix4x4T<Type>::Rotation(Type radians, weave::Vector3T<Type> const & vec) { Rotation(radians, vec.x, vec.y, vec.z); }

//Creates a rotational matrix using euler angles. Replaces the current matrix. Angles are in radians.
template<typename Type>
inline void Matrix4x4T<Type>::Rotation(Type pitch, Type yaw, Type roll) {
	Type A = std::cos(pitch);
	Type B = std::sin(pitch);
	Type C = std::cos(yaw);
	Type D = std::sin(yaw);
	Type E = std::cos(roll);
	Type F = std::sin(roll);
	Type AD = A * D;
	Type BD = B * D;

	Set(
		C*E, -C*F, D, 0,
		BD*E + A*F, -BD*F + A*E, -B*C, 0,
		-AD*E + B*F, AD*F + B*E, A*C, 0,
		0, 0, 0, 1,
		true
	);
}
template<typename Type>
inline void Matrix4x4T<Type>::Rotation(weave::Vector3T<Type> const & pitchYawRoll) { Rotation(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z); }

//Creates a reflection matrix given a reflection plane
template<typename Type>
inline void Matrix4x4T<Type>::Reflection(Plane const & plane) {
	Set(
		1.0f - 2.0f*plane.x*plane.x, -2.0f*plane.x*plane.y, -2.0f*plane.x*plane.z, -2.0f*plane.x*plane.w,
		-2.0f*plane.y*plane.x, 1.0f - 2.0f*plane.y*plane.y, -2.0f*plane.y*plane.z, -2.0f*plane.y*plane.w,
		-2.0f*plane.z*plane.x, -2.0f*plane.z*plane.y, 1.0f - 2.0f*plane.z*plane.z, -2.0f*plane.z*plane.w,
		0, 0, 0, 1, true
	);
}

//Loads an orthographic projection matrix
template<typename Type>
inline void Matrix4x4T<Type>::Ortho(Type left, Type right, Type bottom, Type top, Type zNear, Type zFar) {
	*this = (OrthoCtor(left, right, bottom, top, zNear, zFar));
}

//Loads a perspective matrix. Angle is supplied in degrees.
template<typename Type>
inline void Matrix4x4T<Type>::PerspectiveDeg(Type fovyDeg, Type ratio, Type zNear, Type zFar) {
	*this = (PerspectiveDegCtor(fovyDeg, ratio, zNear, zFar));
}

//Loads a perspective matrix. Angle is supplied in radians.
template<typename Type>
inline void Matrix4x4T<Type>::PerspectiveRad(Type fovyRad, Type ratio, Type zNear, Type zFar) {
	*this = (PerspectiveRadCtor(fovyRad, ratio, zNear, zFar));
}

//Loads a perspective matrix in right hand zero convention. Angle is supplied in degrees.
template<typename Type>
inline void Matrix4x4T<Type>::PerspectiveRhzDeg(Type fovyDeg, Type ratio, Type zNear, Type zFar) {
	*this = (PerspectiveRhzDegCtor(fovyDeg, ratio, zNear, zFar));
}

//Loads a perspective matrix in right hand zero convention. Angle is supplied in radians.
template<typename Type>
inline void Matrix4x4T<Type>::PerspectiveRhzRad(Type fovyRad, Type ratio, Type zNear, Type zFar) {
	*this = (PerspectiveRhzRadCtor(fovyRad, ratio, zNear, zFar));
}

//Applies a look at transformation to the matrix with legacy opengl style
template<typename Type>
inline void Matrix4x4T<Type>::LookAt(Type eyex, Type eyey, Type eyez, Type centerx, Type centery, Type centerz, Type upx, Type upy, Type upz) {
	using namespace algebra;
	Vector3T<Type> f = normalize(Vector3T<Type>(centerx - eyex, centery - eyey, centerz - eyez));
	Vector3T<Type> up = normalize(Vector3T<Type>(upx, upy, upz));

	Vector3T<Type> s = normalize(cross(f, up));
	Vector3T<Type> u = normalize(cross(s, f));

	//What follows is a fast way of retrieving what would be the inverse of the coordinate system defined by u,s,f and the translation.
	//The inverse is needed to transform a point from world space into the space defined by the view

	//The transpose of the coordinate system is the inverse of the rotational part of the matrix
	*this *= (Matrix4x4T<Type>(
		s[0], s[1], s[2], 0.0f,
		u[0], u[1], u[2], 0.0f,
		-f[0], -f[1], -f[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		true //Matrix is supplied in paper-written form (so columns look like columns, not transposed)
		));
	Translate(-eyex, -eyey, -eyez);
}

//Static Ctors, used to build specific matrices on the fly
//Constructs a translation matrix and returns it
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::TranslationCtor(Type x, Type y, Type z) {
	//Translation matrix sets the W vector. 
	//Note that the matrix is in column major (X,Y,Z,W).
	//To make the code asignment look like a paper written matrix, the last parameter flag as "true" makes sure the asigment is done correctly in memory
	return Matrix4x4T(
		1, 0, 0, x,
		0, 1, 0, y,
		0, 0, 1, z,
		0, 0, 0, 1,
		true
	);
}
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::TranslationCtor(weave::Vector3T<Type> const & vec) { return TranslationCtor(vec.x, vec.y, vec.z); }
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::TranslationCtor(weave::Vector4T<Type> const & vec) { return TranslationCtor(vec.x, vec.y, vec.z); }

//Constructs a scaling matrix and returns it
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::ScalingCtor(Type x, Type y, Type z) {
	//Sets the scaling coeficients on X, Y and Z vectors
	return Matrix4x4T(
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1
	);
}
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::ScalingCtor(weave::Vector3T<Type> const & vec) { return ScalingCtor(vec.x, vec.y, vec.z); }
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::ScalingCtor(weave::Vector4T<Type> const & vec) { return ScalingCtor(vec.x, vec.y, vec.z); }

//Constructs a rotational matrix and returns it. Angles are in radians.
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::RotationCtor(Type radians, Type x, Type y, Type z) {
	Type mag = std::sqrt(x*x + y*y + z*z);
	x /= mag; y /= mag; z /= mag;
	Type c = std::cos(radians);
	Type s = std::sin(radians);
	Type umc = 1 - c;

	return Matrix4x4T(
		x*x*umc + c, x*y*umc - z*s, x*z*umc + y*s, 0,
		y*x*umc + z*s, y*y*umc + c, y*z*umc - x*s, 0,
		z*x*umc - y*s, z*y*umc + x*s, z*z*umc + c, 0,
		0, 0, 0, 1,
		true //Indicates the matrix is being supplied in "paper written" form instead of each vector linearly
	);

}
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::RotationCtor(Type radians, weave::Vector3T<Type> const & vec) { return RotationCtor(radians, vec.x, vec.y, vec.z); }

//Constructs a rotational matrix using euler angles and returns it. Angles are in radians.
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::RotationCtor(Type pitch, Type yaw, Type roll) {
	Type A = std::cos(pitch);
	Type B = std::sin(pitch);
	Type C = std::cos(yaw);
	Type D = std::sin(yaw);
	Type E = std::cos(roll);
	Type F = std::sin(roll);
	Type AD = A * D;
	Type BD = B * D;

	return Matrix4x4T(
		C*E, -C*F, D, 0,
		BD*E + A*F, -BD*F + A*E, -B*C, 0,
		-AD*E + B*F, AD*F + B*E, A*C, 0,
		0, 0, 0, 1,
		true
	);
}
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::RotationCtor(weave::Vector3T<Type> const & pitchYawRoll) { return RotationCtor(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z); }

//Constructs a reflection matrix given a reflection plane
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::ReflectionCtor(Plane const & plane) {
	return Matrix4x4T(
		1.0f - 2.0f*plane.x*plane.x, -2.0f*plane.x*plane.y, -2.0f*plane.x*plane.z, -2.0f*plane.x*plane.w,
		-2.0f*plane.y*plane.x, 1.0f - 2.0f*plane.y*plane.y, -2.0f*plane.y*plane.z, -2.0f*plane.y*plane.w,
		-2.0f*plane.z*plane.x, -2.0f*plane.z*plane.y, 1.0f - 2.0f*plane.z*plane.z, -2.0f*plane.z*plane.w,
		0, 0, 0, 1, true
	);
}

//Constructs an orthographic projection matrix
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::OrthoCtor(Type left, Type right, Type bottom, Type top, Type zNear, Type zFar) {
	return Matrix4x4T<Type>(
		(Type(2.0) / (right - left)), 0, 0, (-(right + left) / (right - left)),
		0, (Type(2.0) / (top - bottom)), 0, (-(top + bottom) / (top - bottom)),
		0, 0, (Type(-2.0) / (zFar - zNear)),(-(zFar + zNear) / (zFar - zNear)),
		0, 0, 0, 1,
		true//Matrix is supplied in paper-written form (so columns look like columns, not transposed)
	);
}

//Constructs a perspective matrix. Angle is supplied in degrees.
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::PerspectiveDegCtor(Type fovyDeg, Type ratio, Type zNear, Type zFar) {
	return Matrix4x4T<Type>::PerspectiveRadCtor(algebra::deg2rad(fovyDeg), ratio, zNear, zFar);
}

//Constructs a perspective matrix. Angle is supplied in radians.
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::PerspectiveRadCtor(Type fovyRad, Type ratio, Type zNear, Type zFar) {
	Type f = Type(1.0) / std::tan(fovyRad * Type(0.5));

	return Matrix4x4T<Type>(
		(f / ratio), 0, 0, 0,
		0, f, 0, 0,
		0, 0, -((zFar) / (zFar - zNear)), -((zNear*zFar) / (zFar - zNear)),
		0, 0, Type(-1.0), 0,
		true //Matrix is supplied in paper-written form (so columns look like columns, not transposed)
	);
}

//Constructs a perspective matrix in right hand zero notation. Angle is supplied in degrees.
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::PerspectiveRhzDegCtor(Type fovyDeg, Type ratio, Type zNear, Type zFar) {
	return Matrix4x4T<Type>::PerspectiveRhzRadCtor(algebra::deg2rad(fovyDeg), ratio, zNear, zFar);
}

//Constructs a perspective matrix in right hand zero notation. Angle is supplied in radians.
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::PerspectiveRhzRadCtor(Type fovyRad, Type ratio, Type zNear, Type zFar) {
	Type f = Type(1.0) / std::tan(fovyRad * Type(0.5));

	return weave::algebra::PerspectiveRhzCorrectionMatrix * Matrix4x4T<Type>(
		(f / ratio), 0, 0, 0,
		0, f, 0, 0,
		0, 0, -((zFar) / (zFar - zNear)), -((zNear * zFar) / (zFar - zNear)),
		0, 0, Type(-1.0), 0,
		true //Matrix is supplied in paper-written form (so columns look like columns, not transposed)
		);
}

//Constructs a look at transformation to the matrix
template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::LookAtCtor(Type eyex, Type eyey, Type eyez, Type centerx, Type centery, Type centerz, Type upx, Type upy, Type upz) {
	Vector3T<Type> f = algebra::normalize(Vector3T<Type>(centerx - eyex, centery - eyey, centerz - eyez));
	Vector3T<Type> up = algebra::normalize(Vector3T<Type>(upx, upy, upz));

	Vector3T<Type> s = algebra::normalize(algebra::cross(f, up));
	Vector3T<Type> u = algebra::normalize(algebra::cross(s, f));

	//What follows is a fast way of retrieving what would be the inverse of the coordinate system defined by u,s,f and the translation.
	//The inverse is needed to transform a point from world space into the space defined by the view
	//The transpose of the coordinate system is the inverse of the rotational part of the matrix
	Matrix4x4T<Type> mAux(
		s[0], s[1], s[2], 0,
		u[0], u[1], u[2], 0,
		-f[0], -f[1], -f[2], 0,
		0, 0, 0, 1,
		true //Matrix is supplied in paper-written form (so columns look like columns, not transposed)
	);

	mAux.Translate(-eyex, -eyey, -eyez);

	return mAux;
}

template<typename Type>
inline Matrix4x4T<Type>& Matrix4x4T<Type>::operator = (Matrix3x3T<Type> const &mIn) {
	X = mIn.X;
	Y = mIn.Y;
	Z = mIn.Z;
	W.Set(0, 0, 0, 1);
	return *this;
}
template<typename Type>
inline Matrix4x4T<Type>& Matrix4x4T<Type>::operator = (Matrix4x4T<Type> const &mIn) = default;
template<typename Type>
inline Matrix4x4T<Type>& Matrix4x4T<Type>::operator = (Matrix4x4T<Type> &&mIn) = default;

template<typename Type>
inline Matrix4x4T<Type>& Matrix4x4T<Type>::operator = (Type const *mIn) { std::memcpy(data, mIn, sizeof(data)); return *this; }

template<typename Type>
inline Matrix4x4T<Type>& Matrix4x4T<Type>::operator += (Matrix4x4T<Type> const &mIn) {
	data[0] += mIn[0]; data[1] += mIn[1]; data[2] += mIn[2]; data[3] += mIn[3];
	data[4] += mIn[4]; data[5] += mIn[5]; data[6] += mIn[6]; data[7] += mIn[7];
	data[8] += mIn[8]; data[9] += mIn[9]; data[10] += mIn[10]; data[11] += mIn[11];
	data[12] += mIn[12]; data[13] += mIn[13]; data[14] += mIn[14]; data[15] += mIn[15];
	
	return *this;
}

template<typename Type>
inline Matrix4x4T<Type>& Matrix4x4T<Type>::operator -= (Matrix4x4T<Type> const &mIn) {
	data[0] -= mIn[0]; data[1] -= mIn[1]; data[2] -= mIn[2]; data[3] -= mIn[3];
	data[4] -= mIn[4]; data[5] -= mIn[5]; data[6] -= mIn[6]; data[7] -= mIn[7];
	data[8] -= mIn[8]; data[9] -= mIn[9]; data[10] -= mIn[10]; data[11] -= mIn[11];
	data[12] -= mIn[12]; data[13] -= mIn[13]; data[14] -= mIn[14]; data[15] -= mIn[15];

	return *this;
}

template<typename Type>
inline Matrix4x4T<Type>& Matrix4x4T<Type>::operator *= (Matrix4x4T<Type> const &mIn) { 
	*this = *this * mIn;
	return *this; 
}

template<typename Type>
inline Matrix4x4T<Type>& Matrix4x4T<Type>::operator *= (Type s) {
	data[0] *= s; data[1] *= s; data[2] *= s; data[3] *= s;
	data[4] *= s; data[5] *= s; data[6] *= s; data[7] *= s;
	data[8] *= s; data[9] *= s; data[10] *= s; data[11] *= s;
	data[12] *= s; data[13] *= s; data[14] *= s; data[15] *= s;

	return *this;
}

template<typename Type>
inline Matrix4x4T<Type>& Matrix4x4T<Type>::operator /= (Type s) {
	data[0] /= s; data[1] /= s; data[2] /= s; data[3] /= s;
	data[4] /= s; data[5] /= s; data[6] /= s; data[7] /= s;
	data[8] /= s; data[9] /= s; data[10] /= s; data[11] /= s;
	data[12] /= s; data[13] /= s; data[14] /= s; data[15] /= s;

	return *this;
}

template<typename Type>
inline Matrix4x4T<Type>& Matrix4x4T<Type>::operator = (QuaternionT<Type> const &q) { *this = q.ToMatrix4(); return *this; }

template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::operator + (Matrix4x4T<Type> const &mIn) const {
	return Matrix4x4T<Type>(
		data[0] + mIn[0], data[1] + mIn[1], data[2] + mIn[2], data[3] + mIn[3],
		data[4] + mIn[4], data[5] + mIn[5], data[6] + mIn[6], data[7] + mIn[7],
		data[8] + mIn[8], data[9] + mIn[9], data[10] + mIn[10], data[11] + mIn[11],
		data[12] + mIn[12], data[13] + mIn[13], data[14] + mIn[14], data[15] + mIn[15]
		);
}

template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::operator - (Matrix4x4T<Type> const &mIn) const {
	return Matrix4x4T<Type>(
		data[0] - mIn[0], data[1] - mIn[1], data[2] - mIn[2], data[3] - mIn[3],
		data[4] - mIn[4], data[5] - mIn[5], data[6] - mIn[6], data[7] - mIn[7],
		data[8] - mIn[8], data[9] - mIn[9], data[10] - mIn[10], data[11] - mIn[11],
		data[12] - mIn[12], data[13] - mIn[13], data[14] - mIn[14], data[15] - mIn[15]
		);
}

template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::operator * (Matrix4x4T<Type> const &mIn) const {
	return Matrix4x4T<Type>(

		this->data[0] * mIn[0] + this->data[4] * mIn[1] + this->data[8] * mIn[2] + this->data[12] * mIn[3],
		this->data[1] * mIn[0] + this->data[5] * mIn[1] + this->data[9] * mIn[2] + this->data[13] * mIn[3],
		this->data[2] * mIn[0] + this->data[6] * mIn[1] + this->data[10] * mIn[2] + this->data[14] * mIn[3],
		this->data[3] * mIn[0] + this->data[7] * mIn[1] + this->data[11] * mIn[2] + this->data[15] * mIn[3],

		this->data[0] * mIn[4] + this->data[4] * mIn[5] + this->data[8] * mIn[6] + this->data[12] * mIn[7],
		this->data[1] * mIn[4] + this->data[5] * mIn[5] + this->data[9] * mIn[6] + this->data[13] * mIn[7],
		this->data[2] * mIn[4] + this->data[6] * mIn[5] + this->data[10] * mIn[6] + this->data[14] * mIn[7],
		this->data[3] * mIn[4] + this->data[7] * mIn[5] + this->data[11] * mIn[6] + this->data[15] * mIn[7],

		this->data[0] * mIn[8] + this->data[4] * mIn[9] + this->data[8] * mIn[10] + this->data[12] * mIn[11],
		this->data[1] * mIn[8] + this->data[5] * mIn[9] + this->data[9] * mIn[10] + this->data[13] * mIn[11],
		this->data[2] * mIn[8] + this->data[6] * mIn[9] + this->data[10] * mIn[10] + this->data[14] * mIn[11],
		this->data[3] * mIn[8] + this->data[7] * mIn[9] + this->data[11] * mIn[10] + this->data[15] * mIn[11],

		this->data[0] * mIn[12] + this->data[4] * mIn[13] + this->data[8] * mIn[14] + this->data[12] * mIn[15],
		this->data[1] * mIn[12] + this->data[5] * mIn[13] + this->data[9] * mIn[14] + this->data[13] * mIn[15],
		this->data[2] * mIn[12] + this->data[6] * mIn[13] + this->data[10] * mIn[14] + this->data[14] * mIn[15],
		this->data[3] * mIn[12] + this->data[7] * mIn[13] + this->data[11] * mIn[14] + this->data[15] * mIn[15]
		);
}

template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::operator * (Type s) const {
	return Matrix4x4T<Type>(
		data[0] * s, data[1] * s, data[2] * s, data[3] * s,
		data[4] * s, data[5] * s, data[6] * s, data[7] * s,
		data[8] * s, data[9] * s, data[10] * s, data[11] * s,
		data[12] * s, data[13] * s, data[14] * s, data[15] * s
		);
}

template<typename Type>
inline Matrix4x4T<Type> Matrix4x4T<Type>::operator / (Type s) const {
	return Matrix4x4T<Type>(
		data[0] / s, data[1] / s, data[2] / s, data[3] / s,
		data[4] / s, data[5] / s, data[6] / s, data[7] / s,
		data[8] / s, data[9] / s, data[10] / s, data[11] / s,
		data[12] / s, data[13] / s, data[14] / s, data[15] / s
		);
}

template<typename Type>
inline Vector3T<Type> Matrix4x4T<Type>::operator * (Vector3T<Type> const &vIn) const {
	return Vector3T<Type>
		(data[0] * vIn.data[0] + data[4] * vIn.data[1] + data[8] * vIn.data[2] + data[12],
		 data[1] * vIn.data[0] + data[5] * vIn.data[1] + data[9] * vIn.data[2] + data[13],
		 data[2] * vIn.data[0] + data[6] * vIn.data[1] + data[10] * vIn.data[2] + data[14]);
}

template<typename Type>
inline Vector4T<Type> Matrix4x4T<Type>::operator * (Vector4T<Type> const &vIn) const {
	return Vector4T<Type>
		(data[0] * vIn.data[0] + data[4] * vIn.data[1] + data[8] * vIn.data[2] + data[12] * vIn.data[3],
		 data[1] * vIn.data[0] + data[5] * vIn.data[1] + data[9] * vIn.data[2] + data[13] * vIn.data[3],
		 data[2] * vIn.data[0] + data[6] * vIn.data[1] + data[10] * vIn.data[2] + data[14] * vIn.data[3],
		 data[3] * vIn.data[0] + data[7] * vIn.data[1] + data[11] * vIn.data[2] + data[15] * vIn.data[3]);
}

template<typename Type>
inline Vector3T<Type> Matrix4x4T<Type>::operator % (Vector3T<Type> const &vIn) const {
	return this->TransformNormal(vIn);
}

template<typename Type>
inline Vector4T<Type> Matrix4x4T<Type>::operator % (Vector4T<Type> const &vIn) const {
	return this->TransformNormal(vIn.xyz);
}

template<typename Type>
template<typename Index>
inline Type& Matrix4x4T<Type>::operator [] (Index i) { return data[i]; }
template<typename Type>
template<typename Index>
inline Type Matrix4x4T<Type>::operator [] (Index i) const { return data[i]; }

template<typename Type>
template<typename IndexR, typename IndexC>
inline Type& Matrix4x4T<Type>::operator() (IndexR row, IndexC col) { return data[row + col * 4]; }
template<typename Type>
template<typename IndexR, typename IndexC>
inline Type  Matrix4x4T<Type>::operator() (IndexR row, IndexC col) const { return data[row + col * 4]; }

template<typename Type>
inline bool Matrix4x4T<Type>::operator== (Matrix4x4T<Type> const &mCmp) const {
	for(uint32_t i = 0; i < NumComps(); ++i) {
		if(data[i] != mCmp.data[i])
			return false;
	}
	return true;
}

template<typename Type>
inline bool Matrix4x4T<Type>::operator!= (Matrix4x4T<Type> const &mCmp) const {
	return !(*this == mCmp);
}

template<typename Type>
inline Matrix4x4T<Type> operator * (Type s, Matrix4x4T<Type> const &m) { return m * s; }

template<typename Type>
inline Matrix4x4T<Type> operator / (Type s, Matrix4x4T<Type> const &m) { return m / s; }

template<typename Type>
inline constexpr uint32_t Matrix4x4T<Type>::NumComps() {
	return 16;
}


//---------
//Mat3 impl

template<typename Type>
inline Matrix3x3T<Type>::Matrix3x3T() 
	: X(1,0,0), Y(0,1,0), Z(0,0,1) {
}
template<typename Type>
inline Matrix3x3T<Type>::Matrix3x3T(Type Xx, Type Xy, Type Xz, Type Yx, Type Yy, Type Yz, Type Zx, Type Zy, Type Zz, bool transpose) {
	//Set matrix on column major (X, Y, Z)
	if(!transpose) {
		data[0] = Xx; data[3] = Yx; data[6] = Zx;
		data[1] = Xy; data[4] = Yy; data[7] = Zy;
		data[2] = Xz; data[5] = Yz; data[8] = Zz;
	}
	else {
		//unless they asked us to transpose it. Transposed assignment is useful for visualizing the matrix in code as it would be on paper
		data[0] = Xx; data[3] = Xy; data[6] = Xz;
		data[1] = Yx; data[4] = Yy; data[7] = Yz;
		data[2] = Zx; data[5] = Zy; data[8] = Zz;
	}
}
template<typename Type>
inline Matrix3x3T<Type>::Matrix3x3T(Type const * mIn) {
	std::memcpy(data, mIn, sizeof(data));
}
template<typename Type>
inline Matrix3x3T<Type>::Matrix3x3T(Matrix4x4T<Type> const & m4x4) {
	*this = m4x4.GetNormalTransform();
}
template<typename Type>
inline Matrix3x3T<Type>::Matrix3x3T(Vector3T<Type> const & X, Vector3T<Type> const & Y, Vector3T<Type> const & Z)
	: X(X), Y(Y), Z(Z){
}
template<typename Type>
inline Matrix3x3T<Type>::Matrix3x3T(QuaternionT<Type> const & q) {
	*this = q;
}

template<typename Type>
inline void Matrix3x3T<Type>::Identity() {
	X.Set(1, 0, 0);
	Y.Set(0, 1, 0);
	Z.Set(0, 0, 1);
}
template<typename Type>
inline void Matrix3x3T<Type>::Set(Type Xx, Type Xy, Type Xz, Type Yx, Type Yy, Type Yz, Type Zx, Type Zy, Type Zz, bool transpose) {
	//Set matrix on column major (X, Y, Z, W)
	if(!transpose) {
		data[0] = Xx; data[3] = Yx; data[6] = Zx;
		data[1] = Xy; data[4] = Yy; data[7] = Zy;
		data[2] = Xz; data[5] = Yz; data[8] = Zz;
	}
	else {
		//unless they asked us to transpose it. Transposed assignment is useful for visualizing the matrix in code as it would be on paper
		data[0] = Xx; data[3] = Xy; data[6] = Xz;
		data[1] = Yx; data[4] = Yy; data[7] = Yz;
		data[2] = Zx; data[5] = Zy; data[8] = Zz;
	}
}
template<typename Type>
inline void Matrix3x3T<Type>::SetRows(Vector3T<Type> const & a, Vector3T<Type> const & b, Vector3T<Type> const & c) {
	//Assign as row major
	data[0] = a[0]; data[3] = a[1]; data[6] = a[2];
	data[1] = b[0]; data[4] = b[1]; data[7] = b[2];
	data[2] = c[0]; data[5] = c[1]; data[8] = c[2];
}
template<typename Type>
template<typename Index>
inline void Matrix3x3T<Type>::SetRow(Index num, Vector3T<Type> const & a) {
	data[0 + num] = a[0]; data[3 + num] = a[1]; data[6 + num] = a[2];
}

//Returns the scale applies to the matrix as a vector3.
template<typename Type>
inline Vector3T<Type> Matrix3x3T<Type>::GetScalingFactor() const {
	//The scaling factor for each vector is found as the magnitud of each column
	//Support for negative scaling can be added by detecting the sign of the determinant and then multiplying the returned vector by it
	//auto flip = (algebra::dot(X, algebra::cross(Y, Z)) < 0) ? Type(-1.0) : Type(1.0);

	return Vector3T<Type>(
		weave::algebra::length(X)/* * flip*/,
		weave::algebra::length(Y),
		weave::algebra::length(Z)
		);
}

//Removes the scaling factor by normalizing the axes and returns said scaling factor
template<typename Type>
inline Vector3T<Type> Matrix3x3T<Type>::RemoveScalingFactor() {
	auto factor = GetScalingFactor();
	X /= factor.x;
	Y /= factor.y;
	Z /= factor.z;

	return factor;
}
template<typename Type>
inline void Matrix3x3T<Type>::GetRows(Vector3T<Type>& a, Vector3T<Type>& b, Vector3T<Type>& c) const {
	a[0] = data[0]; a[1] = data[3]; a[2] = data[6];
	b[0] = data[1]; b[1] = data[4]; b[2] = data[7];
	c[0] = data[2]; c[1] = data[5]; c[2] = data[8];
}
template<typename Type>
template<typename Index>
inline Vector3T<Type> Matrix3x3T<Type>::GetRow(Index num) const {
	return { data[0 + num], data[3 + num], data[6 + num] };
}
template<typename Type>
inline Vector3T<Type> Matrix3x3T<Type>::GetDiagonal() const {
	return Vector3T<Type>(data[0], data[4], data[8]);
}

//Transposes the matrix
template<typename Type>
inline void Matrix3x3T<Type>::Transpose() {
	*this = Matrix3x3T(
		data[0], data[1], data[2],
		data[3], data[4], data[5],
		data[6], data[7], data[8],
		true
	);;
}
template<typename Type>
inline Matrix3x3T<Type> Matrix3x3T<Type>::Transposed() const {
	return Matrix3x3T<Type>(
		data[0], data[1], data[2],
		data[3], data[4], data[5],
		data[6], data[7], data[8],
		true
		);
}
template<typename Type>
inline int Matrix3x3T<Type>::Invert() {
	Type t1 = data[0] * data[4];
	Type t2 = data[0] * data[5];
	Type t3 = data[1] * data[3];
	Type t4 = data[2] * data[3];
	Type t5 = data[1] * data[6];
	Type t6 = data[2] * data[6];

	// Calculate the determinant.
	Type det = t1*data[8] - t2*data[7] - t3*data[8] + t4*data[7] + t5*data[5] - t6*data[4];
	// Make sure the determinant is non-zero.
	if(det == 0.0f)
		return 0;

	Type invd = 1.0f / det;

	Matrix3x3T<Type> kInv;
	kInv[0] = (data[4] * data[8] - data[5] * data[7])*invd;
	kInv[1] = -(data[1] * data[8] - data[2] * data[7])*invd;
	kInv[2] = (data[1] * data[5] - data[2] * data[4])*invd;
	kInv[3] = -(data[3] * data[8] - data[5] * data[6])*invd;
	kInv[4] = (data[0] * data[8] - t6)*invd;
	kInv[5] = -(t2 - t4)*invd;
	kInv[6] = (data[3] * data[7] - data[4] * data[6])*invd;
	kInv[7] = -(data[0] * data[7] - t5)*invd;
	kInv[8] = (t1 - t3)*invd;

	*this = kInv;

	return 1;
}

template<typename Type>
inline Matrix3x3T<Type> Matrix3x3T<Type>::Inverted() const {
	Matrix3x3T<Type> ret = *this;
	ret.Invert();
	return ret;
}

template<typename Type>
inline void Matrix3x3T<Type>::InvertOrtho() {
	//This operation is only a valid inverse if the matrix is othogonal
	Transpose();
}

template<typename Type>
inline Matrix3x3T<Type> Matrix3x3T<Type>::InvertedOrtho() const {
	return Transposed();
}

template<typename Type>
inline void Matrix3x3T<Type>::SkewSymmetric(Vector3T<Type> const & v) {
	data[0] = 0;    data[3] = -v.z; data[6] = v.y;
	data[1] = v.z;  data[4] = 0;    data[7] = -v.x;
	data[2] = -v.y; data[5] = v.x;  data[8] = 0;
}

template<typename Type>
inline Type Matrix3x3T<Type>::Trace() const { return data[0] + data[4] + data[8]; }

template<typename Type>
inline Type Matrix3x3T<Type>::Determinant() const {
	Type t1 = data[0] * data[4];
	Type t2 = data[0] * data[5];
	Type t3 = data[1] * data[3];
	Type t4 = data[2] * data[3];
	Type t5 = data[1] * data[6];
	Type t6 = data[2] * data[6];

	// Calculate the determinant.
	Type det = t1*data[8] - t2*data[7] - t3*data[8] + t4*data[7] + t5*data[5] - t6*data[4];
	return det;
}

template<typename Type>
inline void Matrix3x3T<Type>::Weight(Matrix3x3T * ms, int const * indices, Type const * weights, unsigned int count) {
	Type *wmatrix = ms[indices[0]].data;

	for(unsigned int j = 0; j<9; ++j) {
		this->data[j] = wmatrix[j] * weights[0];
	}

	for(unsigned int i = 1; i<count; ++i) {
		wmatrix = ms[indices[i]].data;
		for(unsigned int j = 0; j<9; ++j) {
			this->data[j] += wmatrix[j] * weights[i];
		}
	}
}

//Rotates the matrix. The current matrix is transformed by the specified rotation. Angles are in radians.
template<typename Type>
inline void Matrix3x3T<Type>::Rotate(Type radians, Type x, Type y, Type z) {
	Type mag = std::sqrt(x*x + y*y + z*z);
	x /= mag; y /= mag; z /= mag;
	Type c = std::cos(radians);
	Type s = std::sin(radians);
	Type umc = 1 - c;

	*this *= Matrix3x3T(
		x*x*umc + c, x*y*umc - z*s, x*z*umc + y*s,
		y*x*umc + z*s, y*y*umc + c, y*z*umc - x*s,
		z*x*umc - y*s, z*y*umc + x*s, z*z*umc + c,
		true //Indicates the matrix is being supplied in "paper written" form instead of each vector linearly
	);
}
template<typename Type>
inline void Matrix3x3T<Type>::Rotate(Type radians, weave::Vector3T<Type> const & vec) { Rotate(radians, vec.x, vec.y, vec.z); }
template<typename Type>
inline void Matrix3x3T<Type>::Rotate(Type pitch, Type yaw, Type roll) {
	Type A = std::cos(pitch);
	Type B = std::sin(pitch);
	Type C = std::cos(yaw);
	Type D = std::sin(yaw);
	Type E = std::cos(roll);
	Type F = std::sin(roll);
	Type AD = A * D;
	Type BD = B * D;

	*this *= Matrix3x3T(
		C*E, -C*F, D,
		BD*E + A*F, -BD*F + A*E, -B*C,
		-AD*E + B*F, AD*F + B*E, A*C,
		true
	);
}
template<typename Type>
inline void Matrix3x3T<Type>::Rotate(weave::Vector3T<Type> const & pitchYawRoll) { Rotate(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z); }

//Scales the matrix. The current matrix is transformed by the specified scaling
template<typename Type>
inline void Matrix3x3T<Type>::Scale(Type x, Type y, Type z) {
	//Sets the scaling coeficients on X, Y and Z vectors
	*this *= Matrix3x3T(
		x, 0, 0,
		0, y, 0,
		0, 0, z
	);
}
template<typename Type>
inline void Matrix3x3T<Type>::Scale(weave::Vector3T<Type> const& vec) { Scale(vec.x, vec.y, vec.z); }

//Creates a rotational transformation matrix instead of multiplying the transformation over. The current matrix is replaced. Angles are in radians.
template<typename Type>
inline void Matrix3x3T<Type>::Rotation(Type radians, Type x, Type y, Type z) {
	Type mag = std::sqrt(x*x + y*y + z*z);
	x /= mag; y /= mag; z /= mag;
	Type c = std::cos(radians);
	Type s = std::sin(radians);
	Type umc = 1 - c;

	Set(
		x*x*umc + c, x*y*umc - z*s, x*z*umc + y*s,
		y*x*umc + z*s, y*y*umc + c, y*z*umc - x*s,
		z*x*umc - y*s, z*y*umc + x*s, z*z*umc + c,
		true //Indicates the matrix is being supplied in "paper written" form instead of each vector linearly
	);
}
template<typename Type>
inline void Matrix3x3T<Type>::Rotation(Type radians, weave::Vector3T<Type> const& vec) { Rotate(radians, vec.x, vec.y, vec.z); }
template<typename Type>
inline void Matrix3x3T<Type>::Rotation(Type pitch, Type yaw, Type roll) {
	Type A = std::cos(pitch);
	Type B = std::sin(pitch);
	Type C = std::cos(yaw);
	Type D = std::sin(yaw);
	Type E = std::cos(roll);
	Type F = std::sin(roll);
	Type AD = A * D;
	Type BD = B * D;

	Set(
		C*E, -C*F, D,
		BD*E + A*F, -BD*F + A*E, -B*C,
		-AD*E + B*F, AD*F + B*E, A*C,
		true
	);
}
template<typename Type>
inline void Matrix3x3T<Type>::Rotation(weave::Vector3T<Type> const & pitchYawRoll) { Rotate(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z); }

//Creates a scaling transformation matrix instead of multiplying the transformation over. The current matrix is replaced.
template<typename Type>
inline void Matrix3x3T<Type>::Scaling(Type x, Type y, Type z) {
	Set(
		x, 0.0f, 0.0f,
		0.0f, y, 0.0f,
		0.0f, 0.0f, z
	);
}
template<typename Type>
inline void Matrix3x3T<Type>::Scaling(weave::Vector3T<Type> const& vec) { Scale(vec.x, vec.y, vec.z); }


//Inline operators
template<typename Type>
inline Matrix3x3T<Type>& Matrix3x3T<Type>::operator = (Matrix3x3T<Type> const &mIn) = default;
template<typename Type>
inline Matrix3x3T<Type>& Matrix3x3T<Type>::operator = (Matrix3x3T<Type> &&mIn) = default;

template<typename Type>
inline Matrix3x3T<Type>& Matrix3x3T<Type>::operator = (Type const *mIn) { std::memcpy(data, mIn, sizeof(data)); return *this; }

template<typename Type>
inline Matrix3x3T<Type>& Matrix3x3T<Type>::operator = (Matrix4x4T<Type> const &mIn) { *this = mIn.GetNormalTransform(); return *this; }

template<typename Type>
inline Matrix3x3T<Type>& Matrix3x3T<Type>::operator = (QuaternionT<Type> const &q) { *this = q.ToMatrix(); return *this; }

template<typename Type>
inline Matrix3x3T<Type> Matrix3x3T<Type>::operator + (Matrix3x3T<Type> const &mIn) const {
	return Matrix3x3T<Type>(
		data[0] + mIn[0], data[1] + mIn[1], data[2] + mIn[2], data[3] + mIn[3],
		data[4] + mIn[4], data[5] + mIn[5], data[6] + mIn[6], data[7] + mIn[7],
		data[8] + mIn[8]
		);
}


template<typename Type>
inline Matrix3x3T<Type>& Matrix3x3T<Type>::operator += (Matrix3x3T<Type> const &mIn) {
	data[0] += mIn.data[0]; data[1] += mIn.data[1]; data[2] += mIn.data[2]; data[3] += mIn.data[3];
	data[4] += mIn.data[4]; data[5] += mIn.data[5]; data[6] += mIn.data[6]; data[7] += mIn.data[7];
	data[8] += mIn.data[8];

	return *this;
}

template<typename Type>
inline Matrix3x3T<Type> Matrix3x3T<Type>::operator - (Matrix3x3T<Type> const &mIn) const {
	return Matrix3x3T<Type>(
		data[0] - mIn[0], data[1] - mIn[1], data[2] - mIn[2], data[3] - mIn[3],
		data[4] - mIn[4], data[5] - mIn[5], data[6] - mIn[6], data[7] - mIn[7],
		data[8] - mIn[8]
		);
}


template<typename Type>
inline Matrix3x3T<Type>& Matrix3x3T<Type>::operator -= (Matrix3x3T<Type> const &mIn) {
	data[0] -= mIn[0]; data[1] -= mIn[1]; data[2] -= mIn[2]; data[3] -= mIn[3];
	data[4] -= mIn[4]; data[5] -= mIn[5]; data[6] -= mIn[6]; data[7] -= mIn[7];
	data[8] -= mIn[8];

	return *this;
}

template<typename Type>
inline Matrix3x3T<Type>& Matrix3x3T<Type>::operator *= (Matrix3x3T<Type> const &mIn) { *this = (*this) * mIn; return *this; }

template<typename Type>
inline Matrix3x3T<Type>& Matrix3x3T<Type>::operator *= (Type s) {
	data[0] *= s; data[1] *= s; data[2] *= s;
	data[3] *= s; data[4] *= s; data[5] *= s;
	data[6] *= s; data[7] *= s; data[8] *= s;

	return *this;
}

template<typename Type>
inline Matrix3x3T<Type>& Matrix3x3T<Type>::operator /= (Type s) {
	data[0] /= s; data[1] /= s; data[2] /= s;
	data[3] /= s; data[4] /= s; data[5] /= s;
	data[6] /= s; data[7] /= s; data[8] /= s;

	return *this;
}

template<typename Type>
inline Matrix3x3T<Type> Matrix3x3T<Type>::operator * (Matrix3x3T<Type> const &mIn) const {
	return Matrix3x3T<Type>(
		this->data[0] * mIn[0] + this->data[3] * mIn[1] + this->data[6] * mIn[2],
		this->data[1] * mIn[0] + this->data[4] * mIn[1] + this->data[7] * mIn[2],
		this->data[2] * mIn[0] + this->data[5] * mIn[1] + this->data[8] * mIn[2],
		this->data[0] * mIn[3] + this->data[3] * mIn[4] + this->data[6] * mIn[5],
		this->data[1] * mIn[3] + this->data[4] * mIn[4] + this->data[7] * mIn[5],
		this->data[2] * mIn[3] + this->data[5] * mIn[4] + this->data[8] * mIn[5],
		this->data[0] * mIn[6] + this->data[3] * mIn[7] + this->data[6] * mIn[8],
		this->data[1] * mIn[6] + this->data[4] * mIn[7] + this->data[7] * mIn[8],
		this->data[2] * mIn[6] + this->data[5] * mIn[7] + this->data[8] * mIn[8]
		);
}



template<typename Type>
inline Matrix3x3T<Type> Matrix3x3T<Type>::operator * (Type s) const {
	return Matrix3x3T<Type>(
		data[0] * s, data[1] * s, data[2] * s, 
		data[3] * s, data[4] * s, data[5] * s, 
		data[6] * s, data[7] * s, data[8] * s
		);
}

template<typename Type>
inline Matrix3x3T<Type> Matrix3x3T<Type>::operator / (Type s) const {
	return Matrix3x3T<Type>(
		data[0] / s, data[1] / s, data[2] / s,
		data[3] / s, data[4] / s, data[5] / s,
		data[6] / s, data[7] / s, data[8] / s
		);
}

template<typename Type>
inline Vector3T<Type> Matrix3x3T<Type>::operator * (Vector3T<Type> const &vIn) const {
	return Vector3T<Type>
		(data[0] * vIn.data[0] + data[3] * vIn.data[1] + data[6] * vIn.data[2],
		 data[1] * vIn.data[0] + data[4] * vIn.data[1] + data[7] * vIn.data[2],
		 data[2] * vIn.data[0] + data[5] * vIn.data[1] + data[8] * vIn.data[2]);
}

template<typename Type>
template<typename Index>
inline Type& Matrix3x3T<Type>::operator [] (Index i) { return data[i]; }
template<typename Type>
template<typename Index>
inline Type Matrix3x3T<Type>::operator [] (Index i) const { return data[i]; }

template<typename Type>
template<typename IndexR, typename IndexC>
inline Type& Matrix3x3T<Type>::operator() (IndexR row, IndexC col) { return data[row + col * 3]; }
template<typename Type>
template<typename IndexR, typename IndexC>
inline Type  Matrix3x3T<Type>::operator() (IndexR row, IndexC col) const { return data[row + col * 3]; }

template<typename Type>
inline Matrix3x3T<Type> operator * (Type s, Matrix3x3T<Type> const &m) { return m * s; }

template<typename Type>
inline Matrix3x3T<Type> operator / (Type s, Matrix3x3T<Type> const &m) { return m / s; }

template<typename Type>
inline bool Matrix3x3T<Type>::operator== (Matrix3x3T<Type> const &mCmp) const {
	for (uint32_t i = 0; i < NumComps(); ++i) {
		if (data[i] != mCmp.data[i])
			return false;
	}
	return true;
}

template<typename Type>
inline bool Matrix3x3T<Type>::operator!= (Matrix3x3T<Type> const &mCmp) const {
	return !(*this == mCmp);
}

template<typename Type>
inline constexpr uint32_t Matrix3x3T<Type>::NumComps() {
	return 9;
}


//---------
//Mat2 impl

template<typename Type>
inline Matrix2x2T<Type>::Matrix2x2T() 
	: X(1,0), Y(0,1) {
}

template<typename Type>
inline Matrix2x2T<Type>::Matrix2x2T(Type Xx, Type Xy, Type Yx, Type Yy, bool transpose) {

	//Set matrix on column major (X, Y)
	if(!transpose) {
		data[0] = Xx; data[2] = Yx;
		data[1] = Xy; data[3] = Yy;
	}
	else {
		//unless they asked us to transpose it. Transposed assignment is useful for visualizing the matrix in code as it would be on paper
		data[0] = Xx; data[2] = Xy;
		data[1] = Yx; data[3] = Yy;
	}
}

template<typename Type>
inline Matrix2x2T<Type>::Matrix2x2T(Type const * mIn) {
	std::memcpy(data, mIn, sizeof(data));
}

template<typename Type>
inline Matrix2x2T<Type>::Matrix2x2T(Vector2T<Type> const & X, Vector2T<Type> const & Y)
	: X(X), Y(Y) {
}

template<typename Type>
inline Matrix2x2T<Type>::~Matrix2x2T() {}

template<typename Type>
inline void Matrix2x2T<Type>::Identity() {
	X.Set(1, 0);
	Y.Set(0, 1);
}

template<typename Type>
inline void Matrix2x2T<Type>::Set(Type Xx, Type Xy, Type Yx, Type Yy, bool transpose) {

	//Set matrix on column major (X, Y)
	if(!transpose) {
		data[0] = Xx; data[2] = Yx;
		data[1] = Xy; data[3] = Yy;
	}
	else {
		//unless they asked us to transpose it. Transposed assignment is useful for visualizing the matrix in code as it would be on paper
		data[0] = Xx; data[2] = Xy;
		data[1] = Yx; data[3] = Yy;
	}
}

template<typename Type>
inline void Matrix2x2T<Type>::SetRows(Vector2T<Type> const & a, Vector2T<Type> const & b) {
	//Assign as row major
	data[0] = a[0]; data[2] = a[1];
	data[1] = b[0]; data[3] = b[1];
}

template<typename Type>
template<typename Index>
inline void Matrix2x2T<Type>::SetRow(Index num, Vector2T<Type> const & a) {
	data[0 + num] = a[0]; data[2 + num] = a[1];
}

//Returns the scale applies to the matrix as a vector2.
template<typename Type>
inline Vector2T<Type> Matrix2x2T<Type>::GetScalingFactor() const {
	//The scaling factor for each vector is found as the magnitud of each column
	return Vector2T<Type>(
		weave::algebra::length(X),
		weave::algebra::length(Y)
		);
}

//Removes the scaling factor from the matrix by normalizing the axes and returns the removed scaling factor
template<typename Type>
inline Vector2T<Type> Matrix2x2T<Type>::RemoveScalingFactor() {
	auto factor = GetScalingFactor();
	X /= factor.x;
	Y /= factor.y;

	return factor;
}

template<typename Type>
inline void Matrix2x2T<Type>::GetRows(Vector2T<Type>& a, Vector2T<Type>& b) const {
	a[0] = data[0]; a[1] = data[2];
	b[0] = data[1]; b[1] = data[3];
}

template<typename Type>
template<typename Index>
inline Vector2T<Type> Matrix2x2T<Type>::GetRow(Index num) const {
	return Vector2T<Type>(data[0 + num], data[2 + num]);
}

template<typename Type>
inline Vector2T<Type> Matrix2x2T<Type>::GetDiagonal() const {
	return Vector2T<Type>(data[0], data[1]);
}


template<typename Type>
inline void Matrix2x2T<Type>::Transpose() {
	Type tmp = data[1];
	data[1] = data[2];
	data[2] = tmp;
}

template<typename Type>
inline int Matrix2x2T<Type>::Invert() {
	Transpose();
	return 1;
}

template<typename Type>
inline Matrix2x2T<Type> Matrix2x2T<Type>::Transposed() const {
	return Matrix2x2T<Type> { data[0], data[2], data[1], data[3] };
}

template<typename Type>
inline Matrix2x2T<Type> Matrix2x2T<Type>::Inverted() const {
	Matrix2x2T ret = *this;
	ret.Invert();
	return ret;
}

template<typename Type>
inline void Matrix2x2T<Type>::InvertOrtho() {
	Transpose();
}

template<typename Type>
inline Matrix2x2T<Type> Matrix2x2T<Type>::InvertedOrtho() const {
	Matrix2x2T ret = *this;
	ret.Invert();
	return ret;
}

template<typename Type>
inline Type Matrix2x2T<Type>::Trace() const { return data[0] + data[3]; }

template<typename Type>
inline Type Matrix2x2T<Type>::Determinant() const { return data[0] * data[2] + data[1] * data[3]; }

template<typename Type>
inline void Matrix2x2T<Type>::Weight(Matrix2x2T * ms, int const * indices, Type const * weights, unsigned int count) {
	Type *wmatrix = ms[indices[0]].m;
	//Inicializar matriz a los valores sopesados de la primera matriz de entrada
	for(unsigned int j = 0; j<4; ++j) {
		this->data[j] = wmatrix[j] * weights[0];
	}
	//Sopesar cada matriz y acumular en la matriz actual
	for(unsigned int i = 1; i<count; ++i) {
		wmatrix = ms[indices[i]].m;
		for(unsigned int j = 0; j<4; ++j) {
			this->data[j] += wmatrix[j] * weights[i];
		}
	}
}

template<typename Type>
inline void Matrix2x2T<Type>::Scale(Type x, Type y) {
	//Sets the scaling coeficients on X, Y and Z vectors
	Matrix2x2T mAux(
		x, 0.0f,
		0.0f, y
	);

	*this *= (mAux);
}

template<typename Type>
inline void Matrix2x2T<Type>::Rotate(Type radians, Type x, Type y) {
	Type mag = std::sqrt(x*x + y*y);
	x /= mag; y /= mag;
	Type c = std::cos(radians);
	//Type s = std::sin(radians);
	Type umc = 1 - c;

	Matrix2x2T rotmatrix(
		x*x*umc + c, x*y*umc,
		y*x*umc, y*y*umc + c,
		true //Indicates the matrix is being supplied in "paper written" form instead of each vector linearly
	);

	*this *= (rotmatrix);
}

//Creates a scaling matrix. Replaces the current matrix
template<typename Type>
inline void Matrix2x2T<Type>::Scaling(Type x, Type y) {
	Set(
		x, 0,
		0, y
	);
}

//Creates a rotational matrix. Replaces the current matrix. Angles in radians.
template<typename Type>
inline void Matrix2x2T<Type>::Rotation(Type radians, Type x, Type y) {
	Type mag = std::sqrt(x*x + y*y);
	x /= mag; y /= mag;
	Type c = std::cos(radians);
	//Type s = std::sin(radians);
	Type umc = 1 - c;

	Set(
		x*x*umc + c, x*y*umc,
		y*x*umc, y*y*umc + c,
		true //Indicates the matrix is being supplied in "paper written" form instead of each vector linearly
	);
}

//Inline operators
template<typename Type>
inline Matrix2x2T<Type>& Matrix2x2T<Type>::operator = (Matrix2x2T<Type> const &mIn) = default;
template<typename Type>
inline Matrix2x2T<Type>& Matrix2x2T<Type>::operator = (Matrix2x2T<Type> &&mIn) = default;
template<typename Type>
inline Matrix2x2T<Type>& Matrix2x2T<Type>::operator = (Type const *mIn) { memcpy(data, mIn, 4 * sizeof(Type)); return *this; }

template<typename Type>
inline Matrix2x2T<Type>& Matrix2x2T<Type>::operator += (Matrix2x2T<Type> const &mIn) {
	data[0] += mIn.data[0]; data[1] += mIn.data[1];
	data[2] += mIn.data[2]; data[3] += mIn.data[3];

	return *this;
}

template<typename Type>
inline Matrix2x2T<Type>& Matrix2x2T<Type>::operator -= (Matrix2x2T<Type> const &mIn) {
	data[0] -= mIn[0]; data[1] -= mIn[1];
	data[2] -= mIn[2]; data[3] -= mIn[3];

	return *this;
}

template<typename Type>
inline Matrix2x2T<Type>& Matrix2x2T<Type>::operator *= (Matrix2x2T<Type> const &mIn) { *this = (*this) * mIn; return *this; }

template<typename Type>
inline Matrix2x2T<Type>& Matrix2x2T<Type>::operator *= (Type s) {
	data[0] *= s; data[1] *= s;
	data[2] *= s; data[3] *= s;

	return *this;
}

template<typename Type>
inline Matrix2x2T<Type>& Matrix2x2T<Type>::operator /= (Type s) {
	data[0] /= s; data[1] /= s;
	data[2] /= s; data[3] /= s;

	return *this;
}

template<typename Type>
inline Matrix2x2T<Type> Matrix2x2T<Type>::operator * (Matrix2x2T<Type> const &mIn) const {
	Matrix2x2T<Type> mtemp;

	mtemp.data[0] = this->data[0] * mIn[0] + this->data[2] * mIn[1];
	mtemp.data[1] = this->data[1] * mIn[0] + this->data[3] * mIn[1];
	mtemp.data[2] = this->data[0] * mIn[2] + this->data[2] * mIn[3];
	mtemp.data[3] = this->data[1] * mIn[2] + this->data[3] * mIn[3];

	return mtemp;
}

template<typename Type>
inline Matrix2x2T<Type> Matrix2x2T<Type>::operator * (Type s) const {
	return Matrix2x2T<Type>(
		data[0] * s, data[1] * s, 
		data[2] * s, data[3] * s
		);
}

template<typename Type>
inline Matrix2x2T<Type> Matrix2x2T<Type>::operator / (Type s) const {
	return Matrix2x2T<Type>(
		data[0] / s, data[1] / s,
		data[2] / s, data[3] / s
		);
}

template<typename Type>
inline Vector2T<Type> Matrix2x2T<Type>::operator * (Vector2T<Type> const &vIn) const {
	return Vector2T<Type>
		(data[0] * vIn.x + data[2] * vIn.y,
		 data[1] * vIn.x + data[3] * vIn.y);
}

template<typename Type>
inline Matrix2x2T<Type> Matrix2x2T<Type>::operator + (Matrix2x2T<Type> const &mIn) const {
	return Matrix2x2T<Type>(
		data[0] + mIn[0], data[1] + mIn[1], 
		data[2] + mIn[2], data[3] + mIn[3]
		);
}




template<typename Type>
inline Matrix2x2T<Type> Matrix2x2T<Type>::operator - (Matrix2x2T<Type> const &mIn) const {
	return Matrix2x2T<Type>(
		data[0] - mIn[0], data[1] - mIn[1], 
		data[2] - mIn[2], data[3] - mIn[3]
		);
}




template<typename Type>
template<typename Index>
inline Type& Matrix2x2T<Type>::operator [] (Index i) { return data[i]; }
template<typename Type>
template<typename Index>
inline Type Matrix2x2T<Type>::operator [] (Index i) const { return data[i]; }

template<typename Type>
template<typename IndexR, typename IndexC>
inline Type& Matrix2x2T<Type>::operator() (IndexR row, IndexC col) { return data[row + col * 2]; }
template<typename Type>
template<typename IndexR, typename IndexC>
inline Type  Matrix2x2T<Type>::operator() (IndexR row, IndexC col) const { return data[row + col * 2]; }

template<typename Type>
inline bool Matrix2x2T<Type>::operator== (Matrix2x2T<Type> const &mCmp) const {
	for (uint32_t i = 0; i < NumComps(); ++i) {
		if (data[i] != mCmp.data[i])
			return false;
	}
	return true;
}

template<typename Type>
inline bool Matrix2x2T<Type>::operator!= (Matrix2x2T<Type> const &mCmp) const {
	return !(*this == mCmp);
}

template<typename Type>
inline constexpr uint32_t Matrix2x2T<Type>::NumComps() {
	return 4;
}

template<typename Type>
inline Matrix2x2T<Type> operator * (Type s, Matrix2x2T<Type> const &m) { return m * s; }
template<typename Type>
inline Matrix2x2T<Type> operator / (Type s, Matrix2x2T<Type> const &m) { return m / s; }

//---------
//Quat impl


template<typename Type>
inline constexpr QuaternionT<Type>::QuaternionT() : ijks(0, 0, 0, 1) {}

//Initializes the quaternion by setting its direct values
template<typename Type>
inline QuaternionT<Type>::QuaternionT(Type i, Type j, Type k, Type s) : ijks(i,j,k,s) {  }

template<typename Type>
inline QuaternionT<Type>::QuaternionT(Type const * vIn, Type s) : ijks(vIn[0], vIn[1], vIn[2], s) {  }

template<typename Type>
inline QuaternionT<Type>::QuaternionT(Vector3T<Type> const & vIn, Type s) : ijks(vIn, s) { }

template<typename Type>
inline QuaternionT<Type>::QuaternionT(Type const * vIn) : ijks(vIn) {  }

template<typename Type>
inline QuaternionT<Type>::QuaternionT(Vector3T<Type> const & euler) { Orientation(euler); }

template<typename Type>
inline QuaternionT<Type>::QuaternionT(Vector4T<Type> const & v4) : ijks(v4) {  }

//Initializes the quaternion by converting a matrix
template<typename Type>
inline QuaternionT<Type>::QuaternionT(Matrix3x3T<Type> const & r) { *this = r; }

template<typename Type>
inline QuaternionT<Type>::QuaternionT(Matrix4x4T<Type> const & r) { *this = r; }

//Initializes the quaterion with an orientation determined by a rotation in radians and an axis
template<typename Type>
inline QuaternionT<Type>::QuaternionT(Type radians, Type x, Type y, Type z, bool normalize) { Orientation(radians, x, y, z, normalize); }

template<typename Type>
inline QuaternionT<Type>::QuaternionT(Type radians, weave::Vector3T<Type> const & axis, bool normalize) { Orientation(radians, axis.x, axis.y, axis.z, normalize); }

template<typename Type>
inline void QuaternionT<Type>::Set(Type x, Type y, Type z, Type w) { ijks.Set(x, y, z, w); }

template<typename Type>
inline void QuaternionT<Type>::Set(Type x, Type const * vIn) { this->s = x; ijk = vIn; }

//QuaternionT conjugate
template<typename Type>
inline void QuaternionT<Type>::Conjugate() {
	ijk = -ijk;
}

//Normalization
template<typename Type>
inline void QuaternionT<Type>::Normalize() {
	Type mag = weave::algebra::length(ijks);

	if(mag == 0.0f) {
		s = 1;
		return;
	}

	ijks /= mag;
}

//Inversion
template<typename Type>
inline void QuaternionT<Type>::Invert() {
	Type maginv = 1 / weave::algebra::length(ijks);

	ijk = -ijk; //Conjugate
	ijks *= maginv; //Scalar mult
}

template<typename Type>
inline QuaternionT<Type> QuaternionT<Type>::Inverted() const {
	QuaternionT q = *this;
	q.Invert();
	return q;
}

//Sets the quaternion identity (0,0,0,1)
template<typename Type>
inline constexpr void QuaternionT<Type>::Identity() {
	ijks.Set(0, 0, 0, 1);
}

//Sets the orientation of a quaternion given an angle in radians and a rotation vector
template<typename Type>
inline void QuaternionT<Type>::Orientation(Type radians, Type x, Type y, Type z, bool normalize) {
	//float radians = deg2rad(deg)/2.0f;
	radians *= Type(0.5);
	Type sinus = std::sin(radians);
	if(normalize) {
		Type mag = Type(1) / std::sqrt(x*x + y*y + z*z);
		x *= mag; y *= mag; z *= mag;
	}

	s = std::cos(radians);
	i = x*sinus;
	j = y*sinus;
	k = z*sinus;
}

template<typename Type>
inline void QuaternionT<Type>::Orientation(Type radians, weave::Vector3T<Type> const & axis, bool normalize) { Orientation(radians, axis.x, axis.y, axis.z, normalize); }

//Sets the orientation of a quaternion given euler angles in radians
template<typename Type>
inline void QuaternionT<Type>::Orientation(Type pitch, Type yaw, Type roll) {
	//Create 3 quaternions, one per axis. 
	QuaternionT qX(pitch, 1, 0, 0, false);
	QuaternionT qY(yaw, 0, 1, 0, false);
	QuaternionT qZ(roll, 0, 0, 1, false);

	//Multiply them together
	*this = qX * qY * qZ;

	this->Normalize();
}

template<typename Type>
inline void QuaternionT<Type>::Orientation(weave::Vector3T<Type> const & pitchYawRoll) { Orientation(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z); }

//Rotates the quaternion given a scaled vector that contains the axis and angle in radians
template<typename Type>
inline void QuaternionT<Type>::RotateScaled(weave::Vector3T<Type> const & scaledVector) {
	/*
	//Create a quaternion out of the desired rotation
	QuaternionT q(0,scaledVector);
	//Multiply the quaternion to apply the transformation
	q *= *this;
	s += q.s * 0.5f;
	v.x += q.v.x * 0.5f;
	v.y += q.v.y * 0.5f;
	v.z += q.v.z * 0.5f;
	*/

	Type mag = weave::algebra::length(scaledVector);
	if(mag > 0) {
		Type rad = mag*Type(0.5);
		Type sinus = std::sin(rad) / mag;

		QuaternionT q(std::cos(rad),
					  scaledVector.x*sinus,
					  scaledVector.y*sinus,
					  scaledVector.z*sinus);
		//this->Mult(q);
		*this *= q;
	}
}

//Rotates the quaternion by the defined axis and angle in radians
template<typename Type>
inline void QuaternionT<Type>::Rotate(Type radians, Type x, Type y, Type z, bool normalize) {
	QuaternionT q;
	q.Orientation(radians, x, y, z, normalize);

	//this->Mult(q);
	*this = q * *this;
}

template<typename Type>
inline void QuaternionT<Type>::Rotate(Type radians, weave::Vector3T<Type> const & axis, bool normalize) { Rotate(radians, axis.x, axis.y, axis.z, normalize); }

//Rotates the quaternion by the defined euler angles in radians
template<typename Type>
inline void QuaternionT<Type>::Rotate(Type pitch, Type yaw, Type roll) {
	//Create 3 quaternions, one per axis. 
	QuaternionT qX(pitch, 1, 0, 0, false);
	QuaternionT qY(yaw, 0, 1, 0, false);
	QuaternionT qZ(roll, 0, 0, 1, false);

	//Multiply them together
	*this *= qX * qY * qZ;
}

template<typename Type>
inline void QuaternionT<Type>::Rotate(weave::Vector3T<Type> const & pitchYawRoll) { Rotate(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z); }

//Rotates applying only a pitch rotation
template<typename Type>
inline void QuaternionT<Type>::RotatePitch(Type pitch) {
	QuaternionT qX(pitch, 1, 0, 0, false);

	//Multiply the orientation
	*this *= qX;
}

//Rotates applying only a yaw rotation
template<typename Type>
inline void QuaternionT<Type>::RotateYaw(Type yaw) {
	QuaternionT qY(yaw, 0, 1, 0, false);

	//Multiply the orientation
	*this *= qY;
}

//Rotates applying only a roll rotation
template<typename Type>
inline void QuaternionT<Type>::RotateRoll(Type roll) {
	QuaternionT qZ(roll, 0, 0, 1, false);

	//Multiply the orientation
	*this *= qZ;
}

template<typename Type>
inline void QuaternionT<Type>::ToEuler(Type & pitch, Type & yaw, Type & roll) const {
	Type r11 = -2 * (j*k - s*i);
	Type r12 = s*s - i*i - j*j + k*k;
	Type r21 = 2 * (i*k + s*j);
	Type r31 = -2 * (i*j - s*k);
	Type r32 = s*s + i*i - j*j - k*k;

	pitch = std::atan2(r11, r12);
	yaw = std::asin(algebra::clamp(r21, Type {-1}, Type {1}));
	roll = std::atan2(r31, r32);
}

template<typename Type>
weave::Vector3T<Type> QuaternionT<Type>::ToEuler() const {
	weave::Vector3T<Type> pitchYawRoll;
	ToEuler(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
	return pitchYawRoll;
}

//Convert quaternion to 3x3 rotational matrix
template<typename Type>
inline Matrix3x3T<Type> QuaternionT<Type>::ToMatrix() const {
	Type ds = Type(2)*s;
	Type vxx = Type(2) * i * i;
	Type vxy = Type(2) * i * j;
	Type vxz = Type(2) * i * k;
	Type vyy = Type(2) * j * j;
	Type vyz = Type(2) * j * k;
	Type vzz = Type(2) * k * k;

	return {
		Type(1) - vyy - vzz, vxy + ds * k, vxz - ds * j,
		vxy - ds * k, Type(1) - vxx - vzz, vyz + ds * i,
		vxz + ds * j, vyz - ds * i, Type(1) - vxx - vyy
	};
	/*
	m[0] = Type(1) - vyy - vzz;
	m[3] = vxy - ds*k;
	m[6] = vxz + ds*j;

	m[1] = vxy + ds*k;
	m[4] = Type(1) - vxx - vzz;
	m[7] = vyz - ds*i;

	m[2] = vxz - ds*j;
	m[5] = vyz + ds*i;
	m[8] = Type(1) - vxx - vyy;
	*/
}

//Convert quaternion to 4x4 rotational matrix
template<typename Type>
inline Matrix4x4T<Type> QuaternionT<Type>::ToMatrix4() const {
	Type ds = Type(2)*s;
	Type vxx = Type(2) * i * i;
	Type vxy = Type(2) * i * j;
	Type vxz = Type(2) * i * k;
	Type vyy = Type(2) * j * j;
	Type vyz = Type(2) * j * k;
	Type vzz = Type(2) * k * k;

	return {
		Type(1) - vyy - vzz, vxy + ds * k, vxz - ds * j, 0,
		vxy - ds * k, Type(1) - vxx - vzz, vyz + ds * i, 0,
		vxz + ds * j, vyz - ds * i, Type(1) - vxx - vyy, 0,
		0,0,0,Type(1)
	};
}

//Convert a 3x3 matrix to quaternion
template<typename Type>
inline void QuaternionT<Type>::FromMatrix(Matrix3x3T<Type> const & m) {
	Type trace = m[0] + m[4] + m[8] + Type(1.0);

	if(trace > Type(0.00000001)) {
		//Trace is greater than 0, can instantly calculate the quaternion
		s = std::sqrt(trace) * 2;

		i = (m[5] - m[7]) / s;
		j = (m[6] - m[2]) / s;
		k = (m[1] - m[3]) / s;
		s = s * 0.25f;
	}
	else {
		//Trace is zero, must identify the diagonal element with the highest value.
		if(m[0] > m[4] && m[0] > m[8]) {
			// Column 0: 
			s = std::sqrt(Type(1.0) + m[0] - m[4] - m[8]) * 2.0f;
			i = 0.25f * s;
			j = (m[1] + m[3]) / s;
			k = (m[6] + m[2]) / s;
			s = (m[5] - m[7]) / s;
		}
		else if(m[4] > m[8]) {
			// Column 1: 
			s = std::sqrt(Type(1.0) + m[4] - m[0] - m[8]) * 2.0f;
			i = (m[1] + m[3]) / s;
			j = 0.25f * s;
			k = (m[5] + m[7]) / s;
			s = (m[6] - m[2]) / s;
		}
		else {						// Column 2:
			s = std::sqrt(Type(1.0) + m[8] - m[0] - m[4]) * 2.0f;
			i = (m[6] + m[2]) / s;
			j = (m[5] + m[7]) / s;
			k = 0.25f * s;
			s = (m[1] - m[3]) / s;
		}
	}

	Normalize();
}

//Convert a 4x4 matrix to quaternion (ignoring translation)
template<typename Type>
inline void QuaternionT<Type>::FromMatrix(Matrix4x4T<Type> const & m) {
	Type trace = m[0] + m[5] + m[10] + Type(1.0);

	if(trace > 0.00000001f) {
		//Trace is greater than 0, can instantly calculate the quaternion
		s = std::sqrt(trace) * 2;

		i = (m[6] - m[9]) / s;
		j = (m[8] - m[2]) / s;
		k = (m[1] - m[4]) / s;
		s = s * 0.25f;
	}
	else {
		//Trace is zero, must identify the the diagonal element with the highest value.
		if(m[0] > m[5] && m[0] > m[10]) {
			// Column 0: 
			s = std::sqrt(Type(1.0) + m[0] - m[5] - m[10]) * 2.0f;
			i = 0.25f * s;
			j = (m[1] + m[4]) / s;
			k = (m[8] + m[2]) / s;
			s = (m[6] - m[9]) / s;
		}
		else if(m[5] > m[10]) {
			// Column 1: 
			s = std::sqrt(Type(1.0) + m[5] - m[0] - m[10]) * 2.0f;
			i = (m[1] + m[4]) / s;
			j = 0.25f * s;
			k = (m[6] + m[9]) / s;
			s = (m[8] - m[2]) / s;
		}
		else {						// Column 2:
			s = std::sqrt(Type(1.0) + m[10] - m[0] - m[5]) * 2.0f;
			i = (m[8] + m[2]) / s;
			j = (m[6] + m[9]) / s;
			k = 0.25f * s;
			s = (m[1] - m[4]) / s;
		}
	}

	//Normalize the quaternion
	Normalize();
}

//Transforms a vector and returns the result 
template<typename Type>
inline Vector3T<Type> QuaternionT<Type>::Transform(Vector3T<Type> const & v) const {
	//return Q * (vector in quaternion form) * Q conjugate
	return ((*this)*(QuaternionT(v,0)*~(*this))).ijk;
}

template<typename Type>
inline QuaternionT<Type>& QuaternionT<Type>::operator = (QuaternionT<Type> const &q) { ijks = q.ijks; return *this; }
template<typename Type>
inline QuaternionT<Type>& QuaternionT<Type>::operator = (QuaternionT<Type> &&q) { ijks = q.ijks; return *this; }

template<typename Type>
inline QuaternionT<Type>& QuaternionT<Type>::operator = (Type const *vIn) { ijks = vIn; return *this; }
template<typename Type>
inline QuaternionT<Type>& QuaternionT<Type>::operator = (Vector4T<Type> const &vIn) { ijks = vIn; return *this; }
template<typename Type>
inline QuaternionT<Type>& QuaternionT<Type>::operator = (Matrix3x3T<Type> const &m) { this->FromMatrix(m); return *this; }
template<typename Type>
inline QuaternionT<Type>& QuaternionT<Type>::operator = (Matrix4x4T<Type> const &m) { this->FromMatrix(m); return *this; }
template<typename Type>
inline QuaternionT<Type>& QuaternionT<Type>::operator += (QuaternionT<Type> const &q) { ijks += q.ijks; return *this; }
template<typename Type>
inline QuaternionT<Type>& QuaternionT<Type>::operator -= (QuaternionT<Type> const &q) { ijks -= q.ijks; return *this; }
template<typename Type>
inline QuaternionT<Type>& QuaternionT<Type>::operator *= (Type sc) { ijks *= sc; return *this; }
template<typename Type>
inline QuaternionT<Type>& QuaternionT<Type>::operator *= (QuaternionT<Type> const &q2) { *this = (*this) * q2; return *this; }

template<typename Type>
inline QuaternionT<Type>& QuaternionT<Type>::operator /= (Vector3T<Type> const &vIn) { ijk /= vIn; return *this; }
template<typename Type>
inline QuaternionT<Type>& QuaternionT<Type>::operator /= (Type x) { ijks /= x; return *this; }
template<typename Type>
inline QuaternionT<Type> QuaternionT<Type>::operator + (QuaternionT<Type> const &qIn) const { return QuaternionT(ijks + qIn.ijks); }
template<typename Type>
inline QuaternionT<Type> QuaternionT<Type>::operator - (QuaternionT<Type> const &qIn) const { return QuaternionT(ijks - qIn.ijks); }
template<typename Type>
inline QuaternionT<Type> QuaternionT<Type>::operator - () const { return QuaternionT(-ijks); }
template<typename Type>
inline QuaternionT<Type> QuaternionT<Type>::operator ~ () const { return QuaternionT(-ijk, s); }
template<typename Type>
inline QuaternionT<Type> QuaternionT<Type>::operator + (Type x) const { return QuaternionT(ijk, s + x); }
template<typename Type>
inline QuaternionT<Type> QuaternionT<Type>::operator - (Type x) const { return QuaternionT(ijk, s - x); }
template<typename Type>
inline QuaternionT<Type> QuaternionT<Type>::operator * (QuaternionT<Type> const &q2) const { 
	return {
		s*q2.i + i*q2.s + j*q2.k - k*q2.j,
		s*q2.j + j*q2.s + k*q2.i - i*q2.k,
		s*q2.k + k*q2.s + i*q2.j - j*q2.i,
		s*q2.s - i*q2.i - j*q2.j - k*q2.k
	};
}
template<typename Type>
inline QuaternionT<Type> QuaternionT<Type>::operator * (Type x) const { return QuaternionT(ijks*x); }
template<typename Type>
inline Vector3T<Type> QuaternionT<Type>::operator * (Vector3T<Type> const &vIn) const { return Transform(vIn); }
template<typename Type>
inline Vector3T<Type> QuaternionT<Type>::operator * (Vector4T<Type> const &vIn) const { return Transform(vIn.xyz); }
template<typename Type>
inline QuaternionT<Type> QuaternionT<Type>::operator / (Type x)  const { return QuaternionT(ijks / x); }
template<typename Type>
inline bool QuaternionT<Type>::operator == (QuaternionT<Type> const &vIn) const { return (ijks == vIn.ijks).All(); }
template<typename Type>
inline bool QuaternionT<Type>::operator != (QuaternionT<Type> const &vIn) const { return !((ijks == vIn.ijks).All()); }

template<typename Type>
template<typename Index>
inline Type QuaternionT<Type>::operator [] (Index i) const { return ijks[i]; }
template<typename Type>
template<typename Index>
inline Type& QuaternionT<Type>::operator[] (Index i) { return ijks[i]; }

template<typename Type>
inline constexpr uint32_t QuaternionT<Type>::NumComps() {
	return 4;
}

template<typename Type>
inline QuaternionT<Type> operator * (Type s, QuaternionT<Type> const &q) { return q*s; }
template<typename Type>
inline QuaternionT<Type> operator * (Vector3T<Type> const &v1, QuaternionT<Type> const &q) { return QuaternionT<Type>(v1, 0) * q; }
template<typename Type>
inline QuaternionT<Type> operator * (Vector4T<Type> const &v1, QuaternionT<Type> const &q) { return QuaternionT<Type>(v1.xyz, 0) * q; }

} //namespace weave

namespace weave::types {
	
	template<>
	struct TypeTraits<Vector2>
		: DataTypeTraits<DataType::Float_2> {
	};

	template<>
	struct TypeTraits<Vector3>
		: DataTypeTraits<DataType::Float_3> {
	};

	template<>
	struct TypeTraits<Vector4>
		: DataTypeTraits<DataType::Float_4> {
	};

	template<>
	struct TypeTraits<DVector2>
		: DataTypeTraits<DataType::Double_2> {
	};

	template<>
	struct TypeTraits<DVector3>
		: DataTypeTraits<DataType::Double_3> {
	};

	template<>
	struct TypeTraits<DVector4>
		: DataTypeTraits<DataType::Double_4> {
	};

	template<>
	struct TypeTraits<IVector2>
		: DataTypeTraits<DataType::Int32_2> {
	};

	template<>
	struct TypeTraits<IVector3>
		: DataTypeTraits<DataType::Int32_3> {
	};

	template<>
	struct TypeTraits<IVector4>
		: DataTypeTraits<DataType::Int32_4> {
	};

	template<>
	struct TypeTraits<UVector2>
		: DataTypeTraits<DataType::UInt32_2> {
	};

	template<>
	struct TypeTraits<UVector3>
		: DataTypeTraits<DataType::UInt32_3> {
	};

	template<>
	struct TypeTraits<UVector4>
		: DataTypeTraits<DataType::UInt32_4> {
	};

	template<>
	struct TypeTraits<BVector2>
		: DataTypeTraits<DataType::Bool_2> {
	};

	template<>
	struct TypeTraits<BVector3>
		: DataTypeTraits<DataType::Bool_3> {
	};

	template<>
	struct TypeTraits<BVector4>
		: DataTypeTraits<DataType::Bool_4> {
	};

	template<>
	struct TypeTraits<Matrix2x2>
		: DataTypeTraits<DataType::Float_2x2> {
	};

	template<>
	struct TypeTraits<Matrix3x3>
		: DataTypeTraits<DataType::Float_3x3> {
	};

	template<>
	struct TypeTraits<Matrix4x4>
		: DataTypeTraits<DataType::Float_4x4> {
	};

	template<>
	struct TypeTraits<DMatrix2x2>
		: DataTypeTraits<DataType::Double_2x2> {
	};

	template<>
	struct TypeTraits<DMatrix3x3>
		: DataTypeTraits<DataType::Double_3x3> {
	};

	template<>
	struct TypeTraits<DMatrix4x4>
		: DataTypeTraits<DataType::Double_4x4> {
	};

	template<>
	struct TypeTraits<Quaternion>
		: DataTypeTraits<DataType::Float_4> {
	};

	template<>
	struct TypeTraits<DQuaternion>
		: DataTypeTraits<DataType::Double_4> {
	};

} // namespace weave::types


#ifdef _MSC_VER
#pragma warning( pop )
#endif
