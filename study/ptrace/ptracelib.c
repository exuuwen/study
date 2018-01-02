#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <string.h>

/* Enable debug messages */
int debug = 1;
int attached = 0;

pid_t target_pid;

static void ptrace_detach(pid_t);

#define dbgprintf(...) \
	do { \
		if (debug) \
		fprintf(stdout, __VA_ARGS__); \
	} while(0)

#define die(...) \
	do { \
		fprintf(stderr, __VA_ARGS__); \
		if (attached) \
			ptrace_detach(target_pid); \
		exit(1); \
	} while(0)


static void usage(void)
{
	dbgprintf("hot patch qemu-kvm-1.5.3 for writethrough\n");
	dbgprintf("<program> <mode> <pid>\n\n");
	dbgprintf("mode is number, takes only 0, 1, 2\n\n");
	dbgprintf("0: qemu-kvm-1.5.3-60.10.el6.ucloud\n");
	dbgprintf("   qemu-kvm-1.5.3-60.11.el6.ucloud\n\n");
	dbgprintf("1: qemu-kvm-1.5.3-60.2.el6.ucloud\n");
	dbgprintf("   qemu-kvm-1.5.3-60.2.1.el6.ucloud\n");
	dbgprintf("   qemu-kvm-1.5.3-60.2.2.el6.ucloud\n");
	dbgprintf("   qemu-kvm-1.5.3-60.2.3.el6.ucloud\n");
	dbgprintf("   qemu-kvm-1.5.3-60.2.4.el6.ucloud\n");
	dbgprintf("   qemu-kvm-1.5.3-60.2.5.el6.ucloud\n");
	dbgprintf("   qemu-kvm-1.5.3-60.3.el6.ucloud\n\n");
	dbgprintf("2: qemu-kvm-1.5.3-60.8.el6.ucloud\n");
	exit(1);
}

static void ptrace_peektext(pid_t pid, void *addr, char *buf, int size)
{
	long data;

	dbgprintf("ptrace_peektext pid %d at addr %#018lx, size %d\n", pid, addr, size);

	for (; size > 0; size -= sizeof(long), addr += sizeof(long), buf += sizeof(long)) {
		errno = 0;
		data = ptrace(PTRACE_PEEKTEXT, pid, addr, NULL);
		if (data == -1 && errno) {
			die("Failed to ptrace_peektext %d at %#018lx, %s\n", pid, addr, strerror(errno));
		}
		memcpy(buf, &data, sizeof(long));
	}
}

static void ptrace_poketext(pid_t pid, void *addr, char *buf, int size)
{
	dbgprintf("ptrace_poketext pid %d at addr %#018lx, data: %#018lx\n", pid, addr, *(long *)buf);
	for (; size > 0; size -= sizeof(long), addr += sizeof(long), buf += sizeof(long)) {
		if (ptrace(PTRACE_POKETEXT, pid, addr, *(long *)buf)) {
			die("Failed to ptrace_poketext %d at %#018lx, %s\n", pid, addr, strerror(errno));
		}
	}
}

static void ptrace_getregs(pid_t pid, struct user_regs_struct *regs)
{
	dbgprintf("ptrace_getregs pid %d\n", pid);
	ptrace(PTRACE_GETREGS, pid, NULL, regs);
}

static void ptrace_setregs(pid_t pid, struct user_regs_struct *regs)
{
	dbgprintf("ptrace_setregs pid %d\n", pid);
	ptrace(PTRACE_SETREGS, pid, NULL, regs);
}

static void ptrace_cont(pid_t pid)
{
	pid_t w;
	int status;

	dbgprintf("ptrace_cont pid %d\n", pid);

	if (ptrace(PTRACE_CONT, pid, NULL, NULL)) {
		die("Failed to ptrace-cont to PID %d, %s", pid, strerror(errno));
	}

	if (waitpid(pid, &status, __WALL) < 0) {
		die("Failed to wait for PID %d, %s\n", pid, strerror(errno));
	}

	if (WIFEXITED(status) || WIFSIGNALED(status)) {
		die("Pid %d was terminated\n", pid);
	}
}

static void ptrace_attach(pid_t pid)
{
	pid_t w;
	int status;

	dbgprintf("ptrace_attach %d\n", pid);

	if (ptrace(PTRACE_ATTACH, pid, NULL, NULL)) {
		die("Failed to ptrace-attach to PID %d, %s\n", pid, strerror(errno));
	}

	attached = 1;

	if (waitpid(pid, &status, __WALL) < 0) {
		die("Failed to wait for PID %d, %s\n", pid, strerror(errno));
	}

	if (WIFEXITED(status) || WIFSIGNALED(status)) {
		die("Pid %d was terminated\n", pid);
	}
}

static void ptrace_detach(pid_t pid)
{
	dbgprintf("ptrace_detach %d\n", pid);

	if (ptrace(PTRACE_DETACH, pid, NULL, NULL)) {
		die("Failed to ptrace-detach from PID %d, %s\n", pid, strerror(errno));
	}
}

/* Retrieve libc start address by scanning /proc/pid/maps */
static unsigned long get_text_start_addr(const char *mapfile)
{
	FILE *fp;
	char mapbuf[4096], perms[32], libpath[4096];
	unsigned long start, end, file_offset, inode;
	char dev_major, dev_minor;
	const char *qemu_file = "/usr/libexec/qemu-kvm";

	fp = fopen(mapfile, "rb");
	if (!fp) {
		die("Failed to open %s\n", mapfile);
	}

	while (fgets(mapbuf, sizeof(mapbuf), fp)) {
		sscanf(mapbuf, "%llx-%llx %s %llx %x:%x %llu %s", &start,
				&end, perms, &file_offset, &dev_major, &dev_minor, &inode, libpath);

		if (!strncmp(perms, "r-xp", 4) && !strncmp(libpath, qemu_file, strlen(qemu_file))) {
			dbgprintf("%s: %08llx-%08llx %s %llx %s\n", mapfile, start, end, perms, file_offset, libpath );
			fclose(fp);
			return start;
		}
	}

	fclose(fp);
	die("Failed to find qemu text start address from %s, wrong qemu pid?\n", mapfile);
}

char* endswith(const char *s, const char *postfix) {
	size_t sl, pl;

	sl = strlen(s);
	pl = strlen(postfix);

	if (pl == 0)
		return (char*) s + sl;

	if (sl < pl)
		return NULL;

	if (memcmp(s + sl - pl, postfix, pl) != 0)
		return NULL;

	return (char*) s + sl - pl;
}

static inline char *startswith(const char *s, const char *prefix) {
	size_t l;

	l = strlen(prefix);
	if (strncmp(s, prefix, l) == 0)
		return (char*) s + l;

	return NULL;
}

const char *pattern1 = "/opt/data/limg/02aa9f12-291c-4bff-8bd3-ea0866568d78.img";
const char *pattern2 = "/opt/data/disk/c08ed84e-fd50-4eee-9bf5-0e204eb226f0.disk";

static int validate_filename(char *filename)
{
	if (filename[0] == '\0') {
		fprintf(stderr, "filename null\n");
		return 1;
	}

	if (strlen(filename) != strlen(pattern1) && strlen(filename) != strlen(pattern2)) {
		fprintf(stderr, "filename strlen doesn't match\n");
		return 1;
	}

	if (!(strlen(filename) == strlen(pattern1) && startswith(filename, "/opt/data/limg/") && endswith(filename, ".img")) &&
	    !(strlen(filename) == strlen(pattern2) && startswith(filename, "/opt/data/disk/") && endswith(filename, ".disk"))) {
		fprintf(stderr, "filename pattern doesn't match\n");
		return 1;
	}

	if (access(filename, F_OK)) {
		perror("ABORT");
		return 1;
	}
	return 0;
}

// (gdb) p bdrv_states->tqh_first->list.tqe_next->

#define PATH_MAX 1024

//              bdrv_states	filename	enable_write_cache	list	open_flags

// 1.5.3-60.10  0x68fd60	0x48		0x2198			0x21e0	0xc
// 1.5.3-60.11  ^^
// 1.5.3-60.2   0x68ed60	0x48		0x2198			0x21e0	0xc
// 1.5.3-60.2.1 ^^
// 1.5.3-60.2.2 ^^
// 1.5.3-60.2.3 ^^
// 1.5.3-60.2.4 ^^
// 1.5.3-60.2.5 ^^
// 1.5.3-60.3   ^^
// 1.5.3-60.8   0x693d60	0x50		0x21a8			0x21f0	0xc

#define BDRV_O_CACHE_WB 0x0040

unsigned long offset_bdrv_states[3] = {0x68fd60, 0x68ed60, 0x693d60};

unsigned long offset_filename[3] = {0x48, 0x48, 0x50};
unsigned long offset_enable_write_cache[3] = {0x2198, 0x2198, 0x21a8};
unsigned long offset_list[3] = {0x21e0, 0x21e0, 0x21f0};
unsigned long offset_open_flags = 0xc;

int main(int argc, char *argv[]) {
	char qemu_mapfile[256];
	unsigned long qemu_text_start;
	unsigned long bdrv_states_sym, bdrv_states_val;
	unsigned long enable_write_cache = -1;
	int zero = 0;
	char *filename;
	int i = 0, ret = 1;
	unsigned long open_flags;

	if (argc != 3) {
		usage();
	}

	i = atoi(argv[1]);
	target_pid = atoi(argv[2]);

	dbgprintf("%d %d\n", i, target_pid);

	if ((i != 0 && i != 1 && i != 2) || target_pid <= 0) {
		usage();
	}

	filename = malloc(PATH_MAX);

	/* Find qemu text start addr */
	sprintf(qemu_mapfile, "/proc/%d/maps", target_pid);
	qemu_text_start = get_text_start_addr(qemu_mapfile);

	ptrace_attach(target_pid);

	dbgprintf("Trying with offset_bdrv_states[%d]: 0x%lx\n", i, offset_bdrv_states[i]);
	bdrv_states_sym = qemu_text_start + offset_bdrv_states[i];
	dbgprintf("  bdrv_states_sym = %#018lx\n", bdrv_states_sym);

	// bdrv_states->tqh_first
	ptrace_peektext(target_pid, (void *)(bdrv_states_sym), (char *)&bdrv_states_val, sizeof(bdrv_states_val));
	dbgprintf("  bdrv_states->tqh_first = %#018lx\n", bdrv_states_val);

	while (bdrv_states_val) {

		//bdrv_states->tqh_first->filename
		ptrace_peektext(target_pid, (void *)(bdrv_states_val + offset_filename[i]), (char *)filename, 1024);
		dbgprintf("  bs->filename is %s\n", filename);
		if (validate_filename(filename)) {
			ret = 1;
			break;
		}
		ptrace_peektext(target_pid, (void *)(bdrv_states_val + offset_enable_write_cache[i] - 8), (char *)&enable_write_cache, sizeof(enable_write_cache));
		dbgprintf("  bs->enable_write_cache-8: %#018lx\n", (enable_write_cache));
		ptrace_peektext(target_pid, (void *)(bdrv_states_val + offset_enable_write_cache[i] + 8), (char *)&enable_write_cache, sizeof(enable_write_cache));
		dbgprintf("  bs->enable_write_cache+8: %#018lx\n", (enable_write_cache));
		ptrace_peektext(target_pid, (void *)(bdrv_states_val + offset_enable_write_cache[i]), (char *)&enable_write_cache, sizeof(enable_write_cache));
		dbgprintf("  bs->enable_write_cache: %#018lx\n", (enable_write_cache));
		if ((int)enable_write_cache == 1) {
			enable_write_cache = enable_write_cache>>(2<<sizeof(int))<<(2<<sizeof(int));
			dbgprintf("  modified: enable_write_cache: %#018lx\n", enable_write_cache);
			//open_flags = open_flags>>(2<<sizeof(int)<<(2<<sizeof(int)) + (unsigned int)open_flags & ~BDRV_O_CACHE_WB
			ptrace_poketext(target_pid, (void *)(bdrv_states_val + offset_enable_write_cache[i]), (char *)&(enable_write_cache), sizeof(enable_write_cache));
			ret = 0;
		} else if ((int)enable_write_cache == 0) {
			dbgprintf("!NOT modified, enable_write_cache already set to 0\n");
			ret = 2;
		}
		ptrace_peektext(target_pid, (void *)(bdrv_states_val + offset_open_flags), (char *)&offset_open_flags, sizeof(offset_open_flags));
		dbgprintf("  bs->open_flags: %#018lx\n", open_flags);
		ptrace_peektext(target_pid, (void *)(bdrv_states_val + offset_enable_write_cache[i] - 8), (char *)&enable_write_cache, sizeof(enable_write_cache));
		dbgprintf("  bs->enable_write_cache-8: %#018lx\n", (enable_write_cache));
		ptrace_peektext(target_pid, (void *)(bdrv_states_val + offset_enable_write_cache[i] + 8), (char *)&enable_write_cache, sizeof(enable_write_cache));
		dbgprintf("  bs->enable_write_cache+8: %#018lx\n", (enable_write_cache));
		ptrace_peektext(target_pid, (void *)(bdrv_states_val + offset_enable_write_cache[i]), (char *)&enable_write_cache, sizeof(enable_write_cache));
		dbgprintf("  bs->enable_write_cache: %#018lx\n", (enable_write_cache));

		//bdrv_states->tqh_first->list->tqe_next
		ptrace_peektext(target_pid, (void *)(bdrv_states_val + offset_list[i]), (char *)&bdrv_states_val, sizeof(bdrv_states_val));
		dbgprintf("bdrv_states->tqh_first->list->tqh_next = %#018lx\n", bdrv_states_val);
	}

	if (!ret)
		goto out;

out:
	ptrace_detach(target_pid);
	if (ret == 1) {
		dbgprintf("fail\n");
	} else if (ret == 2) {
		dbgprintf("already patched\n");
	} else {
		dbgprintf("success\n");
	}
	return ret;
}
