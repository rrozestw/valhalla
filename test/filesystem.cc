#include "filesystem.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <vector>

#include <gtest/gtest.h>


// TODO: move this out of the test and into the filesystem replacement
// and then remove filesystem from mjolnir as well
#ifdef _WIN32
#include <windows.h>
int mkdir(const char* dir_name, long) {
  if (!CreateDirectory(dir_name, NULL))
    return -1;
  return 0;
}
constexpr int perm755 = 0;
#else
constexpr int perm755 = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
#endif

namespace {

TEST(Filesystem, exists) {
  // drop a file
  { std::fstream fs(".touched_file", std::ios::out); }
  // check this dir is there
  EXPECT_TRUE(filesystem::exists(".")) << "the current directory doesnt exist?";
  EXPECT_TRUE(filesystem::exists(".touched_file")) << "couldn't create a file?";
}

TEST(Filesystem, directory) {
  // todo: any specific reason it was repeated 2 times?
  EXPECT_TRUE(filesystem::is_directory(".")) << "the current directory isnt a directory?";
  EXPECT_TRUE(filesystem::is_directory(".")) << "the current directory isnt a directory?";
}

TEST(Filesystem, regular_file) {
  EXPECT_TRUE(filesystem::is_regular_file(".touched_file"));
}

void try_mkdir(const std::string& p) {
  EXPECT_EQ(mkdir(p.c_str(), perm755), 0) << "couldnt create directory";
}

TEST(Filesystem, recursive_directory_listing) {
  // make a directory tree with these entries
  std::string s(1, filesystem::path::preferred_separator);
  std::vector<std::string> dirs{"foo", "foo" + s + "qux", "foo" + s + "quux"};
  std::vector<std::string> files{"foo" + s + "bar", "foo" + s + "bar",
                                 "foo" + s + "quux" + s + "corge"};

  // create them
  for (const auto& d : dirs)
    try_mkdir(d.c_str());
  for (const auto& f : files)
    std::fstream fs(f, std::ios::out);
  // unlike the posix find command the directory iterator doesnt give you access to
  // root or starting directory just the stuff under it so lets pop that off
  dirs.erase(dirs.begin());
  // go get whats there
  for (filesystem::recursive_directory_iterator i("foo"), end; i != end; ++i) {
    if (i->is_directory()) {
      auto pos = std::remove(dirs.begin(), dirs.end(), i->path().string());
      ASSERT_NE(pos, dirs.end()) << "unexpected directory";
      dirs.erase(pos, dirs.end());
    } else if (i->is_regular_file()) {
      auto pos = std::remove(files.begin(), files.end(), i->path().string());
      ASSERT_NE(pos, files.end()) << "unexpected file";
      files.erase(pos, files.end());
    } else {
      FAIL() << "unexpected entry type";
    }
  }
  // if we didnt get everything we have a problem
  EXPECT_TRUE(dirs.empty());
  EXPECT_TRUE(files.empty());

  // cleanup the stuff we made, 2 tests in one ;o)
  EXPECT_TRUE(filesystem::remove_all("foo")) << "why cant we delete the stuff we just made";
}

TEST(Filesystem, remove_any) {
  // delete non existant thing
  EXPECT_FALSE(filesystem::remove(".foobar")) << ".foobar should not exist";

  // make and delete a file
  { std::fstream fs(".foobar", std::ios::out); }
  EXPECT_TRUE(filesystem::remove(".foobar"));
  EXPECT_FALSE(filesystem::exists(".foobar"))
    << ".foobar file should have been deleted";

  // make and delete a file
  try_mkdir(".foobar");
  EXPECT_TRUE(filesystem::remove(".foobar"));
  EXPECT_FALSE(filesystem::exists(".foobar"))
    << ".foobar dir should have been deleted";
}

} // namespace


int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
