#include "midgard/ellipse.h"
#include "midgard/point2.h"
#include <cstdint>

#include "midgard/aabb2.h"
#include "midgard/linesegment2.h"

#include <gtest/gtest.h>

using namespace std;
using namespace valhalla::midgard;

namespace {

void TryLineIntersection(const Ellipse<Point2>& a,
                         const LineSegment2<Point2>& line,
                         const uint32_t expected) {
  Point2 p1, p2;
  EXPECT_EQ(a.Intersect(line, p1, p2), expected)
      << "Ellipse: LineSegment intersect test failed: expected: " + std::to_string(expected);
}

TEST(Ellipse, TestLineIntersection) {
  Ellipse<Point2> e({1, 1}, {5, -1}, 45.0f);

  // TODO - could check intersect points
  TryLineIntersection(e, LineSegment2<Point2>({-5, 5}, {5, 5}), 0);
  TryLineIntersection(e, LineSegment2<Point2>({-5, 0}, {5, 0}), 2);
  TryLineIntersection(e, LineSegment2<Point2>({3, 0}, {3, 5}), 1);
}

void TryAABBIntersection(const Ellipse<Point2>& a,
                         const AABB2<Point2>& box,
                         const IntersectCase expected) {
  EXPECT_EQ(a.DoesIntersect(box), expected)
      << "Ellipse AABB2 does intersect test failed: expected: " + std::to_string(expected);
}

TEST(Ellipse, TestAABBIntersection) {
  Ellipse<Point2> e({1, 1}, {5, -1}, 45.0f);

  // Test for ellipse containing rectangle
  TryAABBIntersection(e, AABB2<Point2>({2.5f, -0.5f}, {3.5f, 0.5f}), IntersectCase::kContains);

  // Test for ellipse within rectangle
  TryAABBIntersection(e, AABB2<Point2>({-2, -2}, {6, 3}), IntersectCase::kWithin);

  // Test for rectangle outside ellipse
  TryAABBIntersection(e, AABB2<Point2>({-2, -2}, {0, 3}), IntersectCase::kOutside);

  // Test for rectangle intersecting ellipse
  TryAABBIntersection(e, AABB2<Point2>({1, 0}, {3, 1}), IntersectCase::kIntersects);
}

void TryContains(const Ellipse<Point2>& a, const Point2& pt, const bool expected) {
  EXPECT_EQ(a.Contains(pt), expected)
      << "Ellipse Contains test failed: expected: " + std::to_string(expected);
}

TEST(Ellipse, TestContains) {
  Ellipse<Point2> e({1, 1}, {5, -1}, 45.0f);
  TryContains(e, {3, 0}, true);
  TryContains(e, {1, 0}, false);
  TryContains(e, {5, 0}, false);
  TryContains(e, {3, 1}, true);
  TryContains(e, {3, -1}, true);
}

} // namespace

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}