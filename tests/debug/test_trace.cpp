// test_trace.cpp

#include <ulib/debug/trace.h>
#include <ulib/debug/common.h>

#include <stdlib.h>
#include <sys/time.h>

static int routine2(int a, int b) // masked
{
   U_TRACE(5,"routine2(%d,%d)",a,b)

   int c = a * b;

   U_RETURN(c);
}

static int routine1(int a, int b)
{
   U_INTERNAL_TRACE("routine1(%d,%d)",a,b)

   raise(SIGUSR2); // enable

   U_TRACE(5+256,"routine1(%d,%d)",a,b)

   int c = routine2(a,b);

   U_RETURN(c);
}

static int fd;
static char buffer[4096];

static int test_stat(const char* file)
{
   U_TRACE(5,"test_stat(%S)",file)

   struct stat buf;
   int result = U_SYSCALL(stat,"%S,%p",file,&buf);

   // for display bandwith

   (void) U_SYSCALL(read,"%d,%S,%d",fd,buffer,sizeof(buffer));
   (void) U_SYSCALL(write,"%d,%S,%d",fd,buffer,sizeof(buffer));

   U_RETURN(result);
}

static RETSIGTYPE manage_sigpipe(int signo)
{
   U_TRACE(5,"manage_sigpipe(%d)",signo)

   fd = U_SYSCALL(open,"%S,%o,%o",
      "/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp"
      "/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp"
      "/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/prova",O_RDWR,0666);

   fd = U_SYSCALL(open,"%S,%o,%o","tmp/prova",O_RDWR|O_CREAT,0666);
}

static struct itimerval timeval = { { 0, 5000 }, { 0, 5000 } };

static RETSIGTYPE manage_alarm(int signo)
{
   U_TRACE(5,"[SIGALRM} manage_alarm(%d)",signo)

   int result = U_SYSCALL(setitimer,"%d,%p,%p",ITIMER_REAL,&timeval,0);

   U_DUMP("result setitimer() = %d", result)
}

int U_EXPORT main(int argc, char* argv[])
{
   u_init_ulib(argv);

                  putenv("UTRACE=1 5M 0");     // for test trace
   if (argc >= 2) putenv("USIMERR=error.sim"); // for test simulation error

   u_debug_init();

   U_TRACE(5,"main(%d)",argc)

   raise(SIGUSR2);         // disable -> flag_signal = true;
   u_trace_dtor(false, 0); // call handlerSignal()

   int c = routine1(2, 3);

   U_DUMP("c = %d", c)

   (void) U_SYSCALL(signal,"%d,%p",SIGILL,(sighandler_t)&manage_sigpipe);

   int result = U_SYSCALL(raise,"%d",SIGILL);

   U_DUMP("result raise() = %d", result)

   // test for simulation error

   if (argc >= 2)
      {
      int iteration = 5;

      if (argc == 3)
         {
         iteration = atoi(argv[2]);

         (void) U_SYSCALL(signal,"%d,%p",SIGALRM,(sighandler_t)&manage_alarm);

         (void) U_SYSCALL(setitimer,"%d,%p,%p",ITIMER_REAL,&timeval,0);
         }

      for (int i = 0; i < iteration; ++i)
         {
         if (argc == 3)
            {
            result = test_stat("error.sim");

            U_DUMP("test_stat() = %d", result)
            }
         else
            {
            U_DUMP("test_stat() = %d", test_stat("error.sim"))
            }
         }

      U_DUMP("malloc() = %p", U_SYSCALL(malloc,"%u",1000))
      }

   U_RETURN(0);
}
