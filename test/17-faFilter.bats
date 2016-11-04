#!/usr/bin/env bats

load test_helper

@test "faFilter: minSize" {
    if ! hash faFilter 2>/dev/null ; then
        skip "Can't find faFilter"
    fi

    exp=$(faFilter -minSize=10 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep '^>')
    res=$($BATS_TEST_DIRNAME/../faops filter -a 10 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep '^>')
    assert_equal "$exp" "$res"
}

@test "faFilter: maxSize" {
    if ! hash faFilter 2>/dev/null ; then
        skip "Can't find faFilter"
    fi

    exp=$(faFilter -maxSize=50 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep '^>')
    res=$($BATS_TEST_DIRNAME/../faops filter -a 1 -z 50 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep '^>')
    assert_equal "$exp" "$res"
}

@test "faFilter: minSize maxSize" {
    if ! hash faFilter 2>/dev/null ; then
        skip "Can't find faFilter"
    fi

    exp=$(faFilter -minSize=10 -maxSize=50 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep '^>')
    res=$($BATS_TEST_DIRNAME/../faops filter -a 10 -z 50 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep '^>')
    assert_equal "$exp" "$res"
}

@test "faFilter: uniq" {
    if ! hash faFilter 2>/dev/null ; then
        skip "Can't find faFilter"
    fi

    exp=$(faFilter -uniq <(cat $BATS_TEST_DIRNAME/ufasta.fa $BATS_TEST_DIRNAME/ufasta.fa) stdout | grep '^>')
    res=$($BATS_TEST_DIRNAME/../faops filter -u -a 1 <(cat $BATS_TEST_DIRNAME/ufasta.fa $BATS_TEST_DIRNAME/ufasta.fa) stdout | grep '^>')
    assert_equal "$exp" "$res"
}
