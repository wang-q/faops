#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <stdint.h> // uint_fast8_t etc
#include <unistd.h> // getopt
#include <getopt.h> // http://stackoverflow.com/questions/22575940/getopt-not-included-implicit-declaration-of-function-getopt

// create dir
#ifdef __MINGW32__
#include <direct.h>
#else

#include <sys/stat.h>
#include <sys/types.h>

#endif

// kseq support gzipped files
#include "kseq.h"

KSEQ_INIT(gzFile, gzread)

// **str** will be the name of HASH throughout the whole file
// key is string, value is int
#include "khash.h"

KHASH_MAP_INIT_STR(str, int)

// this macro doesn't work on a decayed pointer,
// e.g. array name passed to a subroutine
#define ArraySize(a) (sizeof(a) / sizeof((a)[0]))

enum {
    T_BASE_VAL = 0,
    U_BASE_VAL = 0,
    C_BASE_VAL = 1,
    A_BASE_VAL = 2,
    G_BASE_VAL = 3,
    N_BASE_VAL = 4
};

int nt_val[256];
int nt_comp[256];

// Code =>  Nucleic Acid(s)
//  A   =>  Adenine
//  C   =>  Cytosine
//  G   =>  Guanine
//  T   =>  Thymine
//  U   =>  Uracil
//  M   =>  A or C (amino)
//  R   =>  A or G (purine)
//  W   =>  A or T (weak)
//  S   =>  C or G (strong)
//  Y   =>  C or T (pyrimidine)
//  K   =>  G or T (keto)
//  V   =>  A or C or G
//  H   =>  A or C or T
//  D   =>  A or G or T
//  B   =>  C or G or T
//  N   =>  A or G or C or T (any)

void init_nt_val() {
    for (int i = 0; i < ArraySize(nt_val); i++) {
        nt_val[i] = -1;
    }

    nt_val['t'] = nt_val['T'] = T_BASE_VAL;
    nt_val['u'] = nt_val['U'] = U_BASE_VAL;
    nt_val['c'] = nt_val['C'] = C_BASE_VAL;
    nt_val['a'] = nt_val['A'] = A_BASE_VAL;
    nt_val['g'] = nt_val['G'] = G_BASE_VAL;
    nt_val['m'] = nt_val['M'] = N_BASE_VAL;
    nt_val['r'] = nt_val['R'] = N_BASE_VAL;
    nt_val['w'] = nt_val['W'] = N_BASE_VAL;
    nt_val['s'] = nt_val['S'] = N_BASE_VAL;
    nt_val['y'] = nt_val['Y'] = N_BASE_VAL;
    nt_val['k'] = nt_val['K'] = N_BASE_VAL;
    nt_val['v'] = nt_val['V'] = N_BASE_VAL;
    nt_val['h'] = nt_val['H'] = N_BASE_VAL;
    nt_val['d'] = nt_val['D'] = N_BASE_VAL;
    nt_val['b'] = nt_val['B'] = N_BASE_VAL;
    nt_val['n'] = nt_val['N'] = N_BASE_VAL;
}

//$seq =~ tr/ACGTMRWSYKVHDBNacgtmrwsykvhdbn-/TGCAKYWSRMBDHVNtgcakywsrmbdhvn-/;
void init_nt_comp() {
    memset(nt_comp, '\0', sizeof(nt_comp));

    nt_comp[' '] = ' ';
    nt_comp['-'] = '-';

    nt_comp['A'] = 'T';
    nt_comp['C'] = 'G';
    nt_comp['G'] = 'C';
    nt_comp['T'] = 'A';
    nt_comp['M'] = 'K';
    nt_comp['R'] = 'Y';
    nt_comp['W'] = 'W';
    nt_comp['S'] = 'S';
    nt_comp['Y'] = 'R';
    nt_comp['K'] = 'M';
    nt_comp['V'] = 'B';
    nt_comp['H'] = 'D';
    nt_comp['D'] = 'H';
    nt_comp['B'] = 'V';
    nt_comp['N'] = 'N';
    nt_comp['U'] = 'A';

    nt_comp['a'] = 't';
    nt_comp['c'] = 'g';
    nt_comp['g'] = 'c';
    nt_comp['t'] = 'a';
    nt_comp['m'] = 'k';
    nt_comp['r'] = 'y';
    nt_comp['w'] = 'w';
    nt_comp['s'] = 's';
    nt_comp['y'] = 'r';
    nt_comp['k'] = 'm';
    nt_comp['v'] = 'b';
    nt_comp['h'] = 'd';
    nt_comp['d'] = 'h';
    nt_comp['b'] = 'v';
    nt_comp['n'] = 'n';
    nt_comp['u'] = 'a';
}

void reverse_str(char *str, long length) {
    long half_length = (length >> 1);
    char *end = str + length;
    char c;
    while (--half_length >= 0) {
        c = *str;
        *str++ = *--end;
        *end = c;
    }
}

void complement_str(char *str, long length) {
    for (long i = 0; i < length; ++i) {
        str[i] = nt_comp[(int) str[i]];
    }
}

// Count Ns in sequence
int count_n(char *str, long length) {
    int n_count = 0;

    for (int i = 0; i < length; i++) {
        int base_val = nt_val[(int) (str[i])];
        if (base_val == N_BASE_VAL) {
            n_count++;
        }
    }

    return n_count;
}

// https://biowize.wordpress.com/2013/03/05/using-kseq-h-with-stdin/
// stream_in
FILE *source_in(char *file) {
    FILE *stream;

    if (strcmp(file, "stdin") == 0) {
        stream = stdin;
    } else {
        if ((stream = fopen(file, "r")) == NULL) {
            fprintf(stderr, "Cannot open input file [%s]\n", file);
            exit(1);
        }
    }

    return stream;
}

// stream_out
FILE *source_out(char *file) {
    FILE *stream;

    if (strcmp(file, "stdout") == 0) {
        stream = stdout;
    } else {
        if ((stream = fopen(file, "w")) == NULL) {
            fprintf(stderr, "Cannot open output file [%s]\n", file);
            exit(1);
        }
    }

    return stream;
}

int fa_count(int argc, char *argv[]) {
    if (argc == optind) {
        fprintf(stderr,
                "\n"
                        "faops count - count base statistics in FA file(s).\n"
                        "usage:\n"
                        "    faops count <in.fa> [more_files.fa]\n"
                        "\n");
        exit(1);
    }

    unsigned long total_length = 0;
    unsigned long total_base_count[5] = {0};

    gzFile fp;
    kseq_t *seq;

    printf("#seq\tlen\tA\tC\tG\tT\tN");
    printf("\n");

    for (int f = 0; f < argc; ++f) {
        fp = gzopen(argv[f], "r");
        seq = kseq_init(fp);

        int l;
        while ((l = kseq_read(seq)) >= 0) {
            unsigned long length = 0;
            unsigned long base_count[5] = {0};

            for (int i = 0; i < seq->seq.l; i++) {
                int base_val = nt_val[(int) (seq->seq.s[i])];

                if (base_val >= 0 && base_val <= 4) {
                    length++;
                    base_count[base_val]++;
                }
            }

            printf("%s\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu", seq->name.s, length,
                   base_count[A_BASE_VAL], base_count[C_BASE_VAL],
                   base_count[G_BASE_VAL], base_count[T_BASE_VAL],
                   base_count[N_BASE_VAL]);
            printf("\n");

            total_length += length;
            for (int i = 0; i < ArraySize(base_count); i++)
                total_base_count[i] += base_count[i];
        }

        kseq_destroy(seq);
        gzclose(fp);
    }

    printf("total\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu", total_length,
           total_base_count[A_BASE_VAL], total_base_count[C_BASE_VAL],
           total_base_count[G_BASE_VAL], total_base_count[T_BASE_VAL],
           total_base_count[N_BASE_VAL]);
    printf("\n");

    return 0;
}

int fa_size(int argc, char *argv[]) {
    if (argc == optind) {
        fprintf(stderr,
                "\n"
                        "faops size - count total bases in FA file(s).\n"
                        "             DO NOT support reading from stdin.\n"
                        "usage:\n"
                        "    faops size <in.fa> [more_files.fa]\n"
                        "\n");
        exit(1);
    }

    gzFile fp;
    kseq_t *seq;

    for (int f = 0; f < argc; ++f) {
        fp = gzopen(argv[f], "r");
        seq = kseq_init(fp);

        int l;
        while ((l = kseq_read(seq)) >= 0) {
            printf("%s\t%lu", seq->name.s, seq->seq.l);
            printf("\n");
        }

        kseq_destroy(seq);
        gzclose(fp);
    }

    return 0;
}

int fa_frag(int argc, char *argv[]) {
    int option = 0, line = 80;

    while ((option = getopt(argc, argv, "l:")) != -1) {
        switch (option) {
            case 'l':
                line = atoi(optarg);
                break;
        }
    }

    if (optind + 4 > argc || !isdigit(argv[optind + 1][0]) ||
        !isdigit(argv[optind + 2][0])) {
        fprintf(stderr,
                "\n"
                        "faops frag - Extract a piece of DNA from a .fa file.\n"
                        "usage:\n"
                        "    faops frag [options] <in.fa> <start> <end> <out.fa>\n"
                        "\n"
                        "options:\n"
                        "    -l INT     sequence line length [%d]\n"
                        "\n"
                        "in.fa  == stdin  means reading from stdin\n"
                        "out.fa == stdout means writing to stdout\n"
                        "\n",
                line);
        exit(1);
    }

    char *file_in = argv[optind];
    int start = atoi(argv[optind + 1]);
    int end = atoi(argv[optind + 2]);
    char *file_out = argv[optind + 3];

    if (start >= end) {
        fprintf(stderr, "start [%d] >= end [%d]\n", start, end);
        exit(1);
    }

    FILE *stream_in = source_in(file_in);
    FILE *stream_out = source_out(file_out);
    gzFile fp;
    kseq_t *seq;
    char seq_name[512];
    int is_first = 1;

    fp = gzdopen(fileno(stream_in), "r");
    seq = kseq_init(fp);

    int l;
    while ((l = kseq_read(seq)) >= 0) {
        if (!is_first) {
            fprintf(stderr, "More than one sequence in %s, just using first\n",
                    file_in);
            break;
        }

        if (end > seq->seq.l) {
            fprintf(stderr, "%s only has %zu bases, truncating\n", seq->name.s,
                    seq->seq.l);
            end = seq->seq.l;
            if (start >= end) {
                fprintf(stderr, "Sorry, no sequence left after truncating\n");
                exit(1);
            }
        }

        sprintf(seq_name, "%s:%d-%d", seq->name.s, start, end);
        fprintf(stream_out, ">%s\n", seq_name);

        for (int i = 0; i < end - start + 1; i++) {
            if (line != 0 && i != 0 && (i % line) == 0) {
                fputc('\n', stream_out);
            }
            fputc(seq->seq.s[i + start - 1], stream_out);
        }
        fputc('\n', stream_out);

        is_first = 0;
    }

    kseq_destroy(seq);
    gzclose(fp);
    if (strcmp(file_out, "stdout") != 0) {
        fclose(stream_out);
    }
    return 0;
}

int fa_rc(int argc, char *argv[]) {
    int flag_n = 0, flag_r = 0, flag_c = 0;
    int option = 0, line = 80;

    while ((option = getopt(argc, argv, "nrcl:")) != -1) {
        switch (option) {
            case 'n':
                flag_n = 1;
                break;
            case 'r':
                flag_r = 1;
                break;
            case 'c':
                flag_c = 1;
                break;
            case 'l':
                line = atoi(optarg);
                break;
        }
    }

    char *prefix = flag_n ? "" : (flag_r ? "R_" : (flag_c ? "C_" : "RC_"));

    if (optind + 2 > argc) {
        fprintf(stderr,
                "\n"
                        "faops rc - Reverse complement a FA file.\n"
                        "usage:\n"
                        "    faops rc [options] <in.fa> <out.fa>\n"
                        "\n"
                        "options:\n"
                        "    -n         keep name identical (don't prepend RC_)\n"
                        "    -r         Just Reverse, prepends R_\n"
                        "    -c         Just Complement, prepends C_\n"
                        "    -l INT     sequence line length [%d]\n"
                        "\n"
                        "in.fa  == stdin  means reading from stdin\n"
                        "out.fa == stdout means writing to stdout\n"
                        "\n",
                line);
        exit(1);
    }

    char *file_in = argv[optind];
    char *file_out = argv[optind + 1];

    FILE *stream_in = source_in(file_in);
    FILE *stream_out = source_out(file_out);
    gzFile fp;
    kseq_t *seq;
    char seq_name[512];

    fp = gzdopen(fileno(stream_in), "r");
    seq = kseq_init(fp);

    int l;
    while ((l = kseq_read(seq)) >= 0) {
        sprintf(seq_name, "%s%s", prefix, seq->name.s);
        fprintf(stream_out, ">%s\n", seq_name);

        if (flag_r) {
            reverse_str(seq->seq.s, seq->seq.l);
        } else if (flag_c) {
            complement_str(seq->seq.s, seq->seq.l);
        } else {
            reverse_str(seq->seq.s, seq->seq.l);
            complement_str(seq->seq.s, seq->seq.l);
        }

        for (int i = 0; i < seq->seq.l; i++) {
            if (line != 0 && i != 0 && (i % line) == 0) {
                fputc('\n', stream_out);
            }
            fputc(seq->seq.s[i], stream_out);
        }
        fputc('\n', stream_out);
    }

    kseq_destroy(seq);
    gzclose(fp);
    if (strcmp(file_out, "stdout") != 0) {
        fclose(stream_out);
    }
    return 0;
}

int fa_some(int argc, char *argv[]) {
    int flag_i = 0;
    int option = 0, line = 80;

    while ((option = getopt(argc, argv, "il:")) != -1) {
        switch (option) {
            case 'i':
                flag_i = 1;
                break;
            case 'l':
                line = atoi(optarg);
                break;
        }
    }

    if (optind + 3 > argc) {
        fprintf(stderr,
                "\n"
                        "faops some - Extract multiple fa sequences\n"
                        "usage:\n"
                        "    faops some [options] <in.fa> <list.file> <out.fa>\n"
                        "\n"
                        "options:\n"
                        "    -i         Invert, output sequences not in the list\n"
                        "    -l INT     sequence line length [%d]\n"
                        "\n"
                        "in.fa  == stdin  means reading from stdin\n"
                        "out.fa == stdout means writing to stdout\n"
                        "\n",
                line);
        exit(1);
    }

    char *file_in = argv[optind];
    char *file_list = argv[optind + 1];
    char *file_out = argv[optind + 2];

    FILE *stream_in = source_in(file_in);
    FILE *stream_out = source_out(file_out);
    gzFile fp;
    kseq_t *seq;
    FILE *fp_list;
    char seq_name[512];

    fp = gzdopen(fileno(stream_in), "r");
    seq = kseq_init(fp);

    //  Read list.file to a hash table
    if ((fp_list = fopen(file_list, "r")) == NULL) {
        fprintf(stderr, "Cannot open list file [%s]\n", file_list);
        exit(1);
    }

    // variables for hashing
    // from Heng Li's replay to http://www.biostars.org/p/10353/
    int buf_size = 4096;
    char buf[buf_size];   // buffers for names in list.file
    khash_t(str) *hash;  // the hash
    hash = kh_init(str);
    int ret;           // return value from hashing
    int flag_key = 0;  // check keys' exists

    while (fscanf(fp_list, "%s\n", buf) == 1) {
        kh_put(str, hash, strdup(buf), &ret);  // FIXME: check ret
    }
    fclose(fp_list);

    int l;
    while ((l = kseq_read(seq)) >= 0) {
        sprintf(seq_name, "%s", seq->name.s);

        flag_key = (kh_get(str, hash, seq_name) != kh_end(hash));
        // fprintf(stderr, "Seq: [%s];\tExists:[%d]\n", seq_name, flag_key);

        //          invert 0    invert 1
        // key  1      1            0
        // key  0      0            1
        // xor
        if ((!flag_key) != (!flag_i)) {
            fprintf(stream_out, ">%s\n", seq_name);
            for (int i = 0; i < seq->seq.l; i++) {
                if (line != 0 && i != 0 && (i % line) == 0) {
                    fputc('\n', stream_out);
                }
                fputc(seq->seq.s[i], stream_out);
            }
            fputc('\n', stream_out);
        }
    }

    kh_destroy(str, hash);  // FIXME: free keys before destroy
    kseq_destroy(seq);
    gzclose(fp);
    if (strcmp(file_out, "stdout") != 0) {
        fclose(stream_out);
    }
    return 0;
}

int fa_filter(int argc, char *argv[]) {
    int flag_u = 0;
    int min_size = -1, max_size = -1, max_n = -1;
    int option = 0, line = 80;

    while ((option = getopt(argc, argv, "ua:z:n:l:")) != -1) {
        switch (option) {
            case 'u':
                flag_u = 1;
                break;
            case 'a':
                min_size = atoi(optarg);
                break;
            case 'z':
                max_size = atoi(optarg);
                break;
            case 'n':
                max_n = atoi(optarg);
                break;
            case 'l':
                line = atoi(optarg);
                break;
        }
    }

    if (optind + 2 > argc) {
        fprintf(
                stderr,
                "\n"
                        "faops filter - Filter fa records\n"
                        "usage:\n"
                        "    faops filter [options] <in.fa> <out.fa>\n"
                        "\n"
                        "options:\n"
                        "    -a INT     pass sequences at least this big ('a'-smallest)\n"
                        "    -z INT     pass sequences this size or smaller ('z'-biggest)\n"
                        "    -n INT     pass sequences with fewer than this number of N's\n"
                        "    -u         Unique, removes duplicate ids, keeping the first\n"
                        "    -l INT     sequence line length [%d]\n"
                        "\n"
                        "in.fa  == stdin  means reading from stdin\n"
                        "out.fa == stdout means writing to stdout\n"
                        "\n"
                        "Not all faFilter options were implemented.\n"
                        "Names' wildcards are easily accomplished by \"faops some\".\n"
                        "\n",
                line);
        exit(1);
    }

    char *file_in = argv[optind];
    char *file_out = argv[optind + 1];

    FILE *stream_in = source_in(file_in);
    FILE *stream_out = source_out(file_out);
    gzFile fp;
    kseq_t *seq;
    char seq_name[512];
    int flag_pass;

    fp = gzdopen(fileno(stream_in), "r");
    seq = kseq_init(fp);

    // variables for hashing
    khash_t(str) *hash;  // the hash
    hash = kh_init(str);
    int ret;  // return value from hashing

    int l;
    while ((l = kseq_read(seq)) >= 0) {
        sprintf(seq_name, "%s", seq->name.s);
        flag_pass = 1;

        if ((min_size >= 0) && (seq->seq.l < min_size)) {
            flag_pass = 0;  // filter by -a min_size
        } else if ((max_size >= 0) && (seq->seq.l > max_size)) {
            flag_pass = 0;  // filter by -z max_size
        } else if (max_n >= 0) {
            if (count_n(seq->seq.s, seq->seq.l) > max_n) {
                flag_pass = 0;  // filter by -n max_n
            }
        } else if (flag_u) {
            // Extra return code:
            //     -1 if the operation failed;
            //     0 if the key is present in the hash table;
            //     1 if the bucket is empty (never used);
            //     2 if the element in the bucket has been deleted [int*]
            kh_put(str, hash, strdup(seq_name), &ret);
            if (ret == 0) {
                flag_pass = 0;
            }
        }

        if (flag_pass) {
            fprintf(stream_out, ">%s\n", seq_name);
            for (int i = 0; i < seq->seq.l; i++) {
                if (line != 0 && i != 0 && (i % line) == 0) {
                    fputc('\n', stream_out);
                }
                fputc(seq->seq.s[i], stream_out);
            }
            fputc('\n', stream_out);
        }
    }

    kseq_destroy(seq);
    gzclose(fp);
    if (strcmp(file_out, "stdout") != 0) {
        fclose(stream_out);
    }
    return 0;
}

int fa_split_name(int argc, char *argv[]) {
    int option = 0, line = 80;

    while ((option = getopt(argc, argv, "l:")) != -1) {
        switch (option) {
            case 'l':
                line = atoi(optarg);
                break;
        }
    }

    if (optind + 2 > argc) {
        fprintf(stderr,
                "\n"
                        "faops split-name - Split an fa file into several files\n"
                        "                   Using sequence names as file names\n"
                        "usage:\n"
                        "    faops split-name [options] <in.fa> <outdir>\n"
                        "\n"
                        "options:\n"
                        "    -l INT     sequence line length [%d]\n"
                        "\n"
                        "in.fa  == stdin  means reading from stdin\n"
                        "\n",
                line);
        exit(1);
    }

    char *file_in = argv[optind];
    char *path_out = argv[optind + 1];

    FILE *stream_in = source_in(file_in);
    gzFile fp;
    kseq_t *seq;
    FILE *fp_out;
    char seq_name[512];
    char file_out[1024];

#ifdef __MINGW32__
    _mkdir(path_out);
#else
    mkdir(path_out, 0777);
#endif

    fp = gzdopen(fileno(stream_in), "r");
    seq = kseq_init(fp);

    int l;
    while ((l = kseq_read(seq)) >= 0) {
        sprintf(seq_name, "%s", seq->name.s);

        sprintf(file_out, "%s/%s.fa", path_out, seq_name);
        if ((fp_out = fopen(file_out, "w")) == NULL) {
            fprintf(stderr, "Can't open output file [%s]\n", file_out);
            exit(1);
        }

        fprintf(fp_out, ">%s\n", seq_name);

        for (int i = 0; i < seq->seq.l; i++) {
            if (line != 0 && i != 0 && (i % line) == 0) {
                fputc('\n', fp_out);
            }
            fputc(seq->seq.s[i], fp_out);
        }
        fputc('\n', fp_out);

        fclose(fp_out);
    }

    kseq_destroy(seq);
    gzclose(fp);
    return 0;
}

int fa_split_about(int argc, char *argv[]) {
    int option = 0, line = 80;

    while ((option = getopt(argc, argv, "l:")) != -1) {
        switch (option) {
            case 'l':
                line = atoi(optarg);
                break;
        }
    }

    if (optind + 3 > argc) {
        fprintf(
                stderr,
                "\n"
                        "faops split-about - Split an fa file into several files\n"
                        "                    of about approx_size bytes each by record\n"
                        "usage:\n"
                        "    faops split-about [options] <in.fa> <approx_size> <outdir>\n"
                        "\n"
                        "options:\n"
                        "    -l INT     sequence line length [%d]\n"
                        "\n",
                line);
        exit(1);
    }

    char *file_in = argv[optind];
    int approx_size = atoi(argv[optind + 1]);
    char *path_out = argv[optind + 2];

    FILE *stream_in = source_in(file_in);
    gzFile fp;
    kseq_t *seq;
    FILE *fp_out;
    char seq_name[512];
    char file_out[1024];
    int cur_size = 0, file_count = 0, flag_first = 1;

#ifdef __MINGW32__
    _mkdir(path_out);
#else
    mkdir(path_out, 0777);
#endif

    fp = gzdopen(fileno(stream_in), "r");
    seq = kseq_init(fp);

    int l;
    while ((l = kseq_read(seq)) >= 0) {
        if (cur_size == 0) {
            if (flag_first) {
                flag_first = 0;
            } else {
                fclose(fp_out);
            }

            sprintf(file_out, "%s/%03d.fa", path_out, file_count);
            file_count++;

            if ((fp_out = fopen(file_out, "w")) == NULL) {
                fprintf(stderr, "Can't open output file [%s]\n", file_out);
                exit(1);
            }
        }
        sprintf(seq_name, "%s", seq->name.s);
        fprintf(fp_out, ">%s\n", seq_name);

        for (int i = 0; i < seq->seq.l; i++) {
            if (line != 0 && i != 0 && (i % line) == 0) {
                fputc('\n', fp_out);
            }
            fputc(seq->seq.s[i], fp_out);
        }
        fputc('\n', fp_out);

        cur_size += seq->seq.l;
        if (cur_size >= approx_size) {
            cur_size = 0;
        }
    }

    kseq_destroy(seq);
    gzclose(fp);
    return 0;
}

char *version = "0.2.2";
char *message =
        "\n"
                "Usage:     faops <command> [options] <arguments>\n"
                "Version:   %s\n"
                "\n"
                "Commands:\n"
                "    help           print this message\n"
                "    count          Count base statistics in FA file(s)\n"
                "    size           Count total bases in FA file(s)\n"
                "    frag           Extract subsequences from a FA file\n"
                "    rc             Reverse complement a FA file\n"
                "    some           Extract some fa records.\n"
                "    filter         Filter fa records.\n"
                "    split-name     Splitting by sequence names\n"
                "    split-about    Splitting to chunks about specified size\n"
                "\n"
                "Options:\n"
                "    There're no global options.\n"
                "    Type \"faops command-name\" for detailed options of each command.\n"
                "    Options *MUST* be placed just after subcommand.\n"
                "\n";

static int usage() {
    fprintf(stderr, message, version);
    return 1;
}

static int help() {
    fprintf(stdout, message, version);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc == 1) return usage();

    init_nt_val();
    init_nt_comp();

    if (strcmp(argv[1], "count") == 0)
        fa_count(argc - 1, argv + 1);
    else if (strcmp(argv[1], "size") == 0)
        fa_size(argc - 1, argv + 1);
    else if (strcmp(argv[1], "frag") == 0)
        fa_frag(argc - 1, argv + 1);
    else if (strcmp(argv[1], "rc") == 0)
        fa_rc(argc - 1, argv + 1);
    else if (strcmp(argv[1], "some") == 0)
        fa_some(argc - 1, argv + 1);
    else if (strcmp(argv[1], "filter") == 0)
        fa_filter(argc - 1, argv + 1);
    else if (strcmp(argv[1], "split-name") == 0)
        fa_split_name(argc - 1, argv + 1);
    else if (strcmp(argv[1], "split-about") == 0)
        fa_split_about(argc - 1, argv + 1);
    else if (strcmp(argv[1], "help") == 0)
        return help();
    else {
        fprintf(stderr, "[main] unrecognized commad '%s'. Abort!\n", argv[1]);
        return 1;
    }
    return 0;
}
