
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/moduleparam.h>

#include <linux/syscalls.h>


static int uid = 0;
module_param(uid, int, S_IRUGO);



// https://web.iiit.ac.in/~arjun.nath/random_notes/modifying_sys_call.html
//
// $ cat ../goldfish/System.map | grep call_table
// ffffffff80a00300 R sys_call_table

// #define	SYS_CALL_TABLE_ADDRESS	0xffffffff80a00300

void **sys_call_table = (void *) 0xffffffff80a00300;

// R means memory is write protected
//
// $ cat ../goldfish/System.map | grep set_pages
// ffffffff8022e995 T set_pages_ro
// ffffffff8022e9c2 T set_pages_rw

void (*pages_rw)(struct page *page, int numpages) = (void *) 0xffffffff8022e9c2;
void (*pages_ro)(struct page *page, int numpages) = (void *) 0xffffffff8022e995;

// Control Register is set
//
// cat /proc/cpuinfo | grep wp
// wp : yes




// # ps -A
//
// # strace -f -p <PID> 2>&1 | grep txt
// faccessat(AT_FDCWD, "/data/local/tmp/test.txt", F_OK)


int (*original_faccessat)(int dirfd, const char *pathname, int mode, int flags);

uid_t (*original_getuid)(void);
pid_t (*original_getpid)(void);


int mystrcmp(const char *str1, const char *str2)
{
	while (*str1 && *str2)
	{
		if (*(str1++) != *(str2++))
		{
			return -1;
		}
	}

	return 0;
}

int hooked_faccessat(int dirfd, const char *pathname, int mode, int flags)
{
	int result;
	uid_t current_uid;

	// Get the current uid, we hook only to the uid passed as an argument
	current_uid = original_getuid();

	// Check if we have to hook
	if (uid == current_uid)
	{
		printk(KERN_INFO "Hooked faccessat, { %s } \n", pathname);

		// Hide test.txt
		if (!mystrcmp(pathname, "/data/local/tmp/test.txt"))
		{
			printk(KERN_INFO "Hide test.txt\n");
			return EACCES;
		}
	}

	// Call the original function
	result = (*original_faccessat)(dirfd, pathname, mode, flags);

	return result;
}


void unprotect_mem(void)
{
	struct page *sys_call_table_temp;

	// Set the control register to 0, (not write protected)
	write_cr0 (read_cr0 () & (~ 0x10000));

	// Set the sys_call_table memory to rw
	sys_call_table_temp = virt_to_page(&sys_call_table[__NR_open]);
	pages_rw(sys_call_table_temp, 1);
}

void protect_mem(void)
{
	struct page *sys_call_table_temp;

	// Set the sys_call_table memory to ro
	sys_call_table_temp = virt_to_page(&sys_call_table[__NR_open]);
	pages_ro(sys_call_table_temp, 1);

	// Set the control register to 1, (write protected)
	write_cr0 (read_cr0 () | ( 0x10000));
}


int init_module(void)
{
	printk(KERN_INFO "Load module\n");

	unprotect_mem();

	// Save the original function entry and replace it with a modified one
	original_faccessat = sys_call_table[__NR_faccessat];
	sys_call_table[__NR_faccessat] = hooked_faccessat;

	protect_mem();

	// Get other function pointers
	original_getuid = sys_call_table[__NR_getuid];
	original_getpid = sys_call_table[__NR_getpid];

	return 0;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Unload module\n");

	unprotect_mem();

	// Restore the sys_call_table
	sys_call_table[__NR_faccessat] = original_faccessat;

	protect_mem();
}


MODULE_AUTHOR("Viktor Horvath");
MODULE_LICENSE("GPL");
MODULE_VERSION("1");
MODULE_DESCRIPTION("Hook open");

