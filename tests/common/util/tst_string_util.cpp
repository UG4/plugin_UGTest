#include "common/util/string_util.h"
#include "UGTest.h"

BOOST_AUTO_TEST_SUITE(Testsuite_string_util)

    BOOST_AUTO_TEST_CASE(RemoveWhitespaceFromString_case)
    {
        std::string s="";
        RemoveWhitespaceFromString(s);
        BOOST_CHECK(s=="");

        // s=nullptr;
        // BOOST_WARN_THROW(RemoveWhitespaceFromString(s),std::bad_alloc);

        s="abcdefg";
        RemoveWhitespaceFromString(s);
        BOOST_CHECK(s=="abcdefg");

        s="abc defg";
        RemoveWhitespaceFromString(s);
        BOOST_CHECK(s=="abcdefg");

        s="abc defg ";
        RemoveWhitespaceFromString(s);
        BOOST_CHECK(s=="abcdefg");

        s=" abc defg";
        RemoveWhitespaceFromString(s);
        BOOST_CHECK(s=="abcdefg");

        s="abc de fg";
        RemoveWhitespaceFromString(s);
        BOOST_CHECK(s=="abcdefg");
    }

    BOOST_AUTO_TEST_CASE(NumberOfDigits_case){

        BOOST_CHECK_EQUAL(NumberOfDigits(0),1);
        BOOST_CHECK_EQUAL(NumberOfDigits(1),1);
        BOOST_CHECK_EQUAL(NumberOfDigits(-1),1);

        BOOST_CHECK_EQUAL(NumberOfDigits(10),2);
        BOOST_CHECK_EQUAL(NumberOfDigits(14),2);
        BOOST_CHECK_EQUAL(NumberOfDigits(-20),2);

        BOOST_CHECK_EQUAL(NumberOfDigits(INT_MAX),10);
        BOOST_CHECK_EQUAL(NumberOfDigits(INT_MIN),10);

    }

BOOST_AUTO_TEST_SUITE_END()   