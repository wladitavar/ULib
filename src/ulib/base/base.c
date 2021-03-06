/** ============================================================================
//
// = LIBRARY
//    ulibase - c library
//
// = FILENAME
//    base.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */
 
/*
#define DEBUG_DEBUG
*/

#include <ulib/base/hash.h>
#include <ulib/base/error.h>
#include <ulib/base/utility.h>
#include <ulib/base/coder/escape.h>

#ifdef DEBUG
#  include <ulib/debug/common.h>
#  define U_INTERNAL_ERROR(assertion,format,args...) \
      if ((bool)(assertion) == false) { \
         u_internal_print(true, \
         " file: %s\n" \
         " line: %d\n" \
         " function: %s\n" \
         " assertion: \"(%s)\" \n" \
         "-------------------------------------\n" \
         format "\n", \
         __FILE__, \
         __LINE__, \
         __PRETTY_FUNCTION__, \
         #assertion , ##args); \
      }
#else
#  define U_INTERNAL_ERROR(assertion,format,args...)
/*
#  undef  U_INTERNAL_TRACE
#  define U_INTERNAL_TRACE(format,args...) u_internal_print(false, format"\n" , ##args);
#  undef  U_INTERNAL_PRINT
#  define U_INTERNAL_PRINT(format,args...) U_INTERNAL_TRACE(format,args)
*/
#endif

#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef USE_LIBSSL
#  include <openssl/err.h>
#endif

#ifdef HAVE_ENDIAN_H
#  include <endian.h>
#endif

#ifndef __MINGW32__
#  include <pwd.h>
#  include <sys/uio.h>
#  include <sys/utsname.h>
#endif

/* For TIOCGWINSZ and friends: */
#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif
#ifdef HAVE_TERMIOS_H
#  include <termios.h>
#endif

/* Startup */
bool                 u_is_tty;
pid_t                u_pid;
uint32_t             u_pid_str_len;
uint32_t             u_progname_len;
      char* restrict u_pid_str;
const char* restrict u_progpath;
const char* restrict u_progname;

/* Current working directory */
char*                u_cwd;
uint32_t             u_cwd_len;

/* Location info */
uint32_t             u_num_line;
const char* restrict u_name_file;
const char* restrict u_name_function;

/* Temporary buffer: for example is used by u_ftw */
char     u_buffer[4096];
uint32_t u_buffer_len; /* signal that is busy */

/* Time services */
bool   u_daylight;
time_t u_start_time;
time_t u_now_adjust; /* GMT based time */
struct tm u_strftime_tm;
void*  u_pthread_time; /* pthread clock */

struct timeval u_timeval;
struct timeval* u_now = &u_timeval;

const char* u_months[]    = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };
const char* u_months_it[] = { "gen", "feb", "mar", "apr", "mag", "giu", "lug", "ago", "set", "ott", "nov", "dic" };

/* Scan services */
const char* restrict u_line_terminator     = U_LF;
uint32_t             u_line_terminator_len = 1;

/* Services */
int                  u_errno; /* An errno value */
int                  u_flag_exit;
int                  u_flag_test;
int                  u_mime_index;
bool                 u_recursion;
bool                 u_exec_failed;
char                 u_hostname[255];
char                 u_user_name[32];
int                  u_printf_fileno = STDERR_FILENO;
int32_t              u_printf_string_max_length;
uint32_t             u_hostname_len, u_user_name_len;
const char* restrict u_tmpdir;
const unsigned char  u_alphabet[]  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const unsigned char  u_hex_upper[] = "0123456789ABCDEF";
const unsigned char  u_hex_lower[] = "0123456789abcdef";

/*
"FATAL: kernel too old"

Even if you recompile the code with -static compiler command-line option to avoid any dependency on the dynamic Glibc library,
you could still encounter the error in question, and your code will exit with Segmentation Fault error.

This kernel version check is done by DL_SYSDEP_OSCHECK macro in Glibc's sysdeps/unix/sysv/linux/dl-osinfo.h
It calls _dl_discover_osversion to get current kernel's version.

The fix (or hack) is to add the following function in your code and compile your code with -static compiler command-line option.

int _dl_discover_osversion() { return 0xffffff; }
*/

__pure const char* u_basename(const char* restrict path)
{
   const char* restrict base;

   U_INTERNAL_TRACE("u_basename(%s)", path)

#ifdef __MINGW32__
   if (u__isalpha(path[0]) && path[1] == ':') path += 2; /* Skip over the disk name in MSDOS pathnames */
#endif

   for (base = path; *path; ++path) if (IS_DIR_SEPARATOR(*path)) base = path + 1;

   return base;
}

__pure const char* u_getsuffix(const char* restrict path, uint32_t len)
{
   const char* restrict ptr;

   U_INTERNAL_TRACE("u_getsuffix(%.*s,%u)", U_min(len,128), path, len)

   U_INTERNAL_ASSERT_POINTER(path)

   // NB: we can have something like 'www.sito1.com/tmp'...

   ptr = (const char*) memrchr(path, '/', len);

   if (ptr) len -= (++ptr - path);
   else     ptr  = path;

   U_INTERNAL_PRINT("ptr = %.*s len = %u", len, ptr, len)

   ptr = (const char*) memrchr(ptr, '.', len);

   return ptr;
}

void u_setPid(void)
{
   pid_t pid_copy;
   static char buffer[10];

   U_INTERNAL_TRACE("u_setPid()")

   u_pid_str = buffer + sizeof(buffer);

   u_pid = getpid();

#ifdef __MINGW32__
   if (u_pid <= 0) u_pid = 9999;
#endif

   pid_copy = u_pid;

   while (pid_copy >= 10)
      {
      *--u_pid_str = (pid_copy % 10) + '0';

      pid_copy /= 10;
      }

   *--u_pid_str = pid_copy + '0';

   u_pid_str_len = buffer + sizeof(buffer) - u_pid_str;
}

void u_init_ulib_username(void)
{
   struct passwd* restrict pw;

   U_INTERNAL_TRACE("u_init_ulib_username()")

   pw = getpwuid(getuid());

   if (pw == 0) u__memcpy(u_user_name, "root", (u_user_name_len = 4), __PRETTY_FUNCTION__);
   else
      {
      u_user_name_len = u__strlen(pw->pw_name);

      U_INTERNAL_ASSERT_MAJOR(u_user_name_len,0)

      u__memcpy(u_user_name, pw->pw_name, u_user_name_len, __PRETTY_FUNCTION__);
      }
}

void u_init_ulib_hostname(void)
{
   U_INTERNAL_TRACE("u_init_ulib_hostname()")

   if (gethostname(u_hostname, sizeof(u_hostname)))
      {
      char* restrict tmp = getenv("HOSTNAME"); /* bash setting... */

      if (tmp && *tmp) u_strcpy(u_hostname, tmp);
      else
         {
#     ifndef __MINGW32__
         FILE* node = (FILE*) fopen("/proc/sys/kernel/hostname", "r");

         if (node)
            {
            (void) fscanf(node, "%254s", u_hostname);

            (void) fclose(node);
            }
         else
            {
            struct utsname buf;

            if (uname(&buf) == 0) (void) strcpy(u_hostname, buf.nodename);
            }
#     endif
         }
      }

   u_hostname_len = u__strlen(u_hostname);

   if (u_hostname_len == 0) (void) strncpy(u_hostname, "localhost", (u_hostname_len = 9));
}

void u_getcwd(void) /* get current working directory */
{
   size_t newsize = 256;

   U_INTERNAL_TRACE("u_getcwd()")

loop:
   u_cwd = (char*) malloc(newsize);

   if (getcwd(u_cwd, newsize) == 0 && errno == ERANGE)
      {
      newsize += 256;

      U_WARNING("current working directory need a bigger buffer (%u bytes), doing reallocation", newsize);

      free(u_cwd);

      goto loop;
      }

#ifdef __MINGW32__
   (void) u_strcpy(u_cwd, u_slashify(u_cwd, PATH_SEPARATOR, '/'));
#endif

   u_cwd_len = u__strlen(u_cwd);

   U_INTERNAL_ASSERT_MAJOR(u_cwd_len, 0)
   U_INTERNAL_ASSERT_MINOR(u_cwd_len, newsize)
}

__pure int u_getMonth(const char* buf)
{
   int i;
   const char* ptr;

   U_INTERNAL_TRACE("u_getMonth(%s)", buf)

   for (i = 0; i < 12; ++i)
      {
      ptr = u_months[i];

      if ((ptr[0] == u__tolower(buf[0])) &&
          (ptr[1] == u__tolower(buf[1])) &&
          (ptr[2] == u__tolower(buf[2])))
         {
         return i+1;
         }

      ptr = u_months_it[i];

      if ((ptr[0] == u__tolower(buf[0])) &&
          (ptr[1] == u__tolower(buf[1])) &&
          (ptr[2] == u__tolower(buf[2])))
         {
         return i+1;
         }
      }

   return 0;
}

bool u_setStartTime(void)
{
   struct tm tm;
   time_t t, lnow;
   const char* compilation_date;

   U_INTERNAL_TRACE("u_setStartTime()")

   U_INTERNAL_ASSERT_POINTER(u_now)

   /* calculate number of seconds between UTC to current time zone
    *
    *         time() returns the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.
    * gettimeofday() gives the number of seconds and microseconds since the Epoch (see time(2)). The tz argument is a struct timezone:
    *
    * struct timezone {
    *    int tz_minuteswest;  // minutes west of Greenwich
    *    int tz_dsttime;      // type of DST correction
    * };
    */

   (void) gettimeofday(u_now, 0);

   /* initialize time conversion information */

   tzset();

   /* The localtime() function converts the calendar time to broken-time representation, expressed relative
    * to the user's specified timezone. The function acts as if it called tzset(3) and sets the external
    * variables tzname with information about the current timezone, timezone with the difference between
    * Coordinated Universal Time (UTC) and local standard time in seconds, and daylight to a non-zero value
    * if daylight savings time rules apply during some part of the year. The return value points to a
    * statically allocated struct which might be overwritten by subsequent calls to any of the date and time
    * functions. The localtime_r() function does the same, but stores the data in a user-supplied struct. It
    * need not set tzname, timezone, and daylight
    *
    * This variable (daylight) has a nonzero value if Daylight Saving Time rules apply. A nonzero value does
    * not necessarily mean that Daylight Saving Time is now in effect; it means only that Daylight Saving Time
    * is sometimes in effect.
    *
    * This variable (timezone) contains the difference between UTC and the latest local standard time, in seconds
    * west of UTC. For example, in the U.S. Eastern time zone, the value is 5*60*60. Unlike the tm_gmtoff member
    * of the broken-down time structure, this value is not adjusted for daylight saving, and its sign is reversed.
    * In GNU programs it is better to use tm_gmtoff, since it contains the correct offset even when it is not the latest one.
    */

   (void) localtime_r(&(u_now->tv_sec), &u_strftime_tm);

#ifdef TM_HAVE_TM_GMTOFF
   u_daylight = (daylight && (timezone != -u_strftime_tm.tm_gmtoff));
#endif

   /* The timegm() function converts the broken-down time representation, expressed in Coordinated Universal
    * Time (UTC) to calendar time
    */

   lnow = timegm(&u_strftime_tm);

   u_now_adjust = (lnow - u_now->tv_sec);

   U_INTERNAL_PRINT("u_now_adjust = %ld timezone = %ld daylight = %d u_daylight = %d tzname[2] = { %s, %s }",
                     u_now_adjust,      timezone,      daylight,     u_daylight,     tzname[0], tzname[1])

   U_INTERNAL_ASSERT(u_now_adjust <= ((daylight ? 3600 : 0) - timezone))

   /* NB: check if current date is OK (>= compilation_date) */

   compilation_date = __DATE__; /* Dec  6 2012 */

// (void) memset(&tm, 0, sizeof(struct tm));

   tm.tm_min   = 0;
   tm.tm_hour  = 0;
   tm.tm_mday  =       atoi(compilation_date+4);
   tm.tm_mon   = u_getMonth(compilation_date)   -    1; /* tm relative format month - range from 0-11 */
   tm.tm_year  =       atoi(compilation_date+7) - 1900; /* tm relative format year  - is number of years since 1900 */
   tm.tm_sec   = 1;
   tm.tm_wday  = 0; /* day of the week */
   tm.tm_yday  = 0; /* day in the year */
   tm.tm_isdst = -1;

   t = mktime(&tm); /* NB: The timelocal() function is equivalent to the POSIX standard function mktime(3) */

   U_INTERNAL_PRINT("lnow = %ld t = %ld", lnow, t)

   if (lnow >= t)
      {
      u_start_time = lnow; /* u_now->tv_sec + u_now_adjust */

      /* The mktime() function modifies the fields of the tm structure as follows: tm_wday and tm_yday are set to values
       * determined from the contents of the other fields; if structure members are outside their valid interval, they will
       * be normalized (so that, for example, 40 October is changed into 9 November); tm_isdst is set (regardless of its
       * initial value) to a positive value or to 0, respectively, to indicate whether DST is or is not in effect at the
       * specified time.  Calling mktime() also sets the external variable tzname with information about the current timezone.
       */

#  ifndef TM_HAVE_TM_GMTOFF
      u_daylight = (tm.tm_isdst != 0);

      U_INTERNAL_PRINT("u_daylight = %d tzname[2] = { %s, %s }",
                        u_daylight,     tzname[0], tzname[1])
#  endif

      return true;
      }

   return false;
}

void u_init_ulib(char** restrict argv)
{
#ifndef __MINGW32__
   const char* restrict pwd;
#endif

   U_INTERNAL_TRACE("u_init_ulib()")

   u_setPid();

   u_progpath = *argv;
   u_progname = u_basename(u_progpath);

   U_INTERNAL_ASSERT_POINTER(u_progname)

   u_progname_len = u__strlen(u_progname);

#ifdef __MINGW32__
   u_init_ulib_mingw();
#endif

   u_getcwd(); /* get current working directory */

#ifndef __MINGW32__
   /* check for bash setting... */

   if ((pwd = getenv("PWD")) &&
       strncmp(u_cwd, pwd, u_cwd_len))
      {
      U_WARNING("current working directory from environment (PWD): %s differ from system getcwd(): %.*s", pwd, u_cwd_len, u_cwd);
      }
#endif

   u_is_tty = isatty(STDERR_FILENO);

   /* initialize AT EXIT */

   (void) atexit(u_exit);

   (void) u_setStartTime();
}

/*
 * Places characters into the array pointed to by s as controlled by the string
 * pointed to by format. If the total number of resulting characters including
 * the terminating null character is not more than maxsize, returns the number
 * of characters placed into the array pointed to by s (not including the
 * terminating null character); otherwise zero is returned and the contents of
 * the array indeterminate.
 */

uint32_t u__strftime(char* restrict s, uint32_t maxsize, const char* restrict format)
{
   static const char* dname[7]  = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
   static const char* mname[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September",
                                    "October", "November", "December" };

   static const int dname_len[7]  = { 6, 6, 7, 9, 8, 6, 8 };
   static const int mname_len[12] = { 7, 8, 5, 5, 3, 4, 4, 6, 9, 7, 8, 8 };

   char ch;                      /* character from format */
   int i, n;                     /* handy integer (short term usage) */
   uint32_t count = 0;           /* return value accumulator */
   const char* restrict fmark;   /* for remembering a place in format */

   U_INTERNAL_TRACE("u__strftime(%u,%s)", maxsize, format)

   U_INTERNAL_ASSERT_POINTER(format)
   U_INTERNAL_ASSERT_MAJOR(maxsize,0)

   /*
   %a An abbreviation for the day of the week
   %A The full name for the day of the week
   %b An abbreviation for the month name
   %h Equivalent to %b. (SU)
   %B The full name of the month
   %c A string representing the complete date and time, in the form Mon Apr 01 13:13:13 1992
   %d The day of the month, formatted with two digits
   %e Like %d, the day of the month as a decimal number, but a leading zero is replaced by a space
   %H The hour (on a 24-hour clock), formatted with two digits
   %I The hour (on a 12-hour clock), formatted with two digits
   %j The count of days in the year, formatted with three digits (from 1 to 366)
   %m The month number, formatted with two digits
   %M The minute, formatted with two digits
   %p Either AM or PM as appropriate
   %S The second, formatted with two digits
   %U The week number, formatted with two digits (from 0 to 53;
      week number 1 is taken as beginning with the first Sunday in a year). See also %W
   %w A single digit representing the day of the week: Sunday is day 0
   %W Another version of the week number: like %U, but counting week 1
      as beginning with the first Monday in a year
   %x A string representing the complete date, in a format like Mon Apr 01 1992
   %X A string representing the full time of day (hours, minutes, and seconds), in a format like 13:13:13
   %T The time in 24-hour notation (%H:%M:%S). (SU)
   %y The last two digits of the year
   %Y The full year, formatted with four digits to include the century
   %z The +hhmm or -hhmm numeric timezone (that is, the hour and minute offset from UTC)
   %Z Defined by ANSI C as eliciting the time zone if available
   %% A single character, %
   */

   while (count < maxsize)
      {
      fmark = format;

      while ((ch = *format) != '\0')
         {
         if (ch == '%') break;

         ++format;
         }

      if ((n = (format - fmark)) != 0)
         {
         while (n--) s[count++] = *fmark++;
         }

      if (ch == '\0') break;

      ++format; /* skip over '%' */

      ch = *format++;

      switch (ch)
         {
         case 'a': /* %a An abbreviation for the day of the week */
            {
            for (i = 0; i < 3; ++i) s[count++] = dname[u_strftime_tm.tm_wday][i];
            }
         break;

         case 'A': /* %A The full name for the day of the week */
            {
            for (i = 0; i < dname_len[u_strftime_tm.tm_wday]; ++i) s[count++] = dname[u_strftime_tm.tm_wday][i];
            }
         break;

         case 'b': /* %b An abbreviation for the month name */
         case 'h': /* %h Equivalent to %b. (SU) */
            {
            for (i = 0; i < 3; ++i) s[count++] = mname[u_strftime_tm.tm_mon][i];
            }
         break;

         case 'B': /* %B The full name of the month */
            {
            for (i = 0; i < mname_len[u_strftime_tm.tm_mon]; ++i) s[count++] = mname[u_strftime_tm.tm_mon][i];
            }
         break;

         case 'c': /* %c A string representing the complete date and time, in the form Mon Apr 01 13:13:13 1992 */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-24))

         // if (count >= (maxsize - 24)) return 0;

            for (i = 0; i < 3; ++i) s[count++] = dname[u_strftime_tm.tm_wday][i];
                                    s[count++] = ' ';
            for (i = 0; i < 3; ++i) s[count++] = mname[u_strftime_tm.tm_mon][i];

            (void) sprintf(&s[count], " %.2d %2.2d:%2.2d:%2.2d %.4d", u_strftime_tm.tm_mday, u_strftime_tm.tm_hour,
                                                                      u_strftime_tm.tm_min,  u_strftime_tm.tm_sec,
                                                                      1900 + u_strftime_tm.tm_year);

            count += 17;
            }
         break;

         case 'd': /* %d The day of the month, formatted with two digits */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-2))

         // if (count >= (maxsize - 2)) return 0;

            (void) sprintf(&s[count], "%2.2d", u_strftime_tm.tm_mday);

            count += 2;
            }
         break;

         case 'e': /* %e Like %d, the day of the month as a decimal number, but a leading zero is replaced by a space */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-2))

         // if (count >= (maxsize - 2)) return 0;

            (void) sprintf(&s[count], "%.2d", u_strftime_tm.tm_mday);

            count += 2;
            }
         break;

         case 'H': /* %H The hour (on a 24-hour clock), formatted with two digits */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-2))

         // if (count >= (maxsize - 2)) return 0;

            (void) sprintf(&s[count], "%2.2d", u_strftime_tm.tm_hour);

            count += 2;
            }
         break;

         case 'I': /* %I The hour (on a 12-hour clock), formatted with two digits */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-2))

         // if (count >= (maxsize - 2)) return 0;

            if (u_strftime_tm.tm_hour == 0 ||
                u_strftime_tm.tm_hour == 12)
               {
               s[count++] = '1';
               s[count++] = '2';
               }
            else
               {
               (void) sprintf(&s[count], "%.2d", u_strftime_tm.tm_hour % 12);

               count += 2;
               }
            }
         break;

         case 'j': /* %j The count of days in the year, formatted with three digits (from 1 to 366) */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-3))

         // if (count >= (maxsize - 3)) return 0;

            (void) sprintf(&s[count], "%.3d", u_strftime_tm.tm_yday + 1);

            count += 3;
            }
         break;

         case 'm': /* %m The month number, formatted with two digits */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-2))

         // if (count >= (maxsize - 2)) return 0;
            
            (void) sprintf(&s[count], "%.2d", u_strftime_tm.tm_mon + 1);

            count += 2;
            }
         break;

         case 'M': /* %M The minute, formatted with two digits */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-2))

         // if (count >= (maxsize - 2)) return 0;

            (void) sprintf(&s[count], "%2.2d", u_strftime_tm.tm_min);

            count += 2;
            }
         break;

         case 'p': /* %p Either AM or PM as appropriate */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-2))

         // if (count >= (maxsize - 2)) return 0;

            s[count++] = ((u_strftime_tm.tm_hour < 12) ? 'A' : 'P');
            s[count++] = 'M';
            }
         break;

         case 'S': /* %S The second, formatted with two digits */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-2))

         // if (count >= (maxsize - 2)) return 0;

            (void) sprintf(&s[count], "%2.2d", u_strftime_tm.tm_sec);

            count += 2;
            }
         break;

         case 'U': /* %U The week number, formatted with two digits (from 0 to 53; week number 1
                      is taken as beginning with the first Sunday in a year). See also %W */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-2))

         // if (count >= (maxsize - 2)) return 0;

            (void) sprintf(&s[count], "%2.2d", (u_strftime_tm.tm_yday + 7 - u_strftime_tm.tm_wday) / 7);

            count += 2;
            }
         break;

         case 'w': /* %w A single digit representing the day of the week: Sunday is day 0 */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-1))

         // if (count >= (maxsize - 1)) return 0;

            (void) sprintf(&s[count], "%1.1d", u_strftime_tm.tm_wday);

            ++count;
            }
         break;

         case 'W': /* %W Another version of the week number: like %U, but counting week 1 as beginning with the first Monday in a year */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-2))

         // if (count >= (maxsize - 2)) return 0;

            (void) sprintf(&s[count], "%2.2d", (u_strftime_tm.tm_yday + ((8 - u_strftime_tm.tm_wday) % 7)) / 7);

            count += 2;
            }
         break;

         case 'x': /* %x A string representing the complete date, in a format like Mon Apr 01 1992 */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-15))

         // if (count >= (maxsize - 15)) return 0;

            for (i = 0; i < 3; i++) s[count++] = dname[u_strftime_tm.tm_wday][i];
                                    s[count++] = ' ';
            for (i = 0; i < 3; i++) s[count++] = mname[u_strftime_tm.tm_mon][i];

            (void) sprintf(&s[count], " %.2d %.4d", u_strftime_tm.tm_mday, 1900 + u_strftime_tm.tm_year);

            count += 8;
            }
         break;

         case 'X': /* %X A string representing the full time of day (hours, minutes, and seconds), in a format like 13:13:13 */
         case 'T': /* %T The time in 24-hour notation (%H:%M:%S). (SU) */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-8))

         // if (count >= (maxsize - 8)) return 0;

            (void) sprintf(&s[count], "%2.2d:%2.2d:%2.2d", u_strftime_tm.tm_hour, u_strftime_tm.tm_min, u_strftime_tm.tm_sec);

            count += 8;
            }
         break;

         case 'y': /* %y The last two digits of the year */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-2))

         // if (count >= (maxsize - 2)) return 0;

            /* The year could be greater than 100, so we need the value modulo 100.
             * The year could be negative, so we need to correct for a possible negative remainder.
             */

            (void) sprintf(&s[count], "%2.2d", (u_strftime_tm.tm_year % 100 + 100) % 100);

            count += 2;
            }
         break;

         case 'Y': /* %Y The full year, formatted with four digits to include the century */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-4))

         // if (count >= (maxsize - 4)) return 0;

            (void) sprintf(&s[count], "%.4d", 1900 + u_strftime_tm.tm_year);

            count += 4;
            }
         break;

         case 'z': /* %z The +hhmm or -hhmm numeric timezone (that is, the hour and minute offset from UTC) */
            {
            U_INTERNAL_ASSERT(count <= (maxsize-5))

         // if (count >= (maxsize - 5)) return 0;

            (void) sprintf(&s[count], "%+.2ld%.2lu", (u_now_adjust / 3600), (u_now_adjust % 3600));

            count += 5;
            }
         break;

         case 'Z': /* %Z Defined by ANSI C as eliciting the time zone if available */
            {
            n = u__strlen(tzname[u_daylight]);

            U_INTERNAL_ASSERT(count <= (maxsize-n))

         // if (count >= (maxsize - n)) return 0;

            (void) u_strncpy(&s[count], tzname[u_daylight], n);

            count += n;
            }
         break;

         case '%':
            {
            /* "%%" prints % */

            s[count++] = '%';

            continue;
            }

         default:
            {
            /* "%?" prints %?, unless ? is NULL */

            s[count++] = '%';

            if (ch == '\0') break;

            /* pretend it was %c with argument ch */

            s[count++] = ch;

            continue;
            }
         }
      }

   s[count] = '\0';

   U_INTERNAL_PRINT("count = %u maxsize = %u", count, maxsize)

   U_INTERNAL_ASSERT(count <= maxsize)

   return count;
}

uint32_t u_strftime(char* restrict s, uint32_t maxsize, const char* restrict format, time_t t)
{
   U_INTERNAL_TRACE("u_strftime(%u,%s,%ld)", maxsize, format, t)

   U_INTERNAL_ASSERT_POINTER(format)
   U_INTERNAL_ASSERT_MAJOR(maxsize,0)

   (void) memset(&u_strftime_tm, 0, sizeof(struct tm));

   (void) gmtime_r(&t, &u_strftime_tm);

   return u__strftime(s, maxsize, format);
}

#ifdef DEBUG
#  include <ulib/base/trace.h>

static bool u_askForContinue(void)
{
   U_INTERNAL_TRACE("u_askForContinue()")

   if (u_is_tty &&
       isatty(STDIN_FILENO))
      {
      char ch[2];

      // NB: we use U_MESSAGE here, but we are already inside u__printf()...

      int u_flag_exit_save = u_flag_exit;
                             u_flag_exit = 0;

      U_MESSAGE("Press '%Wc%W' to continue, '%W%s%W' to exit: %W", GREEN, YELLOW, RED, "Enter", YELLOW, RESET);

      (void) read(STDIN_FILENO, ch, 1);

      if (ch[0] == 'c')
         {
         (void) read(STDIN_FILENO, ch, 1); /* get 'return' key */

         return true;
         }

      u_flag_exit = u_flag_exit_save;
      }

   return false;
}
#endif

void u_internal_print(bool abrt, const char* restrict format, ...)
{
   uint32_t bytes_written;
   char u_internal_buf[16 * 1024];

   va_list argp;
   va_start(argp, format);

   (void) vsnprintf(u_internal_buf, sizeof(u_internal_buf), format, argp);

   va_end(argp);

#ifdef DEBUG
   if (abrt) u_printError();
#endif

   bytes_written = strlen(u_internal_buf);

   (void) write(STDERR_FILENO, u_internal_buf, bytes_written);

#ifdef DEBUG
   if (abrt)
      {
      /* NB: registra l'errore sul file di trace, check stderr per evitare duplicazione messaggio a video */

      if (u_trace_fd > STDERR_FILENO)
         {
         struct iovec iov[1] = { { (caddr_t)u_internal_buf, bytes_written } };

         u_trace_writev(iov, 1);
         }

      if (u_askForContinue() == false)
         {
         u_debug_at_exit();

         u_execOnExit();

         abort();
         }
      }
#endif
}

#ifndef __MINGW32__
static const char* tab_color[] = { U_RESET_STR,
   U_BLACK_STR,       U_RED_STR,           U_GREEN_STR,       U_YELLOW_STR,
   U_BLUE_STR,        U_MAGENTA_STR,       U_CYAN_STR,        U_WHITE_STR,
   U_BRIGHTBLACK_STR, U_BRIGHTRED_STR,     U_BRIGHTGREEN_STR, U_BRIGHTYELLOW_STR,
   U_BRIGHTBLUE_STR,  U_BRIGHTMAGENTA_STR, U_BRIGHTCYAN_STR,  U_BRIGHTWHITE_STR };
#endif

/* Print with format extension: bBCDHMNOPQrRSUYwW
----------------------------------------------------------------------------
'%b': print bool ("true" or "false")
'%B': print bit conversion of integer
'%C': print formatted char
'%H': print name host
'%M': print memory dump
'%N': print name program
'%P': print pid process
'%Q': sign for call to exit() or abort() (var-argument is param to exit)
'%r': print u_getExitStatus(exit_value)
'%R': print var-argument (msg) "-" u_getSysError()
'%S': print formatted string
'%O': print formatted temporary string + free(string)
'%U': print name login user
'%Y': print u_getSysSignal(signo)
'%w': print current working directory
'%W': print COLOR (index to ANSI ESCAPE STR)
'%D': print date and time in various format
----------------------------------------------------------------------------
*/

uint32_t u__vsnprintf(char* restrict buffer, uint32_t buffer_size, const char* restrict format, va_list argp)
{
   int pads;     /* extra padding size */
   int dpad;     /* extra 0 padding needed for integers */
   int bpad;     /* extra blank padding needed */
   int size;     /* size of converted field or string */
   int width;    /* width from format (%8d), or 0 */
   int prec;     /* precision from format (%.3d), or -1 */
   int dprec;    /* a copy of prec if [diouxX], 0 otherwise */
   int fieldsz;  /* field size expanded by sign, dpad etc */

   char sign;                      /* sign prefix (' ', '+', '-', or \0) */
   const char* restrict fmark;     /* for remembering a place in format */
   unsigned char buf_number[32];   /* space for %[cdiouxX] */
   unsigned char* restrict cp = 0; /* handy char pointer (short term usage) */

   uint64_t argument = 0;          /* integer arguments %[diIouxX] */
   enum { OCT, DEC, HEX } base;    /* base for [diIouxX] conversion */

   int flags; /* flags as above */

   /* Flags used during conversion */

#define LONGINT            0x001 /* long integer */
#define LLONGINT           0x002 /* long long integer */
#define LONGDBL            0x004 /* long double */
#define SHORTINT           0x008 /* short integer */
#define ALT                0x010 /* alternate form */
#define LADJUST            0x020 /* left adjustment */
#define ZEROPAD            0x040 /* zero (as opposed to blank) */
#define HEXPREFIX          0x080 /* add 0x or 0X prefix */
#define THOUSANDS_GROUPED  0x100 /* For decimal conversion (i,d,u,f,F,g,G) the output is to be grouped with thousands */

   /* To extend shorts properly, we need both signed and uint32_t argument extraction methods */

#define VA_ARG(type) va_arg(argp, type)

#define SARG() (flags & LLONGINT ?                      VA_ARG(int64_t) : \
                flags &  LONGINT ? (int64_t)            VA_ARG(long) : \
                flags & SHORTINT ? (int64_t)(int16_t)   VA_ARG(int)  : \
                                   (int64_t)            VA_ARG(int))
#define UARG() (flags & LLONGINT ?                      VA_ARG(uint64_t) : \
                flags &  LONGINT ? (uint64_t)           VA_ARG(unsigned long) : \
                flags & SHORTINT ? (uint64_t)(uint16_t) VA_ARG(int) : \
                                   (uint64_t)           VA_ARG(unsigned int))

   /* Scan the format for conversions (`%' character) */

   int n;                  /* handy integer (short term usage) */
   char ch;                /* character from format */
   uint32_t len, ret = 0;  /* return value accumulator */
   char* restrict bp = buffer;

   U_INTERNAL_TRACE("u__vsnprintf(%p,%u,%s)", buffer, buffer_size, format)

   U_INTERNAL_ERROR(buffer_size, "ZERO BUFFER SIZE at u__vsnprintf()", 0);

   while (true)
      {
      U_INTERNAL_ERROR(ret <= buffer_size, "BUFFER OVERFLOW at u__vsnprintf() ret = %u buffer_size = %u", ret, buffer_size);

      fmark = format;

      while ((ch = *format) != '\0')
         {
         if (ch == '%') break;

         ++format;
         }

      if ((n = (format - fmark)) != 0)
         {
         ret += n;

         while (n--) *bp++ = *fmark++;
         }

      if (ch == '\0') break;

      ++format; /* skip over '%' */

      prec = -1;
      sign = '\0';
      size = width = flags = dprec = 0;

rflag:
      ch = *format++;

reswitch:
      U_INTERNAL_PRINT("ch = %c", ch)

      switch (ch)
         {
         /* Start field flag characters: # 0 - ' ' + ' I */

         case '#':
            {
            flags |= ALT;

            goto rflag;
            }

         case '0':
            {
            /* Note that 0 is taken as a flag, not as the beginning of a field width */

            if (!(flags & LADJUST)) flags |= ZEROPAD; /* '-' disables '0' */

            goto rflag;
            }

         case '-':
            {
minus:
            flags |= LADJUST;
            flags &= ~ZEROPAD; /* '-' disables '0' */

            goto rflag;
            }

         case ' ':
            {
            /* If the space and + flags both appear, the space flag will be ignored */

            if (!sign) sign = ' ';

            goto rflag;
            }

         case '+':
            {
            sign = '+';

            goto rflag;
            }

         case '\'': /* For decimal conversion (i,d,u,f,F,g,G) the output is to be grouped with thousands */
            {
            flags |= THOUSANDS_GROUPED;

            goto rflag;
            }

         /* For decimal integer conversion (i,d,u) the output uses the locale alternative output digits
         case 'I':
         break;
         */

         /* End field flag characters: # 0 - ' ' + ' I */

         /* Start field width: [1-9]... */

         /* An optional decimal digit string (with nonzero first digit) specifying a minimum field width */

         case '1': case '2': case '3': case '4':
         case '5': case '6': case '7': case '8': case '9':
            {
            n = 0;

            do {
               n  = 10 * n + (ch - '0');
               ch = *format++;
               }
            while (u__isdigit(ch));

            width = n;

            goto reswitch;
            }

         case '*':
            {
            /* A negative field width argument is taken as a - flag followed by a positive field width.
               They don't exclude field widths read from args */

            if ((width = VA_ARG(int)) >= 0) goto rflag;

            width = -width;

            goto minus;
            }

         /* End field width: [1-9]... */

         /* The field precision '.' */

         case '.':
            {
            if ((ch = *format++) == '*')
               {
               n    = VA_ARG(int);
               prec = (n < 0 ? -1 : n);

               goto rflag;
               }

            for (n = 0; u__isdigit(ch); ch = *format++) n = 10 * n + (ch - '0');

            prec = (n < 0 ? -1 : n);

            goto reswitch;
            }

         /* Start field length modifier: h hh l ll L q j z t */

         case 'h':
            {
            flags |= SHORTINT;

            goto rflag;
            }

         case 'l':
            {
            flags |= (flags & LONGINT ? LLONGINT : LONGINT);

            goto rflag;
            }

         case 'L':
            {
            flags |= LONGDBL;

            goto rflag;
            }

         case 'q': /* `quad'. This is a synonym for ll */
            {
            flags |= LLONGINT;

            goto rflag;
            }

         case 'j': /* A following integer conversion corresponds to an intmax_t or uintmax_t argument */
                   /* case 'z':    A following integer conversion corresponds to a size_t or ssize_t argument */
                   /* case 't':    A following integer conversion corresponds to a ptrdiff_t argument */
            {
            goto rflag;
            }

         /* End field length modifier: h hh l ll L q j z t */

         /* Start field conversion specifier: % c s n p [d,i] [o,u,x,X] [e,E] [f,F] [g,G] [a,A] (C P S) */

         case '%':
            {
            /* "%%" prints % */

            *bp++ = '%';

            ++ret;

            continue;
            }

         case 'c':
            {
            *(cp = buf_number) = VA_ARG(int);

            sign = '\0';
            size = 1;
            }
         break;

         case 's':
            {
#        ifdef DEBUG
            int32_t remaining = (buffer_size - ret);
#        endif

            cp = VA_ARG(unsigned char*);

            if (!cp) cp = (unsigned char* restrict)"(null)";

            sign = '\0';
            size = (prec >= 0 ? prec : (int) u__strlen((const char*)cp));

            U_INTERNAL_ERROR(size <= remaining, "WE ARE GOING TO OVERFLOW BUFFER at u__vsnprintf() size = %u remaining = %u", size, remaining);

            /* if a width from format is specified, the 0 flag for padding will be ignored... */

            if (width >= 0) flags &= ~ZEROPAD;
            }
         break;

         case 'n': /* NB: stores the number of characters printed so far to an integer pointer it pops off the stack */
            {
            if (flags & LLONGINT)
               {
               long long* p = VA_ARG(long long*);

               U_INTERNAL_ERROR(p, "NULL pointer at u__vsnprintf() - CHECK THE PARAMETERS", 0);

               *p = ret;
               }
            else if (flags &  LONGINT)
               {
               long* p = VA_ARG(long*);

               U_INTERNAL_ERROR(p, "NULL pointer at u__vsnprintf() - CHECK THE PARAMETERS", 0);

               *p = ret;
               }
            else if (flags & SHORTINT)
               {
               short* p = VA_ARG(short*);

               U_INTERNAL_ERROR(p, "NULL pointer at u__vsnprintf() - CHECK THE PARAMETERS", 0);

               *p = ret;
               }
            else
               {
               int* p = VA_ARG(int*);

               U_INTERNAL_ERROR(p, "NULL pointer at u__vsnprintf() - CHECK THE PARAMETERS", 0);

               *p = ret;
               }

            continue; /* no output */
            }

         case 'p':
            {
            /* The argument shall be a pointer to void. The value of the pointer is converted to a sequence
            of printable characters, in an implementation-defined manner */

            argument = (uint64_t) (long) VA_ARG(void*)
#        ifndef HAVE_ARCH64
                  & 0x00000000ffffffff
#        endif
            ;

            if (argument)
               {
               ch     = 'x';
               base   = HEX;
               flags |= HEXPREFIX;

               goto nosign;
               }

            (void) U_MEMCPY(bp, "(nil)");

            bp  += 5;
            ret += 5;

            continue;
            }

         case 'd':
         case 'i':
            {
#if SIZEOF_OFF_T == 4 || defined(__MINGW32__) || defined(DISABLE_LFS)
integer:
#endif
            argument = SARG();

            if ((int64_t) argument < 0LL)
               {
               sign     = '-';
               argument = -argument;
               }

            base = DEC;

            goto number;
            }

         case 'u':
            {
            base     = DEC;
            argument = UARG();

            goto nosign;
            }

         case 'o':
            {
            base     = OCT;
            argument = UARG();

            goto nosign;
            }

         case 'X':
         case 'x':
            {
            base     = HEX;
            argument = UARG();

            /* leading 0x/X only if non-zero */

            if ((flags & ALT) &&
                (argument != 0LL))
               {
               flags |= HEXPREFIX;
               }

            /* uint32_t conversions */

nosign:     sign = '\0';

            /* ... diouXx conversions ... if a precision is specified, the 0 flag will be ignored */

number:     if ((dprec = prec) >= 0) flags &= ~ZEROPAD;

            /* The result of converting a zero value with an explicit precision of zero is no characters */

            cp = buf_number + sizeof(buf_number);

            if ((argument != 0LL) ||
                (prec     != 0))
               {
               const unsigned char* restrict xdigs; /* digits for [xX] conversion */

               /* uint32_t mod is hard, and uint32_t mod by a constant is easier than that by a variable; hence this switch */

               switch (base)
                  {
                  case OCT:
                     {
                     do {
                        *--cp = (argument & 7LL) + '0';

                        argument >>= 3L;
                        }
                     while (argument);

                     /* handle octal leading 0 */

                     if (flags & ALT && *cp != '0') *--cp = '0';
                     }
                  break;

                  case DEC:
                     {
                     n = 1;

                     /* many numbers are 1 digit */

                     while (argument >= 10LL)
                        {
                        *--cp = (argument % 10L) + '0';

                        argument /= 10L;

                        if (flags & THOUSANDS_GROUPED && !(n++ % 3)) *--cp = ',';
                        }

                     *--cp = argument + '0';
                     }
                  break;

                  case HEX:
                     {
                     xdigs = (ch == 'X' ? u_hex_upper
                                        : u_hex_lower);

                     do {
                        *--cp = xdigs[argument & 15L];

                        argument /= 16L;
                        }
                     while (argument);
                     }
                  break;
                  }
               }

            size = (ptrdiff_t)(buf_number + sizeof(buf_number) - cp);
            }
         break;

         case 'a':
         case 'A':
         case 'e':
         case 'E':
         case 'f':
         case 'F':
         case 'g':
         case 'G':
            {
            unsigned char fmt_float[32] = { '%' };

            cp = fmt_float + 1;

            if (flags & ALT)               *cp++ = '#';
            if (flags & ZEROPAD)           *cp++ = '0';
            if (flags & LADJUST)           *cp++ = '-';
            if (flags & THOUSANDS_GROUPED) *cp++ = '\'';
            if (sign != '\0')              *cp++ = sign;

            *cp++ = '*'; /* width */
            *cp++ = '.';
            *cp++ = '*'; /* prec */

            if (flags & LONGDBL) *cp++ = 'L';

            *cp++ = ch;
            *cp   = '\0';

            if (flags & LONGDBL)
               {
               long double ldbl = VA_ARG(long double);

               (void) sprintf(bp, (const char* restrict)fmt_float, width, prec, ldbl);
               }
            else
               {
               double dbl = VA_ARG(double);

               (void) sprintf(bp, (const char* restrict)fmt_float, width, prec, dbl);
               }

            len = u__strlen(bp);

            bp  += len;
            ret += len;

            continue;
            }

         /* End field conversion specifier: % c s n p [d,i] [o,u,x,X] [e,E] [f,F] [g,G] [a,A] (C P S) */

         /* Start extension : bBCDHIMNOPQrRSUYwW */

         case 'I': /* print off_t */
            {
#        if SIZEOF_OFF_T == 4 || defined(__MINGW32__) || defined(DISABLE_LFS)
            goto integer;
#        endif

            flags |= LLONGINT;

            argument = SARG();

            if ((int64_t)argument < 0LL)
               {
               sign     = '-';
               argument = -argument;
               }

            base = DEC;

            goto number;
            }
         break;

         case 'W': /* insert COLOR (ANSI ESCAPE STR) */
            {
            n = VA_ARG(int);

#        ifndef __MINGW32__
            if (u_is_tty)
               {
               U_INTERNAL_ERROR(n <= BRIGHTWHITE, "INVALID COLOR(%d) at u__vsnprintf() - CHECK THE PARAMETERS", n);

               len = sizeof(U_RESET_STR) - (n == RESET);

               u__memcpy(bp, tab_color[n], len, __PRETTY_FUNCTION__);

               bp  += len;
               ret += len;
               }
#        endif

            continue;
            }

         case 'H': /* print host name */
            {
            U_INTERNAL_ERROR(u_hostname_len, "HOSTNAME NULL at u__vsnprintf() - CHECK THE PARAMETERS", 0);

            u__memcpy(bp, u_hostname, u_hostname_len, __PRETTY_FUNCTION__);

            bp  += u_hostname_len;
            ret += u_hostname_len;

            continue;
            }

         case 'w': /* print current working directory */
            {
            U_INTERNAL_ERROR(u_cwd_len, "CURRENT WORKING DIRECTORY NULL at u__vsnprintf() - CHECK THE PARAMETERS", 0);

            u__memcpy(bp, u_cwd, u_cwd_len, __PRETTY_FUNCTION__);

            bp  += u_cwd_len;
            ret += u_cwd_len;

            continue;
            }

         case 'N': /* print program name */
            {
            u__memcpy(bp, u_progname, u_progname_len, __PRETTY_FUNCTION__);

            bp  += u_progname_len;
            ret += u_progname_len;

            continue;
            }

         case 'P': /* print process pid */
            {
            u__memcpy(bp, u_pid_str, u_pid_str_len, __PRETTY_FUNCTION__);

            bp  += u_pid_str_len;
            ret += u_pid_str_len;

            continue;
            }

         case 'Q': /* call exit() or abort() (var-argument is the arg passed to exit) */
            {
            u_flag_exit = VA_ARG(int);

            continue;
            }

         case 'R': /* print msg - u_getSysError() */
            {
            uint32_t l               = U_CONSTANT_SIZE(" - ");
            const char* restrict ccp = VA_ARG(const char* restrict);

            U_INTERNAL_PRINT("ccp = %s", ccp)

            if (ccp)
               {
               len = u__strlen(ccp);

               u__memcpy(bp, ccp, len, __PRETTY_FUNCTION__);

               bp  += len;
               ret += len;
               }

            (void) U_MEMCPY(bp, " - ");

            bp  += l;
            ret += l;

            if (errno == 0) errno = u_errno;

#        ifdef __MINGW32__
            if (errno < 0)
               {
               errno = - errno;

               ccp = getSysError_w32(&len);

               u__memcpy(bp, ccp, len, __PRETTY_FUNCTION__);

               bp  += len;
               ret += len;

               (void) U_MEMCPY(bp, " - ");

               bp  += l;
               ret += l;

               MAP_WIN32_ERROR_TO_POSIX
               }
#        endif

            ccp = u_getSysError(&len);

            u__memcpy(bp, ccp, len, __PRETTY_FUNCTION__);

            bp  += len;
            ret += len;

            continue;
            }

         case 'U': /* print user name */
            {
            U_INTERNAL_ERROR(u_user_name_len, "USER NAME NULL at u__vsnprintf() - CHECK THE PARAMETERS", 0);

            u__memcpy(bp, u_user_name, u_user_name_len, __PRETTY_FUNCTION__);

            bp  += u_user_name_len;
            ret += u_user_name_len;

            continue;
            }

         case 'Y': /* print u_getSysSignal(signo) */
            {
            const char* restrict str;

            n   = VA_ARG(int);
            str = u_getSysSignal(n, &len);

            u__memcpy(bp, str, len, __PRETTY_FUNCTION__);

            bp  += len;
            ret += len;

            continue;
            }

         case 'r': /* print u_getExitStatus(exit_value) */
            {
            const char* restrict str;

            n   = VA_ARG(int);
            str = u_getExitStatus(n, &len);

            u__memcpy(bp, str, len, __PRETTY_FUNCTION__);

            bp  += len;
            ret += len;

            continue;
            }

         case 'D': /* print date and time in various format */
            {
            /*
            --------------------------------------------------------------
                        0  => format: %d/%m/%y
            with flag  '1' => format:          %T (=> "%H:%M:%S)
            with flag  '2' => format:          %T (=> "%H:%M:%S) +n days
            with flag  '3' => format: %d/%m/%Y %T
            with flag  '4' => format: %d%m%y_%H%M%S_millisec (for file name, backup, etc...)
            with flag  '5' => format: %a, %d %b %Y %H:%M:%S %Z
            with flag  '6' => format: %Y/%m/%d %T
            with flag  '9' => format: %d/%m/%y %T
            --------------------------------------------------------------
            with flag '10' => format: %d/%m/%y %T *** private for ULog ***
            with flag '11' => format: %d/%b/%Y:%H:%M:%S %z
            with flag '12' => format: %a, %d %b %Y %H:%M:%S GMT
            --------------------------------------------------------------
            */
            time_t t;
            const char* restrict fmtdate =
                (width ==  0 ? "%d/%m/%y"                  :
                 width <=  2 ? "%T"                        :
                 width ==  3 ? "%d/%m/%Y %H:%M:%S"         :
                 width ==  4 ? "%d%m%y_%H%M%S"             :
                 width ==  5 ? "%a, %d %b %Y %H:%M:%S %Z"  :
                 width ==  6 ? "%Y/%m/%d %T"               :
              /* ------------------------------------------- */
                 width <= 10 ? "%d/%m/%y %H:%M:%S"         :
                 width == 11 ? "%d/%b/%Y:%H:%M:%S %z"      :
                               "%a, %d %b %Y %H:%M:%S GMT");

#        if defined(HAVE_PTHREAD_H) && defined(ENABLE_THREAD)
            if (width >= 10 &&
                u_pthread_time)
               {
               /*
               NB: optimization if it is enough a time resolution of one second...

               typedef struct shared_data {
                  ...........
                  struct timeval _timeval;
                  long last_sec[3];
                  char data_1[17]; // 18/06/12 18:45:56
                  char  null1[1];  // 123456789012345678901234567890
                  char data_2[26]; // 04/Jun/2012:18:18:37 +0200
                  char  null2[1];  // 123456789012345678901234567890
                  char data_3[29]; // Wed, 20 Jun 2012 11:43:17 GMT
                  char  null3[1];  // 123456789012345678901234567890
               } shared_data;
               
               u_now = &(ptr_shared_data->_timeval);
               */

               long* last_sec = (long*)((char*)u_now    + sizeof(struct timeval));
               char* ptr      =         (char*)last_sec + sizeof(long) * 3;

               len = (width == 10 ?                        17             :
                      width == 11 ? (last_sec += 1, ptr += 17+1,      26) :
                                    (last_sec += 2, ptr += 17+1+26+1, 29));

               if (*last_sec != u_now->tv_sec)
                  {
                   *last_sec  = u_now->tv_sec;

                       if (width == 10) (void) u_strftime(ptr, 17, "%d/%m/%y %H:%M:%S",         *last_sec + u_now_adjust);
                  else if (width == 11) (void) u_strftime(ptr, 26, "%d/%b/%Y:%H:%M:%S %z",      *last_sec + u_now_adjust);
                  else                  (void) u_strftime(ptr, 29, "%a, %d %b %Y %H:%M:%S GMT", *last_sec);
                  }

               u__memcpy(bp, ptr, len, __PRETTY_FUNCTION__);

               U_INTERNAL_ERROR(u_isBinary((const unsigned char*)bp, len) == false,
                                "binary data at date time optimization:\n"
                                "--------------------------------------\n"
                                "%s", u_memoryDump((unsigned char*)bp, len));

               bp  += len;
               ret += len;

               continue;
               }
#        endif

            /* flag '#' => var-argument */

            if (flags & ALT) t = VA_ARG(time_t);
            else
               {
               (void) gettimeofday(u_now, 0);

               t = (width == 12 ? u_now->tv_sec :
                                  u_now->tv_sec + u_now_adjust);
               }

            len = u_strftime(bp, 36, fmtdate, t);

            if (width == 2) /* check for days */
               {
               if (flags & ALT &&
                   t > U_ONE_DAY_IN_SECOND)
                  {
                  char tmp[16];
                  uint32_t len1;

                  (void) sprintf(tmp, " +%ld days", t / U_ONE_DAY_IN_SECOND);

                  len1 = u__strlen(tmp);

                  u__memcpy(bp+len, tmp, len1, __PRETTY_FUNCTION__);

                  len += len1;
                  }
               }
            else if (width == 4) /* _millisec */
               {
               char tmp[16];
               uint32_t len1;

               (void) sprintf(tmp, "_%03lu", u_now->tv_usec / 1000L);

               len1 = u__strlen(tmp);

               u__memcpy(bp+len, tmp, len1, __PRETTY_FUNCTION__);

               len += len1;
               }

            bp  += len;
            ret += len;

            continue;
            }

         case 'b': /* print bool */
            {
            n = VA_ARG(int);

            if (n)
               {
               (void) U_MEMCPY(bp, "true");

               bp  += 4;
               ret += 4;
               }
            else
               {
               (void) U_MEMCPY(bp, "false");

               bp  += 5;
               ret += 5;
               }

            continue;
            }

         case 'C': /* print formatted char */
            {
            unsigned char c = VA_ARG(int);

            char* restrict _base = bp;

            *bp++ = '\'';

            if (c == '"')
               {
               *bp++ = '"';
               }
            else if (c == '\'')
               {
               *bp++ = '\\';
               *bp++ = '\'';
               }
            else
               {
               bp += u_sprintc(bp, c, false);
               }

            *bp++ = '\'';

            ret += (bp - _base);

            continue;
            }

         case 'S': /* print formatted           string */
         case 'O': /* print formatted temporary string + plus free(string) */
            {
            char* restrict _base = bp;
            int32_t h, maxlen, remaining;

            cp = VA_ARG(unsigned char* restrict);

            if (!cp)
               {
               (void) U_MEMCPY(bp, "(null)");

               bp  += 6;
               ret += 6;

               continue;
               }

            U_INTERNAL_PRINT("prec = %d u_printf_string_max_length = %d", prec, u_printf_string_max_length)

            maxlen = (u_printf_string_max_length > 0 ? u_printf_string_max_length : 128);

            if (prec == -1) prec = maxlen; /* NB: no size... */

            if (prec > maxlen && ((flags & ALT) == 0)) prec = maxlen; /* NB: # -> force print of all size (compatible with buffer)... */

            *bp++ = '"';

            remaining = (buffer_size - ret - 100);

            for (n = 0; n < prec; ++n)
               {
               if (cp[n] == '\0' && ((flags & ALT) == 0)) break;

               h = u_sprintc(bp, cp[n], false);

               bp        += h;
               remaining -= h;

               if (remaining <= 0) break;
               }

            *bp++ = '"';

            if (cp[n] &&
                n == prec)
               {
               (void) U_MEMCPY(bp, "...");

               bp += 3;
               }

            ret += (bp - _base);

#        ifdef DEBUG
            if (ch == 'O') free(cp);
#        endif

            continue;
            }

#     ifdef DEBUG  /* BM */
         case 'B': /* print bit conversion of int */
            {
            unsigned char c;
            int i = sizeof(int), j;

            n  = VA_ARG(int);
            cp = (unsigned char* restrict)&n;

#        if __BYTE_ORDER == __BIG_ENDIAN
            cp += sizeof(int);
#        endif

            *bp++ = '<';

            while (true)
               {
#           if __BYTE_ORDER == __LITTLE_ENDIAN
               c = *cp++;
#           else
               c = *--cp;
#           endif

               if (c)
                  {
#           if __BYTE_ORDER == __LITTLE_ENDIAN
                  for (j = 0; j <= 7; ++j)
#           else
                  for (j = 7; j >= 0; --j)
#           endif
                     {
                     *bp++ = (u_test_bit(j,c) ? '1' : '0');
                     }
                  }
               else
                  {
                  (void) U_MEMCPY(bp, "00000000");

                  bp += 8;
                  }

               if (--i == 0) break;

               *bp++ = ' ';
               }

            *bp++ = '>';

            ret += sizeof(int) * 8 + sizeof(int) - 1 + 2;

            continue;
            }

         case 'M': /* print memory dump */
            {
            cp = VA_ARG(unsigned char* restrict);
            n  = VA_ARG(int);

            len = u_memory_dump(bp, cp, n);

            bp  += len;
            ret += len;

            continue;
            }
#     endif

         /* End extension : bBCDHMNOPQrRSUYwW */

         default:
            {
            /* "%?" prints %?, unless ? is NULL */

            *bp++ = '%';

            ++ret;

            if (ch == '\0') goto done;

            /* pretend it was %c with argument ch */

            *bp++ = ch;

            ++ret;

            continue;
            }
         }

      /* ----------------------------------------------------------------------
      char sign   - sign prefix (' ', '+', '-', or \0)
      int size    - size of converted field or string
      int width   - width from format (%8d), or 0
      int fieldsz - field size expanded by sign, dpad etc
      int pads    - extra padding size
      int dpad    - extra 0 padding needed for integers
      int bpad    - extra blank padding needed
      int prec    - precision from format (%.3d), or -1
      int dprec   - a copy of prec if [diouxX], 0 otherwise
       ----------------------------------------------------------------------
      All reasonable formats wind up here. At this point, `cp' points to
      a string which (if not flags & LADJUST) should be padded out to
      'width' places. If flags & ZEROPAD, it should first be prefixed by any
      sign or other prefix (%010d = "-000123456"); otherwise, it should be
      blank padded before the prefix is emitted (%10d = "   -123456"). After
      any left-hand padding and prefixing, emit zeroes required by a decimal
      [diouxX] precision, then print the string proper, then emit zeroes
      required by any leftover floating precision; finally, if LADJUST, pad with blanks.
      ---------------------------------------------------------------------- */

      U_INTERNAL_PRINT("size = %d width = %d prec = %d dprec = %d sign = %c", size, width, prec, dprec, sign)

      dpad = dprec - size; /* compute actual size, so we know how much to pad */

      if (dpad < 0) dpad = 0;

      fieldsz = size + dpad;

      pads = width - fieldsz;

      if (pads < 0) pads = 0;

      U_INTERNAL_PRINT("fieldsz = %d pads = %d dpad = %d", fieldsz, pads, dpad)

      /* adjust ret */

      ret += (width > fieldsz ? width : fieldsz);

      U_INTERNAL_ERROR(ret <= buffer_size, "BUFFER OVERFLOW at u__vsnprintf() ret = %u buffer_size = %u", ret, buffer_size);

      /* right-adjusting blank padding */

      bpad = 0;

      if (pads && (flags & (LADJUST | ZEROPAD)) == 0)
         {
         for (bpad = pads; pads; --pads) *bp++ = ' ';
         }

      /* prefix */

      if (sign != '\0')
         {
         if (bpad) --bp;
         else
            {
            if (pads) --pads;
            else      ++ret;
            }

         *bp++ = sign;
         }

      else if (flags & HEXPREFIX)
         {
         if (bpad) bp -= 2;
         else
            {
            if (pads) pads -= 2;
            else      ret  += 2;
            }

         *bp++ = '0';
         *bp++ = ch;
         }

      /* right-adjusting zero padding */

      if ((flags & (LADJUST | ZEROPAD)) == ZEROPAD)
         {
         for (; pads; --pads) *bp++ = '0';
         }

      /* leading zeroes from decimal precision */

      for (; dpad; --dpad) *bp++ = '0';

      /* the string or number proper */

      for (; size; --size) *bp++ = *cp++;

      /* left-adjusting padding (always blank) */

      if (flags & LADJUST)
         {
         for (; pads; --pads) *bp++ = ' ';
         }
      }

done:
   U_INTERNAL_PRINT("ret = %u buffer_size = %u", ret, buffer_size)

   if (ret < buffer_size) *bp = '\0';

   U_INTERNAL_ERROR(ret <= buffer_size, "BUFFER OVERFLOW at u__vsnprintf() ret = %u buffer_size = %u", ret, buffer_size);

   return ret;
}

void u_printf2(int fd, const char* restrict format, ...)
{
   char buffer[4096];

   va_list argp;
   va_start(argp, format);

   U_INTERNAL_TRACE("u_printf2(%d,%s)", fd, format)

   (void) write(fd, buffer, u__vsnprintf(buffer, sizeof(buffer), format, argp));

   va_end(argp);
}

uint32_t u__snprintf(char* restrict buffer, uint32_t buffer_size, const char* restrict format, ...)
{
   uint32_t bytes_written;

   va_list argp;
   va_start(argp, format);

   U_INTERNAL_TRACE("u__snprintf(%p,%u,%s)", buffer, buffer_size, format)

   bytes_written = u__vsnprintf(buffer, buffer_size, format, argp);

   va_end(argp);

   return bytes_written;
}

void u__printf(const char* format, ...)
{
   char buffer[4096];
   uint32_t bytes_written;

   va_list argp;
   va_start(argp, format);

   U_INTERNAL_TRACE("u__printf(%s)", format)

   bytes_written = u__vsnprintf(buffer, sizeof(buffer)-1, format, argp);

   va_end(argp);

   buffer[bytes_written++] = '\n';

#ifdef DEBUG
   if (u_flag_exit < 0) u_printError();
#endif

   (void) write(u_printf_fileno, buffer, bytes_written);

   if (u_flag_exit)
      {
#  ifdef DEBUG
      /* NB: registra l'errore sul file di trace, check stderr per evitare duplicazione messaggio a video */

      if (u_trace_fd > STDERR_FILENO)
         {
         /* check if warning due to syscall */

         if (u_flag_exit != 2 || errno == 0)
            {
            struct iovec iov[1] = { { (caddr_t)buffer, bytes_written } };

            u_trace_writev(iov, 1);
            }
         }
#  endif

      /* check if warning */

      if (u_flag_exit == 2)
         {
         u_flag_exit = 0;

         return;
         }

#  ifdef DEBUG
      if (u_flag_exit < 0)
         {
         if (u_flag_test > 0) /* check if to force continue - test */
            {
            --u_flag_test;

            u_flag_exit = 0;

            return;
            }

         if (u_askForContinue()) return;
         }

      u_debug_at_exit(); /* manage for U_ERROR(), U_ABORT(), etc... */
#  endif

#  ifdef USE_LIBSSL
      ERR_print_errors_fp(stderr);
#  endif

      u_execOnExit();

      U_INTERNAL_PRINT("u_flag_exit = %d", u_flag_exit)

      if (u_flag_exit == -2) abort(); /* some assertion false - core dumped... */

      exit(u_flag_exit);
      }
}

/* AT EXIT */

vPF u_fns[32];
int u_fns_index;

void u_atexit(vPF function)
{
   int i;

   U_INTERNAL_TRACE("u_atexit(%p)", function)

   U_INTERNAL_ASSERT_POINTER(function)

   for (i = u_fns_index - 1; i >= 0; --i)
      {
      if (u_fns[i] == function) return;
      }

   u_fns[u_fns_index++] = function;
}

void u_unatexit(vPF function)
{
   int i;

   U_INTERNAL_TRACE("u_unatexit(%p)", function)

   U_INTERNAL_ASSERT_POINTER(function)

   for (i = u_fns_index - 1; i >= 0; --i)
      {
      if (u_fns[i] == function)
         {
         u_fns[i] = 0;

         break;
         }
      }
}

void u_exit(void)
{
   int i;

   U_INTERNAL_TRACE("u_exit()")

   U_INTERNAL_PRINT("u_fns_index = %d", u_fns_index)

   for (i = u_fns_index - 1; i >= 0; --i)
      {
      if (u_fns[i])
         {
         U_INTERNAL_PRINT("u_fns[%d] = %p", i, u_fns[i])

         u_fns[i]();
         }
      }
}

/*
#if defined(U_ALL_C) && !defined(DEBUG)
# undef  U_INTERNAL_TRACE
# define U_INTERNAL_TRACE(format,args...)
# undef  U_INTERNAL_PRINT
# define U_INTERNAL_PRINT(format,args...)
#endif
*/
