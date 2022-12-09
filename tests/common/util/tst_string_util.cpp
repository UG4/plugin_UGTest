#include "common/util/string_util.h"
#include "UGTest.h"

using namespace ug;
using namespace std;
BOOST_AUTO_TEST_SUITE(tst_string_util)
    BOOST_AUTO_TEST_CASE(tst_string_util_TokenizeString){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_TokenizeTrimString){

    }

    BOOST_AUTO_TEST_CASE(tst_string_util_RemoveWhitespaceFromString)
    {
        std::string s="";
        RemoveWhitespaceFromString(s);
        BOOST_CHECK_EQUAL(s, "");

        //BOOST_WARN_THROW(RemoveWhitespaceFromString(nullptr),std::bad_alloc);

        s="abcdefg";
        RemoveWhitespaceFromString(s);
        BOOST_CHECK_EQUAL(s, "abcdefg");

        s="abc defg";
        RemoveWhitespaceFromString(s);
        BOOST_CHECK_EQUAL(s, "abcdefg");

        s="abc defg ";
        RemoveWhitespaceFromString(s);
        BOOST_CHECK_EQUAL(s, "abcdefg");

        s=" abc defg";
        RemoveWhitespaceFromString(s);
        BOOST_CHECK_EQUAL(s, "abcdefg");

        s="abc de fg";
        RemoveWhitespaceFromString(s);
        BOOST_CHECK_EQUAL(s, "abcdefg");
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_TrimString){
        std::string s="";
        BOOST_CHECK_EQUAL(TrimString(s), s);

        s="abcdefg";
        BOOST_CHECK_EQUAL(TrimString(s), s);

        s="abc defg";
        BOOST_CHECK_EQUAL(TrimString(s), s);

        s="abc defg ";
        BOOST_CHECK_EQUAL(TrimString(s), "abc defg");

        s=" abc defg";
        BOOST_CHECK_EQUAL(TrimString(s), "abc defg");

        s="abc de fg";
        BOOST_CHECK_EQUAL(TrimString(s), s);
    }
    
    BOOST_AUTO_TEST_CASE(tst_string_util_SnipString){

    }

    BOOST_AUTO_TEST_CASE(tst_string_util_SnipStringFront){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_NumberOfDigits){

        BOOST_CHECK_EQUAL(NumberOfDigits(0), 1);
        BOOST_CHECK_EQUAL(NumberOfDigits(1), 1);
        BOOST_CHECK_EQUAL(NumberOfDigits(-1), 1);

        BOOST_CHECK_EQUAL(NumberOfDigits(10), 2);
        BOOST_CHECK_EQUAL(NumberOfDigits(14), 2);
        BOOST_CHECK_EQUAL(NumberOfDigits(-20), 2);

        BOOST_CHECK_EQUAL(NumberOfDigits(INT_MAX), 10);
        BOOST_CHECK_EQUAL(NumberOfDigits(INT_MIN), 10);

    }

    BOOST_AUTO_TEST_CASE(tst_string_util_AppendCounterToString){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_AppendSpacesToString){
        std::string s="";
        BOOST_CHECK_EQUAL(AppendSpacesToString(s,0), s);

        s="abcdefg";
        BOOST_CHECK_EQUAL(AppendSpacesToString(s,10), "abcdefg   ");

        BOOST_WARN_NO_THROW(AppendSpacesToString(s,-1));
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_hash_key){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_GetDirectorySeperatorPos){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_FilenameWithoutPath){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_PathFromFilename){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_FilenameWithoutExtension){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_FilenameAndPathWithoutExtension){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_GetFilenameExtension){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_ReplaceAll){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_StartsWith){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_Contains){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_ToLower){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_ToUpper){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_FindDuplicates){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_repeat){
        BOOST_CHECK_EQUAL(repeat({},0), "");
        BOOST_CHECK_EQUAL(repeat(' ',0), "");
        BOOST_CHECK_EQUAL(repeat(' ',1), " ");
        BOOST_CHECK_EQUAL(repeat(' ',-1), "");
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_LevenshteinDistance){
        std::string words [] = {"hat", "Hut", "tun"};

        BOOST_CHECK_EQUAL(LevenshteinDistance("", ""), 0);

        BOOST_CHECK_EQUAL(LevenshteinDistance(words[0], words[1]), 2);
        BOOST_CHECK_GT(LevenshteinDistance(words[0], words[2]), LevenshteinDistance(words[1], words[2]));
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_GetFileLines){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_GetFileLine){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_IsLonger){
        BOOST_CHECK(!IsLonger("", ""));
        BOOST_CHECK(!IsLonger(" ", ""));
        BOOST_CHECK(IsLonger("", " "));
        //Designed for
        //int maxLength = (*max_element(vecStr.begin(), vecStr.end(), IsLonger)).size();
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_ToString){
        //template
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_XMLStringEscape){

        BOOST_REQUIRE_EQUAL(XMLStringEscape("&"), "&amp;");
        BOOST_REQUIRE_EQUAL(XMLStringEscape("\""), "&quot;");
        BOOST_REQUIRE_EQUAL(XMLStringEscape("'"), "&apos;");
        BOOST_REQUIRE_EQUAL(XMLStringEscape("<"), "&lt;");
        BOOST_REQUIRE_EQUAL(XMLStringEscape(">"), "&gt;");

        //some random data sets or actual xml files
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_WildcardMatch){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_ConfigShift){
        
    }

    /*boost::mpl::list<std::string, int, long> OstreamShift_test_types;

    BOOST_AUTO_TEST_CASE_TEMPLATE(tst_string_util_OstreamShift, T, OstreamShift_test_types){
        BOOST_NO_THROW(OstreamShift(T));
    }*/

    BOOST_AUTO_TEST_CASE(tst_string_util_GetBytesSizeString){
        size_t kb=1024, z=0, n=-1, mb=2411724;

        BOOST_CHECK_EQUAL(GetBytesSizeString(kb), "1024 b");
        BOOST_CHECK_EQUAL(GetBytesSizeString(mb), "2.3 Mb");

        BOOST_CHECK_EQUAL(GetBytesSizeString(z), "0 b");

        //BOOST_CHECK_THROW(GetBytesSizeString(n), exception);
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_TrueFalseString){
        BOOST_CHECK_EQUAL(TrueFalseString(true), "TRUE");
        BOOST_CHECK_EQUAL(TrueFalseString(false), "FALSE");
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_OnOffString){
        BOOST_CHECK_EQUAL(OnOffString(true), "ON");
        BOOST_CHECK_EQUAL(OnOffString(false), "OFF");
    }
  
BOOST_AUTO_TEST_SUITE_END()   
