#define _POSIX_C_SOURCE 200809L
#include "renderer.h"
#include "claw_art_data.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ART_PIXELS ((size_t)CLAW_ART_WIDTH * (size_t)CLAW_ART_HEIGHT)
#define CELL_LIMIT 2000000U

static uint16_t *pixels = NULL;
static const char RAMP[] = " .claw";
static const char LETTERS[] = "Claw";
static const double LETTER_EDGES[] = {0.3155, 0.4665, 0.6563};

typedef struct { int r, g, b; } RGB;

static int b64_value(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static unsigned char *decode_base64_parts(const char *const *parts, size_t part_count,
                                          size_t *out_len) {
    size_t len = 0, n = 0;
    for (size_t p = 0; p < part_count; ++p) len += strlen(parts[p]);
    size_t cap = (len * 3U) / 4U + 4U;
    unsigned char *out = (unsigned char *)malloc(cap);
    if (!out) return NULL;
    unsigned int acc = 0;
    int bits = 0;
    for (size_t p = 0; p < part_count; ++p) {
        const char *s = parts[p];
        for (size_t i = 0; s[i] != '\0'; ++i) {
            if (s[i] == '=') { *out_len = n; return out; }
            int v = b64_value(s[i]);
            if (v < 0) continue;
            acc = (acc << 6) | (unsigned int)v;
            bits += 6;
            if (bits >= 8) {
                bits -= 8;
                out[n++] = (unsigned char)((acc >> bits) & 0xffU);
            }
        }
    }
    *out_len = n;
    return out;
}

int renderer_init(void) {
    size_t encoded_len = 0;
    unsigned char *encoded = decode_base64_parts(claw_art_rle_base64_parts,
                                                  CLAW_ART_RLE_PART_COUNT,
                                                  &encoded_len);
    if (!encoded) return 0;
    pixels = (uint16_t *)calloc(ART_PIXELS, sizeof(uint16_t));
    if (!pixels) { free(encoded); return 0; }
    size_t in = 0, out = 0;
    int ok = 1;
    while (in + 3U <= encoded_len && out < ART_PIXELS) {
        unsigned char type = encoded[in++];
        size_t count = (size_t)encoded[in] | ((size_t)encoded[in + 1] << 8);
        in += 2;
        if (count == 0 || count > ART_PIXELS - out) { ok = 0; break; }
        if (type == 0) {
            memset(pixels + out, 0, count * sizeof(uint16_t));
            out += count;
        } else if (type == 1) {
            if (in + 2U * count > encoded_len) { ok = 0; break; }
            for (size_t j = 0; j < count; ++j) {
                pixels[out++] = (uint16_t)encoded[in] | ((uint16_t)encoded[in + 1] << 8);
                in += 2;
            }
        } else { ok = 0; break; }
    }
    free(encoded);
    if (!ok || out != ART_PIXELS) { free(pixels); pixels = NULL; return 0; }
    return 1;
}

void renderer_free(void) { free(pixels); pixels = NULL; }
double renderer_image_aspect(void) { return (double)CLAW_ART_WIDTH / (double)CLAW_ART_HEIGHT; }

static int fit_size(int max_cols, int max_rows, double aspect, int *cols_out, int *rows_out) {
    if (max_cols < 1 || max_rows < 1 || aspect < 0.5 || aspect > 4.0) return 0;
    double picture_aspect = renderer_image_aspect();
    int cols = max_cols;
    int by_height = (int)((double)max_rows * aspect * picture_aspect);
    if (by_height < cols) cols = by_height;
    if (cols < 1) cols = 1;
    int rows = (int)((double)cols / (aspect * picture_aspect) + 0.5);
    if (rows > max_rows) {
        rows = max_rows;
        cols = (int)((double)rows * aspect * picture_aspect);
    }
    if (cols < 1) cols = 1;
    if (rows < 1) rows = 1;
    *cols_out = cols; *rows_out = rows;
    return 1;
}

static int compare_double(const void *a, const void *b) {
    double aa = *(const double *)a, bb = *(const double *)b;
    return (aa > bb) - (aa < bb);
}

static unsigned int color_256(RGB c) {
    if (abs(c.r-c.g) < 10 && abs(c.g-c.b) < 10) {
        int v = (c.r+c.g+c.b)/3;
        if (v < 8) return 16;
        if (v > 248) return 231;
        return 232U + (unsigned int)((v-8)*24/240);
    }
    int r = (int)(c.r * 5.0 / 255.0 + 0.5);
    int g = (int)(c.g * 5.0 / 255.0 + 0.5);
    int b = (int)(c.b * 5.0 / 255.0 + 0.5);
    return 16U + 36U*(unsigned int)r + 6U*(unsigned int)g + (unsigned int)b;
}

static unsigned int color_key(RGB c, ColorMode mode) {
    if (mode == COLOR_256) return 0x10000U + color_256(c);
    if (mode == COLOR_16) return c.b > c.r + 60 ? 0x20001U : 0x20000U;
    return ((unsigned int)c.r << 16) | ((unsigned int)c.g << 8) | (unsigned int)c.b;
}

static void emit_color(RGB c, ColorMode mode, unsigned int key) {
    if (mode == COLOR_TRUECOLOR) printf("\033[38;2;%d;%d;%dm", c.r, c.g, c.b);
    else if (mode == COLOR_256) printf("\033[38;5;%um", key - 0x10000U);
    else if (mode == COLOR_16) fputs(key == 0x20001U ? "\033[94m" : "\033[97m", stdout);
}

static double fast_sqrt(double x) {
    if (x <= 0.0) return 0.0;
    double g = x < 1.0 ? 1.0 : x;
    for (int i = 0; i < 10; ++i) g = 0.5 * (g + x / g);
    return g;
}

static int round_even_scaled(int index, int source_size, int cell_count) {
    long long numerator = (long long)index * source_size;
    long long q = numerator / cell_count;
    long long r = numerator % cell_count;
    long long twice = 2LL * r;
    if (twice > cell_count || (twice == cell_count && (q & 1LL))) ++q;
    return (int)q;
}

static void draw_cells(int cols, int rows, int left, ColorMode mode) {
    size_t count = (size_t)cols * (size_t)rows;
    double *density = (double *)malloc(count * sizeof(double));
    double *blue = (double *)malloc(count * sizeof(double));
    double *bright = (double *)malloc(count * sizeof(double));
    if (!density || !blue || !bright || count > CELL_LIMIT) {
        free(density); free(blue); free(bright);
        fputs("CLAW splash: not enough memory for this terminal size.\n", stderr);
        return;
    }
    int *xs = (int *)malloc(((size_t)cols + 1U) * sizeof(int));
    int *ys = (int *)malloc(((size_t)rows + 1U) * sizeof(int));
    if (!xs || !ys) {
        free(xs); free(ys); free(density); free(blue); free(bright);
        fputs("CLAW splash: not enough memory for terminal grid.\n", stderr);
        return;
    }
    for (int x = 0; x <= cols; ++x) xs[x] = round_even_scaled(x, CLAW_ART_WIDTH, cols);
    for (int y = 0; y <= rows; ++y) ys[y] = round_even_scaled(y, CLAW_ART_HEIGHT, rows);

    size_t ci = 0, solid_count = 0;
    for (int y = 0; y < rows; ++y) {
        int y0 = ys[y], y1 = ys[y+1]; if (y1 <= y0) y1 = y0 + 1;
        for (int x = 0; x < cols; ++x, ++ci) {
            int x0 = xs[x], x1 = xs[x+1]; if (x1 <= x0) x1 = x0 + 1;
            unsigned long long sum_lum = 0, sum_blue = 0;
            for (int py = y0; py < y1 && py < CLAW_ART_HEIGHT; ++py) {
                size_t pos = (size_t)py * CLAW_ART_WIDTH + (size_t)x0;
                for (int px = x0; px < x1 && px < CLAW_ART_WIDTH; ++px, ++pos) {
                    unsigned int v = pixels[pos];
                    unsigned int l = v >> 4, s = v & 15U;
                    sum_lum += l;
                    sum_blue += (unsigned long long)l * s;
                }
            }
            unsigned long long area = (unsigned long long)(x1-x0) * (unsigned long long)(y1-y0);
            density[ci] = (double)sum_lum / ((double)area * CLAW_ART_LUMA_MAX);
            blue[ci] = sum_lum ? (double)sum_blue / ((double)sum_lum * CLAW_ART_SAT_MAX) : 0.0;
            if (density[ci] > 0.02) bright[solid_count++] = density[ci];
        }
    }
    qsort(bright, solid_count, sizeof(double), compare_double);
    double ref = solid_count ? bright[(size_t)((double)solid_count * 0.80) < solid_count ? (size_t)((double)solid_count * 0.80) : solid_count-1U] : 1.0;
    if (ref < 0.05) ref = 0.05;
    const int top = (int)(sizeof(RAMP)-2U);
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < left; ++x) putchar(' ');
        int last_color_valid = 0;
        unsigned int last_color = 0;
        for (int x = 0; x < cols; ++x) {
            size_t i = (size_t)y * cols + x;
            char ch = ' ';
            double t = 0.0;
            double v = density[i] / ref;
            if (v > 1.0) v = 1.0;
            /* v^0.75 = sqrt(v * sqrt(v)); avoids an extra math-library dependency. */
            if (v < 0.0) v = 0.0;
            double root = fast_sqrt(v);
            v = fast_sqrt(v * root);
            if (v >= 0.07) {
                int idx = (int)(v * top + 0.5);
                if (idx < 1) idx = 1;
                if (idx > top) idx = top;
                ch = RAMP[idx];
                t = blue[i] * 1.25;
                if (t > 1.0) t = 1.0;
                if (t > 0.5) {
                    if (idx <= 1) ch = '.';
                    else {
                        double frac = ((double)x + 0.5) / cols;
                        int li = (frac > LETTER_EDGES[0]) + (frac > LETTER_EDGES[1]) + (frac > LETTER_EDGES[2]);
                        ch = LETTERS[li];
                    }
                }
            }
            if (mode != COLOR_NONE && ch != ' ') {
                RGB c = {
                    (int)(226.0 + (40.0-226.0)*t),
                    (int)(226.0 + (160.0-226.0)*t),
                    (int)(228.0 + (255.0-228.0)*t)
                };
                unsigned int key = mode == COLOR_16 ? (t > 0.5 ? 0x20001U : 0x20000U) : color_key(c, mode);
                if (!last_color_valid || key != last_color) {
                    emit_color(c, mode, key);
                    last_color = key; last_color_valid = 1;
                }
            }
            putchar(ch);
        }
        if (mode != COLOR_NONE && last_color_valid) fputs("\033[0m", stdout);
        putchar('\n');
    }
    free(xs); free(ys); free(density); free(blue); free(bright);
}

void renderer_draw(int max_cols, int max_rows, double cell_aspect,
                   ColorMode color_mode, int center_vertically) {
    int cols = 1, rows = 1;
    if (!pixels || !fit_size(max_cols, max_rows, cell_aspect, &cols, &rows)) return;
    if ((size_t)cols * (size_t)rows > CELL_LIMIT) {
        fputs("CLAW splash: terminal size is too large to render safely.\n", stderr);
        return;
    }
    int top = center_vertically ? (max_rows - rows) / 2 : 0;
    for (int y = 0; y < top; ++y) putchar('\n');
    int left = (max_cols - cols) / 2;
    draw_cells(cols, rows, left, color_mode);
    fflush(stdout);
}
