#include "Geometry.h"

using namespace weave;
using namespace weave::algebra;

vec2 geometry::centroid(vec2 const *p, unsigned int pcount) {
	vec2 accum;
	for(unsigned int i=0; i<pcount; ++i) {
		accum += p[i];
	}

	accum /= float(pcount);

	return accum;
}

vec3 geometry::centroid(vec3 const *p, unsigned int pcount) {
	vec3 accum;
	for(unsigned int i=0; i<pcount; ++i) {
		accum += p[i];
	}

	accum /= float(pcount);

	return accum;
}

vec4 geometry::centroid(vec4 const *p, unsigned int pcount) {
	vec4 accum;
	for(unsigned int i=0; i<pcount; ++i) {
		accum += p[i];
	}

	accum /= float(pcount);

	return accum;
}

//Calculates the circumcenter of a triangle (the center of the circle that contains all three points of a triangle)
//Algorithm adapted from: http://www.delphitricks.com/source-code/math/determine_the_circumcenter_of_a_2d_triangle.html
vec2 geometry::circumcenter(vec2 const &p1, vec2 const &p2, vec2 const &p3) {
	vec2 v = p2 - p1;
	vec2 w = p3 - p1;

	float e = v.x*(p1.x + p2.x) + v.y*(p1.y + p3.y);
	float f = w.x*(p1.x + p2.x) + w.y*(p1.y + p3.y);

	float g = 2.0f * (v.x*(p3.y - p2.y) - v.y*(p3.x - p2.x));
	if(g == 0.0f)
		return p1;
	else
		return vec2((w.y*e - v.x*f) / g, (v.x*f - w.x*e) / g);
}

//Algorithm adapted from: http://mathforum.org/kb/message.jspa?messageID=4602531 and http://mathforum.org/library/drmath/view/62814.html
vec3 geometry::circumcenter(vec3 const &p1, vec3 const &p2, vec3 const &p3) {
	vec3 v = p2 - p1;
	vec3 w = p3 - p1;

	vec3 normal = algebra::cross(v, w);

	vec3 mid21 = (p1 + p2)*0.5f;
	vec3 mid31 = (p1 + p3)*0.5f;
	//Perpendicular vectors to v and w on the plane given by normal
	vec3 perpv = cross(normal, v);
	vec3 perpw = cross(normal, w);

	//Two lines are defined now by:
	//L1 = mid21 + t1*perpv
	//L2 = mid31 + t2*perpw
	//The circumcenter is the point where these two lines meet
	float t = length(cross((mid31 - mid21), perpw)) / length(cross(perpv, perpw));
	return mid21 + t * perpv;

}

//Point in triangle algorithm taken from: http://www.peroxide.dk/papers/collision/collision.pdf
//Returns true if the supplied point is inside the triangle defined by pA,pB,PC
//NOTE: This looks rather dangerous as it's directly assuming 32bit ints and converting floats to those. Take care.
#define in(a) ((uint32_t&) a)
unsigned int geometry::pointInTriangle(vec3 const &point, vec3 const &pa, vec3 const &pb, vec3 const &pc) {
	vec3 e10 = pb-pa;
	vec3 e20 = pc-pa;
	float a = dot(e10,e10);
	float b = dot(e10,e20);
	float c = dot(e20,e20);
	float ac_bb = (a*c) - (b*b);
	
	vec3 vp(point.x - pa.x, point.y - pa.y, point.z - pa.z);
	float d = dot(vp,e10);
	float e = dot(vp,e20);
	float x = (d*c) - (e*b);
	float y = (e*a) - (d*b);
	float z = x + y - ac_bb;
	return (( in(z)& ~(in(x)|in(y)) ) & 0x80000000);
}
#undef in