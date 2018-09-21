/* Test HMAC_SHA256 and PBKDF2_HMAC_SHA256 implementations.
 *
 * Written by Zack Weinberg <zackw at panix.com> in 2018.
 * Incorporates standard test vectors from sources documented below.
 * To the extent possible under law, the named authors have waived all
 * copyright and related or neighboring rights to this work.
 *
 * See https://creativecommons.org/publicdomain/zero/1.0/ for further
 * details.
 */

#include "crypt-port.h"
#include "alg-sha256.h"

#include <stdio.h>

#if INCLUDE_scrypt || INCLUDE_yescrypt

struct hmac_sha256_test
{
  const char *key;
  const char *message;
  uint8_t digest[32];
};

/* HMAC-SHA256 test vectors from RFC 4231.  */
static const struct hmac_sha256_test hmac_sha256_tests[] = {
  { "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"
    "\x0b\x0b\x0b\x0b",
    "Hi There",
    "\xb0\x34\x4c\x61\xd8\xdb\x38\x53\x5c\xa8\xaf\xce\xaf\x0b\xf1\x2b"
    "\x88\x1d\xc2\x00\xc9\x83\x3d\xa7\x26\xe9\x37\x6c\x2e\x32\xcf\xf7"
  },
  { "Jefe",
    "what do ya want for nothing?",
    "\x5b\xdc\xc1\x46\xbf\x60\x75\x4e\x6a\x04\x24\x26\x08\x95\x75\xc7"
    "\x5a\x00\x3f\x08\x9d\x27\x39\x83\x9d\xec\x58\xb9\x64\xec\x38\x43"
  },
  { "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa",
    "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
    "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
    "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
    "\xdd\xdd",
    "\x77\x3e\xa9\x1e\x36\x80\x0e\x46\x85\x4d\xb8\xeb\xd0\x91\x81\xa7"
    "\x29\x59\x09\x8b\x3e\xf8\xc1\x22\xd9\x63\x55\x14\xce\xd5\x65\xfe"
  },
  { "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10"
    "\x11\x12\x13\x14\x15\x16\x17\x18\x19",
    "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
    "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
    "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
    "\xcd\xcd",
    "\x82\x55\x8a\x38\x9a\x44\x3c\x0e\xa4\xcc\x81\x98\x99\xf2\x08\x3a"
    "\x85\xf0\xfa\xa3\xe5\x78\xf8\x07\x7a\x2e\x3f\xf4\x67\x29\x66\x5b"
  },
  { "\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c"
    "\x0c\x0c\x0c\x0c",
    "Test With Truncation",
    /* N.B. the RFC only supplies the high 16 bytes of this vector; the rest
       were filled in with the current implementation's output.  */
    "\xa3\xb6\x16\x74\x73\x10\x0e\xe0\x6e\x0c\x79\x6c\x29\x55\x55\x2b"
    "\xfa\x6f\x7c\x0a\x6a\x8a\xef\x8b\x93\xf8\x60\xaa\xb0\xcd\x20\xc5"
 },
  { "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa",
    "Test Using Larger Than Block-Size Key - Hash Key First",
    "\x60\xe4\x31\x59\x1e\xe0\xb6\x7f\x0d\x8a\x26\xaa\xcb\xf5\xb7\x7f"
    "\x8e\x0b\xc6\x21\x37\x28\xc5\x14\x05\x46\x04\x0f\x0e\xe3\x7f\x54"
  },
  { "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    "\xaa\xaa\xaa",
    "This is a test using a larger than block-size key and a larger t"
    "han block-size data. The key needs to be hashed before being use"
    "d by the HMAC algorithm.",
    "\x9b\x09\xff\xa7\x1b\x94\x2f\xcb\x27\x63\x5f\xbc\xd5\xb0\xe9\x44"
    "\xbf\xdc\x63\x64\x4f\x07\x13\x93\x8a\x7f\x51\x53\x5c\x3a\x35\xe2"
  }
};

struct pbkdf2_hmac_sha256_test
{
  const char *passwd;
  const char *salt;
  uint32_t plen;
  uint32_t slen;
  uint32_t c;
  uint32_t dklen;
  const char *dk; /* [dklen] */
};

struct pbkdf2_hmac_sha256_test pbkdf2_hmac_sha256_tests[] = {
  /* PBKDF2-HMAC-SHA256 test vectors from RFC 7914.  */
  { "passwd", "salt", 6, 4, 1, 64,
    "\x55\xac\x04\x6e\x56\xe3\x08\x9f\xec\x16\x91\xc2\x25\x44\xb6\x05"
    "\xf9\x41\x85\x21\x6d\xde\x04\x65\xe6\x8b\x9d\x57\xc2\x0d\xac\xbc"
    "\x49\xca\x9c\xcc\xf1\x79\xb6\x45\x99\x16\x64\xb3\x9d\x77\xef\x31"
    "\x7c\x71\xb8\x45\xb1\xe3\x0b\xd5\x09\x11\x20\x41\xd3\xa1\x97\x83"
  },
  { "Password", "NaCl", 8, 4, 80000, 64,
    "\x4d\xdc\xd8\xf6\x0b\x98\xbe\x21\x83\x0c\xee\x5e\xf2\x27\x01\xf9"
    "\x64\x1a\x44\x18\xd0\x4c\x04\x14\xae\xff\x08\x87\x6b\x34\xab\x56"
    "\xa1\xd4\x25\xa1\x22\x58\x33\x54\x9a\xdb\x84\x1b\x51\xc9\xb3\x17"
    "\x6a\x27\x2b\xde\xbb\xa1\xd0\x78\x47\x8f\x62\xb3\x97\xf3\x3c\x8d"
  },
  /* Test vectors from RFC 6070 (which defines PBKDF2-HMAC-SHA1)
     recalculated with SHA256 instead of SHA1 and a larger dklen by
     'aaz' at <https://stackoverflow.com/a/5136918/388520>.  */
  { "password", "salt", 8, 4, 1, 32,
    "\x12\x0f\xb6\xcf\xfc\xf8\xb3\x2c\x43\xe7\x22\x52\x56\xc4\xf8\x37"
    "\xa8\x65\x48\xc9\x2c\xcc\x35\x48\x08\x05\x98\x7c\xb7\x0b\xe1\x7b"
  },
  { "password", "salt", 8, 4, 2, 32,
    "\xae\x4d\x0c\x95\xaf\x6b\x46\xd3\x2d\x0a\xdf\xf9\x28\xf0\x6d\xd0"
    "\x2a\x30\x3f\x8e\xf3\xc2\x51\xdf\xd6\xe2\xd8\x5a\x95\x47\x4c\x43"
  },
  { "password", "salt", 8, 4, 4096, 32,
    "\xc5\xe4\x78\xd5\x92\x88\xc8\x41\xaa\x53\x0d\xb6\x84\x5c\x4c\x8d"
    "\x96\x28\x93\xa0\x01\xce\x4e\x11\xa4\x96\x38\x73\xaa\x98\x13\x4a"
  },
#ifdef SLOW_TESTS
  /* With this test vector included, the program takes 40 seconds to run
     to completion on a 2017-generation x86.  Without, half a second.  */
  { "password", "salt", 8, 4, 16777216, 32,
    "\xcf\x81\xc6\x6f\xe8\xcf\xc0\x4d\x1f\x31\xec\xb6\x5d\xab\x40\x89"
    "\xf7\xf1\x79\xe8\x9b\x3b\x0b\xcb\x17\xad\x10\xe3\xac\x6e\xba\x46"
  },
#endif
  { "passwordPASSWORDpassword", "saltSALTsaltSALTsaltSALTsaltSALTsalt",
    24, 36, 4096, 40,
    "\x34\x8c\x89\xdb\xcb\xd3\x2b\x2f\x32\xd8\x14\xb8\x11\x6e\x84\xcf"
    "\x2b\x17\x34\x7e\xbc\x18\x00\x18\x1c\x4e\x2a\x1f\xb8\xdd\x53\xe1"
    "\xc6\x35\x51\x8c\x7d\xac\x47\xe9"
  },
  { "pass\0word", "sa\0lt", 9, 5, 4096, 16,
    "\x89\xb6\x9d\x05\x16\xf8\x29\x89\x3c\x69\x62\x26\x65\x0a\x86\x87"
  }
};


static void
report_failure(const char *tag, size_t n, size_t len,
               const uint8_t expected[], const uint8_t actual[])
{
  size_t i;
  printf ("FAIL: %s/%zu:\n  exp:", tag, n);
  for (i = 0; i < len; i++)
    {
      if (i % 4 == 0)
        putchar (' ');
      printf ("%02x", (unsigned int)(unsigned char)expected[i]);
    }
  printf ("\n  got:");
  for (i = 0; i < len; i++)
    {
      if (i % 4 == 0)
        putchar (' ');
      printf ("%02x", (unsigned int)(unsigned char)actual[i]);
    }
  putchar ('\n');
  putchar ('\n');
}

static int
test_hmac_sha256 (void)
{
  uint8_t output[32];
  int status = 0;
  for (size_t i = 0; i < ARRAY_SIZE (hmac_sha256_tests); i++)
    {
      const struct hmac_sha256_test *t = &hmac_sha256_tests[i];
      HMAC_SHA256_Buf (t->key, strlen (t->key),
                       t->message, strlen (t->message),
                       output);
      if (memcmp (output, t->digest, 32))
        {
          report_failure ("HMAC-SHA256", i, 32, t->digest, output);
          status = 1;
        }
    }
  return status;
}

static int
test_pbkdf2_hmac_sha256 (void)
{
  uint8_t output[64];
  int status = 0;
  for (size_t i = 0; i < ARRAY_SIZE (pbkdf2_hmac_sha256_tests); i++)
    {
      const struct pbkdf2_hmac_sha256_test *t = &pbkdf2_hmac_sha256_tests[i];
      assert (t->dklen <= sizeof output);

      PBKDF2_SHA256 ((const unsigned char *)t->passwd, t->plen,
                     (const unsigned char *)t->salt, t->slen,
                     t->c, output, t->dklen);
      if (memcmp (output, t->dk, t->dklen))
        {
          report_failure ("PBKDF2-HMAC-SHA256", i, t->dklen,
                          (const unsigned char *)t->dk, output);
          status = 1;
        }
    }
  return status;
}


int
main (void)
{
  int status = 0;
  status |= test_hmac_sha256 ();
  status |= test_pbkdf2_hmac_sha256 ();
  return status;
}

#else /* INCLUDE_scrypt || INCLUDE_yescrypt */

int
main (void)
{
  return 77; /* UNSUPPORTED */
}

#endif
