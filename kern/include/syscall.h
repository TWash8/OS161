#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);
int sys_getpid(int*);
int sys_waitpid(pid_t, int*, int);
int sys_fork(struct trapframe* tf/*, int* retval*/);
int sys__exit (int);
int sys_write(int fd, char* c, size_t size, int *retval);
int sys_read(int fd, char* buf, size_t size, int *retval);
int sys_execv(char *progname, char **args);
int sys_sbrk(int byte);

// Supporting functions
struct pid_list* get_process (pid_t pid);
void remove_all_exited_children (pid_t);
// Supporting functions --END--


#endif /* _SYSCALL_H_ */
