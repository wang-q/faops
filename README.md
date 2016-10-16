[![Travis](https://img.shields.io/travis/wang-q/faops.svg)](https://travis-ci.org/wang-q/faops)

# `faops` operates fasta files

```
$ ./faops help

Usage:     faops <command> [options] <arguments>
Version:   0.2.2

Commands:
    help           print this message
    count          Count base statistics in FA file(s)
    size           Count total bases in FA file(s)
    frag           Extract subsequences from a FA file
    rc             Reverse complement a FA file
    some           Extract some fa records
    filter         Filter fa records
    split-name     Splitting by sequence names
    split-about    Splitting to chunks about specified size

Options:
    There're no global options.
    Type "faops command-name" for detailed options of each command.
    Options *MUST* be placed just after command.
```

## Compiling

`faops` can be compiled under Windows with MinGW. 

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

Done with [bats](https://github.com/sstephenson/bats).
Useful articles:

  * https://blog.engineyard.com/2014/bats-test-command-line-tools
  * http://blog.spike.cx/post/60548255435/testing-bash-scripts-with-bats

```bash
#brew install bats
make test
```

## Dependency

* `zlib`
* `kseq.h` and `khash.h` from [`klib`](https://github.com/attractivechaos/klib) (bundled)
