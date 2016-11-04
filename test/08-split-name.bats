#!/usr/bin/env bats

load test_helper

@test "split-name: all sequences" {
    mytmpdir=`mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir'`

    run bash -c "
        $BATS_TEST_DIRNAME/../faops split-name $BATS_TEST_DIRNAME/ufasta.fa $mytmpdir \
        && find $mytmpdir -name '*.fa' | wc -l | xargs echo
    "
    assert_equal 50 "${output}"

    rm -fr ${mytmpdir}
}

@test "split-name: size restrict" {
    mytmpdir=`mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir'`

    run bash -c "
        $BATS_TEST_DIRNAME/../faops filter -a 10 $BATS_TEST_DIRNAME/ufasta.fa stdout \
        | $BATS_TEST_DIRNAME/../faops split-name stdin $mytmpdir \
        && find $mytmpdir -name '*.fa' | wc -l | xargs echo
    "
    assert_equal 44 "${output}"

    rm -fr ${mytmpdir}
}
