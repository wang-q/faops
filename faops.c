#define _GNU_SOURCE

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

// khash init
#include "khash.h"

// **str2int** will be the name of HASH throughout the whole file
// key is string, value is int
KHASH_MAP_INIT_STR(str2int, int)

// **str2str** will be the name of HASH throughout the whole file
// key is string, value is string
KHASH_MAP_INIT_STR(str2str, char*)

// this macro doesn't work on a decayed pointer,
// e.g. array name passed to a subroutine
#define ArraySize(a) (sizeof(a) / sizeof((a)[0]))

enum {
    BUFFER_SIZE = 2 ^ 16,
    BUFFER_SIZE_LONG = 2 ^ 32
};

enum {
    T_BASE_VAL = 0,
    U_BASE_VAL = 0,
    C_BASE_VAL = 1,
    A_BASE_VAL = 2,
    G_BASE_VAL = 3,
    N_BASE_VAL = 4
};

// Standard IUB/IUPAC Nucleic Acid Codes
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
int nt_val[256];

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
int nt_comp[256];

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

// for qsort
static int compare_long_desc(const void *a, const void *b) {
    long arg1 = *(const long *) a;
    long arg2 = *(const long *) b;

    if (arg1 < arg2) return 1;
    if (arg1 > arg2) return -1;
    return 0;
}

// count Ns in sequence
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

int is_n(char nt) {
    return (nt_val[(int) nt] == N_BASE_VAL || nt_val[(int) nt] == -1);
}

// convert IUPAC ambiguous codes to 'N'
char convert_n(char nt) {
    if (is_n(nt)) {
        return 'N';
    } else {
        return nt;
    }
}

// convert to Upper case
char convert_U(char nt) {
    return (char) toupper(nt);
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

    printf("#seq\tlen\tA\tC\tG\tT\tN");
    printf("\n");

    for (int f = 1; f < argc; ++f) {
        gzFile fp = gzdopen(fileno(source_in(argv[f])), "r");
        kseq_t *seq = kseq_init(fp);

        while (kseq_read(seq) >= 0) {
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
                "usage:\n"
                "    faops size <in.fa> [more_files.fa]\n"
                "\n"
                "in.fa  == stdin  means reading from stdin\n"
                "\n");
        exit(1);
    }

    for (int f = 1; f < argc; ++f) {
        gzFile fp = gzdopen(fileno(source_in(argv[f])), "r");
        kseq_t *seq = kseq_init(fp);

        while (kseq_read(seq) >= 0) {
            printf("%s\t%lu", seq->name.s, seq->seq.l);
            printf("\n");
        }

        kseq_destroy(seq);
        gzclose(fp);
    }

    return 0;
}

int fa_masked(int argc, char *argv[]) {
    int flag_g = 0;
    int option = 0;

    while ((option = getopt(argc, argv, "g")) != -1) {
        switch (option) {
            case 'g':
                flag_g = 1;
                break;
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
        }
    }

    if (argc == optind) {
        fprintf(
                stderr,
                "\n"
                "faops masked - Masked (or gaps) regions in fasta files\n"
                "usage:\n"
                "    faops masked [options] <in.fa> [more_files.fa]\n"
                "\n"
                "options:\n"
                "    -g         only record regions of N/n\n"
                "\n"
                "in.fa  == stdin  means reading from stdin\n"
                "\n");
        exit(1);
    }

    for (int f = optind; f < argc; ++f) {
        gzFile fp = gzdopen(fileno(source_in(argv[f])), "r");
        kseq_t *seq = kseq_init(fp);

        while (kseq_read(seq) >= 0) {
            int begin = -1, end = -1;
            for (int i = 0; i < seq->seq.l; i++) {
                if (flag_g
                    ? is_n(seq->seq.s[i])
                    : (is_n(seq->seq.s[i]) || islower(seq->seq.s[i]))) {
                    if (begin == -1) {
                        begin = i, end = i;
                    } else {
                        end = i;
                    }
                } else if (begin != -1) {
                    if (begin == end) {
                        printf("%s:%u\n", seq->name.s, begin + 1);
                    } else {
                        printf("%s:%u-%u\n", seq->name.s, begin + 1, end + 1);
                    }
                    // reset
                    begin = -1, end = -1;
                }


            }

            // last region
            if (begin != -1) {
                if (begin == end) {
                    printf("%s:%u\n", seq->name.s, begin + 1);
                } else {
                    printf("%s:%u-%u\n", seq->name.s, begin + 1, end + 1);
                }
            }
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
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
        }
    }

    if (optind + 4 > argc || !isdigit(argv[optind + 1][0]) ||
        !isdigit(argv[optind + 2][0])) {
        fprintf(stderr,
                "\n"
                "faops frag - Extract a piece of DNA from a FA file.\n"
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

    if (start > end) {
        fprintf(stderr, "start [%d] > end [%d]\n", start, end);
        exit(1);
    }

    gzFile fp = gzdopen(fileno(source_in(file_in)), "r");
    kseq_t *seq = kseq_init(fp);

    FILE *stream_out = source_out(file_out);

    char seq_name[512];
    int is_first = 1;

    while (kseq_read(seq) >= 0) {
        if (!is_first) {
            fprintf(stderr, "More than one sequence in %s, just using first\n",
                    file_in);
            break;
        }

        if (end > seq->seq.l) {
            fprintf(stderr, "%s only has %zu bases\n", seq->name.s,
                    seq->seq.l);
            exit(1);
        }

        if (start < end) {
            sprintf(seq_name, "%s:%d-%d", seq->name.s, start, end);
        } else {
            sprintf(seq_name, "%s:%d", seq->name.s, start);
        }
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
    char *fn_list = "";
    int option = 0, line = 80;

    while ((option = getopt(argc, argv, "nrcf:l:")) != -1) {
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
            case 'f':
                fn_list = strdup(optarg);
                break;
            case 'l':
                line = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
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
                "    -r         just Reverse, prepends R_\n"
                "    -c         just Complement, prepends C_\n"
                "    -f STR     only RC sequences in this list.file\n"
                "    -l INT     sequence line length [%d]\n"
                "\n"
                "in.fa  == stdin  means reading from stdin\n"
                "out.fa == stdout means writing to stdout\n"
                "\n",
                line);
        exit(1);
    }

    //  Read list.file to a hash table
    khash_t(str2int) *hash;
    hash = kh_init(str2int);

    int serial = 0; // serials in list.file
    if (strcmp(fn_list, "") != 0) {
        FILE *fp_list;

        if ((fp_list = fopen(fn_list, "r")) == NULL) {
            fprintf(stderr, "Cannot open list file [%s]\n", fn_list);
            exit(1);
        }

        int ret;            // return value from hashing
        int buf_size = BUFFER_SIZE;
        char buf[buf_size]; // buffers for names in list.file
        while (fscanf(fp_list, "%s\n", buf) == 1) {
            khint_t entry = kh_put(str2int, hash, strdup(buf), &ret);
            kh_val(hash, entry) = serial;
            serial++;
        }
        fclose(fp_list);
    }

    // in and out
    char *fn_in = argv[optind];
    char *fn_out = argv[optind + 1];

    gzFile fp = gzdopen(fileno(source_in(fn_in)), "r");
    kseq_t *seq = kseq_init(fp);

    FILE *stream_out = source_out(fn_out);

    while (kseq_read(seq) >= 0) {
        char seq_name[512];
        sprintf(seq_name, "%s", seq->name.s);
        if (strcmp(fn_list, "") != 0) {
            if (kh_get(str2int, hash, seq_name) != kh_end(hash)) {
                sprintf(seq_name, "%s%s", prefix, seq->name.s);

                if (flag_r) {
                    reverse_str(seq->seq.s, seq->seq.l);
                } else if (flag_c) {
                    complement_str(seq->seq.s, seq->seq.l);
                } else {
                    reverse_str(seq->seq.s, seq->seq.l);
                    complement_str(seq->seq.s, seq->seq.l);
                }
            }
        } else {
            sprintf(seq_name, "%s%s", prefix, seq->name.s);

            if (flag_r) {
                reverse_str(seq->seq.s, seq->seq.l);
            } else if (flag_c) {
                complement_str(seq->seq.s, seq->seq.l);
            } else {
                reverse_str(seq->seq.s, seq->seq.l);
                complement_str(seq->seq.s, seq->seq.l);
            }
        }

        fprintf(stream_out, ">%s\n", seq_name);

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
    if (strcmp(fn_out, "stdout") != 0) {
        fclose(stream_out);
    }
    return 0;
}

int fa_one(int argc, char *argv[]) {
    int option = 0, line = 80;

    while ((option = getopt(argc, argv, "l:")) != -1) {
        switch (option) {
            case 'l':
                line = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
        }
    }

    if (optind + 3 > argc) {
        fprintf(stderr,
                "\n"
                "faops one - Extract one fa sequence\n"
                "usage:\n"
                "    faops some [options] <in.fa> <name> <out.fa>\n"
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
    char *name = argv[optind + 1];
    char *file_out = argv[optind + 2];

    gzFile fp = gzdopen(fileno(source_in(file_in)), "r");
    kseq_t *seq = kseq_init(fp);

    FILE *stream_out = source_out(file_out);

    char seq_name[512];

    while (kseq_read(seq) >= 0) {
        sprintf(seq_name, "%s", seq->name.s);

        if (strcmp(seq_name, name) == 0) {
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
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
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

    gzFile fp = gzdopen(fileno(source_in(file_in)), "r");
    kseq_t *seq = kseq_init(fp);

    FILE *stream_out = source_out(file_out);

    char seq_name[512];

    // variables for hashing
    // from Heng Li's replay to http://www.biostars.org/p/10353/
    // and https://github.com/attractivechaos/klib/issues/49
    khash_t(str2int) *hash; // the hash of list
    hash = kh_init(str2int);
    khint_t key;        // the key

    //  Read list.file to a hash table
    int serial = 0;// serials in list.file
    {
        FILE *fp_list;

        if ((fp_list = fopen(file_list, "r")) == NULL) {
            fprintf(stderr, "Cannot open list file [%s]\n", file_list);
            exit(1);
        }

        int ret;            // return value from hashing
        int buf_size = BUFFER_SIZE;
        char buf[buf_size]; // buffers for names in list.file
        while (fscanf(fp_list, "%s\n", buf) == 1) {
            key = kh_put(str2int, hash, strdup(buf), &ret);
            kh_val(hash, key) = serial;
            serial++;
        }
        fclose(fp_list);
    }

    int flag_key = 0;   // check keys' exists
    while (kseq_read(seq) >= 0) {
        sprintf(seq_name, "%s", seq->name.s);
        flag_key = (kh_get(str2int, hash, seq_name) != kh_end(hash));

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

    kh_destroy(str2int, hash);
    kseq_destroy(seq);
    gzclose(fp);
    if (strcmp(file_out, "stdout") != 0) {
        fclose(stream_out);
    }
    return 0;
}

static void copy_kstr(kstring_t *dst, const kstring_t *src) {
    if (src->l == 0) return;
    if (src->l + 1 > dst->m) {
        dst->m = src->l + 1;
        kroundup32(dst->m);
        dst->s = realloc(dst->s, dst->m);
    }
    dst->l = src->l;
    memcpy(dst->s, src->s, src->l + 1);
}

static void copy_kseq(kseq_t *dst, const kseq_t *src) {
    copy_kstr(&dst->name, &src->name);
    copy_kstr(&dst->seq, &src->seq);
    copy_kstr(&dst->qual, &src->qual);
    copy_kstr(&dst->comment, &src->comment);
}

int fa_order(int argc, char *argv[]) {
    int option = 0, line = 80;

    while ((option = getopt(argc, argv, "il:")) != -1) {
        switch (option) {
            case 'l':
                line = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
        }
    }

    if (optind + 3 > argc) {
        fprintf(stderr,
                "\n"
                "faops order - Extract multiple fa sequences by the given order.\n"
                "              Consume much more memory for load all sequences in memory.\n"
                "usage:\n"
                "    faops order [options] <in.fa> <list.file> <out.fa>\n"
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
    char *file_list = argv[optind + 1];
    char *file_out = argv[optind + 2];

    gzFile fp = gzdopen(fileno(source_in(file_in)), "r");
    kseq_t *seq = kseq_init(fp);

    FILE *stream_out = source_out(file_out);

    // variables for hashing
    // from Heng Li's replay to http://www.biostars.org/p/10353/
    // and https://github.com/attractivechaos/klib/issues/49
    khash_t(str2int) *hash; // the hash of list
    hash = kh_init(str2int);
    khint_t entry;        // the key-value pair

    //  Read list.file to a hash table
    int serial = 0;// serials in list.file
    {
        FILE *fp_list;

        if ((fp_list = fopen(file_list, "r")) == NULL) {
            fprintf(stderr, "Cannot open list file [%s]\n", file_list);
            exit(1);
        }

        int ret;            // return value from hashing
        int buf_size = BUFFER_SIZE;
        char buf[buf_size]; // buffers for names in list.file
        while (fscanf(fp_list, "%s\n", buf) == 1) {
            entry = kh_put(str2int, hash, strdup(buf), &ret);
            kh_val(hash, entry) = serial;
//            fprintf(stderr, "Key: [%s];\tValue:[%d]\n", kh_key(hash, entry), kh_val(hash, entry));

            serial++;
        }
        fclose(fp_list);
    }

    // load all sequences into buffer
    kseq_t *buf_seq = 0;
    buf_seq = calloc((size_t) serial, sizeof(kseq_t));

    while (kseq_read(seq) >= 0) {
        char seq_name[512];
        sprintf(seq_name, "%s", seq->name.s);

        entry = kh_get(str2int, hash, seq_name);
        if (entry != kh_end(hash)) {
            copy_kseq(&buf_seq[kh_val(hash, entry)], seq);
        }
    }

    for (int idx = 0; idx < serial; ++idx) {
        kseq_t *seq_l = &buf_seq[idx];

        fprintf(stream_out, ">%s\n", seq_l->name.s);
        for (int i = 0; i < seq_l->seq.l; i++) {
            if (line != 0 && i != 0 && (i % line) == 0) {
                fputc('\n', stream_out);
            }
            fputc(seq_l->seq.s[i], stream_out);
        }
        fputc('\n', stream_out);

        free(seq_l->seq.s);
        free(seq_l->qual.s);
        free(seq_l->name.s);
    }

    if (buf_seq != NULL) free(buf_seq);
    kh_destroy(str2int, hash);
    kseq_destroy(seq);
    gzclose(fp);
    if (strcmp(file_out, "stdout") != 0) {
        fclose(stream_out);
    }
    return 0;
}

int fa_replace(int argc, char *argv[]) {
    int flag_s = 0;
    int option = 0, line = 80;

    while ((option = getopt(argc, argv, "sl:")) != -1) {
        switch (option) {
            case 's':
                flag_s = 1;
                break;
            case 'l':
                line = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
        }
    }

    if (optind + 3 > argc) {
        fprintf(stderr,
                "\n"
                "faops replace - Replace headers from a FA file\n"
                "usage:\n"
                "    faops replace [options] <in.fa> <replace.tsv> <out.fa>\n"
                "\n"
                "options:\n"
                "    -s         only output sequences in the list, like `faops some`\n"
                "    -l INT     sequence line length [%d]\n"
                "\n"
                "<replace.tsv> is a tab-separated file containing two fields\n"
                "    original_name\treplace_name\n"
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

    gzFile fp = gzdopen(fileno(source_in(file_in)), "r");
    kseq_t *seq = kseq_init(fp);

    FILE *stream_out = source_out(file_out);
    char seq_name[512];

    //  Read replace.tsv to a hash table
    khash_t(str2str) *hash;
    hash = kh_init(str2str);
    {
        FILE *fp_list;

        if ((fp_list = fopen(file_list, "r")) == NULL) {
            fprintf(stderr, "Cannot open list file [%s]\n", file_list);
            exit(1);
        }

        int ret;            // return value from hashing
        int buf_size = BUFFER_SIZE;
        char buf1[buf_size]; // buffers for original_name in replace.tsv
        char buf2[buf_size]; // buffers for replace_name in replace.tsv
        while (fscanf(fp_list, "%s\t%s\n", buf1, buf2) == 2) {
            khint_t entry = kh_put(str2str, hash, strdup(buf1), &ret);
            kh_val(hash, entry) = strdup(buf2);
//            fprintf(stderr, "Key: [%s];\tValue:[%s]\n", kh_key(hash, entry), kh_val(hash, entry));
        }
        fclose(fp_list);
    }

    while (kseq_read(seq) >= 0) {
        sprintf(seq_name, "%s", seq->name.s);

        khint_t entry = kh_get(str2str, hash, seq_name);
        if (entry != kh_end(hash)) {
            fprintf(stream_out, ">%s\n", kh_val(hash, entry));
        } else {
            if (flag_s) {
                continue;
            }
            fprintf(stream_out, ">%s\n", seq_name);
        }

        for (int i = 0; i < seq->seq.l; i++) {
            if (line != 0 && i != 0 && (i % line) == 0) {
                fputc('\n', stream_out);
            }
            fputc(seq->seq.s[i], stream_out);
        }
        fputc('\n', stream_out);
    }

    kh_destroy(str2str, hash);
    kseq_destroy(seq);
    gzclose(fp);
    if (strcmp(file_out, "stdout") != 0) {
        fclose(stream_out);
    }
    return 0;
}

int fa_filter(int argc, char *argv[]) {
    int flag_u = 0, flag_U = 0, flag_b = 0, flag_N = 0, flag_d = 0, flag_s = 0;
    int min_size = -1, max_size = -1, max_n = -1;
    int option = 0, opt_line = 80;

    while ((option = getopt(argc, argv, "uUbNdsa:z:n:l:")) != -1) {
        switch (option) {
            case 'u':
                flag_u = 1;
                break;
            case 'U':
                flag_U = 1;
                break;
            case 'b':
                flag_b = 1;
                break;
            case 'N':
                flag_N = 1;
                break;
            case 'd':
                flag_d = 1;
                break;
            case 's':
                flag_s = 1;
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
                opt_line = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
        }
    }

    if (flag_b) {
        opt_line = 0;
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
                "    -u         Unique, removes duplicated ids, keeping the first\n"
                "    -U         Upper case, converts all sequences to upper cases\n"
                "    -b         pretend to be a blocked fasta file\n"
                "    -N         convert IUPAC ambiguous codes to 'N'\n"
                "    -d         remove dashes '-'\n"
                "    -s         simplify sequence names\n"
                "    -l INT     sequence line length [%d]\n"
                "\n"
                "in.fa  == stdin  means reading from stdin\n"
                "out.fa == stdout means writing to stdout\n"
                "\n"
                "Not all faFilter options were implemented.\n"
                "Names' wildcards are easily accomplished by \"faops some\".\n"
                "\n",
                opt_line);
        exit(1);
    }

    char *file_in = argv[optind];
    char *file_out = argv[optind + 1];

    gzFile fp = gzdopen(fileno(source_in(file_in)), "r");
    kseq_t *seq = kseq_init(fp);

    FILE *stream_out = source_out(file_out);
    char seq_name[512];
    int flag_pass;

    // variables for hashing
    khash_t(str2int) *hash;  // the hash
    hash = kh_init(str2int);
    int ret;  // return value from hashing

    while (kseq_read(seq) >= 0) {
        if (flag_s) {
            sprintf(seq_name, "%s", strtok(seq->name.s, " ,.-"));
        } else {
            sprintf(seq_name, "%s", seq->name.s);
        }
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
            kh_put(str2int, hash, strdup(seq_name), &ret);
            if (ret == 0) {
                flag_pass = 0;
            }
        }

        if (flag_pass) {
            fprintf(stream_out, ">%s\n", seq_name);
            for (int i = 0; i < seq->seq.l; i++) {
                if (opt_line != 0 && i != 0 && (i % opt_line) == 0) {
                    fputc('\n', stream_out);
                }
                if (flag_d) {
                    if (seq->seq.s[i] == '-') {
                        continue;
                    }
                }
                if (flag_N) {
                    if (flag_U) {
                        fputc(convert_U(convert_n(seq->seq.s[i])), stream_out);
                    } else {
                        fputc(convert_n(seq->seq.s[i]), stream_out);
                    }
                } else {
                    if (flag_U) {
                        fputc(convert_U(seq->seq.s[i]), stream_out);
                    } else {
                        fputc(seq->seq.s[i], stream_out);
                    }
                }
            }
            fputc('\n', stream_out);

            if (flag_b) {
                fputc('\n', stream_out);
            }
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
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
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

    gzFile fp = gzdopen(fileno(source_in(file_in)), "r");
    kseq_t *seq = kseq_init(fp);

    FILE *fp_out;
    char seq_name[512];
    char file_out[1024];

#ifdef __MINGW32__
    _mkdir(path_out);
#else
    mkdir(path_out, 0777);
#endif

    while (kseq_read(seq) >= 0) {
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
    int max_parts = 0;
    int flag_even = 0;
    int option = 0, line = 80;

    while ((option = getopt(argc, argv, "el:m:")) != -1) {
        switch (option) {
            case 'e':
                flag_even = 1;
                break;
            case 'm':
                max_parts = atoi(optarg);
                break;
            case 'l':
                line = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
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
                "    -e         sequences in one file should be EVEN\n"
                "    -m INT     max parts\n"
                "    -l INT     sequence line length [%d]\n"
                "\n",
                line);
        exit(1);
    }

    char *file_in = argv[optind];
    long approx_size = atol(argv[optind + 1]);
    char *path_out = argv[optind + 2];

    gzFile fp = gzdopen(fileno(source_in(file_in)), "r");
    kseq_t *seq = kseq_init(fp);

    FILE *fp_out;
    char seq_name[512];
    char file_out[1024];
    long cur_size = 0;
    long cur_count = 0;
    int file_count = 0, flag_first = 1;

#ifdef __MINGW32__
    _mkdir(path_out);
#else
    mkdir(path_out, 0777);
#endif

    while (kseq_read(seq) >= 0) {
        if (cur_size == 0) {
            if (flag_first) {
                flag_first = 0;
            } else {
                fclose(fp_out);
            }

            sprintf(file_out, "%s/%03d.fa", path_out, file_count);
            file_count++;

            if (max_parts > 0 && file_count > max_parts) {
                kseq_destroy(seq);
                gzclose(fp);
                return 0;
            }

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

        if (flag_even) {
            cur_count++;

            if (cur_count % 2 != 0) {
                continue;
            }
        }
        if (cur_size >= approx_size) {
            cur_size = 0;
            cur_count = 0;
        }
    }

    kseq_destroy(seq);
    gzclose(fp);
    return 0;
}

int fa_n50(int argc, char *argv[]) {
    int flag_no_header = 0;
    int flag_sum = 0;
    int flag_average = 0;
    int flag_e_size = 0;
    int flag_count = 0;
    long genome_size = 0;
    int n_given = 50;
    int option = 0;

    while ((option = getopt(argc, argv, "HSAECg:N:")) != -1) {
        switch (option) {
            case 'H':
                flag_no_header = 1;
                break;
            case 'S':
                flag_sum = 1;
                break;
            case 'A':
                flag_average = 1;
                break;
            case 'E':
                flag_e_size = 1;
                break;
            case 'C':
                flag_count = 1;
                break;
            case 'g':
                genome_size = atol(optarg);
                break;
            case 'N':
                n_given = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
        }
    }

    if (optind + 1 > argc) {
        fprintf(stderr,
                "\n"
                "faops n50 - compute N50 and other statistics.\n"
                "usage:\n"
                "    faops n50 [options] <in.fa> [more_files.fa]\n"
                "\n"
                "options:\n"
                "    -H         do not display header\n"
                "    -N INT     compute Nx statistic [%d]\n"
                "    -S         compute sum of size of all entries\n"
                "    -A         compute average length of all entries\n"
                "    -E         compute the E-size (from GAGE)\n"
                "    -C         count entries\n"
                "    -g INT     size of genome, instead of total size in files\n"
                "\n"
                "in.fa  == stdin  means reading from stdin\n"
                "\n",
                n_given);
        exit(1);
    }

    gzFile fp;
    kseq_t *seq;

    long capacity = BUFFER_SIZE;
    long *lengths = (long *) malloc(capacity * sizeof(long)); // store lengths of sequences
    long count = 0; // number of sequences
    long total_size = 0;

    for (int f = optind; f < argc; ++f) {
        FILE *stream_in = source_in(argv[f]);
        fp = gzdopen(fileno(stream_in), "r");
        seq = kseq_init(fp);

        while (kseq_read(seq) >= 0) {
            total_size += seq->seq.l;

            // increase capacity on necessary
            if (count > capacity - 1) {
                capacity = capacity * 2;
                lengths = (long *) realloc(lengths, capacity * sizeof(long));
            }

            lengths[count] = (long) seq->seq.l;
            count++;
        }

        kseq_destroy(seq);
        gzclose(fp);
    }

    qsort(lengths, (size_t) count, sizeof(long), compare_long_desc);

    long goal; // reach n_given% of total_size or genome_size
    if (genome_size > 0) {
        goal = (long) (((double) n_given) * ((double) genome_size) / 100.0);
    } else {
        goal = (long) (((double) n_given) * ((double) total_size) / 100.0);
    }

    long cumulative_size = 0;
    double e_size = 0.0; // GAGE E-size
    long nx_size = 0; // N50 or Nx

    for (long i = 0; i < count; ++i) {
        long cur_size = lengths[i];

        long prev_cumulative_size = cumulative_size;
        cumulative_size += cur_size;

        e_size = ((double) prev_cumulative_size / (double) cumulative_size) * e_size +
                 (double) (cur_size * cur_size) / cumulative_size;

        if ((0 == nx_size) && (cumulative_size > goal)) {
            nx_size = cur_size;
        }
    }

    // print n50, N == 0 to skip this
    if (n_given) {
        if (!flag_no_header) {
            printf("N%d\t", n_given);
        }
        printf("%li\n", nx_size);
    }

    // print sum
    if (flag_sum) {
        if (!flag_no_header) {
            printf("S\t");
        }
        printf("%ld\n", total_size);

    }

    // print average
    if (flag_average) {
        if (!flag_no_header) {
            printf("A\t");
        }
        printf("%.2f\n", (double) total_size / count);
    }

    // print E-size
    if (flag_e_size) {
        if (!flag_no_header) {
            printf("E\t");
        }
        printf("%.2f\n", e_size);
    }

    // print count
    if (flag_count) {
        if (!flag_no_header) {
            printf("C\t");
        }
        printf("%li\n", count);
    }

    return 0;
}

int fa_dazz(int argc, char *argv[]) {
    char *prefix = "read";
    long start_index = 1;
    int flag_all = 0;
    int option = 0, opt_line = 80;

    while ((option = getopt(argc, argv, "p:s:al:")) != -1) {
        switch (option) {
            case 'p':
                prefix = optarg;
                break;
            case 's':
                start_index = atol(optarg);
                break;
            case 'a':
                flag_all = 1;
                break;
            case 'l':
                opt_line = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
        }
    }

    if (optind + 2 > argc) {
        fprintf(
                stderr,
                "\n"
                "faops dazz - Rename records for dazz_db\n"
                "usage:\n"
                "    faops dazz [options] <in.fa> <out.fa>\n"
                "\n"
                "options:\n"
                "    -p STR     prefix of names [read]\n"
                "    -s INT     start index [1]\n"
                "    -a         don't drop duplicated ids\n"
                "    -l INT     sequence line length [%d]\n"
                "\n"
                "in.fa  == stdin  means reading from stdin\n"
                "out.fa == stdout means writing to stdout\n"
                "\n"
                "Sequences with duplicated ids will be dropped, keeping the first one.\n"
                "This command don't write a replace.tsv file, `anchr dazzname` does.\n"
                "\n",
                opt_line);
        exit(1);
    }

    char *file_in = argv[optind];
    char *file_out = argv[optind + 1];

    gzFile fp = gzdopen(fileno(source_in(file_in)), "r");
    kseq_t *seq = kseq_init(fp);

    FILE *stream_out = source_out(file_out);
    char seq_name[512];
    int flag_pass;

    // variables for hashing
    khash_t(str2int) *hash;  // the hash
    hash = kh_init(str2int);
    int ret;  // return value from hashing

    long serial_no = start_index; // serial
    while (kseq_read(seq) >= 0) {
        sprintf(seq_name, "%s", seq->name.s);
        flag_pass = 1;

        if (!flag_all) {
            // Extra return code:
            //    -1 if the operation failed;
            //     0 if the key is present in the hash table;
            //     1 if the bucket is empty (never used);
            //     2 if the element in the bucket has been deleted [int*]
            kh_put(str2int, hash, strdup(seq_name), &ret);
            if (ret == 0) {
                flag_pass = 0;
            }
        }

        if (flag_pass) {
            char new_name[512];
            sprintf(new_name, "%s/%zu/0_%zu", prefix, serial_no, seq->seq.l);

            fprintf(stream_out, ">%s\n", new_name);
            for (int i = 0; i < seq->seq.l; i++) {
                if (opt_line != 0 && i != 0 && (i % opt_line) == 0) {
                    fputc('\n', stream_out);
                }
                fputc(seq->seq.s[i], stream_out);
            }
            fputc('\n', stream_out);

            serial_no++;
        }
    }

    kseq_destroy(seq);
    gzclose(fp);
    if (strcmp(file_out, "stdout") != 0) {
        fclose(stream_out);
    }
    return 0;
}

int fa_interleave(int argc, char *argv[]) {
    int flag_fq = 0;
    char *prefix = "read";
    long start_index = 0;
    int option = 0;

    while ((option = getopt(argc, argv, "qp:s:")) != -1) {
        switch (option) {
            case 'q':
                flag_fq = 1;
                break;
            case 'p':
                prefix = optarg;
                break;
            case 's':
                start_index = atol(optarg);
                break;
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
        }
    }

    if (optind + 1 > argc) {
        fprintf(
                stderr,
                "\n"
                "faops interleave - Interleave two PE files\n"
                "                   One file is also OK, output a single `N`.\n"
                "                   With -q, the quality value set to `!` (33)\n"
                "usage:\n"
                "    faops interleave [options] <R1.fa> [R2.fa]\n"
                "\n"
                "options:\n"
                "    -q         write FQ. The inputs must be FQs\n"
                "    -p STR     prefix of names [read]\n"
                "    -s INT     start index [0]\n"
                "\n"
                "Write to stdout and don't support reading from stdin.\n"
                "\n");
        exit(1);
    }

    // Two files
    if (argc - optind == 2) {
        char *file_in1 = argv[optind];
        char *file_in2 = argv[optind + 1];

        gzFile fp1 = gzdopen(fileno(source_in(file_in1)), "r");
        gzFile fp2 = gzdopen(fileno(source_in(file_in2)), "r");

        kseq_t *seq[2];
        seq[0] = kseq_init(fp1);
        seq[1] = kseq_init(fp2);

        char seq_name[512];

        long serial_no = start_index; // serial
        while (kseq_read(seq[0]) >= 0) {
            if (kseq_read(seq[1]) < 0) {
                fprintf(stderr, "[%s] R2 has fewer records.\n", __func__);
                break;
            }

            if (flag_fq) {
                // R1
                sprintf(seq_name, "%s%zu", prefix, serial_no);
                fprintf(stdout, "@%s/1\n", seq_name);
                fprintf(stdout, "%s\n", seq[0]->seq.s);
                fprintf(stdout, "+\n");
                fprintf(stdout, "%s\n", seq[0]->qual.s);

                // R2
                sprintf(seq_name, "%s%zu", prefix, serial_no);
                fprintf(stdout, "@%s/2\n", seq_name);
                fprintf(stdout, "%s\n", seq[1]->seq.s);
                fprintf(stdout, "+\n");
                fprintf(stdout, "%s\n", seq[1]->qual.s);
            } else {
                // R1
                sprintf(seq_name, "%s%zu", prefix, serial_no);
                fprintf(stdout, ">%s/1\n", seq_name);
                fprintf(stdout, "%s\n", seq[0]->seq.s);

                // R2
                sprintf(seq_name, "%s%zu", prefix, serial_no);
                fprintf(stdout, ">%s/2\n", seq_name);
                fprintf(stdout, "%s\n", seq[1]->seq.s);
            }

            serial_no++;
        }
        if (kseq_read(seq[1]) >= 0) {
            fprintf(stderr, "[%s] R1 has fewer records.\n", __func__);
        }

        kseq_destroy(seq[0]);
        gzclose(fp1);
        kseq_destroy(seq[1]);
        gzclose(fp2);
    } else {
        char *file_in = argv[optind];

        gzFile fp = gzdopen(fileno(source_in(file_in)), "r");
        kseq_t *seq = kseq_init(fp);

        char seq_name[512];

        long serial_no = start_index; // serial
        while (kseq_read(seq) >= 0) {
            if (flag_fq) {
                // R1
                sprintf(seq_name, "%s%zu", prefix, serial_no);
                fprintf(stdout, "@%s/1\n", seq_name);
                fprintf(stdout, "%s\n", seq->seq.s);
                fprintf(stdout, "+\n");
                fprintf(stdout, "%s\n", seq->qual.s);

                // R2
                sprintf(seq_name, "%s%zu", prefix, serial_no);
                fprintf(stdout, "@%s/2\n", seq_name);
                fprintf(stdout, "N\n");
                fprintf(stdout, "+\n");
                fprintf(stdout, "!\n");
            } else {
                // R1
                sprintf(seq_name, "%s%zu", prefix, serial_no);
                fprintf(stdout, ">%s/1\n", seq_name);
                fprintf(stdout, "%s\n", seq->seq.s);

                // R2
                sprintf(seq_name, "%s%zu", prefix, serial_no);
                fprintf(stdout, ">%s/2\n", seq_name);
                fprintf(stdout, "N\n");
            }

            serial_no++;
        }

        kseq_destroy(seq);
        gzclose(fp);
    }

    return 0;
}

int fa_region(int argc, char *argv[]) {
    int flag_strand = 0;
    int option = 0, line = 80;

    while ((option = getopt(argc, argv, "sl:")) != -1) {
        switch (option) {
            case 's':
                flag_strand = 1;
                break;
            case 'l':
                line = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unsupported option\n");
                exit(1);
        }
    }

    if (optind + 3 > argc) {
        fprintf(stderr,
                "\n"
                "faops region - Extract regions from a FA file\n"
                "usage:\n"
                "    faops region [options] <in.fa> <region.txt> <out.fa>\n"
                "\n"
                "options:\n"
                "    -s         add strand '(+)' to headers\n"
                "    -l INT     sequence line length [%d]\n"
                "\n"
                "<region.txt> is a text file containing one field\n"
                "    seq_name:begin-end[,begin-end]\n"
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

    gzFile fp = gzdopen(fileno(source_in(file_in)), "r");
    kseq_t *seq = kseq_init(fp);

    FILE *stream_out = source_out(file_out);
    char seq_name[512];

    //  Read region.txt to hash table
    khash_t(str2str) *hash;
    hash = kh_init(str2str);
    {
        FILE *fp_list;

        if ((fp_list = fopen(file_list, "r")) == NULL) {
            fprintf(stderr, "Cannot open list file [%s]\n", file_list);
            exit(1);
        }

        char *lineptr = NULL;
        size_t len = 0;

        while (getline(&lineptr, &len, fp_list) != -1) {
            int ret;            // return value from hashing
            char buf1[512];    // buffers for seq_name in region.txt
            char buf2[BUFFER_SIZE_LONG];   // buffers for runlist in region.txt
            if (sscanf(lineptr, "%[^:]:%[0-9,-]\n", buf1, buf2) == 2) {
                khint_t entry = kh_put(str2str, hash, strdup(buf1), &ret);
                kh_val(hash, entry) = strdup(buf2);
            }
        }

        free(lineptr);
        fclose(fp_list);
    }

    while (kseq_read(seq) >= 0) {
        sprintf(seq_name, "%s", seq->name.s);

        khint_t entry = kh_get(str2str, hash, seq_name);
        if (entry != kh_end(hash)) {
            char *runlist = kh_val(hash, entry);

            if (strcmp("-", runlist) == 0) {
                continue;
            }

            char str[strlen(runlist) + 1], *token;
            strcpy(str, runlist);
            for (token = strtok(str, ",");
                 token != NULL;
                 token = strtok(NULL, ",")
                    ) {
                int begin, end;

                int number = sscanf(token, "%d-%d", &begin, &end);
                if (number == 1) {
                    end = begin;
                }

                if (flag_strand) {
                    if (begin < end) {
                        fprintf(stream_out, ">%s(+):%d-%d\n", seq_name, begin, end);
                    } else {
                        fprintf(stream_out, ">%s(+):%d\n", seq_name, begin);
                    }
                } else {
                    if (begin < end) {
                        fprintf(stream_out, ">%s:%d-%d\n", seq_name, begin, end);
                    } else {
                        fprintf(stream_out, ">%s:%d\n", seq_name, begin);
                    }
                }

                for (int i = begin - 1; i < end; i++) {
                    if (line != 0 && i != 0 && (i % line) == 0) {
                        fputc('\n', stream_out);
                    }
                    fputc(seq->seq.s[i], stream_out);
                }
                fputc('\n', stream_out);
            }
        }
    }

    kh_destroy(str2str, hash);
    kseq_destroy(seq);
    gzclose(fp);
    if (strcmp(file_out, "stdout") != 0) {
        fclose(stream_out);
    }
    return 0;
}

char *version = "0.8.21";
char *message =
        "\n"
        "Usage:     faops <command> [options] <arguments>\n"
        "Version:   %s\n"
        "\n"
        "Commands:\n"
        "    help           print this message\n"
        "    count          count base statistics in FA file(s)\n"
        "    size           count total bases in FA file(s)\n"
        "    masked         masked (or gaps) regions in FA file(s)\n"
        "    frag           extract sub-sequences from a FA file\n"
        "    rc             reverse complement a FA file\n"
        "    one            extract one fa record\n"
        "    some           extract some fa records\n"
        "    order          extract some fa records by the given order\n"
        "    replace        replace headers from a FA file\n"
        "    filter         filter fa records\n"
        "    split-name     splitting by sequence names\n"
        "    split-about    splitting to chunks about specified size\n"
        "    n50            compute N50 and other statistics\n"
        "    dazz           rename records for dazz_db\n"
        "    interleave     interleave two PE files\n"
        "    region         extract regions from a FA file\n"
        "\n"
        "Options:\n"
        "    There're no global options.\n"
        "    Type \"faops command-name\" for detailed options of each command.\n"
        "    Options *MUST* be placed just after command.\n"
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

    if (strcmp(argv[1], "count") == 0) {
        fa_count(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "size") == 0) {
        fa_size(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "masked") == 0) {
        fa_masked(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "frag") == 0) {
        fa_frag(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "rc") == 0) {
        fa_rc(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "one") == 0) {
        fa_one(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "some") == 0) {
        fa_some(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "order") == 0) {
        fa_order(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "replace") == 0) {
        fa_replace(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "filter") == 0) {
        fa_filter(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "split-name") == 0) {
        fa_split_name(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "split-about") == 0) {
        fa_split_about(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "n50") == 0) {
        fa_n50(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "dazz") == 0) {
        fa_dazz(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "interleave") == 0) {
        fa_interleave(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "region") == 0) {
        fa_region(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "help") == 0) {
        return help();
    } else {
        fprintf(stderr, "[main] unrecognized command '%s'. Abort!\n", argv[1]);
        return 1;
    }

    return 0;
}
