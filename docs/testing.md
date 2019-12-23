# Unit tests

There are currently 2 test frameworks in use:
- Simple test harness - `test/test.h`
- googletest

Converting a test that uses a simple test harness into gtests requires the followng:

1) Modify the `main` function to inlcude 
```
int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
```
2) `#include <gtest/gtest.h>`
3) Use googletest constructs for checking, test declaration etc
4) Have a look at the [googletest primer](https://github.com/google/googletest/blob/master/googletest/docs/primer.md)
