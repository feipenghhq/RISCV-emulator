#include "rvemu.h"

#define GET(reg, name) u64 name = machine_get_gp_reg(m, reg);

typedef u64 (* syscall_t)(machine_t *m);

static u64 sys_unimpl(machine_t *m) {
    fatalf("unimplemented syscall: %ld", machine_get_gp_reg(m, a7));
}

static u64 sys_exit(machine_t *m) {
    GET(a0, status);
    exit(status);
}

static syscall_t syscall_table[] = {
    [SYS_exit] = sys_exit,
    [SYS_exit_group] = sys_unimpl,
    [SYS_getpid] = sys_unimpl,
    [SYS_kill] = sys_unimpl,
    [SYS_read] = sys_unimpl,
    [SYS_write] = sys_unimpl,
    [SYS_openat] = sys_unimpl,
    [SYS_close] = sys_unimpl,
    [SYS_lseek] = sys_unimpl,
    [SYS_brk] = sys_unimpl,
    [SYS_linkat] = sys_unimpl,
    [SYS_unlinkat] = sys_unimpl,
    [SYS_mkdirat] = sys_unimpl,
    [SYS_renameat] = sys_unimpl,
    [SYS_chdir] = sys_unimpl,
    [SYS_getcwd] = sys_unimpl,
    [SYS_fstat] = sys_unimpl,
    [SYS_fstatat] = sys_unimpl,
    [SYS_faccessat] = sys_unimpl,
    [SYS_pread] = sys_unimpl,
    [SYS_pwrite] = sys_unimpl,
    [SYS_uname] = sys_unimpl,
    [SYS_getuid] = sys_unimpl,
    [SYS_geteuid] = sys_unimpl,
    [SYS_getgid] = sys_unimpl,
    [SYS_getegid] = sys_unimpl,
    [SYS_mmap] = sys_unimpl,
    [SYS_munmap] = sys_unimpl,
    [SYS_mremap] = sys_unimpl,
    [SYS_mprotect] = sys_unimpl,
    [SYS_prlimit64] = sys_unimpl,
    [SYS_getmainvars] = sys_unimpl,
    [SYS_rt_sigaction] = sys_unimpl,
    [SYS_writev] = sys_unimpl,
    [SYS_gettimeofday] = sys_unimpl,
    [SYS_times] = sys_unimpl,
    [SYS_fcntl] = sys_unimpl,
    [SYS_ftruncate] = sys_unimpl,
    [SYS_getdents] = sys_unimpl,
    [SYS_dup] = sys_unimpl,
    [SYS_readlinkat] = sys_unimpl,
    [SYS_rt_sigprocmask] = sys_unimpl,
    [SYS_ioctl] = sys_unimpl,
    [SYS_getrlimit] = sys_unimpl,
    [SYS_setrlimit] = sys_unimpl,
    [SYS_getrusage] = sys_unimpl,
    [SYS_clock_gettime] = sys_unimpl,
    [SYS_set_tid_address] = sys_unimpl,
    [SYS_set_robust_list] = sys_unimpl,
    [SYS_open] = sys_unimpl,
    [SYS_link] = sys_unimpl,
    [SYS_unlink] = sys_unimpl,
    [SYS_mkdir] = sys_unimpl,
    [SYS_access] = sys_unimpl,
    [SYS_stat] = sys_unimpl,
    [SYS_lstat] = sys_unimpl,
    [SYS_time] = sys_unimpl,
};

u64 do_syscall(machine_t *m, u64 n) {
    syscall_t f = NULL;
    f = syscall_table[n];
    if (!f) fatal("unknown syscall");
    return f(m);
}