#!/usr/bin/env bash
BIN=${BIN-./bin/cocogen}
CFLAGS=${CFLAGS-}
# RUN_FUNCTIONAL=${RUN_FUNCTIONAL-1}

VALGRIND=${VALGRIND-0}

ALIGN=60

total_tests=0
fail_tests=0

function echo_pass {
    echo -e '\E[27;32m'"\033[1mpass\033[0m"
}

function echo_fail {
    echo -e '\E[27;31m'"\033[1mfail\033[0m"
}

function echo_leaky {
    echo -e '\E[27;31m'"\033[1mleaky\033[0m"
}

# Easy tests, check if the parser, context analysis and typechecking work
# properly by checking if the compiler returns 0 (or non-zero when expected to
# fail)
function check_return {
    file=$1
    expect_failure=$2

    if [ ! -f $file ]; then return; fi

    total_tests=$((total_tests+1))
    printf "%-${ALIGN}s " $file:

    if [ $VALGRIND -eq 1 ]; then
        cmd="valgrind --error-exitcode=127 $BIN"
    else
        cmd=$BIN
    fi

    if $cmd $CFLAGS $file --header-dir ./test/generated_out/ \
        --source-dir ./test/generated_out/ > tmp.out 2>&1
    then
        if [ $expect_failure -eq 1 ]; then
            echo_fail
            fail_tests=$((fail_tests+1))
        else
            echo_pass
        fi
    else
        status_code=$?
        if [ $VALGRIND -eq 1 ] && [ $status_code -eq 127 ]; then
            echo_leaky
            fail_tests=$((fail_tests+1))
        else
            if [ $expect_failure -eq 1 ]; then
                echo_pass
            else
                echo_fail
                echo -------------------------------
                cat tmp.out
                echo -------------------------------
                echo
                fail_tests=$((fail_tests+1))
            fi
        fi
    fi

    rm -f tmp.out
}

function run_dir {
    BASE=$1

    for f in $BASE/pass/*.ast; do
        check_return $f 0
    done

    for f in $BASE/fail/*.ast; do
        check_return $f 1
    done
}

if [ $VALGRIND -eq 1 ]; then
    echo "RUNNING TESTS WITH VALGRIND"
    echo ""
fi

for arg in $@; do
    run_dir $arg
done

rm -rf ./test/generated_out/

echo $total_tests tests, $fail_tests failures
