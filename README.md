[![Travis](https://img.shields.io/travis/wang-q/faops.svg)](https://travis-ci.org/wang-q/faops)

# `faops` operates fasta files

`faops` is a lightweight tool for operating sequences in the fasta
format.

This tool can be regarded as a combination of `faCount`, `faSize`,
`faFrag`, `faRc`, `faSomeRecords`, `faFilter` and `faSplit` from
[UCSC Jim Kent's utilities](http://hgdownload.cse.ucsc.edu/admin/exe/).

Comparing to Kent's `fa*` utilities, `faops` is:

* much smaller (kilo vs mega bytes)
* easy to compile (only one external dependency)
* well tested
* contains only one executable file
* can operate gzipped (bgzipped) files
* and can be run under all major OSes (including Windows).

`faops` is also inspired/influenced/stealing from
[`seqtk`](https://github.com/lh3/seqtk) and
[`ufasta`](http://www.genome.umd.edu/masurca.html).

```
$ ./faops help

Usage:     faops <command> [options] <arguments>
Version:   0.8.11

Commands:
    help           print this message
    count          count base statistics in FA file(s)
    size           count total bases in FA file(s)
    frag           extract sub-sequences from a FA file
    rc             reverse complement a FA file
    some           extract some fa records
    order          extract some fa records by the given order
    replace        replace headers from a FA file
    filter         filter fa records
    split-name     splitting by sequence names
    split-about    splitting to chunks about specified size
    n50            compute N50 and other statistics
    dazz           rename records for dazz_db
    interleave     interleave two PE files
    region         extract regions from a FA file

Options:
    There're no global options.
    Type "faops command-name" for detailed options of each command.
    Options *MUST* be placed just after command.

```

## Examples

* Reverse complement

        faops rc test/ufasta.fa out.fa       # prepend RC_ to names
        faops rc -n test/ufasta.fa out.fa    # keep original names

* Extract sequences with names in `list.file`, one name per line

        faops some test/ufasta.fa list.file out.fa

* Same as above, but from stdin and to stdout

        cat test/ufasta.fa | faops some stdin list.file stdout

* Sort by header strings

        faops order test/ufasta.fa \
            <(cat test/ufasta.fa | grep '>' | sed 's/>//' | sort) \
            out.fa

* Sort by lengths

        faops order test/ufasta.fa \
            <(faops size test/ufasta.fa | sort -n -r -k2,2 | cut -f 1) \
            out2.fa

* Tidy fasta file to 80 characters of sequence per line

        faops filter -l 80 test/ufasta.fa out.fa

* All content written on one line

        faops filter -l 0 test/ufasta.fa out.fa

* Convert fastq to fasta

        faops filter -l 0 in.fq out.fa

* Compute N50, clean result

        faops n50 -H test/ufasta.fa

* Compute N75

        faops n50 -N 75 test/ufasta.fa

* Compute N90, sum and average of contigs with estimated genome size

        faops n50 -N 90 -S -A -g 10000 test/ufasta.fa

## Compiling

`faops` can be compiled under Linux, macOS (gcc or clang) and Windows
(MinGW).

```bash
git clone https://github.com/wang-q/faops
cd faops
make
```

## Installing with Homebrew or Linuxbrew

```bash
brew install wang-q/tap/faops
```

## Tests

Done with [bats](https://github.com/sstephenson/bats). Useful articles:

* https://blog.engineyard.com/2014/bats-test-command-line-tools
* http://blog.spike.cx/post/60548255435/testing-bash-scripts-with-bats

```bash
#brew install bats
make test
```

## Dependency

* `zlib`
* `kseq.h` and `khash.h` from
  [`klib`](https://github.com/attractivechaos/klib) (bundled)

# AUTHOR

Qiang Wang &lt;wang-q@outlook.com&gt;

# COPYRIGHT AND LICENSE

This software is copyright (c) 2014 by Qiang Wang.

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.
