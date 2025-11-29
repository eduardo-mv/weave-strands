#include "Primitives3D.h"

using namespace weave;
using namespace graphics;

MeshData weave::graphics::Point(Vector3 const & point) {
	MeshData mesh;
	auto stream = mesh.AllocateStream(sizeof(Vector3), sizeof(Vector3), &point);
	mesh.AddAttribute({ MeshAttribute::Label::Position, 0, weave::types::DataType::Float_3, 0, stream });
	mesh.NoIndex(1);
	mesh.SingleSection(0, 0);
	mesh.SetPrimitive(MeshPrimitive::Points);

	return mesh;
}

MeshData weave::graphics::Square(bool uv, Vector3 const & btmleft, Vector3 const & btmright, Vector3 const & topright, Vector3 const & topleft) {
	MeshData mesh;
	if(uv) {
		float quv[] = {
			btmleft.x, btmleft.y, btmleft.z, 0.0, 0.0,
			btmright.x, btmright.y, btmright.z, +1.0, 0.0,
			topleft.x, topleft.y, topleft.z, 0.0, +1.0,
			topright.x, topright.y, topright.z, +1.0, +1.0
		};

		auto stream = mesh.AllocateStream(sizeof(float) * 5, sizeof(quv), quv);
		mesh.AddAttribute({ MeshAttribute::Label::Position, 0, weave::types::DataType::Float_3, 0, stream });
		mesh.AddAttribute({ MeshAttribute::Label::UVCoord, 1, weave::types::DataType::Float_2, 3 * sizeof(float), stream });
	}
	else {
		float q[] = {
			btmleft.x, btmleft.y, btmleft.z, 
			btmright.x, btmright.y, btmright.z, 
			topleft.x, topleft.y, topleft.z,
			topright.x, topright.y, topright.z
		};
		auto stream = mesh.AllocateStream(sizeof(float) * 3, sizeof(q), q);
		mesh.AddAttribute({ MeshAttribute::Label::Position, 0, weave::types::DataType::Float_3, 0, stream });
	}
	
	mesh.NoIndex(4);
	mesh.SingleSection(0, 0);
	mesh.SetPrimitive(MeshPrimitive::TriStrip);

	return mesh;
}

MeshData weave::graphics::ScreenQuad(bool uv) {
	MeshData mesh;
	if(uv) {
		float quv[] = {-1.0, -1.0, 0.0, 0.0,   +1.0, -1.0, +1.0, 0.0,   -1.0, +1.0, 0.0, +1.0,   +1.0, +1.0, +1.0, +1.0};
		auto stream = mesh.AllocateStream(sizeof(float) * 4, sizeof(quv), quv);
		mesh.AddAttribute({ MeshAttribute::Label::Position, ~0u, weave::types::DataType::Float_2, 0, stream });
		mesh.AddAttribute({ MeshAttribute::Label::UVCoord, ~0u, weave::types::DataType::Float_2, 2 * sizeof(float), stream });
	}
	else {
		float q[] = {-1.0, -1.0, +1.0, -1.0, -1.0, +1.0, +1.0, +1.0};
		auto stream = mesh.AllocateStream(sizeof(float) * 2, sizeof(q), q);
		mesh.AddAttribute({ MeshAttribute::Label::Position, ~0u, weave::types::DataType::Float_2, 0, stream });
	}

	mesh.NoIndex(4);
	mesh.SingleSection(0, 0);
	mesh.SetPrimitive(MeshPrimitive::TriStrip);
	
	return mesh;
}

MeshData weave::graphics::Plane(float sizex, float sizey, unsigned int samplesx, unsigned int samplesy, graphics::Planes axis, float displace) {
	MeshData mesh;

	if(samplesx < 2 || samplesy < 2)
		return mesh;

	//Allocate the buffers for vertices and triangle index
	auto vertices = mesh.AllocateStream(3 * sizeof(float), samplesx * samplesy * 3 * sizeof(float));
	auto tcoords = mesh.AllocateStream(2 * sizeof(float), samplesx * samplesy * 2 * sizeof(float));
	auto normals = mesh.AllocateStream(3 * sizeof(float), samplesx * samplesy * 3 * sizeof(float));
	auto index = mesh.AllocateIndex(weave::types::DataType::UInt32, (samplesx - 1)*(samplesy - 1) * 2 * 3, false);

	mesh.AddAttribute({ MeshAttribute::Label::Position, ~0u, weave::types::DataType::Float_3, 0, vertices });
	mesh.AddAttribute({ MeshAttribute::Label::UVCoord, ~0u, weave::types::DataType::Float_2, 0, tcoords });
	mesh.AddAttribute({ MeshAttribute::Label::Normal, ~0u, weave::types::DataType::Float_3, 0, normals });
	mesh.SingleSection(0, 0);
	mesh.SetPrimitive(MeshPrimitive::Triangles);

	float distx = sizex / (samplesx - 1);
	float disty = sizey / (samplesy - 1);
	float cornerx = -sizex * 0.5f;
	float cornery = -sizey * 0.5f;

	unsigned int triangles = 0;
	unsigned int vID = 0; //ID of each vertex to easily keep track of them
	float *v = vertices.bufferView.Memory<float>();
	float *n = normals.bufferView.Memory<float>();
	float *uv = tcoords.bufferView.Memory<float>();
	uint32_t *idx = index.bufferView.Memory<uint32_t>();

	for(unsigned int j = 0; j<samplesy; ++j) {
		for(unsigned int i = 0; i<samplesx; ++i) {
			//Create a vertex
			switch(axis) {
			case Planes::XY:
				v[0] = cornerx + disty*j; v[1] = cornery + distx*i; v[2] = displace;
				n[0] = 0.0f; n[1] = 0.0f; n[2] = 1.0f;
				break;
			case Planes::YX:
				v[0] = cornerx + distx*i; v[1] = cornery + disty*j; v[2] = displace;
				n[0] = 0.0f; n[1] = 0.0f; n[2] = -1.0f;
				break;
			case Planes::YZ:
				v[0] = displace; v[1] = cornery + disty*j; v[2] = cornerx + distx*i;
				n[0] = 1.0f; n[1] = 0.0f; n[2] = 0.0f;
				break;
			case Planes::ZY:
				v[0] = displace; v[1] = cornery + distx*i; v[2] = cornerx + disty*j;
				n[0] = -1.0f; n[1] = 0.0f; n[2] = 0.0f;
				break;
			case Planes::ZX:
				v[0] = cornerx + disty*j; v[1] = displace; v[2] = cornery + distx*i;
				n[0] = 0.0f; n[1] = -1.0f; n[2] = 0.0f;
				break;
			case Planes::XZ:
			default:
				v[0] = cornerx + distx*i; v[1] = displace; v[2] = cornery + disty*j;
				n[0] = 0.0f; n[1] = 1.0f; n[2] = 0.0f;
				break;
			}
			uv[0] = (i / (float)(samplesx - 1));
			uv[1] = (j / (float)(samplesy - 1));

			//Advance the pointers
			v += 3;
			n += 3;
			uv += 2;
			
			//Every new vertex on X after the i==1 we can define our triangles
			if(i > 0 && j < samplesy - 1) {
				idx[0] = vID; idx[1] = vID - 1; idx[2] = vID - 1 + samplesx;
				idx += 3;
				idx[0] = vID; idx[1] = vID - 1 + samplesx; idx[2] = vID + samplesx;
				idx += 3;
				triangles += 2;
			}
			vID++;
		}
	}

	return mesh;
}


MeshData weave::graphics::Box(float sizex, float sizey, float sizez, unsigned int samplesx, unsigned int samplesy, unsigned int samplesz, bool insideOut) {
	MeshData mesh;

	if(samplesx < 2 || samplesy < 2 || samplesz < 2)
		return mesh;
	
	//Create 6 planes assembled
	MeshData plane[6];
	//Plane3D front,back, top,bottom, left,right;
	
	if(!insideOut) {
		//+Z, -Z
		plane[0] = graphics::Plane(sizey,sizex,samplesx,samplesy,graphics::Planes::XY, sizez*0.5f);
		plane[1] = graphics::Plane(sizex,sizey,samplesx,samplesy,graphics::Planes::YX, -sizez*0.5f);
	
		//+Y, - Y
		plane[2] = graphics::Plane(sizex,sizez,samplesx,samplesz,graphics::Planes::XZ, sizey*0.5f);
		plane[3] = graphics::Plane(sizez,sizex,samplesx,samplesz,graphics::Planes::ZX, -sizey*0.5f);
	
		//+X,-X
		plane[4] = graphics::Plane(sizez,sizey,samplesy,samplesz,graphics::Planes::YZ, sizex*0.5f);
		plane[5] = graphics::Plane(sizey,sizez,samplesy,samplesz,graphics::Planes::ZY, -sizex*0.5f);
	}
	else {
		plane[0] = graphics::Plane(sizey,sizex,samplesx,samplesy,graphics::Planes::YX, sizez*0.5f);
		plane[1] = graphics::Plane(sizex,sizey,samplesx,samplesy,graphics::Planes::XY, -sizez*0.5f);
	
		plane[2] = graphics::Plane(sizex,sizez,samplesx,samplesz,graphics::Planes::ZX, sizey*0.5f);
		plane[3] = graphics::Plane(sizez,sizex,samplesx,samplesz,graphics::Planes::XZ, -sizey*0.5f);
	
		plane[4] = graphics::Plane(sizez,sizey,samplesy,samplesz,graphics::Planes::ZY, sizez*0.5f);
		plane[5] = graphics::Plane(sizey,sizez,samplesy,samplesz,graphics::Planes::YZ, -sizez*0.5f);
	}

	//Allocate the buffers for vertices and triangle indices
	auto vertices = mesh.AllocateStream(3 * sizeof(float), 3 * 2 * ((samplesx * samplesy) + (samplesx * samplesz) + (samplesy * samplesz)) * sizeof(float));
	auto tcoords = mesh.AllocateStream(2 * sizeof(float), 2 * 2 * ((samplesx * samplesy) + (samplesx * samplesz) + (samplesy * samplesz)) * sizeof(float));
	auto normals = mesh.AllocateStream(3 * sizeof(float), 3 * 2 * ((samplesx * samplesy) + (samplesx * samplesz) + (samplesy * samplesz)) * sizeof(float));
	auto index = mesh.AllocateIndex(weave::types::DataType::UInt32, 3 * 2 * 2 * ((samplesx - 1) * (samplesy - 1) + (samplesx - 1) * (samplesz - 1) + (samplesy - 1) * (samplesz - 1)), false);

	mesh.AddAttribute({ MeshAttribute::Label::Position, ~0u, weave::types::DataType::Float_3, 0, vertices });
	mesh.AddAttribute({ MeshAttribute::Label::UVCoord, ~0u, weave::types::DataType::Float_2, 0, tcoords });
	mesh.AddAttribute({ MeshAttribute::Label::Normal, ~0u, weave::types::DataType::Float_3, 0, normals });
	mesh.SingleSection(0, 0);
	mesh.SetPrimitive(MeshPrimitive::Triangles);

	//Copy the data to the right buffers
	float *v = vertices.bufferView.Memory<float>();
	float *uv = tcoords.bufferView.Memory<float>();
	float *n = normals.bufferView.Memory<float>();
	uint32_t *idx = index.bufferView.Memory<uint32_t>();

	for(unsigned int i=0; i<6; ++i){
		auto vbuffer = plane[i].GetAttribute(MeshAttribute::Label::Position).stream.bufferView;
		auto uvbuffer = plane[i].GetAttribute(MeshAttribute::Label::UVCoord).stream.bufferView;
		auto nbuffer = plane[i].GetAttribute(MeshAttribute::Label::Normal).stream.bufferView;

		std::memcpy(v, vbuffer.Memory(), vbuffer.byteSize);
		std::memcpy(uv, uvbuffer.Memory(), uvbuffer.byteSize);
		std::memcpy(n, nbuffer.Memory(), nbuffer.byteSize);

		v  += vbuffer.byteSize / sizeof(float);
		uv += uvbuffer.byteSize / sizeof(float);
		n  += nbuffer.byteSize / sizeof(float);

		//Copy the indices adjusting for the offset positions
		auto pidx = plane[i].GetIndex();
		for(uint64_t j=0; j<pidx.count; ++j) {
			*idx = pidx.bufferView.Memory<uint32_t>()[j] + i*samplesx*samplesy;
			idx++;
		}
	}

	return mesh;
}


MeshData weave::graphics::Sphere(float radius, unsigned int stacks, unsigned int slices) {
	MeshData mesh;

	if(stacks < 2 || slices < 3)
		return mesh;

	//Allocate the buffers for vertices and triangle indices
	auto vertices = mesh.AllocateStream(3 * sizeof(float), 3 * ((stacks + 1) * (slices + 1)) * sizeof(float));
	auto tcoords = mesh.AllocateStream(2 * sizeof(float), 2 * ((stacks + 1) * (slices + 1)) * sizeof(float));
	auto normals = mesh.AllocateStream(3 * sizeof(float), 3 * ((stacks + 1) * (slices + 1)) * sizeof(float));
	auto sphcoords = mesh.AllocateStream(2 * sizeof(float), 2 * ((stacks + 1) * (slices + 1)) * sizeof(float));
	auto index = mesh.AllocateIndex(weave::types::DataType::UInt32, 3 * (2 * (slices)+2 * (slices)*(stacks - 2) + 1), false); //Caps have less triangles than the rest of the stacks

	mesh.AddAttribute({ MeshAttribute::Label::Position, ~0u, weave::types::DataType::Float_3, 0, vertices });
	mesh.AddAttribute({ MeshAttribute::Label::UVCoord, ~0u, weave::types::DataType::Float_2, 0, tcoords });
	mesh.AddAttribute({ MeshAttribute::Label::Normal, ~0u, weave::types::DataType::Float_3, 0, normals });
	mesh.AddAttribute({ MeshAttribute::Label::UVCoord, ~0u, weave::types::DataType::Float_2, 0, sphcoords });
	mesh.SingleSection(0, 0);
	mesh.SetPrimitive(MeshPrimitive::Triangles);

	float *v = vertices.bufferView.Memory<float>();
	float *uv = tcoords.bufferView.Memory<float>();
	float *suv = sphcoords.bufferView.Memory<float>();
	float *n = normals.bufferView.Memory<float>();
	uint32_t *idx = index.bufferView.Memory<uint32_t>();
	//uint32_t *idx2 = idx;

	float aangle = 0.0f;
	float bangle = 0.0f;
	float astep = weave::algebra::F_PI / stacks;
	float bstep = weave::algebra::F_2PI / (slices);
	unsigned int vindex = 0;

	stacks++;
	slices++;

	//Build the sphere
	for(unsigned int i = 0; i<stacks; ++i, aangle += astep, bangle = 0.0f) {
		for(unsigned int j = 0; j<slices; ++j, bangle += bstep) {
			float x, y, z;

			x = sinf(aangle)*cosf(bangle);
			z = sinf(aangle)*sinf(bangle);
			y = cos(aangle);

			std::memcpy(v, Vector3(radius*x, radius*y, radius*z).data, sizeof(float) * 3); v += 3;
			std::memcpy(n, Vector3(x, y, z).data, sizeof(float) * 3); n += 3;
			std::memcpy(suv, Vector2(x, z).data, sizeof(float) * 2); suv += 2;
			std::memcpy(uv, Vector2(((float)j) / (slices - 1), ((float)i) / (stacks - 1)).data, sizeof(float) * 2); uv += 2;
			//Triangle winding
			//First row is ignored since there's nothing to winde
			if(i>0) {
				//Winding the cap
				//Second row of vertices create single triangles with the upper row (fused at cap)
				if(i == 1) {
					//Starting from the second vertex as we need the previous one to winde the triangle
					if(j>0) {
						std::memcpy(idx, UVector3(vindex, vindex - 1, vindex - 1 - slices).data, sizeof(uint32_t) * 3); idx += 3;
					}
				}

				//Winding the body
				else if(i>1 && i<stacks - 1) {
					//Starting from the second vertex
					if(j>0) {
						std::memcpy(idx, UVector3(vindex, vindex - 1, vindex - 1 - slices).data, sizeof(uint32_t) * 3); idx += 3;
						std::memcpy(idx, UVector3(vindex, vindex - 1 - slices, vindex - slices).data, sizeof(uint32_t) * 3); idx += 3;
					}
				}
				//Winding the lower cap
				else if(i >= stacks - 1) {
					//Can start from the first vertex as we only need the previous row now
					std::memcpy(idx, UVector3(vindex, vindex - slices, vindex + 1 - slices).data, sizeof(uint32_t) * 3); idx += 3;
				}
			}

			vindex++;
		}
	}

	return mesh;
}

MeshData weave::graphics::Frustum(float fovy, float ratio, float zNear, float zFar)
{
	MeshData mesh;
	
	//Allocate the buffers for vertices and triangle indices
	auto vertices = mesh.AllocateStream(3 * sizeof(float), 24 * sizeof(float));
	auto tcoords = mesh.AllocateStream(2 * sizeof(float), 16 * sizeof(float));
	auto normals = mesh.AllocateStream(3 * sizeof(float), 24 * sizeof(float));
	auto index = mesh.AllocateIndex(weave::types::DataType::UInt32, 36, false);

	mesh.AddAttribute({ MeshAttribute::Label::Position, ~0u, weave::types::DataType::Float_3, 0, vertices });
	mesh.AddAttribute({ MeshAttribute::Label::UVCoord, ~0u, weave::types::DataType::Float_2, 0, tcoords });
	mesh.AddAttribute({ MeshAttribute::Label::Normal, ~0u, weave::types::DataType::Float_3, 0, normals });
	mesh.SingleSection(0, 0);
	mesh.SetPrimitive(MeshPrimitive::Triangles);

	Vector3* v = vertices.bufferView.Memory<Vector3>();
	Vector2* uv = tcoords.bufferView.Memory<Vector2>();
	Vector3* n = normals.bufferView.Memory<Vector3>();
	UVector3* idx = index.bufferView.Memory<UVector3>();


	//Calculate the 8 vertices
	auto zfarznear = zFar / zNear;
	auto up = zNear * std::tan(fovy * 0.5f);
	auto right = up * ratio;
	float s[] = { -1,1 };

	//x = (1,0,0), y = (0,1,0), z = (0,0,1), origin = (0,0,0)
	//Near plane: origin + near*z + s0*up*y + s1*right*x
	for (unsigned int i = 0; i < 2; ++i) {
		for (unsigned int j = 0; j < 2; ++j) {
			v[i * 2 + j].Set(s[j] * right, s[i] * up, -zNear);
			n[i * 2 + j] = algebra::normalize(v[i * 2 + j]);
			uv[i * 2 + j] = Vector2(i * 1.0f, j * 1.0f);
		}
	}
	//Far plane: origin + far*z + (far/near) * (s0*up*y + s1*right*x)
	for (unsigned int i = 0; i < 2; ++i) {
		for (unsigned int j = 0; j < 2; ++j) {
			v[4 + i * 2 + j].Set(zfarznear * s[j] * right, zfarznear * s[i] * up, -zFar);
			n[i * 2 + j] = algebra::normalize(v[i * 2 + j]);
			uv[i * 2 + j] = Vector2(i * 1.0f, j * 1.0f);
		}
	}

	//ZNear
	idx[0] = UVector3(0, 1, 2);
	idx[1] = UVector3(1, 3, 2);
	//ZFar
	idx[2] = UVector3(6, 7, 5);
	idx[3] = UVector3(6, 5, 4);
	//Side X+
	idx[4] = UVector3(1, 5, 3);
	idx[5] = UVector3(3, 5, 7);
	//Side X-
	idx[6] = UVector3(4, 0, 6);
	idx[7] = UVector3(0, 2, 6);
	//Side Y+
	idx[8] = UVector3(2, 3, 6);
	idx[9] = UVector3(3, 7, 6);
	//Side Y-
	idx[10] = UVector3(0, 4, 5);
	idx[11] = UVector3(0, 5, 1);
	
	return mesh;
}

MeshData weave::graphics::Pyramid(float height, float baseX, float baseZ) {
	MeshData mesh;

	//Allocate the buffers for vertices and triangle indices
	auto vertices = mesh.AllocateStream(3 * sizeof(float), 15 * sizeof(float));
	auto tcoords = mesh.AllocateStream(2 * sizeof(float), 10 * sizeof(float));
	auto normals = mesh.AllocateStream(3 * sizeof(float), 15 * sizeof(float));
	auto index = mesh.AllocateIndex(weave::types::DataType::UInt32, 18, false);

	mesh.AddAttribute({ MeshAttribute::Label::Position, ~0u, weave::types::DataType::Float_3, 0, vertices });
	mesh.AddAttribute({ MeshAttribute::Label::UVCoord, ~0u, weave::types::DataType::Float_2, 0, tcoords });
	mesh.AddAttribute({ MeshAttribute::Label::Normal, ~0u, weave::types::DataType::Float_3, 0, normals });
	mesh.SingleSection(0, 0);
	mesh.SetPrimitive(MeshPrimitive::Triangles);

	Vector3* v = vertices.bufferView.Memory<Vector3>();
	Vector2* uv = tcoords.bufferView.Memory<Vector2>();
	Vector3* n = normals.bufferView.Memory<Vector3>();
	UVector3*idx = index.bufferView.Memory<UVector3>();

	//The pyramid's point is set at 0,0,0 and the base is displaced by height
	v[0] = Vector3(0, height, 0);
	v[1] = Vector3(baseX * 0.5f, 0.0f, baseZ * 0.5f);
	v[2] = Vector3(baseX * 0.5f, 0.0f, -baseZ * 0.5f);
	v[3] = Vector3(-baseX * 0.5f, 0.0f, baseZ * 0.5f);
	v[4] = Vector3(-baseX * 0.5f, 0.0f, -baseZ * 0.5f);

	n[0] = algebra::normalize(v[0]);
	n[1] = algebra::normalize(v[1]);
	n[2] = algebra::normalize(v[2]);
	n[3] = algebra::normalize(v[3]);
	n[4] = algebra::normalize(v[4]);

	uv[0] = Vector2(0.5f, 0.5f);
	uv[1] = Vector2(1.0f, 1.0f);
	uv[2] = Vector2(1.0f, 0.0f);
	uv[3] = Vector2(0.0f, 1.0f);
	uv[4] = Vector2(0.0f, 0.0f);

	//The 4 triangles that form the top cone
	idx[0] = UVector3(0, 2, 1);
	idx[1] = UVector3(0, 1, 3);
	idx[2] = UVector3(0, 3, 4);
	idx[3] = UVector3(0, 4, 2);

	//And the base
	idx[4] = UVector3(1, 2, 4);
	idx[5] = UVector3(3, 1, 4);

	return mesh;
}

MeshData weave::graphics::Octahedron(float heightUp, float heightDown, float baseX, float baseZ)
{
	MeshData mesh;

	//Allocate the buffers for vertices and triangle indices
	auto vertices = mesh.AllocateStream(3 * sizeof(float), 18 * sizeof(float));
	auto tcoords = mesh.AllocateStream(2 * sizeof(float), 12 * sizeof(float));
	auto normals = mesh.AllocateStream(3 * sizeof(float), 18 * sizeof(float));
	auto index = mesh.AllocateIndex(weave::types::DataType::UInt32, 24, false);

	mesh.AddAttribute({ MeshAttribute::Label::Position, ~0u, weave::types::DataType::Float_3, 0, vertices });
	mesh.AddAttribute({ MeshAttribute::Label::UVCoord, ~0u, weave::types::DataType::Float_2, 0, tcoords });
	mesh.AddAttribute({ MeshAttribute::Label::Normal, ~0u, weave::types::DataType::Float_3, 0, normals });
	mesh.SingleSection(0, 0);
	mesh.SetPrimitive(MeshPrimitive::Triangles);

	Vector3* v = vertices.bufferView.Memory<Vector3>();
	Vector2* uv = tcoords.bufferView.Memory<Vector2>();
	Vector3* n = normals.bufferView.Memory<Vector3>();
	UVector3* idx = index.bufferView.Memory<UVector3>();

	//The pyramid's point is set at 0,0,0 and the base is displaced by height
	v[0] = Vector3(0, heightUp, 0);
	v[1] = Vector3(baseX * 0.5f, 0.0f, baseZ * 0.5f);
	v[2] = Vector3(baseX * 0.5f, 0.0f, -baseZ * 0.5f);
	v[3] = Vector3(-baseX * 0.5f, 0.0f, baseZ * 0.5f);
	v[4] = Vector3(-baseX * 0.5f, 0.0f, -baseZ * 0.5f);
	v[5] = Vector3(0, -heightDown, 0);

	n[0] = algebra::normalize(v[0]);
	n[1] = algebra::normalize(v[1]);
	n[2] = algebra::normalize(v[2]);
	n[3] = algebra::normalize(v[3]);
	n[4] = algebra::normalize(v[4]);
	n[5] = algebra::normalize(v[5]);

	uv[0] = Vector2(0.5f, 0.5f);
	uv[1] = Vector2(1.0f, 1.0f);
	uv[2] = Vector2(1.0f, 0.0f);
	uv[3] = Vector2(0.0f, 1.0f);
	uv[4] = Vector2(0.0f, 0.0f);
	uv[5] = Vector2(0.5f, 0.5f);

	//The 4 triangles that form the top cone
	idx[0] = UVector3(0, 2, 1);
	idx[1] = UVector3(0, 1, 3);
	idx[2] = UVector3(0, 3, 4);
	idx[3] = UVector3(0, 4, 2);

	//The 4 triangles that form the bottom cone
	idx[4] = UVector3(5, 1, 2);
	idx[5] = UVector3(5, 3, 1);
	idx[6] = UVector3(5, 4, 3);
	idx[7] = UVector3(5, 2, 4);

	return mesh;
}

