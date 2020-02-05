#include <boost/test/unit_test.hpp>
#include "../../PluginTests/src/common_fixtures.cpp" //Fixtures provided by PluginTests, path doesn't change

BOOST_AUTO_TEST_SUITE(PluginTests_simple)

    BOOST_AUTO_TEST_CASE(PluginTests_simple_checks)
    {
        BOOST_WARN(false);
        BOOST_CHECK(false);
        BOOST_MESSAGE("if the next test fails, testing for all packges will abort");
        BOOST_REQUIRE(false);
    }
    BOOST_FIXTURE_TEST_CASE(PluginTests_simple_fixture, F)
    {
        BOOST_WARN_EQUAL(1,2);
        BOOST_WARN_GT(1,2);
    }

BOOST_AUTO_TEST_SUITE_END()     