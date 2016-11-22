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
    run $BATS_TEST_DIRNAME/../faops n50 -H -s 10000 $BATS_TEST_DIRNAME/ufasta.fa
    run bash -c "echo \"${output}\" | xargs echo "
    assert_equal "297" "${output}"
}

# ./faops size test/ufasta.fa | perl test/n50.pl stdin
# $HOME/share/MaSuRCA/bin/ufasta n50 -N50 -H test/ufasta.fa
# $HOME/share/MaSuRCA/bin/ufasta n50 -N50 -H -s 5000 test/ufasta.fa
# $HOME/share/MaSuRCA/bin/ufasta n50 -H -N10 -N25 -N50 -N75 -N90 test/ufasta.fa
