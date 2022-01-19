#!/usr/bin/env bats

load test_helper

@test "one: inline names" {
    exp=$($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep -A 1 '^>read12')
    res=$($BATS_TEST_DIRNAME/../faops one -l 0 $BATS_TEST_DIRNAME/ufasta.fa read12 stdout)
    assert_equal "$exp" "$res"
}

@test "faSomeRecords: inline names" {
    if ! hash faSomeRecords 2>/dev/null ; then
        skip "Can't find faSomeRecords"
    fi

    exp=$(faSomeRecords $BATS_TEST_DIRNAME/ufasta.fa <(echo read12) stdout | grep '^>')
    res=$($BATS_TEST_DIRNAME/../faops one $BATS_TEST_DIRNAME/ufasta.fa read12 stdout | grep '^>')
    assert_equal "$exp" "$res"
}
