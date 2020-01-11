#include <cstdint>

#include "baldr/graphtile.h"

#include <vector>

#include <gtest/gtest.h>

using namespace valhalla::baldr;

namespace {

struct testable_graphtile : public valhalla::baldr::GraphTile {
  testable_graphtile(const uint32_t (&offsets)[kBinCount], std::vector<GraphId>& bins) {
    header_ = new GraphTileHeader();
    header_->set_edge_bin_offsets(offsets);
    edge_bins_ = bins.data();
  }
};

TEST(Graphtile, FileSuffix) {
  EXPECT_EQ(GraphTile::FileSuffix(GraphId(2, 2, 0)), "2/000/000/002.gph");
  EXPECT_EQ(GraphTile::FileSuffix(GraphId(4, 2, 0)), "2/000/000/004.gph");
  EXPECT_EQ(GraphTile::FileSuffix(GraphId(1197468, 2, 0)), "2/001/197/468.gph");
  EXPECT_EQ(GraphTile::FileSuffix(GraphId(64799, 1, 0)), "1/064/799.gph");
  EXPECT_EQ(GraphTile::FileSuffix(GraphId(49, 0, 0)), "0/000/049.gph");
  EXPECT_EQ(GraphTile::FileSuffix(GraphId(1000000, 3, 1)), "3/001/000/000.gph");
}

TEST(Graphtile, IdFromString) {
  EXPECT_EQ(GraphTile::GetTileId("foo/bar/baz/qux/corge/1/000/002.gph"), GraphId(2, 1, 0));
  EXPECT_EQ(GraphTile::GetTileId("foo2/8675309/bar/1baz2/qux42corge/1/000/002.gph"),
            GraphId(2, 1, 0));
  EXPECT_EQ(GraphTile::GetTileId("foo2/8675309/bar/1baz2/qux42corge/2/001/000/002.gph"),
            GraphId(1000002, 2, 0));
  EXPECT_EQ(GraphTile::GetTileId("foo2/8675309/bar/1baz2/qux42corge/3/001/000/002.gph"),
            GraphId(1000002, 3, 0));
  EXPECT_EQ(GraphTile::GetTileId("foo2/8675309/bar/1baz2/qux42corge/3/001/000/002"),
            GraphId(1000002, 3, 0));
  EXPECT_EQ(GraphTile::GetTileId("2/000/791/317.gph.gz"), GraphId(791317, 2, 0));

  EXPECT_THROW(GraphTile::GetTileId("foo2/8675309/bar/1baz2/qux42corge/1/000/002/.gph"),
               std::runtime_error);
  EXPECT_THROW(GraphTile::GetTileId("foo2/8675309/bar/1baz2/qux42corge/0/004/050.gph"),
               std::runtime_error);
  EXPECT_THROW(GraphTile::GetTileId("foo/bar/0/004/0-1.gph"), std::runtime_error);
  EXPECT_THROW(GraphTile::GetTileId("foo/bar/0/004//001.gph"), std::runtime_error);
  EXPECT_THROW(GraphTile::GetTileId("foo/bar/1/000/004/001.gph"), std::runtime_error);
  EXPECT_THROW(GraphTile::GetTileId("00/002.gph"), std::runtime_error);
}

TEST(Graphtile, Bin) {
  uint32_t offsets[kBinCount] = {1, 2, 3, 0, 1, 2, 3, 1, 1, 2, 3, 2, 1,
                                 2, 3, 3, 1, 2, 3, 4, 1, 2, 3, 5, 1};
  std::vector<uint32_t> offs = {0};
  std::vector<GraphId> bins;
  uint32_t offset = 0;
  for (size_t i = 0, j; i < kBinCount; ++i) {
    offset += offsets[i];
    offs.push_back(offset);
    offsets[i] = offset;
    for (size_t k = 0; k < offsets[i]; ++k)
      bins.emplace_back(j++);
  }
  testable_graphtile t(offsets, bins);
  for (size_t i = 0; i < kBinCount; ++i) {
    valhalla::midgard::iterable_t<GraphId> itr(bins.data() + offs[i], bins.data() + offs[i + 1]);
    auto idx_itr = t.GetBin(i);
    auto rc_itr = t.GetBin(i % kBinsDim, i / kBinsDim);

    EXPECT_EQ(itr.size(), idx_itr.size()) << "Wrong bin!";
    EXPECT_EQ(itr.size(), rc_itr.size()) << "Wrong bin!";

    for (auto j = itr.begin(), k = idx_itr.begin(), l = rc_itr.begin(); j != itr.end();
         ++j, ++k, ++l) {
      EXPECT_EQ(*j, *k) << "Wrong edge found in bin";
      EXPECT_EQ(*j, *l) << "Wrong edge found in bin";
    }
  }
}

TEST(GraphTileIntegrity, SizeZero) {
  std::vector<char> tile_data(0);
  EXPECT_THROW(GraphTile tile(GraphId(), tile_data.data(), 0), std::runtime_error);
}

TEST(GraphTileIntegrity, SizeLessThanHeader) {
  size_t tileSize = sizeof(GraphTileHeader) - 1;
  std::vector<char> tile_data(tileSize);
  EXPECT_THROW(GraphTile tile(GraphId(), tile_data.data(), tileSize), std::runtime_error);
}

TEST(GraphTileIntegrity, SizeLessThanPayload) {
  size_t tile_size = 10000;

  GraphTileHeader header;
  // set offset not equal to data size
  header.set_end_offset(tile_size - 1);

  std::vector<char> tile_data(tile_size);
  memcpy(tile_data.data(), &header, sizeof(header));

  EXPECT_THROW(GraphTile tile(GraphId(), tile_data.data(), tile_size), std::runtime_error);
}

} // namespace

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
