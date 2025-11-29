/*
3D Primitive Mesh builders
Simple 3D object constructors for MeshData
*/

#pragma once

#include "weave/system/math/VectorMath.h"
#include "MeshData.h"

namespace weave::graphics {

	enum class Planes {
		XY,
		XZ,
		YZ,
		YX,
		ZX,
		ZY
	};

	//Creates a single point
	MeshData Point(Vector3 const &point = Vector3());

	//Creates a square (billboard)
	//@btmleft, btmright, topright, topleft : Points of the square. Defaults to unit square facing +Z
	MeshData Square(bool uv = false, Vector3 const &btmleft = Vector3(-0.5f, -0.5f, 0.0f), Vector3 const &btmright = Vector3(0.5f, -0.5f, 0.0f), Vector3 const &topright = Vector3(0.5f, 0.5f, 0.0f), Vector3 const &topleft = Vector3(-0.5f, 0.5f, 0.0f));
	
	//Creates a 2x2 quad. Useful for full screen effects.
	MeshData ScreenQuad(bool uv = false);

	//Creates a plane
	MeshData Plane(float sizex = 1.0f, float sizey = 1.0f, unsigned int samplesx = 10, unsigned int samplesy = 10, Planes axis = Planes::XZ, float displace = 0.0f);

	//Creates a box of arbitrary size.
	MeshData Box(float sizex = 1.0f, float sizey = 1.0f, float sizez = 1.0f, unsigned int samplesx = 10, unsigned int samplesy = 10, unsigned int samplesz = 10, bool insideOut = false);

	//Creates a stacks & slices sphere
	MeshData Sphere(float radius = 1.0f, unsigned int stacks = 4, unsigned int slices = 8);

	//Creates a cylinder
	//MeshData Cylinder();

	//Creates a pyramid
	MeshData Frustum(float fovy, float ratio, float zNear, float zFar);

	//Creates a pyramid
	MeshData Pyramid(float height = 1.0f, float baseX = 1.0f, float baseZ = 1.0f);

	//Creates an uneven double sided pyramid
	MeshData Octahedron(float heightUp = 1.0f, float heightDown = 1.0f, float baseX = 1.0f, float baseZ = 1.0f);
}

