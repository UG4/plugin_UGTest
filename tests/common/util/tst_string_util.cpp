#include "common/util/string_util.h"
#include "UGTest.h"

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

        // s=nullptr;
        // BOOST_WARN_THROW(RemoveWhitespaceFromString(s),std::bad_alloc);

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

        // s=nullptr;
        // BOOST_WARN_THROW(RemoveWhitespaceFromString(s),std::bad_alloc);

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

        // s=nullptr;
        // BOOST_WARN_THROW(RemoveWhitespaceFromString(s),std::bad_alloc);

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
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_LevenshteinDistance){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_GetFileLines){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_GetFileLine){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_IsLonger){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_ToString){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_XMLStringEscape){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_WildcardMatch){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_ConfigShift){
        
    }

    BOOST_AUTO_TEST_CASE(tst_string_util_GetBytesSizeString){
        
    }
  
BOOST_AUTO_TEST_SUITE_END()   