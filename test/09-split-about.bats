#!/usr/bin/env bats

load test_helper

@test "split-about: 2000 bp " {
    mytmpdir=`mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir'`

    run bash -c "
        $BATS_TEST_DIRNAME/../faops split-about $BATS_TEST_DIRNAME/ufasta.fa 2000 $mytmpdir \
        && find $mytmpdir -name '*.fa' | wc -l | xargs echo
    "
    assert_equal 5 "${output}"

    rm -fr ${mytmpdir}
}

@test "split-about: max parts" {
    mytmpdir=`mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir'`

    run bash -c "
        $BATS_TEST_DIRNAME/../faops split-about -m 2 $BATS_TEST_DIRNAME/ufasta.fa 2000 $mytmpdir \
        && find $mytmpdir -name '*.fa' | wc -l | xargs echo
    "
    assert_equal 2 "${output}"

    rm -fr ${mytmpdir}
}

@test "split-about: 2000 bp and size restrict" {
    mytmpdir=`mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir'`

    run bash -c "
        $BATS_TEST_DIRNAME/../faops filter -a 100 $BATS_TEST_DIRNAME/ufasta.fa stdout \
        | $BATS_TEST_DIRNAME/../faops split-about stdin 2000 $mytmpdir \
        && find $mytmpdir -name '*.fa' | wc -l | xargs echo
    "
    assert_equal 4 "${output}"

    rm -fr ${mytmpdir}
}
