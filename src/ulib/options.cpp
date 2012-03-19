// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    options.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/options.h>
#include <ulib/container/vector.h>

#ifdef HAVE_GETOPT_LONG
#  include <getopt.h>
#  define u_getopt_long(argc, argv, options, longopts, longind) ::getopt_long(argc, argv, options, longopts, longind)
#else
#  include <ulib/replace/getopt.h>
#endif

#ifdef ENABLE_MEMPOOL
#  define MEMORY_POOL_ENABLE "enabled"
#else
#  define MEMORY_POOL_ENABLE "no"
#endif
#ifdef ENABLE_LFS
#  define LFS_ENABLE         "enabled"
#else
#  define LFS_ENABLE         "no"
#endif
#ifdef ENABLE_IPV6
#  define IPV6_ENABLE        "enabled"
#else
#  define IPV6_ENABLE        "no"
#endif
#ifdef ENABLE_ZIP
#  define ZIP_ENABLE         "enabled"
#else
#  define ZIP_ENABLE         "no"
#endif

#ifdef USE_LIBZ
#  define LIBZ_ENABLE        "yes ( " _LIBZ_VERSION " )"
#else
#  define LIBZ_ENABLE        "no"
#endif
#ifdef USE_LIBPCRE
#  define LIBPCRE_ENABLE     "yes ( " _PCRE_VERSION " )"
#else
#  define LIBPCRE_ENABLE     "no" 
#endif
#ifdef USE_LIBSSL
#  define LIBSSL_ENABLE      "yes ( " _SSL_VERSION " )"
#else
#  define LIBSSL_ENABLE      "no"
#endif
#ifdef USE_LIBSSH
#  define LIBSSH_ENABLE      "yes ( " _LIBSSH_VERSION " )"
#else
#  define LIBSSH_ENABLE      "no"
#endif
#ifdef USE_LIBLDAP
#  define LIBLDAP_ENABLE     "yes ( " _LDAP_VERSION " )"
#else
#  define LIBLDAP_ENABLE     "no"
#endif
#ifdef USE_LIBCURL
#  define LIBCURL_ENABLE     "yes ( " _CURL_VERSION " )"
#else
#  define LIBCURL_ENABLE     "no"
#endif
#ifdef USE_LIBEXPAT
#  define LIBEXPAT_ENABLE    "yes ( " _EXPAT_VERSION " )"
#else
#  define LIBEXPAT_ENABLE    "no"
#endif
#ifdef USE_LIBMAGIC
#  define MAGIC_ENABLE       "yes ( " _MAGIC_VERSION " )"
#else
#  define MAGIC_ENABLE       "no"
#endif
#ifdef USE_LIBMYSQL
#  define MYSQL_ENABLE       "yes ( " _MYSQL_VERSION " )" 
#else
#  define MYSQL_ENABLE       "no"
#endif
#ifdef USE_LIBDBI
#  define DBI_ENABLE         "yes ( " _DBI_VERSION " )"
#else
#  define DBI_ENABLE         "no"
#endif
#ifdef USE_LIBUUID            
#  define LIBUUID_ENABLE     "yes ( " _LIBUUID_VERSION " )" 
#else
#  define LIBUUID_ENABLE     "no"
#endif
#ifdef USE_LIBEVENT
#  define LIBEVENT_ENABLE    "yes ( " _LIBEVENT_VERSION " )" 
#else
#  define LIBEVENT_ENABLE    "no"
#endif
#ifdef USE_LIBXML2
#  define LIBXML2_ENABLE     "yes ( " _LIBXML2_VERSION " )"
#else
#  define LIBXML2_ENABLE     "no"
#endif
#ifdef USE_PAGE_SPEED
#  define PAGE_SPEED_ENABLE  "yes ( " _PAGE_SPEED_VERSION " )"
#else
#  define PAGE_SPEED_ENABLE  "no"
#endif
#ifdef USE_LIBV8  
#  define V8_ENABLE          "yes ( " _V8_VERSION " )"
#else
#  define V8_ENABLE          "no"
#endif

struct option UOptions::long_options[128] = {
{ "help",    0, 0, 'h' },
{ "version", 0, 0, 'V' } };

UOptions::UOptions(uint32_t n)
{
   U_TRACE_REGISTER_OBJECT(0, UOptions, "%u", n)

   length   = 0;
   capacity = n;

   item = U_MALLOC_N(n, option_item);
}

UOptions::~UOptions()
{
   U_TRACE_UNREGISTER_OBJECT(0, UOptions)

       package.clear();
       version.clear();
       purpose.clear();
   report_bugs.clear();

   for (uint32_t i = 0; i < length; ++i)
      {
      // NB: si decrementa la reference della stringa...

      item[i].desc->release();
      item[i].value->release();
      item[i].long_opt->release();
      }

   U_FREE_N(item, capacity, option_item);
}

// VALUE OF OPTION

UString UOptions::operator[](uint32_t i)
{
   U_TRACE(0, "UOptions::operator[](%u)", i)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_MINOR(i,length)

   UString str(item[i].value);

   U_RETURN_STRING(str);
}

UString UOptions::operator[](char c)
{
   U_TRACE(0, "UOptions::operator[](%C)", c)

   uint32_t i;

   for (i = 0; i < length; ++i)
      {
      if (item[i].short_opt == c) break;
      }

   UString str = operator[](i);

   U_RETURN_STRING(str);
}

UString UOptions::operator[](const UString& long_opt)
{
   U_TRACE(0, "UOptions::operator[](%.*S)", U_STRING_TO_TRACE(long_opt))

   uint32_t i;

   for (i = 0; i < length; ++i)
      {
      if (long_opt.equal(item[i].long_opt)) break;
      }

   UString str = operator[](i);

   U_RETURN_STRING(str);
}

void UOptions::add(const UString& desc,
                   const UString& long_opt,
                   const UString& default_value,
                   int has_arg, char short_opt)
{
   U_TRACE(0,"UOptions::add(%.*S,%.*S,%.*S,%d,%C)",U_STRING_TO_TRACE(desc),U_STRING_TO_TRACE(long_opt),U_STRING_TO_TRACE(default_value),has_arg,short_opt)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(has_arg != 2 || short_opt != '\0')

   if (length == capacity)
      {
      option_item* old_item = item;
      uint32_t old_capacity = capacity;

      capacity *= 2;

      item = U_MALLOC_N(capacity, option_item);

      (void) u_mem_cpy(item, old_item, length * sizeof(option_item));

      U_FREE_N(old_item, old_capacity, option_item);
      }

   item[length].has_arg   = has_arg;
   item[length].short_opt = short_opt;
   item[length].desc      = desc.rep;
   item[length].value     = default_value.rep;
   item[length].long_opt  = long_opt.rep;

   // NB: si incrementa la reference della stringa...

            desc.hold();
        long_opt.hold();
   default_value.hold();

   ++length;
}

// -----------------------------------------------------------------------------
// [package <PACKNAME>]
// [version <VERSION>]
// [purpose <PURPOSE>]
// [report_bugs <REPORT_BUGS>]
// option <SHORT> <LONG> <HAS_ARG> <DESC> <DEFAULT>
// option <SHORT> <LONG> <HAS_ARG> <DESC> <DEFAULT>
// ....
// -----------------------------------------------------------------------------
// option a option_a               0 "A option without arg"       ""
// option b option_b               1 "A option with arg"          ""
// option c option_c               2 "A option with optional arg" Hello
// option - option_with_no_short_1 0 "A option without short"     ""
// option - option_with_no_short_2 1 "A option with default"      Hello
// -----------------------------------------------------------------------------

void UOptions::load(const UString& str)
{
   U_TRACE(0, "UOptions::load(%.*S)", U_STRING_TO_TRACE(str))

   U_CHECK_MEMORY

   UVector<UString> vec(126);

   char* idx;
   uint32_t n = vec.split(str);

   for (uint32_t i = 0; i < n; ++i)
      {
      idx = (char*) memchr("orpv", vec[i].at(0), 4);

      if (idx == 0) continue;

      switch (*idx)
         {
         case 'p':
            {
            if (vec[i].at(1) == 'a') // [package <PACKNAME>]
               {
               U_ASSERT(vec[i] == U_STRING_FROM_CONSTANT("package"))

               package = vec[++i];
               }
            else                    // [purpose <PURPOSE>]
               {
               U_ASSERT(vec[i] == U_STRING_FROM_CONSTANT("purpose"))

               purpose = vec[++i];
               }
            }
         break;

         case 'v':
            {
            // [version <VERSION>]

            U_ASSERT(vec[i] == U_STRING_FROM_CONSTANT("version"))

            version = vec[++i];
            }
         break;

         case 'r':
            {
            // [report_bugs <REPORT_BUGS>]

            U_ASSERT(vec[i] == U_STRING_FROM_CONSTANT("report_bugs"))

            report_bugs = vec[++i];
            }
         break;

         case 'o':
            {
            // option <SHORT> <LONG> <HAS_ARG> <DESC> <DEFAULT>

            U_ASSERT(vec[i] == U_STRING_FROM_CONSTANT("option"))

            char short_opt = vec[i+1].at(0);

            if (short_opt == '-') short_opt = '\0';

            // must be null terminated... after: (row 412) ptr_long_options->name = item[i].long_opt->data();

            UString long_opt(100U);

            long_opt.assign(vec[i+2]);

            *(long_opt.c_pointer(long_opt.size())) = '\0';

         //         desc, long_opt, default_value,           has_arg, short_opt
            add(vec[i+4], long_opt,      vec[i+5], vec[i+3].strtol(), short_opt);

            i += 5;
            }
         break;
         }
      }
}

void UOptions::printHelp(vPF func)
{
   U_TRACE(0, "UOptions::printHelp(%p)", func)

   U_CHECK_MEMORY

   // Print help and exit

   u_is_tty        = isatty(STDOUT_FILENO);
   u_printf_fileno = STDOUT_FILENO;

   u_printf("%W%.*s%W: %.*s", BRIGHTWHITE, U_STRING_TO_TRACE(package), RESET, U_STRING_TO_TRACE(version));

   if (purpose.size()) u_printf("%WPurpose:%W %.*s", BRIGHTWHITE, RESET, U_STRING_TO_TRACE(purpose));

   u_printf("%WUsage:\n  %W%.*s%W [ %WOptions%W ] %W%.*s\n%WOptions:%W",
               BRIGHTWHITE, BRIGHTCYAN, u_progname_len, u_progname, RESET, BRIGHTGREEN, RESET,
               BRIGHTGREEN, U_STRING_TO_TRACE(args), BRIGHTWHITE, RESET);

   struct option* ptr_long_options = long_options + 2;

   uint32_t i, name_len, name_max_len = 7; // version

   for (i = 0; i < length; ++i, ++ptr_long_options)
      {
      name_len = item[i].long_opt->size();

      if (ptr_long_options->has_arg)
         {
         name_len += 6; // =VALUE

         if (ptr_long_options->has_arg == 2) name_len += 2; // []
         }

      if (name_max_len < name_len) name_max_len = name_len;
      }

   char* ptr;
   char buffer[256] = { ' ', ' ', ' ', '-', 'c', ' ', ' ', '-', '-' };
   ptr_long_options = long_options;

   for (i = 0; i < 2 + length; ++i, ++ptr_long_options)
      {
      ptr = buffer + 3;

      if (ptr_long_options->val)
         {
         *ptr++ = '-';
         *ptr++ = ptr_long_options->val;
         }
      else
         {
         *ptr++ = ' ';
         *ptr++ = ' ';
         }

      ptr += 4;

      name_len = u_str_len(ptr_long_options->name);

      (void) u_mem_cpy(ptr, ptr_long_options->name, name_len); 

      ptr += name_len;

      if (ptr_long_options->has_arg)
         {
         name_len += 6;

         if (ptr_long_options->has_arg == 2)
            {
            name_len += 2;

            *ptr++ = '[';
            }

         (void) U_MEMCPY(ptr, "=VALUE");

         ptr += 6;

         if (ptr_long_options->has_arg == 2) *ptr++ = ']';
         }

      for (; name_len < name_max_len; ++name_len) *ptr++ = ' ';

      *ptr++ = ' ';
      *ptr++ = ' ';

      if (i < 2)
         {
         (void) u_strcpy(ptr, (i ? "Show version information"
                                 : "Show help about options"));

         ptr += u_str_len(ptr);
         }
      else
         {
         uint32_t j = i - 2;
         UStringRep* x = item[j].desc;
         uint32_t n = x->size();

         if (n)
            {
            (void) u_mem_cpy(ptr, x->data(), n);

            ptr += n;
            }

         n = item[j].value->size();

         if (n)
            {
            (void) U_MEMCPY(ptr, " (default=");

            ptr += 10;

            (void) u_mem_cpy(ptr, item[j].value->data(), n);

            ptr += n;

            *ptr++ = ')';
            }
         }

      *ptr = '\0';

      u_printf("%W%s%W", BRIGHTCYAN, buffer, RESET);
      }

   if (func) func();

   if (report_bugs.empty() == false)
      {
      u_printf("%W\nMaintained by Stefano Casazza <stefano.casazza@gmail.com>\nReport bugs to <%.*s>%W", BRIGHTYELLOW, U_STRING_TO_TRACE(report_bugs), RESET);
      }

   U_EXIT(EXIT_SUCCESS);
}

/*
typedef struct option {
   const char* name;    // Is the name of the long option
   int         has_arg; // Is: no_argument       (or 0) if the option does not take an argument
                        //     required_argument (or 1) if the option requires an argument
                        //     optional_argument (or 2) if the option takes an optional argument
   int*        flag;    // Specifies how results are returned for a long option. If flag is NULL,
                        // then getopt_long() returns val. (For example, the calling program may
                        // set val to the equivalent short option character)
                        // Otherwise, getopt_long() returns 0, and flag points to a variable which
                        // is set to val if the option is found, but left unchanged if the option is not found
   int         val;     // Is the value to return, or to load into the variable pointed to by flag
} option;
*/

uint32_t UOptions::getopt(int argc, char** argv, int* poptind)
{
   U_TRACE(1, "UOptions::getopt(%d,%p,%p)", argc, argv, poptind)

   U_CHECK_MEMORY

   char optstring[128] = { 'h', 'V' };

   uint32_t i;
   char* ptr_optstring             = optstring    + 2;
   struct option* ptr_long_options = long_options + 2;

   for (i = 0; i < length; ++i, ++ptr_long_options)
      {
      ptr_long_options->name    = item[i].long_opt->data(); // null terminated
      ptr_long_options->has_arg = item[i].has_arg;
      ptr_long_options->flag    = 0;
      ptr_long_options->val     = item[i].short_opt;

      if (ptr_long_options->val)
         {
         *ptr_optstring++ = ptr_long_options->val;

         if (ptr_long_options->has_arg)
            {
            *ptr_optstring++ = ':';

            if (ptr_long_options->has_arg == 2) *ptr_optstring++ = ':';
            }
         }
      }

   *ptr_optstring = '\0';

   (void) U_SYSCALL(memset, "%p,%d,%u", ptr_long_options, 0, sizeof(struct option));

   U_INTERNAL_ASSERT_MINOR(ptr_optstring    - optstring,    128)
   U_INTERNAL_ASSERT_MINOR(ptr_long_options - long_options, 128)

// optarg = 0;    // if there is text in the current argv-element, it is returned in optarg, otherwise optarg is set to zero
   optind = 0;    // optind is the index in argv of the first argv-element that is not an option
// optopt = '?';  // If getopt() does not recognize an option character, it prints an error message
                  // to stderr, stores the character in optopt, and returns `?'
   opterr = 1;    // The calling program may prevent the error message by setting opterr to 0

   int c;         // Character of the parsed option.
   int longindex; // If longindex is not NULL, it points to a variable which is set to the index of the long option
                  // relative to longopts

   while (true)
      {
      longindex = 0;

      // NB: we can't use U_SYSCALL() here because getopt_long return most -1 which is error for system call...

      c = u_getopt_long(argc, argv, optstring, long_options, &longindex);

      U_INTERNAL_DUMP("c = %C longindex = %d optind = %d optarg = %S optopt = %C opterr = %d", c, longindex, optind, optarg, optopt, opterr)

#  ifdef __MINGW32__
      if (&optind != poptind)
         {
         U_INTERNAL_DUMP("&optind = %p poptind = %p", &optind, poptind)

         *poptind = optind;
         }
#  endif

      switch (c)
         {
         case -1: // If there are no more option characters, getopt() returns -1 (EOF)...
            {
            // ...Then optind is the index in argv of the first argv-element that is not an option

            U_RETURN(argc - optind);
            }

         case '?': // Invalid option. `getopt_long()' already printed an error message
            {
            U_EXIT(EXIT_FAILURE);
            }
         break;

         case 'V': // Print version and exit
            {
            u_is_tty        = isatty(STDOUT_FILENO);
            u_printf_fileno = STDOUT_FILENO;

#        ifdef CONFIGURE_CALL
            const char* p = (const char*) u_find(U_CONSTANT_TO_PARAM(CONFIGURE_CALL), U_CONSTANT_TO_PARAM("configure"));

            U_INTERNAL_ASSERT_POINTER(p)

            p += U_CONSTANT_SIZE("configure");
#        endif

/* -----------------------------------------------------------------------------
         ULib version: 1.1.0
            Build ULib: Shared=yes, Static=yes

            Host setup: x86_64-unknown-linux-gnu
        Install prefix: /usr/local
        Install plugin: /usr/local/libexec/ulib
    Configuration data: /usr/local/etc/ulib

      Operating System: Linux stefano 3.2.11 #1 SMP Wed Mar 14 14:48:12 CET 2012 x86_64 Intel(R) Pentium(R) 4 CPU 2.80GHz GenuineIntel GNU/Linux
          C++ Compiler: g++ ( 4.6.2 )
                Linker: /usr/x86_64-pc-linux-gnu/bin/ld -m elf_x86_64 ( GNU ld (GNU Binutils) 2.20.1.20100303 )
  Standard C   library: GNU C Library stable release version 2.13, by Roland McGrath et al.
  Standard C++ library: libstdc++.so.6.0.16
             Libraries: -ltcc -lxml2 -ldbi -lmysqlclient -lldap -llber -lcurl -lssh -lexpat -lpcre -lssl -lcrypto -lmagic -luuid -lz  -lpthread -ldl

             C   Flags: -g -O2  -Werror-implicit-function-declaration -Wstrict-prototypes -Wc++-compat -Wmissing-prototypes -Wnested-externs -Wdeclaration-after-statement -Wold-style-definition
             C++ Flags: -g -O2  -fno-check-new -fno-exceptions -fno-rtti -Wno-deprecated -fvisibility=hidden -fvisibility-inlines-hidden
          Linker Flags:  -Wl,-O1 -Wl,--as-needed -Wl,-z,now,-O1,--hash-style=gnu,--sort-common -Wl,--as-needed
    Preprocessor Flags:  -DDEBUG -DHAVE_SSL_TS -I/usr/include/libxml2 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -pipe -D_GNU_SOURCE  -fstrict-aliasing -fno-stack-protector -fomit-frame-pointer -finline -findirect-inlining -ftree-switch-conversion -floop-interchange -floop-strip-mine -floop-block -Wstrict-aliasing=2 -Wall -Wextra -Wsign-compare -Wpointer-arith -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wpacked -Wswitch-enum -Wmissing-format-attribute -Winit-self -Wformat -Wformat-extra-args -Wenum-compare -Wno-unused-result -Wshadow -Wsuggest-attribute=pure -Wsuggest-attribute=noreturn -Ofast -flto -Wp,-D_FORTIFY_SOURCE=2 -Wunsafe-loop-optimizations -Wno-unused-parameter

         debug support: enabled
      final build mode: no (--enable-final)
    precompiled header: no (--enable-pch)
   memory pool support: enabled
           LFS support: enabled
          ipv6 support: no (--with-ipv6)
           zip support: enabled
          LIBZ support: yes ( 1.2.6 )
          PCRE support: yes ( 8.12 )
           SSL support: yes ( 1.0.0e )
           SSH support: yes ( 0.4.8 )
          LDAP support: yes ( 3001 )
          cURL support: yes ( 7.24.0 )
           XML support: yes ( 2.0.1 )
         MAGIC support: yes ( 5.11 )
         MySQL support: yes ( 50161 )
           DBI support: yes ( 0.8.3 )
       libuuid support: yes ( 1.41.14 )
      libevent support: no (--with-libevent)
       libxml2 support: yes ( 2.7.8 )
    Page-Speed support: yes ( 1.9 )
 V8 JavaScript support: yes ( 3.8.9 )

      LEX/YACC support: enabled
      Lexical analyzer: flex ( flex 2.5.35 )
      Parser generator: bison -y ( bison (GNU Bison) 2.5 )
----------------------------------------------------------------------------- */

            u_printf("%W%.*s%W (%W%.*s%W): %.*s\n\n"
               "%WDeveloped with ULib (C++ application development framework)%W\n\n"

#           ifdef CONFIGURE_CALL
               "configure arguments..:%W%s%W\n"
#           endif
#           ifdef CONFIGURE_DEFINES
               "compile time defines.:%W" CONFIGURE_DEFINES "%W\n\n"
#           endif

               "Building Environment.:%W " PLATFORM_VAR " (" __DATE__ ")%W\n"
               "Operating System.....:%W " _OS_VERSION "%W\n"
               "C++ Compiler.........:%W " CXX_VAR " ( " GCC_VERSION " )%W\n"
               "Linker...............:%W " LD_VAR " ( " LD_VERSION " )%W\n"
               "Standard C   library.:%W " LIBC_VERSION "%W\n"
               "Standard C++ library.:%W " STDGPP_VERSION "%W\n"
               "Libraries............:%W " LIBS_VAR "%W\n\n"

               "C   Flags............:%W " CFLAGS_VAR "%W\n"
               "C++ Flags............:%W " CXXFLAGS_VAR "%W\n"
               "Linker Flags.........:%W " LDFLAGS_VAR "%W\n"
               "Preprocessor Flags...:%W " CPPFLAGS_VAR "%W\n\n"

               "memory pool support..:%W " MEMORY_POOL_ENABLE "%W\n"
               "LFS support..........:%W " LFS_ENABLE "%W\n"
               "ipv6 support.........:%W " IPV6_ENABLE "%W\n"
               "zip support..........:%W " ZIP_ENABLE "%W\n\n"

               "LIBZ support.........:%W " LIBZ_ENABLE "%W\n"
               "PCRE support.........:%W " LIBPCRE_ENABLE "%W\n"
               "SSL support..........:%W " LIBSSL_ENABLE "%W\n"
               "SSH support..........:%W " LIBSSH_ENABLE "%W\n"
               "LDAP support.........:%W " LIBLDAP_ENABLE "%W\n"
               "LDAP support.........:%W " LIBLDAP_ENABLE "%W\n"
               "XML support..........:%W " LIBEXPAT_ENABLE "%W\n"
               "MAGIC support........:%W " MAGIC_ENABLE "%W\n"
               "MySQL support........:%W " MYSQL_ENABLE "%W\n"
               "DBI support..........:%W " DBI_ENABLE "%W\n"
               "libuuid support......:%W " LIBUUID_ENABLE "%W\n"
               "libevent support.....:%W " LIBEVENT_ENABLE "%W\n"
               "libxml2 support......:%W " LIBXML2_ENABLE "%W\n"
               "Page-Speed support...:%W " PAGE_SPEED_ENABLE "%W\n"
               "V8 JavaScript support:%W " V8_ENABLE "%W\n\n"

               "Lexical analyzer.....:%W " _FLEX_VERSION "%W\n"
               "Parser generator.....:%W " _BISON_VERSION "%W\n",
               BRIGHTCYAN,  U_STRING_TO_TRACE(package), RESET,
               BRIGHTGREEN, U_STRING_TO_TRACE(version), RESET,
               U_STRING_TO_TRACE(purpose), BRIGHTWHITE, RESET,
#           ifdef CONFIGURE_CALL
               BRIGHTCYAN, p, RESET,
#           endif
#           ifdef CONFIGURE_DEFINES
               BRIGHTCYAN, RESET,
#           endif
               // ambient
               BRIGHTCYAN, RESET,
               BRIGHTCYAN, RESET,
               BRIGHTCYAN, RESET,
               BRIGHTCYAN, RESET,
               BRIGHTCYAN, RESET,
               BRIGHTCYAN, RESET,
               BRIGHTCYAN, RESET,
               // flags
               BRIGHTWHITE, RESET,
               BRIGHTWHITE, RESET,
               BRIGHTWHITE, RESET,
               BRIGHTWHITE, RESET,
               // support
               BRIGHTGREEN, RESET,
               BRIGHTGREEN, RESET,
               BRIGHTGREEN, RESET,
               BRIGHTGREEN, RESET,
               // warapping
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET,
               // parser
               BRIGHTYELLOW, RESET,
               BRIGHTYELLOW, RESET);

            // Asking the system what it has

            u_printf("%WRequest:%W", BRIGHTWHITE, BRIGHTGREEN);
#        ifdef _POSIX_SOURCE
            u_printf("\t_POSIX_SOURCE defined");
            u_printf("\t_POSIX_C_SOURCE = %ld", _POSIX_C_SOURCE);
#        else
            u_printf("%W\t_POSIX_SOURCE undefined%W", BRIGHTRED, BRIGHTGREEN);
#        endif

#     ifdef _XOPEN_SOURCE
#        if _XOPEN_SOURCE == 0
            u_printf("\t_XOPEN_SOURCE defined (0 or no value)");
#        else
            u_printf("\t_XOPEN_SOURCE = %d", _XOPEN_SOURCE);
#        endif
#     else
            u_printf("%W\t_XOPEN_SOURCE undefined%W", BRIGHTRED, BRIGHTGREEN);
#     endif

#        ifdef _XOPEN_SOURCE_EXTENDED
            u_printf("\t_XOPEN_SOURCE_EXTENDED defined");
#        else
            u_printf("%W\t_XOPEN_SOURCE_EXTENDED undefined", BRIGHTRED);
#        endif

            u_printf("%WClaims:%W", BRIGHTWHITE, BRIGHTYELLOW);
#        ifdef _POSIX_VERSION
            u_printf("\t_POSIX_VERSION = %ld", _POSIX_VERSION);
#        else
            u_printf("%W\tNot POSIX%W", BRIGHTRED, BRIGHTYELLOW);
#        endif

#     ifdef _XOPEN_UNIX
            u_printf("\tX/Open");
#        ifdef _XOPEN_VERSION
            u_printf("\t_XOPEN_VERSION = %d", _XOPEN_VERSION);
#        else
            u_printf("\tError: _XOPEN_UNIX defined, but not _XOPEN_VERSION");
#        endif
#     else
            u_printf("%W\tNot X/Open%W", BRIGHTRED, BRIGHTYELLOW);
#     endif

            u_printf("%W", RESET);

            U_EXIT(EXIT_SUCCESS);
            }
         break;

         // Print help and exit

         case 'h': printHelp(0); break;

         default: // option
            {
            if (longindex == 0)
               {
               ptr_long_options = long_options + 2;

               for (i = 0; i < length; ++i, ++ptr_long_options)
                  {
                  if (ptr_long_options->val == c)
                     {
                     longindex = 2 + i;

                     break;
                     }
                  }
               }

            U_INTERNAL_ASSERT_EQUALS(long_options[longindex].val,c)

            if (long_options[longindex].has_arg == 0)
               {
               U_INTERNAL_ASSERT_EQUALS(optarg,0)

               static char buffer[] = { '1', '\0' };

               optarg = buffer;
               }

            if (optarg)
               {
               UStringRep::assign(item[longindex - 2].value, optarg, u_str_len(optarg));
               }
            else
               {
               U_INTERNAL_ASSERT_EQUALS(long_options[longindex].has_arg,2)
               }
            }
         break;
         }
      }
}

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* UOptions::dump(bool reset) const
{
   *UObjectIO::os << "item                         " << (void*)item           << '\n'
                  << "length                       " << length                << '\n'
                  << "capacity                     " << capacity              << '\n'
                  << "package     (UString         " << (void*)&package       << ")\n"
                  << "version     (UString         " << (void*)&version       << ")\n"
                  << "purpose     (UString         " << (void*)&purpose       << ")\n"
                  << "report_bugs (UString         " << (void*)&report_bugs   << ")\n";

   for (uint32_t i = 0; i < length; ++i)
      {
      *UObjectIO::os << "\nitem[" << i << "]"
                               " has_arg              " << item[i].has_arg << '\n'
                     << "        short_opt            ";

      if (item[i].short_opt)
         {
         *UObjectIO::os << '\'' << item[i].short_opt << "'\n";
         }
      else
         {
         *UObjectIO::os << "0\n";
         }

      *UObjectIO::os << "        desc     (UStringRep " << (void*)item[i].desc      << ")\n"
                     << "        value    (UStringRep " << (void*)item[i].value     << ")\n"
                     << "        long_opt (UStringRep " << (void*)item[i].long_opt  << ')';
      }

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

#endif
