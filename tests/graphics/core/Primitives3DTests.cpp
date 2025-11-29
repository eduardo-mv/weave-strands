#include "tests/TestEntryPoints.h"

#include "weave/graphics/core/Primitives3D.h"

namespace weave::tests::graphics::core {

TestReport TestPrimitives3D() {
	using namespace weave::graphics;

	TestReport report;

	auto point = Point();
	report.Expect(point.Primitive() == MeshPrimitive::Points, "Point primitive type mismatch");
	report.Expect(point.GetIndex().IsIndexless(), "Point should be indexless");
	report.Expect(point.GetSections().size() == 1, "Point mesh should create a section");

	auto square = Square(false);
	report.Expect(square.Primitive() == MeshPrimitive::TriStrip, "Square should be built as triangle strip");
	report.Expect(square.GetIndex().IsIndexless(), "Square should be indexless");
	report.Expect(square.GetAttributes().size() == 1, "Square without UVs should expose a single attribute");

	auto squareUv = Square(true);
	report.Expect(squareUv.GetAttributes().size() == 2, "Square with UVs should expose two attributes");

	auto quad = ScreenQuad(true);
	report.Expect(quad.GetAttributes().size() == 2, "Screen quad with UVs should expose positions and texcoords");
	report.Expect(quad.GetIndex().count == 4, "Screen quad should expose four vertices via implicit index");

    auto plane = weave::graphics::Plane(1.0f, 1.0f, 3, 2, Planes::XZ, 0.0f);
	report.Expect(plane.HasIndex(), "Plane should use an index buffer");
	report.Expect(plane.GetAttributes().size() == 3, "Plane should emit position, UV, and normal attributes");
	report.Expect(plane.GetIndex().count == (3 - 1) * (2 - 1) * 6, "Plane index count mismatch");

    auto invalidPlane = weave::graphics::Plane(1.0f, 1.0f, 1, 1, Planes::XZ, 0.0f);
	report.Expect(invalidPlane.GetAttributes().empty(), "Plane with insufficient samples should be empty");

	auto box = Box(1.0f, 1.0f, 1.0f, 2, 2, 2, false);
	report.Expect(box.HasIndex(), "Box should build an index buffer");
	report.Expect(box.GetAttributes().size() == 3, "Box should provide position, UV, and normal attributes");
	report.Expect(box.GetSections().size() == 1, "Box should define a single section");

	auto sphere = Sphere(1.0f, 3, 4);
	report.Expect(sphere.HasIndex(), "Sphere should be indexed");
	report.Expect(sphere.GetAttributes().size() == 4, "Sphere should provide four attributes (position, UV, normal, spherical UV)");
	report.Expect(sphere.GetIndex().count > 0, "Sphere index should have data");

	auto invalidSphere = Sphere(1.0f, 1, 2);
	report.Expect(invalidSphere.GetAttributes().empty(), "Sphere with insufficient stacks/slices should be empty");

	auto frustum = Frustum(0.5f, 1.0f, 0.1f, 1.0f);
	report.Expect(frustum.GetIndex().count == 36, "Frustum should emit 12 triangles / 36 indices");
	report.Expect(frustum.GetAttributes().size() == 3, "Frustum should expose three attributes");

	auto pyramid = Pyramid();
	report.Expect(pyramid.GetIndex().count == 18, "Pyramid should emit six triangles / 18 indices");

	auto octahedron = Octahedron();
	report.Expect(octahedron.GetIndex().count == 24, "Octahedron should emit eight triangles / 24 indices");

	return report;
}

} // namespace weave::tests::graphics::core
