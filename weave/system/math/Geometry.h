/*
Title: "Geometry support library"
File: Geometry.h
Author(s): Eduardo Martínez Vidal

Abstract:
	General geometry library with general purpose geometry algorythms

Update Log:
	

*/

#pragma once

#include "VectorMath.h"

namespace weave {

namespace geometry {
	using namespace algebra;

	//Return the centroid of a triangle (the point where the medians of the triangle meet)
	inline vec2 centroid(vec2 const &p1, vec2 const &p2, vec2 const &p3) { return vec2((p1.x + p2.x + p3.x) / 3.0f, (p1.y + p2.y + p3.y) / 3.0f); }
	inline vec3 centroid(vec3 const &p1, vec3 const &p2, vec3 const &p3) { return vec3((p1.x + p2.x + p3.x) / 3.0f, (p1.y + p2.y + p3.y) / 3.0f, (p1.z + p2.z + p3.z) / 3.0f); }
	inline vec4 centroid(vec4 const &p1, vec4 const &p2, vec4 const &p3) { return vec4((p1.x + p2.x + p3.x) / 3.0f, (p1.y + p2.y + p3.y) / 3.0f, (p1.z + p2.z + p3.z) / 3.0f, (p1.w + p2.w + p3.w) / 3.0f); }

	//The centroid of a polygon
	vec2 centroid(vec2 const *p, unsigned int pcount);
	vec3 centroid(vec3 const *p, unsigned int pcount);
	vec4 centroid(vec4 const *p, unsigned int pcount);

	//Calculates the circumcenter of a triangle (the center of the circle that contains all three points of a triangle)
	vec2 circumcenter(vec2 const &p1, vec2 const &p2, vec2 const &p3);
	vec3 circumcenter(vec3 const &p1, vec3 const &p2, vec3 const &p3);

	//Returns true if the supplied point is inside the triangle defined by pA,pB,PC
	unsigned int pointInTriangle(vec3 const &point, vec3 const &pa, vec3 const &pb, vec3 const &pc);
	//Performs a raycasting algorithm to find if point is inside a polygon determined by an array of vec2.
	//Adapted from http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
	inline bool pointInPolygon(vec2 const &point, vec2 const *poly, unsigned int pcount) {
		bool inside = false;
		for(unsigned int i = 0, j = pcount - 1; i < pcount; j = i++) {
			if(((poly[i].y > point.y) != (poly[j].y > point.y)) &&
			   (point.x < (poly[j].x - poly[i].x) * (point.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x)) {
				inside = !inside;
			}
		}
		return inside;
	}

	//See http://geomalgorithms.com/a02-_lines.html#Distance-to-Ray-or-Segment for point to line/ray/segment information
	//Returns the distance between a point and a segment
	template<typename vecr>
	float pointSegmentDistanceSqr(vecr const &point, vecr const &line0, vecr const &line1) {
		//Make sure the line is actually a line and not a point. If it is, just return the distance to that point
		vecr line = line1 - line0;
		vecr w = point - line0;
		float d = lengthSqr(line);
		if(d == 0.0f) {
			return lengthSqr(w);
		}
		//Find the position along the line vector starting from line0 and given by line where the closest point to point lies by projecting the point on the line vector
		float t = dot(w, line) / d;
		if(t < 0.0f) {
			//Point is outside the segment, before line0. The closest point within the segment is the line0
			return lengthSqr(w);
		}
		else if(t > 1.0f) {
			//Point is outside the segment after line1, making line1 the closest point within the segment
			return lengthSqr(point - line1);
		}

		//The closest point within the segment 
		vecr p = line0 + t * line;

		return weave::algebra::lengthSqr(point - p);
	}

	template<typename vecr>
	inline float pointSegmentDistance(vecr const &point, vecr const &line0, vecr const &line1) {
		return sqrtf(pointSegmentDistanceSqr(point, line0, line1));
	}

	//Returns the distance between a point and a ray
	template<typename vecr>
	float pointRayDistanceSqr(vecr const &point, vecr const &p0ray, vecr const &vray) {
		//The vector between the point and the starting point of the ray
		vecr w = point - p0ray;
		//Projection of the vector onto the ray
		float t = dot(w, vray);
		if(t < 0.0f) {
			//Point is outside the ray, before p0. The closest point within the ray is the p0
			return lengthSqr(w);
		}
		
		//The closest point within the ray
		vecr p = p0ray + t * vray;

		return weave::algebra::lengthSqr(point - p);
	}

	template<typename vecr>
	inline float pointRayDistance(vecr const &point, vecr const &p0ray, vecr const &vray) {
		return sqrtf(pointRayDistanceSqr(point, p0ray, vray));
	}

	//Returns the distance between a point and an infinite line (in both directions)
	template<typename vecr>
	float pointLineDistanceSqr(vecr const &point, vecr const &pointInLine, vecr const & lineVector) {
		vecr w = point - pointInLine;
		float d = algebra::lengthSqr(algebra::cross(lineVector, w));
		return d;
	}

	template<typename vecr>
	float pointLineDistance(vecr const &point, vecr const &pointInLine, vecr const & lineVector) {
		return sqrt(pointLineDistanceSqr(point, pointInLine, lineVector));
	}


	//Returns the closest point on a segment from an arbitrary point
	template<typename vecr>
	vecr closestPointOnSegment(vecr const &point, vecr const &line0, vecr const &line1) {
		//Make sure the line is actually a line and not a point. If it is, just return that point
		vecr line = line1 - line0;
		float d = lengthSqr(line);
		if(d == 0.0f) {
			return line0;
		}

		vecr w = point - line0;

		//Find the position along the line vector starting from line0 and given by line where the closest point to point lies by projecting the point on the line vector
		float t = dot(w, line) / d;
		if(t < 0.0f) {
			//Point is outside the segment, before line0. The closest point within the segment is the line0
			return line0;
		}
		else if(t > 1.0f) {
			//Point is outside the segment after line1, making line1 the closest point within the segment
			return line1;
		}

		//The closest point within the segment 
		return line0 + t * line;
	}

	//Returns the closest displacement t along a ray from an arbitrary point
	template<typename vecr>
	float closestPointAlongRay(vecr const &point, vecr const &p0ray, vecr const &vray) {
		//The vector between the point and the starting point of the ray
		vecr w = point - p0ray;
		//Projection of the vector onto the ray
		float t = dot(w, vray);
		if(t < 0.0f) {
			//Point is outside the ray, before p0. The closest point within the ray is the p0
			return 0.0f;
		}

		//The closest point within the ray
		return t;
	}

	//Returns the closest point on a ray from an arbitrary point
	template<typename vecr>
	vecr closestPointOnRay(vecr const &point, vecr const &p0ray, vecr const &vray) {
		//The vector between the point and the starting point of the ray
		vecr w = point - p0ray;
		//Projection of the vector onto the ray
		float t = dot(w, vray);
		if(t < 0.0f) {
			//Point is outside the ray, before p0. The closest point within the ray is the p0
			return p0ray;
		}

		//The closest point within the ray
		return p0ray + t * vray;
	}

	//Returns the closest point on a line from an arbitrary point
	template<typename vecr>
	vecr closestPointOnLine(vecr const &point, vecr const &line0, vecr const &line1) {
		//Make sure the line is actually a line and not a point. If it is, just return that point
		vecr line = line1 - line0;
		float d = lengthSqr(line);
		if(d == 0.0f) {
			return line0;
		}

		vecr w = point - line0;

		//Find the position along the line vector starting from line0 and given by line where the closest point to point lies by projecting the point on the line vector
		float t = dot(w, line) / d;

		//The closest point within the segment 
		return line0 + t * line;
	}

	//Distance of a point to a plane
	template<typename Type>
	Type distancePointToPlane(Vector3T<Type> const &point, PlaneT<Type> const  &plane) {
		return algebra::dot(plane.xyz, point) + plane.w;    //Ax+By+Cz+D=distance;
	}

	//Distance algorythms for line, segments and rays: http://geomalgorithms.com/a07-_distance.html
	//Returns the shortest distance between two lines
	template<typename vecr>
	float distanceLineToLine(vecr const &lineA0, vecr const &lineA1, vecr const &lineB0, vecr const &lineB1) {
		//Implementation of the technique defined in http://geomalgorithms.com/a07-_distance.html
		vecr u = lineA1 - lineA0;
		vecr v = lineB1 - lineB0;
		vecr w = lineA0 - lineB0;

		float a = dot(u, u);
		float b = dot(u, v);
		float c = dot(v, v);
		float d = dot(u, w);
		float e = dot(v, w);
		float D = a*c - b*b;

		//sc is the displacement along u where one of the points of the shortest line between the two lines lies, tc is the other point on line w
		float sc, tc;
		//The denominator will be 0 when lines are parallel, so we solve by arbitrarily choosing sc to be 0
		if(D < minFloat) {
			sc = 0.0f;
			tc = (b > c ? d / b : e / c);
		}
		else {
			sc = (b*e - c*d) / D;
			tc = (a*e - b*d) / D;
		}
		//The vector between the two points discovered
		vecr shortVec = w + sc*u - tc*v;

		return length(shortVec);
	}


	//Returns the shortest distance between two segments
	template<typename vecr>
	float distanceSegmentToSegment(vecr const &segmentA0, vecr const &segmentA1, vecr const &segmentB0, vecr const &segmentB1) {
		//Implementation of the technique defined in http://geomalgorithms.com/a07-_distance.html
		//The same technique as the Line to Line distance is used first
		vecr   u = segmentA1 - segmentA0;
		vecr   v = segmentB1 - segmentB0;
		vecr   w = segmentA0 - segmentB0;
		float    a = dot(u, u);         // always >= 0
		float    b = dot(u, v);
		float    c = dot(v, v);         // always >= 0
		float    d = dot(u, w);
		float    e = dot(v, w);
		float    D = a*c - b*b;        // always >= 0
		float    sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
		float    tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

									   //The denominator will be 0 when lines are parallel, so we solve by arbitrarily choosing sc to be 0
		if(D < minFloat) { // the lines are almost parallel
			sN = 0.0f;         // force using point P0 on segment S1
			sD = 1.0f;         // to prevent possible division by 0.0 later
			tN = e;
			tD = c;
		}
		else {                 // get the closest points on the infinite lines
			sN = (b*e - c*d);
			tN = (a*e - b*d);
			if(sN < 0.0f) {        // sc < 0 => the s=0 edge is visible
				sN = 0.0f;
				tN = e;
				tD = c;
			}
			else if(sN > sD) {  // sc > 1  => the s=1 edge is visible
				sN = sD;
				tN = e + b;
				tD = c;
			}
		}

		if(tN < 0.0f) {            // tc < 0 => the t=0 edge is visible
			tN = 0.0f;
			// recompute sc for this edge
			if(-d < 0.0f)
				sN = 0.0f;
			else if(-d > a)
				sN = sD;
			else {
				sN = -d;
				sD = a;
			}
		}
		else if(tN > tD) {      // tc > 1  => the t=1 edge is visible
			tN = tD;
			// recompute sc for this edge
			if((-d + b) < 0.0f)
				sN = 0;
			else if((-d + b) > a)
				sN = sD;
			else {
				sN = (-d + b);
				sD = a;
			}
		}
		// finally do the division to get sc and tc
		sc = (std::abs(sN) < minFloat ? 0.0f : sN / sD);
		tc = (std::abs(tN) < minFloat ? 0.0f : tN / tD);

		// get the difference of the two closest points
		vecr dP = w + (sc * u) - (tc * v);  // =  S1(sc) - S2(tc)

		return length(dP);   // return the closest distance
	}

	//Returns the shortest distance between a segment and a line (note the order!)
	template<typename vecr>
	float distanceSegmentToLine(vecr const &segment0, vecr const &segment1, vecr const &line0, vecr const &line1) {
		//Implementation of the technique defined in http://geomalgorithms.com/a07-_distance.html
		//The same technique as the Line to Line distance is used first
		vecr   u = segment1 - segment0;
		vecr   v = line1 - line0;
		vecr   w = segment0 - line0;
		float    a = dot(u, u);         // always >= 0
		float    b = dot(u, v);
		float    c = dot(v, v);         // always >= 0
		float    d = dot(u, w);
		float    e = dot(v, w);
		float    D = a*c - b*b;        // always >= 0
		float    sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
		float    tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

									   //The denominator will be 0 when lines are parallel, so we solve by arbitrarily choosing sc to be 0
		if(D < minFloat) { // the lines are almost parallel
			sN = 0.0f;         // force using point P0 on segment S1
			sD = 1.0f;         // to prevent possible division by 0.0 later
			tN = e;
			tD = c;
		}
		else {                 // get the closest points on the infinite lines
			sN = (b*e - c*d);
			tN = (a*e - b*d);
			if(sN < 0.0f) {        // sc < 0 => the s=0 edge is visible
				sN = 0.0f;
				tN = e;
				tD = c;
			}
			else if(sN > sD) {  // sc > 1  => the s=1 edge is visible
				sN = sD;
				tN = e + b;
				tD = c;
			}
		}

		// finally do the division to get sc and tc
		sc = (std::abs(sN) < minFloat ? 0.0f : sN / sD);
		tc = (std::abs(tN) < minFloat ? 0.0f : tN / tD);

		// get the difference of the two closest points
		vecr dP = w + (sc * u) - (tc * v);  // =  S1(sc) - S2(tc)

		return length(dP);   // return the closest distance
	}

	//Returns the shortest distance between a line and a segment (note the order!)
	template<typename vecr>
	float distanceLineToSegment(vecr const &line0, vecr const &line1, vecr const &segment0, vecr const &segment1) {
		return distanceSegmentToLine(segment0, segment1, line0, line1);
	}

	//Returns the shortest distance between a segment and a ray (note the order!)
	template<typename vecr>
	float distanceSegmentToRay(vecr const &segmentA0, vecr const &segmentA1, vecr const &p0ray, vecr const &vray) {
		//Implementation of the technique defined in http://geomalgorithms.com/a07-_distance.html
		//The same technique as the Line to Line distance is used first
		vecr   u = segmentA1 - segmentA0;
		vecr   v = vray;
		vecr   w = segmentA0 - p0ray;
		float    a = dot(u, u);         // always >= 0
		float    b = dot(u, v);
		float    c = dot(v, v);         // always >= 0
		float    d = dot(u, w);
		float    e = dot(v, w);
		float    D = a*c - b*b;        // always >= 0
		float    sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
		float    tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

									   //The denominator will be 0 when lines are parallel, so we solve by arbitrarily choosing sc to be 0
		if(D < minFloat) { // the lines are almost parallel
			sN = 0.0f;         // force using point P0 on segment S1
			sD = 1.0f;         // to prevent possible division by 0.0 later
			tN = e;
			tD = c;
		}
		else {                 // get the closest points on the infinite lines
			sN = (b*e - c*d);
			tN = (a*e - b*d);
			if(sN < 0.0f) {        // sc < 0 => the s=0 edge is visible
				sN = 0.0f;
				tN = e;
				tD = c;
			}
			else if(sN > sD) {  // sc > 1  => the s=1 edge is visible
				sN = sD;
				tN = e + b;
				tD = c;
			}
		}

		if(tN < 0.0f) {            // tc < 0 => the t=0 edge is visible
			tN = 0.0f;
			// recompute sc for this edge
			if(-d < 0.0f)
				sN = 0.0f;
			else if(-d > a)
				sN = sD;
			else {
				sN = -d;
				sD = a;
			}
		}

		// finally do the division to get sc and tc
		sc = (std::abs(sN) < minFloat ? 0.0f : sN / sD);
		tc = (std::abs(tN) < minFloat ? 0.0f : tN / tD);

		// get the difference of the two closest points
		vecr dP = w + (sc * u) - (tc * v);  // =  S1(sc) - S2(tc)

		return length(dP);   // return the closest distance
	}

	//Returns the shortest distance between a ray and a segment (note the order!)
	template<typename vecr>
	float distanceRayToSegment(vecr const &p0ray, vecr const &vray, vecr const &segmentA0, vecr const &segmentA1) {
		return distanceSegmentToRay(segmentA0, segmentA1, p0ray, vray);
	}

	//Returns the shortest distance between two rays
	template<typename vecr>
	float distanceRayToRay(vecr const &p0rayA, vecr const &vrayA, vecr const &p0rayB, vecr const &vrayB) {
		//Implementation of the technique defined in http://geomalgorithms.com/a07-_distance.html
		//The same technique as the Line to Line distance is used first
		vecr   u = vrayA;
		vecr   v = vrayB;
		vecr   w = p0rayA - p0rayB;
		float    a = dot(u, u);         // always >= 0
		float    b = dot(u, v);
		float    c = dot(v, v);         // always >= 0
		float    d = dot(u, w);
		float    e = dot(v, w);
		float    D = a*c - b*b;        // always >= 0
		float    sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
		float    tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

									   //The denominator will be 0 when lines are parallel, so we solve by arbitrarily choosing sc to be 0
		if(D < minFloat) { // the lines are almost parallel
			sN = 0.0f;         // force using point P0 on segment S1
			sD = 1.0f;         // to prevent possible division by 0.0 later
			tN = e;
			tD = c;
		}
		else {                 // get the closest points on the infinite lines
			sN = (b*e - c*d);
			tN = (a*e - b*d);
			if(sN < 0.0f) {        // sc < 0 => the s=0 edge is visible
				sN = 0.0f;
				tN = e;
				tD = c;
			}
		}

		if(tN < 0.0f) {            // tc < 0 => the t=0 edge is visible
			tN = 0.0f;
			// recompute sc for this edge
			if(-d < 0.0f)
				sN = 0.0f;
			else if(-d > a)
				sN = sD;
			else {
				sN = -d;
				sD = a;
			}
		}

		// finally do the division to get sc and tc
		sc = (std::abs(sN) < minFloat ? 0.0f : sN / sD);
		tc = (std::abs(tN) < minFloat ? 0.0f : tN / tD);

		// get the difference of the two closest points
		vecr dP = w + (sc * u) - (tc * v);  // =  S1(sc) - S2(tc)

		return length(dP);   // return the closest distance
	}


	//Returns true if the 2D line intersects the circle area (including containment)..
	//Adapted from http://mathworld.wolfram.com/Circle-LineIntersection.html
	inline bool intersectLineCircle(vec2 const &p0, vec2 const &p1, vec2 const &circlePos, float radius) {
		float D = (p0.x - circlePos.x)*(p1.y - circlePos.y) - (p1.x - circlePos.x)*(p0.y - circlePos.y);
		return (radius*radius * weave::algebra::lengthSqr(p1 - p0) - D*D) >= 0.0f;
	}

	//Returns 0, or 2 if the 2D line intersects the circle area (including containment), representing the amount of intersection points
	//Returns the point(s) of intersection in outp0 and outp1
	//Adapted from http://mathworld.wolfram.com/Circle-LineIntersection.html
	inline int intersectLineCircle(vec2 const &p0, vec2 const &p1, vec2 const &circlePos, float radius, vec2 &outp0, vec2 &outp1) {
		vec2 d = p1 - p0;
		float D = (p0.x - circlePos.x)*(p1.y - circlePos.y) - (p1.x - circlePos.x)*(p0.y - circlePos.y); //Line points are made relative to the circle's center
		float lenSqr = algebra::lengthSqr(d); //dr^2
		float rSqr = radius * radius; //r^2
		float delta = (rSqr * lenSqr - D * D);

		//Discriminant is negative -> no intersection
		if(delta < 0.0f)
			return 0;

		float deltaRoot = std::sqrt(delta);

		//Solve
		float sgndy = d.y < 0.0f ? -1.0f : 1.0f;
		outp0.x = (D * d.y - sgndy * d.x * deltaRoot) / lenSqr;
		outp0.y = (-D * d.x - std::abs(d.y) * deltaRoot) / lenSqr;

		outp1.x = (D * d.y + sgndy * d.x * deltaRoot) / lenSqr;
		outp1.y = (-D * d.x + std::abs(d.y) * deltaRoot) / lenSqr;

		outp0 += circlePos;
		outp1 += circlePos;

		return delta == 0.0f ? 1 : 2;
	}

	//Returns true if the 2D segment and the circle intersect
	inline bool intersectSegmentCircle(vec2 const &p0, vec2 const &p1, vec2 const &circlePos, float radius) {
		return pointSegmentDistanceSqr(circlePos, p0, p1) < radius*radius;
	}

	//Returns true if a polygon intersects the defined circle
	inline bool intersectPolyCircle(vec2 const *poly, unsigned int pcount, vec2 const &circlePos, float radius) {
		bool inside = false;
		for(unsigned int i = 0, j = pcount - 1; i < pcount; j = i++) {
			if(intersectSegmentCircle(poly[j], poly[i], circlePos, radius))
			   return true;
			if(((poly[i].y > circlePos.y) != (poly[j].y > circlePos.y)) &&
			   (circlePos.x < (poly[j].x - poly[i].x) * (circlePos.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x)) {
				inside = !inside;
			}
		}
		return inside;
	}

	//Returns true if a segment intersects a plane and returns the intersection point
	template<typename Type>
	inline bool intersectSegmentPlane(Vector3T<Type> const &p1, Vector3T<Type> const &p2, PlaneT<Type> const &plane, Vector3T<Type> &cpoint) {
		//We need the distance to the plane for each point
		Type dist1 = distancePointToPlane(p1, plane);
		Type dist2 = distancePointToPlane(p2, plane);
		//No intersection if they are on the same side
		if(dist1 * dist2 >= 0)
			return false;

		//Compute the intersection point
		Vector3T<Type> line = p2 - p1;
		Type mag = -dist1 / algebra::dot(plane.xyz, line);
		cpoint = p1 + (line * mag);
		return true;
	}

	//Returns true if two 2D lines intersect each other and the intersection point
	//Algorithm adapted from: https://stackoverflow.com/questions/563198/whats-the-most-efficent-way-to-calculate-where-two-line-segments-intersect/565282#565282
	template<typename Type>
	bool intersectLines2D(Vector2T<Type> const &segmentA0, Vector2T<Type> const &segmentA1, Vector2T<Type> const &segmentB0, Vector2T<Type> const &segmentB1, Vector2T<Type> &outPoint) {
		Vector2T<Type> s1 = segmentA1 - segmentA0;
		Vector2T<Type> s2 = segmentB1 - segmentB0;
		Type s = (-s1.y * (segmentA0.x - segmentB0.x) + s1.x * (segmentA0.y - segmentB0.y)) / (-s2.x * s1.y + s1.x * s2.y);
		Type t = (s2.x * (segmentA0.y - segmentB0.y) - s2.y * (segmentA0.x - segmentB0.x)) / (-s2.x * s1.y + s1.x * s2.y);

		if(s >= 0 && s <= 1 && t >= 0 && t <= 1) {
			//Collision detected
			outPoint.Set(segmentA0.x + (t * s1.x), segmentA0.y + (t * s1.y));
			return true;
		}

		return false; //No collision
	}
}

} //namespace weave
