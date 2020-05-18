# echo "Source directory: "
# read src
# cp $src ./tests
find . ! -name "*.cpp" -exec rm {} \;
# cd tests
# find . -name "*" -execdir rename -d 's/^/tst_/' *\;