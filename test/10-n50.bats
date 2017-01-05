#!/usr/bin/env bats

load test_helper

@test "n50: display header" {
    run bash -c "$BATS_TEST_DIRNAME/../faops n50 $BATS_TEST_DIRNAME/ufasta.fa"
    assert_equal "N50${tab}314" "${lines[0]}"
}

@test "n50: don't display header" {
    run $BATS_TEST_DIRNAME/../faops n50 -H $BATS_TEST_DIRNAME/ufasta.fa
    run bash -c "echo \"${output}\" | xargs echo "
    assert_equal "314" "${output}"
}

@test "n50: set genome size (NG50)" {
    run $BATS_TEST_DIRNAME/../faops n50 -H -g 10000 $BATS_TEST_DIRNAME/ufasta.fa
    run bash -c "echo \"${output}\" | xargs echo "
    assert_equal "297" "${output}"
}

@test "n50: sum of size" {
    run $BATS_TEST_DIRNAME/../faops n50 -H -S $BATS_TEST_DIRNAME/ufasta.fa
    run bash -c "echo \"${output}\" | xargs echo "
    assert_equal "314 9317" "${output}"
}

@test "n50: sum and average of size" {
    run $BATS_TEST_DIRNAME/../faops n50 -H -S -A $BATS_TEST_DIRNAME/ufasta.fa
    run bash -c "echo \"${output}\" | xargs echo "
    assert_equal "314 9317 186.34" "${output}"
}

@test "n50: E-size" {
    run $BATS_TEST_DIRNAME/../faops n50 -H -E $BATS_TEST_DIRNAME/ufasta.fa
    run bash -c "echo \"${output}\" | xargs echo "
    assert_equal "314 314.70" "${output}"
}

@test "n50: n10" {
    run $BATS_TEST_DIRNAME/../faops n50 -H -N 10 $BATS_TEST_DIRNAME/ufasta.fa
    run bash -c "echo \"${output}\" | xargs echo "
    assert_equal "516" "${output}"
}

@test "n50: n90 with header" {
    run $BATS_TEST_DIRNAME/../faops n50 -N 90 $BATS_TEST_DIRNAME/ufasta.fa
    run bash -c "echo \"${output}\" | xargs echo "
    assert_equal "N90 112" "${output}"
}

@test "n50: only count of sequences" {
    run $BATS_TEST_DIRNAME/../faops n50 -N 0 -C $BATS_TEST_DIRNAME/ufasta.fa
    run bash -c "echo \"${output}\" | xargs echo "
    assert_equal "C 50" "${output}"
}

# ./faops size test/ufasta.fa | perl test/n50.pl stdin
# $HOME/share/MaSuRCA/bin/ufasta n50 -N50 -H test/ufasta.fa
# $HOME/share/MaSuRCA/bin/ufasta n50 -N50 -H -s 5000 test/ufasta.fa
# $HOME/share/MaSuRCA/bin/ufasta n50 -H -N10 -N25 -N50 -N75 -N90 test/ufasta.fa
