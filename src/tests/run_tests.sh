#!/bin/sh

TEST_NAMES="
BasicTest
DuplicateNameTest
"

declare -i total=0
declare -i passed=0

for f in $TEST_NAMES
do
    g++ $f.cpp -std=c++17
    if [ $? -eq 0 ]; then
        total=$((total + 1))
        echo "Processing $f.cpp..."

        OUTPUT=$(./a.out)
        EXPECTED_OUTPUT=$(cat ${f}_EXPECTED.txt)

        if [ "$OUTPUT" = "$EXPECTED_OUTPUT" ]; then
            echo "...OK"
            passed=$((passed + 1))
        else
            echo "Unexpected output:"
            echo ${OUTPUT}
            echo 
        fi
    fi
done


echo 
if [ $passed -eq $total ]; then
    echo "All tests passed successfully!"
else
    echo "$passed of $total total tests passed."
fi
