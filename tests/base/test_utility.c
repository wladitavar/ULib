/* test_utility.c */

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/utility.h>
#include <ulib/base/coder/escape.h>

#include <stdlib.h>

#ifdef HAVE_FNMATCH
#  include <fnmatch.h>
#endif

#ifndef FNM_CASEFOLD
#define FNM_CASEFOLD FNM_IGNORECASE
#endif

#ifndef FNM_LEADING_DIR
#define FNM_LEADING_DIR FNM_PERIOD
#endif

static int compare_str(const void* str1, const void* str2) { return u_strnatcmp(*(const char**)str1, *(const char**)str2); }

static void ftw_push(void)
{
   U_INTERNAL_TRACE("ftw_push()", 0)

   printf("ftw_push() = \"%.*s\" %u %d\n", u_buffer_len, u_buffer, u_buffer_len, u_ftw_ctx.is_directory);
}

static void ftw_up(void)
{
   U_INTERNAL_TRACE("ftw_up()", 0)

   printf("ftw_up()\n");
/* printf("ftw_up()   = \"%.*s\" %u %d\n", u_buffer_len, u_buffer, u_buffer_len, u_ftw_ctx.is_directory); */
}

static void check_match(void)
{
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("01000000002345"),
                                  U_CONSTANT_TO_PARAM("01*2345"), u_pfn_flags) == true )
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("01000000002345"),
                                  U_CONSTANT_TO_PARAM("01*2?46"), u_pfn_flags) == false )
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("01000000002345"),
                                  U_CONSTANT_TO_PARAM("010000000 2?45"), u_pfn_flags) == false )
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("01000000002345"),
                                  U_CONSTANT_TO_PARAM("01*2?4?"), u_pfn_flags) == true )
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("01000000002345"),
                                  U_CONSTANT_TO_PARAM("01*00*00*2?46"), u_pfn_flags) == false )
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("01000000002345"),
                                  U_CONSTANT_TO_PARAM("01*2?46?????????????????????"), u_pfn_flags) == false )
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("01000000002345"),
                                  U_CONSTANT_TO_PARAM("*****01*2?46?????????????????????"), u_pfn_flags) == false )
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("aaAA4b334"),
                                  U_CONSTANT_TO_PARAM("aaAA?b*4"), u_pfn_flags) == true )
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("aaAA4b334"),
                                  U_CONSTANT_TO_PARAM("aaaa?b*4"), u_pfn_flags | FNM_CASEFOLD) == true )
}

#define U_TESTO_SEMPLICE "testosemplicetxt" /* no _.:?! */

int main(int argc, char* argv[])
{
   static char buf[4096], buffer[4096];

   int i, n;
   char buf8[9];
   char* sargv[128];
   uint32_t path_len;
   const char* path_rel;
   char path[U_PATH_MAX + 1];

   const char* vec[] = {
      "libpng-1.0.10.tar.gz",
      "libpng-1.0.11.tar.gz",
      "libpng-1.0.12.tar.gz",
      "libpng-1.0.3.tar.gz",
      "libpng-1.0.5.tar.gz",
      "libpng-1.0.6-patch-a.txt.gz",
      "libpng-1.0.6-patch-b.txt.gz",
      "libpng-1.0.6-patch-c.txt.gz",
      "libpng-1.0.6.tar.gz",
      "libpng-1.0.7.tar.gz",
      "libpng-1.0.8.tar.gz",
      "libpng-1.0.9.tar.gz",
      "libpng-1.2.0.tar.gz" };

   u_init_ulib(argv);

   U_INTERNAL_TRACE("main(%d,%p)", argc, argv)

   U_INTERNAL_ASSERT( u_isText((const unsigned char*)U_CONSTANT_TO_PARAM("binary:  "))  == false )
   U_INTERNAL_ASSERT( u_isBinary((const unsigned char*)U_CONSTANT_TO_PARAM("binary: �g df d")) == true )
   U_INTERNAL_ASSERT( u_isWhiteSpace(U_CONSTANT_TO_PARAM("           \n\t\r")) )
   U_INTERNAL_ASSERT( u_isBase64(U_CONSTANT_TO_PARAM("gXWUj7VekBdkycg3Z9kXuglV9plUl2cs4XkNLSDhe5VHRgE03e63VypMChCWDGI=")) )

   /*
   for (i = 0; i < U_CONSTANT_SIZE(U_TESTO_SEMPLICE); ++i) U_INTERNAL_PRINT("u__istext(%C) = %d", U_TESTO_SEMPLICE[i], u__istext(U_TESTO_SEMPLICE[i]))
   */

   U_INTERNAL_ASSERT( u_isText(U_CONSTANT_TO_PARAM(U_TESTO_SEMPLICE)) )

   strcpy(buf, "HOME=/home/stefano\n\"RSIGN_CMD=./rsa_priv_enc.sh -c rsa_priv_enc.cfg -k INKEY < DIGEST\"\nLOG_FILE=/tmp/log");

   n = strlen(buf);

   U_INTERNAL_ASSERT( u_split(buf, n, sargv, 0) == 3 )
   U_INTERNAL_ASSERT( sargv[3] == 0 )
   U_INTERNAL_ASSERT( U_STREQ(sargv[0], "HOME=/home/stefano") )
   U_INTERNAL_ASSERT( U_STREQ(sargv[1], "RSIGN_CMD=./rsa_priv_enc.sh -c rsa_priv_enc.cfg -k INKEY < DIGEST") )
   U_INTERNAL_ASSERT( U_STREQ(sargv[2], "LOG_FILE=/tmp/log") )

   strcpy(buf, "first second third \
               \"four five six\" \
               'uno due tre'");

   U_INTERNAL_ASSERT( u_split(buf, strlen(buf), sargv, 0) == 5 )
   U_INTERNAL_ASSERT( sargv[5] == 0 )
   U_INTERNAL_ASSERT( U_STREQ(sargv[0], "first") )
   U_INTERNAL_ASSERT( U_STREQ(sargv[1], "second") )
   U_INTERNAL_ASSERT( U_STREQ(sargv[2], "third") )
   U_INTERNAL_ASSERT( U_STREQ(sargv[3], "four five six") )
   U_INTERNAL_ASSERT( U_STREQ(sargv[4], "uno due tre") )

   if (u_pathfind(path, 0, 0, "gzip", R_OK | X_OK))
      {
      U_INTERNAL_PRINT("path = %s", path)

      U_INTERNAL_ASSERT(strstr(path, "gzip") != 0)
      }

   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("a"))          == false )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("A"))          == false )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("1"))          == true )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("234"))        == true )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("A34"))        == false )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("1A3"))        == false )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("1A3d3###"))   == false )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("0123456789")) == true )

   U_INTERNAL_ASSERT( u_isIPv4Addr("127.0.0.1/8", 9)                 == true )
   U_INTERNAL_ASSERT( u_isIPv4Addr(U_CONSTANT_TO_PARAM("127.0.0.1")) == true )

   U_INTERNAL_ASSERT( u_isIPv6Addr(U_CONSTANT_TO_PARAM("2001:db8:85a3::8a2e:370:7334"))            == true )
   U_INTERNAL_ASSERT( u_isIPv6Addr(U_CONSTANT_TO_PARAM("2001:0db8:85a3:0000:0000:8a2e:0370:7334")) == true )

   U_INTERNAL_ASSERT( u_isMacAddr(U_CONSTANT_TO_PARAM("00:16:ec:f0:e4:3b")) == true )
   U_INTERNAL_ASSERT( u_isMacAddr(U_CONSTANT_TO_PARAM("00:16:ec:fk:e4:3b")) == false )

   buf8[8] = '\0';

   u_int2hex(buf8, 123456789);
   U_INTERNAL_ASSERT( U_STREQ(buf8, "075BCD15") )
   U_INTERNAL_ASSERT( u_hex2int(buf8, 8) == 123456789 )

   u_int2hex(buf8,  23456789);
   U_INTERNAL_ASSERT( U_STREQ(buf8, "0165EC15") )
   U_INTERNAL_ASSERT( u_hex2int(buf8, 8) == 23456789 )

   u_setPfnMatch(U_DOSMATCH_WITH_OR, 0);

   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("pluto.au"),  U_CONSTANT_TO_PARAM("*.jpg|*.gif|*.mp3"), u_pfn_flags) == false)
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("pippo.gif"), U_CONSTANT_TO_PARAM("*.jpg|*.gif|*.mp3"), u_pfn_flags) == true)
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("workflow"),  U_CONSTANT_TO_PARAM("??/??/????|??:??:??|workflow"), u_pfn_flags) == true)

   u_setPfnMatch(U_DOSMATCH_WITH_OR, FNM_INVERT);

   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("pluto.au"),  U_CONSTANT_TO_PARAM("*.jpg|*.gif|*.mp3"), u_pfn_flags) == true)
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("pippo.gif"), U_CONSTANT_TO_PARAM("*.jpg|*.gif|*.mp3"), u_pfn_flags) == false)
   U_INTERNAL_ASSERT( u_pfn_match(U_CONSTANT_TO_PARAM("workflow"),  U_CONSTANT_TO_PARAM("??/??/????|??:??:??|workflow"), u_pfn_flags) == false)

   u_setPfnMatch(U_DOSMATCH, 0);
   check_match();

   u_setPfnMatch(U_FNMATCH, 0);
   check_match();

   /* Tests for fnmatch function. */

   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("abbb/.x"), U_CONSTANT_TO_PARAM("a*b/*"), FNM_PATHNAME | FNM_PERIOD) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("abbb/xy"), U_CONSTANT_TO_PARAM("a*b/*"), FNM_PATHNAME | FNM_PERIOD) == true)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("!#%+,-./01234567889"), U_CONSTANT_TO_PARAM("!#%+,-./01234567889"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM(":;=@ABCDEFGHIJKLMNO"), U_CONSTANT_TO_PARAM(":;=@ABCDEFGHIJKLMNO"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("PQRSTUVWXYZ]abcdefg"), U_CONSTANT_TO_PARAM("PQRSTUVWXYZ]abcdefg"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("hijklmnopqrstuvwxyz"), U_CONSTANT_TO_PARAM("hijklmnopqrstuvwxyz"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("^_{}~"),               U_CONSTANT_TO_PARAM("^_{}~"),               0) == true)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("\"$&'()"), U_CONSTANT_TO_PARAM("\\\"\\$\\&\\'\\(\\)"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("*?[\\`|"), U_CONSTANT_TO_PARAM("\\*\\?\\[\\\\\\`\\|"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("<>"),      U_CONSTANT_TO_PARAM("\\<\\>"), 0)              == true)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("?*["), U_CONSTANT_TO_PARAM("[?*[][?*[][?*[]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a/b"), U_CONSTANT_TO_PARAM("?/b"), 0) == true)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a/b"), U_CONSTANT_TO_PARAM("a?b"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a/b"), U_CONSTANT_TO_PARAM("a/?"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("aa/b"), U_CONSTANT_TO_PARAM("?/b"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("aa/b"), U_CONSTANT_TO_PARAM("a?b"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a/bb"), U_CONSTANT_TO_PARAM("a/?"), 0) == false)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("abc"), U_CONSTANT_TO_PARAM("[abc]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("[abc]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[abc]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("["), U_CONSTANT_TO_PARAM("[[abc]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[][abc]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a]"), U_CONSTANT_TO_PARAM("[]a]]"), 0) == true)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("xyz"), U_CONSTANT_TO_PARAM("[!abc]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("[!abc]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[!abc]"), 0) == false)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("]"), U_CONSTANT_TO_PARAM("[][abc]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("abc]"), U_CONSTANT_TO_PARAM("[][abc]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("[]abc"), U_CONSTANT_TO_PARAM("[][]abc"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("]"), U_CONSTANT_TO_PARAM("[!]]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("aa]"), U_CONSTANT_TO_PARAM("[!]a]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("]"), U_CONSTANT_TO_PARAM("[!a]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("]]"), U_CONSTANT_TO_PARAM("[!a]]"), 0) == true)

   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.a.]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[[.-.]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[[.-.][.].]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[[.].][.-.]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[[.-.][=u=]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[![.a.]]"), 0) == false)

   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.b.]]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.b.][.c.]]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.b.][=b=]]"), 0) == false)

   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[=a=]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[[=a=]b]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[[=a=][=b=]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[=a=][=b=]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[=a=][.b.]]"), 0) == true)

   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("="), U_CONSTANT_TO_PARAM("[[=a=]b]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("]"), U_CONSTANT_TO_PARAM("[[=a=]b]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[=b=][=c=]]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[=b=][.].]]"), 0) == false)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[a-c]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[a-c]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[a-c]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[b-c]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("d"), U_CONSTANT_TO_PARAM("[b-c]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("B"), U_CONSTANT_TO_PARAM("[a-c]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[A-C]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("as"), U_CONSTANT_TO_PARAM("[a-ca-z]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.a.]-c]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[a-[.c.]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.a.]-[.c.]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[[.a.]-c]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[a-[.c.]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[[.a.]-[.c.]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[[.a.]-c]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[a-[.c.]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[[.a.]-[.c.]]"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("d"), U_CONSTANT_TO_PARAM("[[.a.]-c]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("d"), U_CONSTANT_TO_PARAM("[a-[.c.]]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("d"), U_CONSTANT_TO_PARAM("[[.a.]-[.c.]]"), 0) == false)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[c-a]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.c.]-a]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[c-[.a.]]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.c.]-[.a.]]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[c-a]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[[.c.]-a]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[c-[.a.]]"), 0) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[[.c.]-[.a.]]"), 0) == false)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[a-c0-9]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("d"), U_CONSTANT_TO_PARAM("[a-c0-9]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("B"), U_CONSTANT_TO_PARAM("[a-c0-9]"), 0) == false)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[-a]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[-b]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[!-a]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[!-b]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[a-c-0-9]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[a-c-0-9]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a:"), U_CONSTANT_TO_PARAM("a[0-9-a]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a:"), U_CONSTANT_TO_PARAM("a[09-a]"), 0) == true)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("asd/sdf"), U_CONSTANT_TO_PARAM("*"), 0) == true)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("as"), U_CONSTANT_TO_PARAM("[a-c][a-z]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("as"), U_CONSTANT_TO_PARAM("??"), 0) == true)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("asd/sdf"), U_CONSTANT_TO_PARAM("as*df"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("asd/sdf"), U_CONSTANT_TO_PARAM("as*"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("asd/sdf"), U_CONSTANT_TO_PARAM("*df"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("asd/sdf"), U_CONSTANT_TO_PARAM("as*dg"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("as*df"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("as*df?"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("as*??"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("a*???"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("*????"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("????*"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("??*?"), 0) == true)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("/"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("/*"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("*/"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("/?"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("?/"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("?"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("."), U_CONSTANT_TO_PARAM("?"), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/."), U_CONSTANT_TO_PARAM("??"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("[!a-c]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("."), U_CONSTANT_TO_PARAM("[!a-c]"), 0) == true)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("/"), FNM_PATHNAME) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("//"), U_CONSTANT_TO_PARAM("//"), FNM_PATHNAME) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/.a"), U_CONSTANT_TO_PARAM("/*"), FNM_PATHNAME) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/.a"), U_CONSTANT_TO_PARAM("/?a"), FNM_PATHNAME) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/.a"), U_CONSTANT_TO_PARAM("/[!a-z]a"), FNM_PATHNAME) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/.a/.b"), U_CONSTANT_TO_PARAM("/*/?b"), FNM_PATHNAME) == true)

#ifndef __MINGW32__
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("?"), FNM_PATHNAME) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("*"), FNM_PATHNAME) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a/b"), U_CONSTANT_TO_PARAM("a?b"), FNM_PATHNAME) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/.a/.b"), U_CONSTANT_TO_PARAM("/*b"), FNM_PATHNAME) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/a./.b."), U_CONSTANT_TO_PARAM("/*/*"), FNM_PATHNAME|FNM_PERIOD) == false)
#endif

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/$"), U_CONSTANT_TO_PARAM("\\/\\$"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/["), U_CONSTANT_TO_PARAM("\\/\\["), 0) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/["), U_CONSTANT_TO_PARAM("\\/["), 0) == false)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/$"), U_CONSTANT_TO_PARAM("\\/\\$"), FNM_NOESCAPE) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/\\$"), U_CONSTANT_TO_PARAM("\\/\\$"), FNM_NOESCAPE) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("\\/\\$"), U_CONSTANT_TO_PARAM("\\/\\$"), FNM_NOESCAPE) == true)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM(".asd"), U_CONSTANT_TO_PARAM(".*"), FNM_PERIOD) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/.asd"), U_CONSTANT_TO_PARAM("*"), FNM_PERIOD) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/as/.df"), U_CONSTANT_TO_PARAM("*/?*f"), FNM_PERIOD) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("..asd"), U_CONSTANT_TO_PARAM(".[!a-z]*"), FNM_PERIOD) == true)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM(".asd"), U_CONSTANT_TO_PARAM("*"), FNM_PERIOD) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM(".asd"), U_CONSTANT_TO_PARAM("?asd"), FNM_PERIOD) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM(".asd"), U_CONSTANT_TO_PARAM("[!a-z]*"), FNM_PERIOD) == false)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/."), U_CONSTANT_TO_PARAM("/."), FNM_PATHNAME|FNM_PERIOD) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/.a./.b."), U_CONSTANT_TO_PARAM("/.*/.*"), FNM_PATHNAME|FNM_PERIOD) == true)


   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/."), U_CONSTANT_TO_PARAM("*"), FNM_PATHNAME|FNM_PERIOD) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/."), U_CONSTANT_TO_PARAM("/*"), FNM_PATHNAME|FNM_PERIOD) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/."), U_CONSTANT_TO_PARAM("/?"), FNM_PATHNAME|FNM_PERIOD) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/."), U_CONSTANT_TO_PARAM("/[!a-z]"), FNM_PATHNAME|FNM_PERIOD) == false)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("foobar"), U_CONSTANT_TO_PARAM("foo*[abc]z"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("foobaz"), U_CONSTANT_TO_PARAM("foo*[abc][xyz]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("foobaz"), U_CONSTANT_TO_PARAM("foo?*[abc][xyz]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("foobaz"), U_CONSTANT_TO_PARAM("foo?*[abc][x/yz]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("foobaz"), U_CONSTANT_TO_PARAM("foo?*[abc]/[xyz]"), FNM_PATHNAME) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("a/"), FNM_PATHNAME) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a/"), U_CONSTANT_TO_PARAM("a"), FNM_PATHNAME) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("//a"), U_CONSTANT_TO_PARAM("/a"), FNM_PATHNAME) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("/a"), U_CONSTANT_TO_PARAM("//a"), FNM_PATHNAME) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("az"), U_CONSTANT_TO_PARAM("[a-]z"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("bz"), U_CONSTANT_TO_PARAM("[ab-]z"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("cz"), U_CONSTANT_TO_PARAM("[ab-]z"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-z"), U_CONSTANT_TO_PARAM("[ab-]z"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("az"), U_CONSTANT_TO_PARAM("[-a]z"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("bz"), U_CONSTANT_TO_PARAM("[-ab]z"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("cz"), U_CONSTANT_TO_PARAM("[-ab]z"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-z"), U_CONSTANT_TO_PARAM("[-ab]z"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("\\"), U_CONSTANT_TO_PARAM("[\\\\-a]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("_"), U_CONSTANT_TO_PARAM("[\\\\-a]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[\\\\-a]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[\\\\-a]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("\\"), U_CONSTANT_TO_PARAM("[\\]-a]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("_"), U_CONSTANT_TO_PARAM("[\\]-a]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[\\]-a]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("]"), U_CONSTANT_TO_PARAM("[\\]-a]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[\\]-a]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("\\"), U_CONSTANT_TO_PARAM("[!\\\\-a]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("_"), U_CONSTANT_TO_PARAM("[!\\\\-a]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[!\\\\-a]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[!\\\\-a]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("!"), U_CONSTANT_TO_PARAM("[\\!-]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[\\!-]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("\\"), U_CONSTANT_TO_PARAM("[\\!-]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("Z"), U_CONSTANT_TO_PARAM("[Z-\\\\]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("["), U_CONSTANT_TO_PARAM("[Z-\\\\]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("\\"), U_CONSTANT_TO_PARAM("[Z-\\\\]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[Z-\\\\]"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("Z"), U_CONSTANT_TO_PARAM("[Z-\\]]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("["), U_CONSTANT_TO_PARAM("[Z-\\]]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("\\"), U_CONSTANT_TO_PARAM("[Z-\\]]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("]"), U_CONSTANT_TO_PARAM("[Z-\\]]"), 0) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[Z-\\]]"), 0) == false)

   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("x"),  FNM_PATHNAME|FNM_LEADING_DIR) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y"), U_CONSTANT_TO_PARAM("x"), FNM_PATHNAME|FNM_LEADING_DIR) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y/z"), U_CONSTANT_TO_PARAM("x"), FNM_PATHNAME|FNM_LEADING_DIR) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("*"),     FNM_PATHNAME|FNM_LEADING_DIR) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y"), U_CONSTANT_TO_PARAM("*"),   FNM_PATHNAME|FNM_LEADING_DIR) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y/z"), U_CONSTANT_TO_PARAM("*"), FNM_PATHNAME|FNM_LEADING_DIR) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("*x"),    FNM_PATHNAME|FNM_LEADING_DIR) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y"), U_CONSTANT_TO_PARAM("*x"),  FNM_PATHNAME|FNM_LEADING_DIR) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y/z"), U_CONSTANT_TO_PARAM("*x"), FNM_PATHNAME|FNM_LEADING_DIR) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("x*"),     FNM_PATHNAME|FNM_LEADING_DIR) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y"), U_CONSTANT_TO_PARAM("x*"),   FNM_PATHNAME|FNM_LEADING_DIR) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y/z"), U_CONSTANT_TO_PARAM("x*"), FNM_PATHNAME|FNM_LEADING_DIR) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("a"),       FNM_PATHNAME|FNM_LEADING_DIR) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y"), U_CONSTANT_TO_PARAM("a"),      FNM_PATHNAME|FNM_LEADING_DIR) == false) 
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y/z"), U_CONSTANT_TO_PARAM("a"),    FNM_PATHNAME|FNM_LEADING_DIR) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("x/y"),       FNM_PATHNAME|FNM_LEADING_DIR) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y"), U_CONSTANT_TO_PARAM("x/y"),   FNM_PATHNAME|FNM_LEADING_DIR) == true)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y/z"), U_CONSTANT_TO_PARAM("x/y"), FNM_PATHNAME|FNM_LEADING_DIR) == true)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("x?y"),      FNM_PATHNAME|FNM_LEADING_DIR) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y"), U_CONSTANT_TO_PARAM("x?y"),     FNM_PATHNAME|FNM_LEADING_DIR) == false)
   // U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("x/y/z"), U_CONSTANT_TO_PARAM("x?y"),    FNM_PATHNAME|FNM_LEADING_DIR) == false)

   /* too long...
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmm"
                                                    "nnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyy"),
                                U_CONSTANT_TO_PARAM("a*b*c*d*e*f*g*h*i*j*k*l*m*n*o*p*q*r*s*t*u*v*w*x*y*z*"), 0) == false)
   U_INTERNAL_ASSERT( u_fnmatch(U_CONSTANT_TO_PARAM("aaaabBBbccCCddddeEEeffffggGGhhhhiIIijjjjKKkklLLlMMMm"
                                                    "nnNnooOOpPPPqqQqrrRRssssttttuuUUvvvvwWWwxxxxYYyy"),
                                U_CONSTANT_TO_PARAM("a*b*c*d*e*f*g*h*i*j*k*l*m*n*o*p*q*r*s*t*u*v*w*x*y*"), FNM_CASEFOLD) == true)
   */

   #define ESCAPE_SEQUENCE "[\\b\\n\\t\\r\\v\\f\\e][\\0\\001\\002\\003\\004\\005\\006\\007\\x08\\x09\\x0a\\x0b\\x0c\\x0d\\x0e\\x0f][\\x0A\\x0B\\x0C\\x0D\\x0E\\x0F]"

   #define decode_ESCAPE_SEQUENCE "[\b\n\t\r\v\f\e][\0\001\002\003\004\005\006\007\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f][\x0A\x0B\x0C\x0D\x0E\x0F]"

   U_INTERNAL_ASSERT( u_escape_decode(U_CONSTANT_TO_PARAM(ESCAPE_SEQUENCE), buffer) == sizeof(decode_ESCAPE_SEQUENCE) - 1)

   U_INTERNAL_ASSERT( U_STRNCMP(buffer, decode_ESCAPE_SEQUENCE) == 0 )

   qsort(vec, 13, sizeof(const char*), compare_str);

   /*
   U_INTERNAL_ASSERT( strcmp(vec[0],  "libpng-1.0.3.tar.gz") == 0 )
   U_INTERNAL_ASSERT( strcmp(vec[1],  "libpng-1.0.5.tar.gz") == 0 )
   U_INTERNAL_ASSERT( strcmp(vec[2],  "libpng-1.0.6-patch-a.txt.gz") == 0 )
   U_INTERNAL_ASSERT( strcmp(vec[3],  "libpng-1.0.6-patch-b.txt.gz") == 0 )
   U_INTERNAL_ASSERT( strcmp(vec[4],  "libpng-1.0.6-patch-c.txt.gz") == 0 )
   U_INTERNAL_ASSERT( strcmp(vec[5],  "libpng-1.0.6.tar.gz") == 0 )
   U_INTERNAL_ASSERT( strcmp(vec[6],  "libpng-1.0.7.tar.gz") == 0 )
   U_INTERNAL_ASSERT( strcmp(vec[7],  "libpng-1.0.8.tar.gz") == 0 )
   U_INTERNAL_ASSERT( strcmp(vec[8],  "libpng-1.0.9.tar.gz") == 0 )
   U_INTERNAL_ASSERT( strcmp(vec[9],  "libpng-1.0.10.tar.gz") == 0 )
   U_INTERNAL_ASSERT( strcmp(vec[10], "libpng-1.0.11.tar.gz") == 0 )
   U_INTERNAL_ASSERT( strcmp(vec[11], "libpng-1.0.12.tar.gz") == 0 )
   U_INTERNAL_ASSERT( strcmp(vec[12], "libpng-1.2.0.tar.gz") == 0 )
   */

   for (i = 0; i < 13; ++i) puts(vec[i]);

   /* FTW */

   u_ftw_ctx.call              = ftw_push;
   u_ftw_ctx.call_if_up        = ftw_up;
   u_ftw_ctx.depth             = true;
   u_ftw_ctx.sort_by           = u_ftw_ino_cmp;
   u_ftw_ctx.call_if_directory = true;

   u_buffer_len = 3;

   (void) strcpy(u_buffer, "tmp");

   u_ftw();

   u_buffer_len = 0;

   fflush(stdout);

   /* PATH RELATIV */

   u_cwd_len = 4;
   (void) strcpy(u_cwd, "/usr");

   path_len = U_CONSTANT_SIZE( "/usr///local/src/pippo.txt");
   path_rel = u_getPathRelativ("/usr///local/src/pippo.txt", &path_len);

   U_INTERNAL_ASSERT( U_STREQ(path_rel, "local/src/pippo.txt") )

   u_cwd_len = 1;
   (void) strcpy(u_cwd, "/");

   path_len = U_CONSTANT_SIZE( "/usr/local/src/pippo.txt");
   path_rel = u_getPathRelativ("/usr/local/src/pippo.txt", &path_len);

   U_INTERNAL_ASSERT( U_STREQ(path_rel, "usr/local/src/pippo.txt") )

   (void) strcpy(u_buffer, "../../pippo");

   u_canonicalize_pathname(u_buffer);

   U_INTERNAL_ASSERT( U_STREQ(u_buffer, "../../pippo") )

   (void) strcpy(u_buffer, "../../pippo/./../pluto");

   u_canonicalize_pathname(u_buffer);

   U_INTERNAL_ASSERT( U_STREQ(u_buffer, "../../pluto") )

   return 0;
}
