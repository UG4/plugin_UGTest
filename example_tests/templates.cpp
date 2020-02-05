#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

typedef boost::mpl::list<int, long, unsigned char> test_types;

BOOST_AUTO_TEST_SUITE(PluginTests_templates)
    BOOST_AUTO_TEST_CASE_TEMPLATE(PluginTests_templates_demo_template, T, test_types)
    {
        BOOST_CHECK(sizeof(T) == (unsigned)4);
    }
BOOST_AUTO_TEST_SUITE_END()