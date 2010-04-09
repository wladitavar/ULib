// ============================================================================
//
// bench.c: invoke Apache Benchmark (ab) for a concurrency range and collect
//          results in a file suitable for OpenOffice.org SpreadSheet charting
// 
//          100.html is a 100-byte file initially designed to avoid testing the
//          kernel (I wanted to compare the CPU efficiency of each Web server).
//
//          The ITER define can be set to 1 to speed up a test but in that case
//          values are not as reliable as when using more rounds (and using a 
//          low ITER[ations] value usually gives lower performances).
//
//          G-WAN for Windows upgrades Registry system values to remove some
//          artificial limits (original values are just renamed), you need to
//          reboot after you run G-WAN for the first time to load those values.
//          Rebooting for each test has an effect on Windows (you are faster), 
//          like testing after IIS 7.0 was tested (you are even faster), and
//          the Windows Vista 64-bit TCP/IP stack is 10% faster (for all) if
//          ASP.Net is *not* installed.
//
//          Linux Ubuntu 8.1 did not show those boot-related side-effects but 
//          here also I have had to tune the system to run the test:
//
//          sudo gedit /etc/security/limits.conf
//              * soft nofile = 200000
//              * hard nofile = 200000
//
//          sudo gedit /etc/sysctl.conf
//              fs.file-max = 200000
//              net.ipv4.ip_local_port_range = 1024 65535
//              net.ipv4.ip_forward = 0
//              net.ipv4.conf.default.rp_filter = 1
//              kernel.sysrq = 0
//              net.core.rmem_max = 262143
//              net.core.rmem_default = 262143
//              net.ipv4.tcp_rmem = 4096 131072 262143
//              net.ipv4.tcp_wmem = 4096 131072 262143
//              net.ipv4.tcp_sack = 0
//              net.ipv4.tcp_timestamps = 0
//              kernel.shmmax = 67108864 
//
//          As I was not able to make the 'open files limit' persist for G-WAN
//          after a reboot G-WAN attemps to setup this to an 'optimal' value 
//          depending on the amount of RAM available on your system:
//
//              fd_max=(256*(tram/4)<200000)?256*(tram/4):1000000;
//
//          Other values may need to be tuned to get optimal Linux performances
//          I just did my best as a one-month Linux newbe.
//
//          NB: this test was 15-20% faster (for all) when ab was running on a 
//              computer and the Web server on another machine via a gigabit 
//              LAN (but absolute performances were less interesting me than
//              relative performances).
//
//          This program is left in the public domain
//          Author: Pierre GAUTHIER, http://trustleap.ch/
//
// ============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
# include <process.h>
# include <windows.h>
#else
#define Sleep(v) sleep(v/1000)
#define strstri strcasestr
#endif

// ----------------------------------------------------------------------------
#define LOOP 1000 // the range to cover (0-1000 concurrent clients)
#define ITER   20 // the number of iterations of concurrency (worse,aver,best)
// ----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
   int   i, j, nbr, best, worse, aver, inc = 10,
			start		= (argv[2]?atoi(argv[2]):0),
			to_sleep = (argv[3]?atoi(argv[3]):0);
   char  str[256], buff[10000];
   FILE *f, *fo=fopen("test.txt", "w+b");

   for(i=start; i<=LOOP; i+=inc)
   {
       (void) sprintf(str,
       "/usr/sbin/ab -n 1000000 -c %u    -t 1"				 // NO Keep-Alives
//     "/usr/sbin/ab -n 1000000 -c %u -k -t 1"				 // KEEP-ALIVES
//     " \"http://%s/usp/benchmarking.usp?name=stefano\"" // ULib / teepeedee2
//     " \"http://%s/csp?benchmarking?name=stefano\""
//     " \"http://%s/100.html\""
       " \"http://%s/1000.html\""
//     " \"http://%s/usp/hello_world.usp\""					 // ULib / teepeedee2
//     " \"http://%s/csp?hello\""
       " > ab.txt", i?i:1, argv[1]);

      for(best=0, aver=0, worse=0xffff0, j=0; j<ITER; j++)
      {
fault:
         system(str);

         if (to_sleep) sleep(to_sleep); // I needs to take a breath after system() calls

         // get the information we need from res.txt
         if(!(f=fopen("ab.txt", "rb")))
         {
            printf("Can't open file\n");
            return 1;
         }
         buff[0] = '\0';
         fread(buff, 1, sizeof(buff)-1, f);
         fclose(f);

         nbr=0;
         if(*buff)
         {
            char *p=(char*)strstri(buff, "Requests per second:");
            if(p) // "Requests per second:    16270.00 [#/sec] (mean)"
            {
               char *n;
               p+=strlen("Requests per second:")+1;
               while(*p==' '||*p=='\t') p++; n=p; 
               while(*p>='0'&&*p<='9') p++; *p=0;
               nbr=atol(n);
            }
            else
               {
               puts("* Requ. per sec. not found!");
               puts(str);
               goto fault;
               }
         }
         if(best<nbr) best=nbr;
         if(worse>nbr) worse=nbr;
         aver+=nbr;
      }
      aver/=ITER;

      // display data for convenience and save data on disk
      printf("##### %4u,%5u,%5u,%5u #####\n", i?i:1, worse, aver, best);
      fprintf(fo, "%u,%u,%u,%u\n", i?i:1, worse, aver, best);
      fflush(fo); // in case we interrupt the test

	//	if (i == 50) inc = 50;
   }
   fclose(fo);
   return 0;
}
// ============================================================================
// End of Source Code
// ============================================================================