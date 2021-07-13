#include <catch2/catch.hpp>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>

#include "geofeatures.hpp"
#include "geom.hpp"
#include "raster.hpp"
#include "find_intersections_linestring.hpp"

#define TOL 0.001

using linestr = std::vector<geometry::Vec2<double>>;

TEST_CASE("LineStrings are decomposed", "[decomposition]") {

  auto test_data = GENERATE(table<linestr, std::vector<linestr>>({
	{
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
	  // Linestring
	  {{0.5, 0.5}, {0.75, 0.5}, {1.5, 0.5}, {1.5, 1.5}},
	  // Expected vector of splits
	  {
	    {{0.5, 0.5}, {0.75, 0.5},{1., 0.5}},
	    {{1., 0.5},{1.5, 0.5},{1.5, 1.}},
	    {{1.5, 1.}, {1.5, 1.5}}
	  }
	},
	{
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
	  // Linestring
	  {{0.5, 0.5}, {0.75, 0.5}, {1.5, 1.5}},
	  // Expected vector of splits
	  {
	    {{0.5, 0.5}, {0.75, 0.5}, {1., 0.8333}},
	    {{1., 0.8333}, {1.125, 1.}},
	    {{1.125, 1.}, {1.5, 1.5}}
	  }
	}
      })
    );

  std::vector<linestr> expected_splits = std::get<1>(test_data);

  Feature f;
  linestr geom = std::get<0>(test_data);
  f.geometry.insert(f.geometry.begin(), geom.begin(), geom.end());

  Ascii test_raster("./tests/test_data/fake_raster.asc");
  std::vector<linestr> splits = findIntersectionsLineString(f, test_raster);

  // Test that we're getting the expected number of splits
  REQUIRE(splits.size() == expected_splits.size());
  // Test that each one of the splits have the expected size
  for (int i = 0; i < splits.size(); i++) {
    REQUIRE(splits[i].size() == expected_splits[i].size());
  }
  // Test that each one of the splits are made of the expected points
  for (int i = 0; i < splits.size(); i++) {
    for (int j = 0; j < splits[i].size(); j++) {
      geometry::Vec2<double> point = splits[i][j];
      geometry::Vec2<double> expected_point = expected_splits[i][j];

      REQUIRE(std::abs(point.x - expected_point.x) < TOL);
      REQUIRE(std::abs(point.y - expected_point.y) < TOL);
    }
  }
}
