#include <catch2/catch.hpp>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "geometry.hpp"
#include "grid.hpp"
#include "transform.hpp"
#include "operations.hpp"

#define TOL 0.001

using linestr = std::vector<snail::geometry::Vec2<double>>;

struct Config {
  linestr linestring;
  std::vector<linestr> expected_splits;
};

TEST_CASE("LineStrings are decomposed", "[decomposition]") {

  // Linestring points are marked by o:
  // Intersection points are marked by (o):
  // +---------------+--------------+
  // |               |              |
  // |               |              |
  // |               |              |
  // |               |       o      |
  // |               |       |      |
  // |               |       |      |
  // |               |       |      |
  // +---------------+------(o)-----+
  // |               |       |      |
  // |               |       |      |
  // |               |       |      |
  // |       o---o--(o)------o      |
  // |               |              |
  // |               |              |
  // |               |              |
  // +---------------+--------------+
  // (0,0)         (1,0)          (2,0)
  Config case1;
  case1.linestring = {{0.5, 0.5}, {0.75, 0.5}, {1.5, 0.5}, {1.5, 1.5}};
  case1.expected_splits = {{{0.5, 0.5}, {0.75, 0.5}, {1., 0.5}},
			   {{1., 0.5}, {1.5, 0.5}, {1.5, 1.}},
			   {{1.5, 1.}, {1.5, 1.5}}};

  // Linestring points are marked by o:
  // Intersection points are marked by (o):
  // +---------------+--------------+
  // |               |              |
  // |               |              |
  // |               |              |
  // |               |       o      |
  // |               |      /       |
  // |               |    /-        |
  // |               |  /-          |
  // +---------------+(o)-----------+
  // |              (o)             |
  // |             /-|              |
  // |            /  |              |
  // |       o---o   |              |
  // |               |              |
  // |               |              |
  // |               |              |
  // +---------------+--------------+
  // (0,0)         (1,0)          (2,0)
  Config case2;
  case2.linestring = {{0.5, 0.5}, {0.75, 0.5}, {1.5, 1.5}};
  case2.expected_splits = {{{0.5, 0.5}, {0.75, 0.5}, {1., 0.8333}},
			   {{1., 0.8333}, {1.125, 1.}},
			   {{1.125, 1.}, {1.5, 1.5}}};

  // Linestring points are marked by o:
  // Intersection points are marked by (o):
  // +---------------+------(o)-----+
  // |               |       |      |
  // |               |       |      |
  // |               |       |      |
  // |               |       |      |
  // |               |       |      |
  // |               |       |      |
  // |               |       |      |
  // +---------------+------(o)-----+
  // |               |     /        |
  // |               |    /         |
  // |               |   /          |
  // |              (o)-/           |
  // |               |              |
  // |               |              |
  // |               |              |
  // +---------------+--------------+
  // (0,0)         (1,0)          (2,0)
  Config case3;
  case3.linestring = {{1.0, 0.5}, {1.5, 1.0}, {1.5, 2.0}};
  case3.expected_splits = {{{1.0, 0.5}, {1.5, 1.0}},
			   {{1.5, 1.0}, {1.5, 2.0}}};

  // Linestring points are marked by o:
  // Intersection points are marked by (o):
  // +---------------+--------------+
  // |               |              |
  // |               |              |
  // |               |              |
  // |               |       o      |
  // |               |              |
  // |               |              |
  // |               |              |
  // +--------------(o)--------------+
  // |               |              |
  // |               |              |
  // |               |              |
  // |       o       |              |
  // |               |              |
  // |               |              |
  // |               |              |
  // +---------------+--------------+
  // (0,0)         (1,0)          (2,0)
  Config case4;
  case4.linestring = {{0.5, 0.5}, {1.5, 1.5}};
  case4.expected_splits = {{{0.5, 0.5}, {1.0, 1.0}},
			   {{1.0, 1.0}, {1.5, 1.5}}};

  auto test_data = GENERATE_COPY(case1, case2, case3, case4);

  std::vector<linestr> expected_splits = test_data.expected_splits;

  linestr geom = test_data.linestring;;
  snail::geometry::LineString<double> line(geom);

  // Using default Affine transform(1, 0, 0, 0, 1, 0)
  snail::grid::Grid test_raster(2, 2, snail::transform::Affine());
  std::vector<linestr> splits = snail::operations::findIntersectionsLineString(line, test_raster);

  // Test that we're getting the expected number of splits
  REQUIRE(splits.size() == expected_splits.size());
  // Test that each one of the splits have the expected size
  for (int i = 0; i < splits.size(); i++) {
    REQUIRE(splits[i].size() == expected_splits[i].size());
  }
  // Test that each one of the splits are made of the expected points
  for (int i = 0; i < splits.size(); i++) {
    for (int j = 0; j < splits[i].size(); j++) {
      snail::geometry::Vec2<double> point = splits[i][j];
      snail::geometry::Vec2<double> expected_point = expected_splits[i][j];

      REQUIRE(std::abs(point.x - expected_point.x) < TOL);
      REQUIRE(std::abs(point.y - expected_point.y) < TOL);
    }
  }
}
