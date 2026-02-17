// This file provides the Boost.Test unit_test_main implementation for
// the Bazel build. The Bazel boost.test module does not compile
// unit_test_main.cpp, so we include it here to make it available
// to the seastar testing library.

#define BOOST_TEST_NO_MAIN
#include <boost/test/impl/unit_test_main.ipp>
