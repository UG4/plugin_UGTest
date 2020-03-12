/**
 * \file testUGUtils.cpp
 * \date 2012-05-11
 * \brief Unit tests for utility functions in \ug4
 */

#include <boost/test/unit_test.hpp>

#include <vector>
#include <string>
#include <algorithm>
#include <fstream>

#include "common/log.h"
#include "common/util/file_util.h"
#include "common/util/path_provider.h"
#include "bindings/lua/externals/lua/llimits.h"


using namespace std;
using namespace ug;
using namespace boost::unit_test;

struct FileUtilEnv {
  string existing_file;
  string non_existing_file;
  string an_empty_file;
  string one_kb_file;
  string the_same_one_kb_file;
  string another_one_kb_file;
  string test_files_dir;
  vector<string> files_in_dir;
  string test_dirs_in_dir;
  vector<string> dirs_in_dir;
  FileUtilEnv() {
	existing_file = PathProvider::get_path(ROOT_PATH) + "/apps/unit_tests/src/testUGUtils.cpp";
	string srcDir = existing_file.substr(0, existing_file.find_last_of("/") );
    non_existing_file = "im_not_a_file_nor_a_dir";
    an_empty_file = srcDir + "/../data/file_utils/an_empty_file";
    one_kb_file = srcDir + "/../data/file_utils/an_one_kb_file";
    the_same_one_kb_file = srcDir + "/../data/file_utils/the_same_one_kb_file";
    another_one_kb_file = srcDir + "/../data/file_utils/another_one_kb_file";
    test_files_dir = srcDir + "/../data/file_utils";
    files_in_dir.push_back("an_empty_file");
    files_in_dir.push_back("an_one_kb_file");
    files_in_dir.push_back("another_one_kb_file");
    files_in_dir.push_back("the_same_one_kb_file");
    test_dirs_in_dir = srcDir + "/../data/file_utils";
    dirs_in_dir.push_back(".");
    dirs_in_dir.push_back("..");
    dirs_in_dir.push_back("empty1");
    dirs_in_dir.push_back("empty2");
  }
};

BOOST_AUTO_TEST_SUITE( testUGUtilsNumProc1 )

BOOST_AUTO_TEST_SUITE( testOSDependentNumProc1 )

BOOST_FIXTURE_TEST_SUITE(testFileUtilNumProc1, FileUtilEnv)

/**
 *
 */
BOOST_AUTO_TEST_CASE( test_subdirectories ) {
  vector<string> dirs;

  // existing directory
  BOOST_REQUIRE( GetDirectoriesInDirectory( dirs, test_dirs_in_dir.c_str() ) );
  
  // dirty hack for svn managed directories
  if ( dirs.size() == (dirs_in_dir.size() + 1) ) {
    for ( size_t i = 0; i < dirs.size(); i++ ) {
      if ( dirs.at(i) == ".svn" ) {
        dirs_in_dir.push_back(".svn");
      }
    }
  }

  sort( dirs.begin(), dirs.end() );
  sort( dirs_in_dir.begin(), dirs_in_dir.end() );

  BOOST_REQUIRE_MESSAGE( dirs.size() == dirs_in_dir.size(),
                         "dir.size=" << dirs.size() <<
                         " == env.dirs_in_dir.size=" << dirs_in_dir.size() );
  for( size_t i = 0; i < dirs.size(); i++ ) {
    BOOST_CHECK_MESSAGE( dirs.at(i) == dirs_in_dir.at(i),
                         "dirs.at(" << i << ")=\"" << dirs.at(i) <<
                         "\" == env.dirs_in_dir.at(" << i << ")=\"" <<
                         dirs_in_dir.at(i) << "\"" );
  }

  // non-existing directory
  BOOST_REQUIRE( !GetDirectoriesInDirectory( dirs, non_existing_file.c_str() ) );
}

/**
 *
 */
BOOST_AUTO_TEST_CASE( test_files_in_dir ) {
  vector<string> files;

  // existing directory
  BOOST_REQUIRE( GetFilesInDirectory( files, test_files_dir.c_str() ) );
  sort( files.begin(), files.end() );
  sort( files_in_dir.begin(), files_in_dir.end() );
  BOOST_REQUIRE_MESSAGE( files.size() == files_in_dir.size(),
                         "files.size=" << files.size() <<
                         " == env.files_in_dir.size=" << files_in_dir.size() );
  for ( size_t i = 0; i < files.size(); i++ ) {
    BOOST_CHECK_MESSAGE( files.at(i) == files_in_dir.at(i),
                         "files.at(" << i << ")=\"" << files.at(i) <<
                         "\" == env.files_in_dir.at(" << i << ")=\"" <<
                         files_in_dir.at(i) << "\"" );
  }

  // non-existing directory
  BOOST_REQUIRE( !GetFilesInDirectory( files, non_existing_file.c_str() ) );
}

/**
 *
 */
BOOST_AUTO_TEST_CASE( test_file_exists ) {
  BOOST_CHECK( FileExists( existing_file.c_str() ) );
  BOOST_CHECK( !FileExists( non_existing_file.c_str() ) );
}

/**
 *
 */
BOOST_AUTO_TEST_CASE( test_file_size ) {
  size_t an_empty_file_size = FileSize( an_empty_file.c_str() );
  size_t one_kb_file_size = FileSize( one_kb_file.c_str() );
  BOOST_CHECK_MESSAGE( an_empty_file_size == 0,
                       "an_empty_file.size=" << an_empty_file_size << " == 0" );
  BOOST_CHECK_MESSAGE( one_kb_file_size == UNIT_KILO,
                       "one_kb_file.size=" << one_kb_file_size << " == " << UNIT_KILO );
}

/**
 *
 */
BOOST_AUTO_TEST_CASE( test_file_comparison ) {
  BOOST_CHECK( FileCompare( an_empty_file.c_str(), an_empty_file.c_str() ) );
  BOOST_CHECK( !FileCompare( one_kb_file.c_str(), an_empty_file.c_str() ) );
  BOOST_CHECK( !FileCompare( one_kb_file.c_str(), another_one_kb_file.c_str() ) );
  BOOST_CHECK( FileCompare( one_kb_file.c_str(), the_same_one_kb_file.c_str() ) );
}

BOOST_AUTO_TEST_SUITE_END(); // End of  testFileUtilNumProc1

BOOST_AUTO_TEST_SUITE_END(); // End of  testOSDependentNumProc1

BOOST_AUTO_TEST_SUITE_END(); // End of  testUGUtilsNumProc1

// EOF
