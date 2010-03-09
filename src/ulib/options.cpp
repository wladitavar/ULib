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

#ifdef HAVE_LFS
#  define LFS_ENABLE          "enabled"
#else
#  define LFS_ENABLE          "no"
#endif

#ifdef HAVE_IPV6
#  define IPV6_ENABLE         "enabled"
#else
#  define IPV6_ENABLE         "no"
#endif

#ifdef U_MEMORY_POOL
#  define MEMORY_POOL_ENABLE  "enabled"
#else
#  define MEMORY_POOL_ENABLE  "no"
#endif

#ifdef HAVE_ZIP
#  define ZIP_ENABLE          "enabled"
#else
#  define ZIP_ENABLE          "no"
#endif

#ifdef HAVE_MAGIC
#  define MAGIC_ENABLE        "enabled"
#else
#  define MAGIC_ENABLE        "no"
#endif

#ifdef HAVE_MYSQL
#  define MYSQL_ENABLE        "enabled"
#else
#  define MYSQL_ENABLE        "no"
#endif

#ifdef HAVE_LIBUUID
#  define LIBUUID_ENABLE     "enabled"
#else
#  define LIBUUID_ENABLE     "no"
#endif

#ifdef HAVE_LIBEVENT
#  define LIBEVENT_ENABLE     "enabled"
#else
#  define LIBEVENT_ENABLE     "no"
#endif

#ifdef HAVE_LIBXML2
#  define LIBXML2_ENABLE     "enabled"
#else
#  define LIBXML2_ENABLE     "no"
#endif

struct option UOptions::long_options[128] = {
{ "help",    0, 0, 'h' },
{ "version", 0, 0, 'V' } };

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

      (void) memcpy(item, old_item, length * sizeof(option_item));

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

   (void) ::printf("%.*s: %.*s\n", U_STRING_TO_TRACE(package), U_STRING_TO_TRACE(version));

   if (purpose.size()) (void) ::printf("Purpose: %.*s\n", U_STRING_TO_TRACE(purpose));

   (void) ::printf("Usage: %.*s [OPTIONS] %.*s...\n", u_progname_len, u_progname, U_STRING_TO_TRACE(args));

// (void) U_SYSCALL(fflush, "%p", stdout);

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

      name_len = u_strlen(ptr_long_options->name);

      (void) memcpy(ptr, ptr_long_options->name, name_len); 

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
         (void) strcpy(ptr, (i ? "Show version information"
                               : "Show help about options"));

         ptr += u_strlen(ptr);
         }
      else
         {
         uint32_t j = i - 2;

         (void) memcpy(ptr, U_STRING_TO_PARAM(*(item[j].desc)));

         ptr += item[j].desc->size();

         if (item[j].value->size())
            {
            (void) U_MEMCPY(ptr, " (default=");

            ptr += 10;

            (void) memcpy(ptr, U_STRING_TO_PARAM(*(item[j].value)));

            ptr += item[j].value->size();

            *ptr++ = ')';
            }
         }

      *ptr++ = '\n';
      *ptr   = '\0';

      (void) ::printf(buffer, 0);
      }

   if (func)
      {
   // (void) U_SYSCALL(fflush, "%p", stdout);

      func();
      }

   if (report_bugs.size()) (void) ::printf("%.*s\n", U_STRING_TO_TRACE(report_bugs));

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
            ::printf("%.*s (%.*s): %.*s\n\n"
               "Developed with ULib (C++ application development framework)\n\n"
               "Building Environment: (" __DATE__ ")\n"
               " Operating System.....: " _OS_VERSION "\n"
               " C++ Compiler.........: " GCC_VERSION "\n"
               " C   Flags............: " CFLAGS_VAR "\n"
               " C++ Flags............: " CXXFLAGS_VAR "\n"
               " Preprocessor Flags...: " CPPFLAGS_VAR "\n"
               " Linker Flags.........: " LDFLAGS_VAR "\n"
               " Linker...............: " LD_VERSION "\n"
               " Libraries............: " LIBS_VAR "\n"
               " Standard C   library.: " LIBC_VERSION "\n"
               " Standard C++ library.: " STDGPP_VERSION "\n"
               " Lexical analyzer.....: " _FLEX_VERSION "\n"
               " Parser generator.....: " _BISON_VERSION "\n"
               " LIBZ  library........: " _LIBZ_VERSION "\n"
               " PCRE  library........: " _PCRE_VERSION "\n"
               " SSL   library........: " _OPENSSL_VERSION "\n"
               " SSH   library........: " _LIBSSH_VERSION "\n"
               " cURL  library........: " _CURL_VERSION "\n"
               " LDAP  library........: " _LDAP_VERSION "\n"
               " Expat library........: " _EXPAT_VERSION "\n"
               " MAGIC library........: " _MAGIC_VERSION "\n"
               " MySQL library........: " _MYSQL_VERSION "\n"
               " libuuid library......: " _LIBUUID_VERSION "\n"
               " libevent library.....: " _LIBEVENT_VERSION "\n"
               " libxml2 library......: " _LIBXML2_VERSION "\n"
               " ZIP support..........: " ZIP_ENABLE "\n"
               " LFS support..........: " LFS_ENABLE "\n"
               " ipv6 support.........: " IPV6_ENABLE "\n"
               " MAGIC support........: " MAGIC_ENABLE "\n"
               " MySQL support........: " MYSQL_ENABLE "\n"
               " libuuid support......: " LIBUUID_ENABLE "\n"
               " libevent support.....: " LIBEVENT_ENABLE "\n"
               " libxml2 support......: " LIBXML2_ENABLE "\n"
               " memory pool support..: " MEMORY_POOL_ENABLE "\n",
               U_STRING_TO_TRACE(package), U_STRING_TO_TRACE(version), U_STRING_TO_TRACE(purpose));

            // Asking the system what it has

            ::printf("\nRequest:\n");
#        ifdef _POSIX_SOURCE
            ::printf("\t_POSIX_SOURCE defined\n");
            ::printf("\t_POSIX_C_SOURCE = %ld\n", _POSIX_C_SOURCE);
#        else
            ::printf("\t_POSIX_SOURCE undefined\n");
#        endif

#     ifdef _XOPEN_SOURCE
#        if _XOPEN_SOURCE == 0
            ::printf("\t_XOPEN_SOURCE defined (0 or no value)\n");
#        else
            ::printf("\t_XOPEN_SOURCE = %d\n", _XOPEN_SOURCE);
#        endif
#     else
            ::printf("\t_XOPEN_SOURCE undefined\n");
#     endif

#        ifdef _XOPEN_SOURCE_EXTENDED
            ::printf("\t_XOPEN_SOURCE_EXTENDED defined\n");
#        else
            ::printf("\t_XOPEN_SOURCE_EXTENDED undefined\n");
#        endif

            ::printf("Claims:\n");
#        ifdef _POSIX_VERSION
            ::printf("\t_POSIX_VERSION = %ld\n", _POSIX_VERSION);
#        else
            ::printf("\tNot POSIX\n");
#        endif

#     ifdef _XOPEN_UNIX
            ::printf("\tX/Open\n");
#        ifdef _XOPEN_VERSION
            ::printf("\t_XOPEN_VERSION = %d\n", _XOPEN_VERSION);
#        else
            ::printf("\tError: _XOPEN_UNIX defined, but not _XOPEN_VERSION\n");
#        endif
#     else
            ::printf("\tNot X/Open\n");
#     endif

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
               UStringRep::assign(item[longindex - 2].value, optarg, u_strlen(optarg));
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
