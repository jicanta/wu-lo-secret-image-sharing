# visualSIS — Visual Secret Image Sharing

Implementation of the secret image sharing scheme described in
**"An Efficient Secret Image Sharing Scheme"** by Luang-Shyr Wu and Tsung-Ming Lo
(National Taiwan University of Technology).

The program hides a grayscale BMP image inside a set of *n* carrier images (shadows),
such that any *k* of them are enough to reconstruct the original. Fewer than *k* shadows
reveal nothing about the secret.

## Build

```
make
```

Requires GCC and standard C99. Produces the `visualSIS` binary.

```
make test    # run unit tests
make clean
```

## Usage

```
./visualSIS -d -secret <image.bmp> -k <num> [-n <num>] [-dir <dir>]
./visualSIS -r -secret <image.bmp> -k <num> [-n <num>] [-dir <dir>]
```

| Flag | Meaning |
|---|---|
| `-d` | Distribute: hide the secret inside carrier images |
| `-r` | Recover: reconstruct the secret from shadow images |
| `-secret` | BMP file to hide (`-d`) or to create (`-r`) |
| `-k` | Minimum shares required to reconstruct (2 ≤ k ≤ 10) |
| `-n` | Total shares to generate; defaults to the number of BMPs in `-dir` |
| `-dir` | Directory of carrier images; defaults to current directory |

**Examples:**

```
# Hide clave.bmp into 4 shadows, requiring 2 to recover
./visualSIS -d -secret clave.bmp -k 2 -n 4 -dir varias

# Recover from the shadows in the current directory
./visualSIS -r -secret recovered.bmp -k 2 -n 4 -dir varias
```

## How it works

### Distribution

1. A random permutation table R is generated from a seed. The secret image O is XOR'd
   with R to produce a randomised image Q, hiding pixel correlation.
2. Q is split into consecutive sections of k pixels each: `[a0, a1, ..., a(k-1)]`.
3. For each section, a polynomial of degree k-1 is constructed over GF(257):
   `f(x) = a0 + a1·x + … + a(k-1)·x^(k-1) mod 257`
4. The polynomial is evaluated at x = 1, 2, …, n to produce one shadow pixel per
   carrier image. If any evaluation equals 256 (which cannot be stored in a byte),
   the first non-zero coefficient is decremented by 1 and the step is retried.
5. Each shadow pixel is hidden inside a carrier BMP using steganography.

GF(257) is used instead of the original GF(251) so that pixel values 0–255 map
directly to field elements without truncation.

### Recovery

1. For each section position, k shadow pixels are collected (one per shadow image),
   together with their x-values (shadow indices).
2. Lagrange interpolation over GF(257) recovers the k polynomial coefficients,
   which are exactly the k pixels of that section in Q.
3. After all sections are processed, Q is XOR'd with R to restore the original image O.

### Steganography

Shadow pixels are embedded into carrier images using LSB replacement: each shadow
byte is split across 8 carrier pixels by replacing the least significant bit of each
one. Touching a single bit changes every carrier pixel by at most ±1, so the hidden
data stays imperceptible for any value of k.

The amount each carrier must hold follows directly from k. If the secret has `m`
pixels, it is split into `⌈m / k⌉` sections, so each shadow stores `⌈m / k⌉` bytes.
At 1 bit per pixel that needs:

```
carrier pixels  ≥  8 · ⌈m / k⌉
```

For k = 8 this is exactly `m`, so the carriers match the secret (the case described
in the assignment). For k < 8 the carriers must be larger than the secret, and for
k > 8 they may be smaller. The carriers must all share the same dimensions; any extra
capacity beyond `8 · ⌈m / k⌉` pixels is left untouched. When `m` is not a multiple of
k the last section is zero-padded, and recovery trims it back using the stored secret
size.

### Metadata stored in shadow BMPs

Two values are embedded in the reserved bytes of each shadow BMP's file header,
exactly as the assignment requires:

- **Bytes 6–7:** PRNG seed used to generate the permutation table R (little-endian uint16)
- **Bytes 8–9:** Shadow index 1..n (little-endian uint16)

The secret's dimensions are needed only for k ≠ 8, where the shadows are not the
same size as the secret. They are stored as two little-endian uint16 (width then
height) in the DIB "important colors" field (bytes 50–53), which is advisory and
ignored by viewers, so it never affects how the shadow displays. For k = 8 nothing
is stored: the secret has the same size as the carriers (enunciado 4.3.2), so
recovery takes the size from the carrier, which also makes externally produced
(8,n) shadows such as the ones handed out by the cátedra recoverable.

## Image requirements

- Format: BMP, 8 bits per pixel (grayscale, uncompressed)
- No extra metadata after the pixel data
- All carrier images must share the same dimensions
- Each carrier must have at least `8 · ⌈m / k⌉` pixels (`m` = pixels in the secret);
  for k = 8 this means the carriers match the secret size

## Source layout

```
src/
  commons.h   shared constants (MOD=257, K_MIN, K_MAX, N_MIN)
  args.h/c    command-line argument parsing
  bmp.h/c     BMP read/write (8 bpp, handles pixel offset and row padding)
  poly.h/c    GF(257) polynomial evaluation and Lagrange interpolation
  main.c      entry point
tests/
  test_poly.c unit tests for polynomial and Lagrange functions
  test_bmp.c  unit tests for BMP read/write including reserved-field round-trips
```
