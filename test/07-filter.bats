#!/usr/bin/env bats

load test_helper

@test "filter: as formatter, sequence in one line" {
    exp=$($BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/ufasta.fa | wc -l | xargs echo)
    res=$($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/ufasta.fa stdout | wc -l | xargs echo)
    assert_equal "$(( exp * 2 ))" "$res"
}

@test "filter: as formatter, blocked fasta files" {
    exp=$($BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/ufasta.fa | wc -l | xargs echo)
    res=$($BATS_TEST_DIRNAME/../faops filter -b $BATS_TEST_DIRNAME/ufasta.fa stdout | wc -l | xargs echo)
    assert_equal "$(( exp * 3 ))" "$res"
}

@test "filter: as formatter, identical headers" {
    exp=$(grep '^>' $BATS_TEST_DIRNAME/ufasta.fa)
    res=$($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep '^>')
    assert_equal "$exp" "$res"
}

@test "filter: as formatter, identical sequences" {
    exp=$(grep -v '^>' $BATS_TEST_DIRNAME/ufasta.fa | perl -ne 'chomp; print')
    res=$($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep -v '^>' | perl -ne 'chomp; print')
    assert_equal "$exp" "$res"
}

@test "filter: as formatter, identical sequences (gz)" {
    exp=$(grep -v '^>' $BATS_TEST_DIRNAME/ufasta.fa | perl -ne 'chomp; print')
    res=$($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/ufasta.fa.gz stdout | grep -v '^>' | perl -ne 'chomp; print')
    assert_equal "$exp" "$res"
}

@test "filter: as formatter, identical sequences (gz) with -N" {
    exp=$(grep -v '^>' $BATS_TEST_DIRNAME/ufasta.fa | perl -ne 'chomp; print')
    res=$($BATS_TEST_DIRNAME/../faops filter -l 0 -N $BATS_TEST_DIRNAME/ufasta.fa.gz stdout | grep -v '^>' | perl -ne 'chomp; print')
    assert_equal "$exp" "$res"
}

@test "filter: convert IUPAC to N" {
    exp=$(printf ">read\n%s\n" ANNG)
    res=$($BATS_TEST_DIRNAME/../faops filter -l 0 -N <(printf ">read\n%s\n" AMRG) stdout)
    assert_equal "$exp" "$res"
}

@test "filter: Upper cases" {
    exp=$(printf ">read\n%s\n" ATCG)
    res=$($BATS_TEST_DIRNAME/../faops filter -l 0 -U <(printf ">read\n%s\n" AtcG) stdout)
    assert_equal "$exp" "$res"
}

@test "filter: simplify seq names" {
    exp=$(printf ">read\n%s\n" ANNG)
    res=$($BATS_TEST_DIRNAME/../faops filter -l 0 -s <(printf ">read.1\n%s\n" ANNG) stdout)
    assert_equal "$exp" "$res"
}

@test "filter: fastq to fasta" {
    run $BATS_TEST_DIRNAME/../faops filter $BATS_TEST_DIRNAME/test.seq stdout
    run bash -c "echo \"${output}\" | wc -l | xargs echo "
    assert_equal 6 "${output}"
}

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
