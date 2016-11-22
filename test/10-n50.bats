#!/usr/bin/env bats

load test_helper

@test "n50: read from file" {
    run $BATS_TEST_DIRNAME/../faops n50 $BATS_TEST_DIRNAME/ufasta.fa
    run bash -c "echo \"${output}\" | xargs echo "
    assert_equal 314 "${output}"
}

# ./faops size test/ufasta.fa | perl test/n50.pl stdin
# $HOME/share/MaSuRCA/bin/ufasta n50 -N50 -H test/ufasta.fa
# $HOME/share/MaSuRCA/bin/ufasta n50 -H -N10 -N25 -N50 -N75 -N90 test/ufasta.fa
