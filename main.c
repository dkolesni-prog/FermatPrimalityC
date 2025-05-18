#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>
#include <gmp.h>

#define INITIAL_CAP 1024

static uint64_t *carmichael_list = NULL;
static size_t   carmichael_count = 0;

// Загружает из файла только те Carmichael-числа, что ≤ max_val
void load_carmichael(const char *filename, uint64_t max_val) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen carmichael");
        exit(1);
    }
    size_t cap = INITIAL_CAP;
    carmichael_list = malloc(cap * sizeof *carmichael_list);
    uint64_t x;
    while (fscanf(f, "%" SCNu64, &x) == 1) {
        if (x > max_val) break;           // дальше в файле все большие
        if (carmichael_count >= cap) {
            cap *= 2;
            carmichael_list = realloc(carmichael_list, cap * sizeof *carmichael_list);
        }
        carmichael_list[carmichael_count++] = x;
    }
    fclose(f);
}

// бинарный поиск: true если n есть в carmichael_list
bool is_carmichael(uint64_t n) {
    size_t lo = 0, hi = carmichael_count;
    while (lo < hi) {
        size_t mid = (lo + hi) >> 1;
        if (carmichael_list[mid] < n) lo = mid + 1;
        else hi = mid;
    }
    return lo < carmichael_count && carmichael_list[lo] == n;
}

// RNG init
void init_rng(gmp_randstate_t state) {
    gmp_randinit_default(state);
    gmp_randseed_ui(state, (unsigned long)time(NULL));
}

// random a in [2..n−2] with gcd(a,n)=1
void generate_random_coprime(mpz_t base, const mpz_t n,
                             gmp_randstate_t state) {
    mpz_t g; mpz_init(g);
    do {
        mpz_urandomm(base, state, n);
        if (mpz_cmp_ui(base, 2) < 0)        mpz_add_ui(base, base, 2);
        else if (mpz_cmp(base, n) >= 0)     mpz_mod(base, base, n);
        mpz_gcd(g, base, n);
    } while (mpz_cmp_ui(g, 1) != 0);
    mpz_clear(g);
}

// Fermat test: returns 1 if passes all k bases, 0 if composite (witness set)
int fermat_test_gmp(const mpz_t n, int k, mpz_t witness,
                    gmp_randstate_t state) {
    mpz_t a, t, n_minus_one;
    mpz_inits(a, t, n_minus_one, NULL);
    mpz_sub_ui(n_minus_one, n, 1);

    for (int i = 0; i < k; i++) {
        generate_random_coprime(a, n, state);
        mpz_powm(t, a, n_minus_one, n);
        if (mpz_cmp_ui(t, 1) != 0) {
            mpz_set(witness, a);
            mpz_clears(a, t, n_minus_one, NULL);
            return 0;
        }
    }
    mpz_set_ui(witness, 0);
    mpz_clears(a, t, n_minus_one, NULL);
    return 1;
}

int main() {
    int     bit_length = 32;
    int     k          = 16;
    uint64_t MAX_VAL   = (1ULL << bit_length) - 1;
    uint64_t CHUNK     = 1000000;

    // 1) загрузить Carmichael ≤ MAX_VAL
    load_carmichael("data/pureCarmichael.txt", MAX_VAL);

    // 2) инициализация GMP
    mpz_t n, witness;
    gmp_randstate_t state;
    mpz_inits(n, witness, NULL);
    init_rng(state);

    // 3) открыть CSV для вывода
    FILE *out = fopen("data/results.csv", "w");
    if (!out) { perror("fopen results.csv"); return 1; }
    // заголовок CSV
    fprintf(out, "n,bit_len,is_probably_prime,elapsed_ns,witness,is_really_prime\n");

    struct timespec t1, t2;
    for (uint64_t start = 1; start <= MAX_VAL; start += CHUNK) {
        uint64_t end = start + CHUNK - 1;
        if (end > MAX_VAL) end = MAX_VAL;

        for (uint64_t x = start; x <= end; ++x) {
            mpz_set_ui(n, x);

            clock_gettime(CLOCK_MONOTONIC, &t1);
            int is_prime = fermat_test_gmp(n, k, witness, state);
            clock_gettime(CLOCK_MONOTONIC, &t2);

            uint64_t elapsed = (t2.tv_sec - t1.tv_sec) * 1000000000ULL
                               + (t2.tv_nsec - t1.tv_nsec);

            // если Fermat сказал «возможно простое», то real = not Carmichael
            // вычислим битовую длину n = x
            int bit_len = (x == 0 ? 1 : 64 - __builtin_clzll(x));
            int is_really_prime = is_prime ? !is_carmichael(x) : 0;

            if (is_prime) {
                // нет witness
                fprintf(out,
                        "%" PRIu64 ",%d,1,%" PRIu64 ",,%d\n",
                        x,
                        bit_len,
                        elapsed,
                        is_really_prime);
            } else {
                char *ws = mpz_get_str(NULL, 10, witness);
                fprintf(out,
                        "%" PRIu64 ",%d,0,%" PRIu64 ",%s,%d\n",
                        x,
                        bit_len,
                        elapsed,
                        ws,
                        is_really_prime);
                free(ws);
            }
        }
    }

    // 4) очистка и выход
    mpz_clears(n, witness, NULL);
    gmp_randclear(state);
    free(carmichael_list);
    fclose(out);
    return 0;
}
