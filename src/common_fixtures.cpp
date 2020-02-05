//This file contains fixtures you migth find usefull, see the README for further information
#include <boost/test/unit_test.hpp>

struct F
{
    F() { BOOST_TEST_MESSAGE("setup fixture F"); }
    ~F() { BOOST_TEST_MESSAGE("teardown fixture F"); }
};