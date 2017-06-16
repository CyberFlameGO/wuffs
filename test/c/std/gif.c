// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/*
To manually run this test:

for cc in clang gcc; do
  $cc -std=c99 -Wall -Werror gif.c && ./a.out
  rm -f a.out
done

Each edition should print "PASS", amongst other information, and exit(0).
*/

#include "../../../gen/c/std/gif.c"
#include "../testlib/testlib.c"

const char* test_filename = "std/gif.c";

#define BUFFER_SIZE (1024 * 1024)

uint8_t global_got_buffer[BUFFER_SIZE];
uint8_t global_want_buffer[BUFFER_SIZE];
uint8_t global_src_buffer[BUFFER_SIZE];

// ---------------- Basic Tests

void test_bad_argument_null() {
  test_funcname = __func__;
  puffs_gif_lzw_decoder dec;
  puffs_gif_lzw_decoder_constructor(&dec, PUFFS_VERSION, 0);
  puffs_gif_status status = puffs_gif_lzw_decoder_decode(&dec, NULL, NULL);
  if (status != puffs_gif_error_bad_argument) {
    FAIL("status: got %d, want %d", status, puffs_gif_error_bad_argument);
  }
}

void test_bad_argument_out_of_range() {
  test_funcname = __func__;
  puffs_gif_lzw_decoder dec;
  puffs_gif_lzw_decoder_constructor(&dec, PUFFS_VERSION, 0);

  // Setting to 8 is in the [2..8] range.
  puffs_gif_lzw_decoder_set_literal_width(&dec, 8);
  if (dec.private_impl.status != puffs_gif_status_ok) {
    FAIL("status: got %d, want %d", dec.private_impl.status,
         puffs_gif_status_ok);
  }

  // Setting to 999 is out of range.
  puffs_gif_lzw_decoder_set_literal_width(&dec, 999);
  if (dec.private_impl.status != puffs_gif_error_bad_argument) {
    FAIL("status: got %d, want %d", dec.private_impl.status,
         puffs_gif_error_bad_argument);
  }

  // That error status code should be sticky.
  puffs_gif_lzw_decoder_set_literal_width(&dec, 8);
  if (dec.private_impl.status != puffs_gif_error_bad_argument) {
    FAIL("status: got %d, want %d", dec.private_impl.status,
         puffs_gif_error_bad_argument);
  }
}

void test_bad_receiver() {
  test_funcname = __func__;
  puffs_base_buf1 dst = {0};
  puffs_base_buf1 src = {0};
  puffs_gif_status status = puffs_gif_lzw_decoder_decode(NULL, &dst, &src);
  if (status != puffs_gif_error_bad_receiver) {
    FAIL("status: got %d, want %d", status, puffs_gif_error_bad_receiver);
  }
}

void test_constructor_not_called() {
  test_funcname = __func__;
  puffs_gif_lzw_decoder dec = {{0}};
  puffs_base_buf1 dst = {0};
  puffs_base_buf1 src = {0};
  puffs_gif_status status = puffs_gif_lzw_decoder_decode(&dec, &dst, &src);
  if (status != puffs_gif_error_constructor_not_called) {
    FAIL("status: got %d, want %d", status,
         puffs_gif_error_constructor_not_called);
  }
}

void test_puffs_version_bad() {
  test_funcname = __func__;
  puffs_gif_lzw_decoder dec;
  puffs_gif_lzw_decoder_constructor(&dec, 0, 0);  // 0 is not PUFFS_VERSION.
  if (dec.private_impl.status != puffs_gif_error_bad_version) {
    FAIL("status: got %d, want %d", dec.private_impl.status,
         puffs_gif_error_bad_version);
    goto cleanup0;
  }
cleanup0:
  puffs_gif_lzw_decoder_destructor(&dec);
}

void test_puffs_version_good() {
  test_funcname = __func__;
  puffs_gif_lzw_decoder dec;
  puffs_gif_lzw_decoder_constructor(&dec, PUFFS_VERSION, 0);
  if (dec.private_impl.magic != PUFFS_MAGIC) {
    FAIL("magic: got %u, want %u", dec.private_impl.magic, PUFFS_MAGIC);
    goto cleanup0;
  }
  if (dec.private_impl.f_literal_width != 8) {
    FAIL("f_literal_width: got %u, want %u", dec.private_impl.f_literal_width,
         8);
    goto cleanup0;
  }
cleanup0:
  puffs_gif_lzw_decoder_destructor(&dec);
}

void test_status_is_error() {
  test_funcname = __func__;
  if (puffs_gif_status_is_error(puffs_gif_status_ok)) {
    FAIL("is_error(ok) returned true");
    return;
  }
  if (!puffs_gif_status_is_error(puffs_gif_error_bad_version)) {
    FAIL("is_error(bad_version) returned false");
    return;
  }
  if (puffs_gif_status_is_error(puffs_gif_status_short_write)) {
    FAIL("is_error(short_write) returned true");
    return;
  }
  if (!puffs_gif_status_is_error(puffs_gif_error_lzw_code_is_out_of_range)) {
    FAIL("is_error(lzw_code_is_out_of_range) returned false");
    return;
  }
}

void test_status_strings() {
  test_funcname = __func__;
  const char* s0 = puffs_gif_status_string(puffs_gif_status_ok);
  const char* t0 = "gif: ok";
  if (strcmp(s0, t0)) {
    FAIL("got \"%s\", want \"%s\"", s0, t0);
    return;
  }
  const char* s1 = puffs_gif_status_string(puffs_gif_error_bad_version);
  const char* t1 = "gif: bad version";
  if (strcmp(s1, t1)) {
    FAIL("got \"%s\", want \"%s\"", s1, t1);
    return;
  }
  const char* s2 = puffs_gif_status_string(puffs_gif_status_short_write);
  const char* t2 = "gif: short write";
  if (strcmp(s2, t2)) {
    FAIL("got \"%s\", want \"%s\"", s2, t2);
    return;
  }
  const char* s3 =
      puffs_gif_status_string(puffs_gif_error_lzw_code_is_out_of_range);
  const char* t3 = "gif: LZW code is out of range";
  if (strcmp(s3, t3)) {
    FAIL("got \"%s\", want \"%s\"", s3, t3);
    return;
  }
  const char* s4 = puffs_gif_status_string(-254);
  const char* t4 = "gif: unknown status";
  if (strcmp(s4, t4)) {
    FAIL("got \"%s\", want \"%s\"", s4, t4);
    return;
  }
}

void test_sub_struct_constructor() {
  test_funcname = __func__;
  puffs_gif_decoder dec;
  puffs_gif_decoder_constructor(&dec, PUFFS_VERSION, 0);
  if (dec.private_impl.magic != PUFFS_MAGIC) {
    FAIL("outer magic: got %u, want %u", dec.private_impl.magic, PUFFS_MAGIC);
    goto cleanup0;
  }
  if (dec.private_impl.f_lzw.private_impl.magic != PUFFS_MAGIC) {
    FAIL("inner magic: got %u, want %u",
         dec.private_impl.f_lzw.private_impl.magic, PUFFS_MAGIC);
    goto cleanup0;
  }
cleanup0:
  puffs_gif_decoder_destructor(&dec);
}

// ---------------- LZW Tests

void test_lzw_decode() {
  test_funcname = __func__;
  puffs_base_buf1 got = {.ptr = global_got_buffer, .len = BUFFER_SIZE};
  puffs_base_buf1 want = {.ptr = global_want_buffer, .len = BUFFER_SIZE};
  puffs_base_buf1 src = {.ptr = global_src_buffer, .len = BUFFER_SIZE};

  // The want .indexes file should be 19200 bytes long, as the image size is
  // 160 * 120 pixels and there is 1 palette index byte per pixel.
  if (!read_file(&want, "../../testdata/bricks-nodither.indexes")) {
    goto cleanup0;
  }
  if (want.wi != 19200) {
    FAIL("want size: got %d, want %d", (int)(want.wi), 19200);
    goto cleanup0;
  }

  // The src .giflzw file should be 13382 bytes long.
  if (!read_file(&src, "../../testdata/bricks-nodither.giflzw")) {
    goto cleanup0;
  }
  if (src.wi != 13382) {
    FAIL("src size: got %d, want %d", (int)(src.wi), 13382);
    goto cleanup0;
  }
  // The first byte in that file, the LZW literal width, should be 0x08.
  uint8_t literal_width = src.ptr[0];
  if (literal_width != 0x08) {
    FAIL("LZW literal width: got %d, want %d", (int)(src.ptr[0]), 0x08);
    goto cleanup0;
  }
  src.ri++;

  puffs_gif_lzw_decoder dec;
  puffs_gif_lzw_decoder_constructor(&dec, PUFFS_VERSION, 0);
  puffs_gif_lzw_decoder_set_literal_width(&dec, literal_width);
  puffs_gif_status status = puffs_gif_lzw_decoder_decode(&dec, &got, &src);
  if (status != puffs_gif_status_ok) {
    FAIL("status: got %d, want %d", status, puffs_gif_status_ok);
    goto cleanup1;
  }

  if (!buf1s_equal(&got, &want)) {
    goto cleanup1;
  }
  // As a sanity check, the first decoded byte should be 0xDC.
  if (got.ptr[0] != 0xDC) {
    FAIL("first decoded byte: got 0x%02x, want 0x%02x", (int)(got.ptr[0]),
         0xDC);
    goto cleanup1;
  }

cleanup1:
  puffs_gif_lzw_decoder_destructor(&dec);
cleanup0:;
}

// ---------------- GIF Tests

void test_gif_decode_input_is_a_xxx(const char* filename,
                                    puffs_gif_status want) {
  puffs_base_buf1 dst = {.ptr = global_got_buffer, .len = BUFFER_SIZE};
  puffs_base_buf1 src = {.ptr = global_src_buffer, .len = BUFFER_SIZE};

  if (!read_file(&src, filename)) {
    goto cleanup0;
  }

  puffs_gif_decoder dec;
  puffs_gif_decoder_constructor(&dec, PUFFS_VERSION, 0);
  puffs_gif_status got = puffs_gif_decoder_decode(&dec, &dst, &src);
  if (got != want) {
    FAIL("status: got %d, want %d", got, want);
    goto cleanup1;
  }

cleanup1:
  puffs_gif_decoder_destructor(&dec);
cleanup0:;
}

void test_gif_decode_input_is_a_gif() {
  test_funcname = __func__;
  test_gif_decode_input_is_a_xxx("../../testdata/bricks-dither.gif",
                                 puffs_gif_status_ok);
}

void test_gif_decode_input_is_a_png() {
  test_funcname = __func__;
  test_gif_decode_input_is_a_xxx("../../testdata/bricks-dither.png",
                                 puffs_gif_error_bad_gif_header);
}

// ---------------- Manifest

// The empty comments forces clang-format to place one element per line.
test tests[] = {
    // Basic Tests
    test_bad_argument_null,          //
    test_bad_argument_out_of_range,  //
    test_bad_receiver,               //
    test_constructor_not_called,     //
    test_puffs_version_bad,          //
    test_puffs_version_good,         //
    test_status_is_error,            //
    test_status_strings,             //
    test_sub_struct_constructor,     //
    // LZW Tests
    test_lzw_decode,  //
    // GIF Tests
    test_gif_decode_input_is_a_gif,  //
    test_gif_decode_input_is_a_png,  //
    // End
    NULL,
};
