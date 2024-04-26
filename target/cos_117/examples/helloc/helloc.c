#include <stdio.h>
#include <sys/syslog.h>

int main(void) {
   syslog("Hello syslog\n", SYSLOG_USER | SYSLOG_SYSTEM, 1, 1);
   puts("Hello COS world!\n");
   return 0;
}
