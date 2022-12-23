#!/bin/sh

TEST_NAMES="
BasicTest
DuplicateNameTest
"

declare -i total=0
declare -i pass=0
declare -i fail=0
declare -i bad_compile=0

for f in $TEST_NAMES
do
    echo "Processing $f.cpp..."
    total=$((total + 1))
    g++ $f.cpp -std=c++17
    if [ $? -eq 0 ]; then

        OUTPUT=$(./a.out)
        EXPECTED_OUTPUT=$(cat ${f}_EXPECTED.txt)

        if [ "$OUTPUT" = "$EXPECTED_OUTPUT" ]; then
            echo "...OK"
            pass=$((pass + 1))
        else
            fail=$((fail + 1))
            echo "Unexpected output:"
            echo ${OUTPUT}
            echo 
        fi
    else
        bad_compile=$((bad_compile + 1))
    fi
done


echo 
if [ $pass -eq $total ]; then
    echo "All tests pass successfully!"
else
    echo "$pass of $total total tests pass."
    if [ $fail -ne 0 ]; then
        echo "    $fail tests produced the wrong output."
    fi
    if [ $bad_compile -ne 0 ]; then
        echo "    $bad_compile tests failed to compile."
    fi
fi
