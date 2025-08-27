#include <linux/types.h>
#include <stdarg.h>

#define ALLOC_BASE_PAGE 	0x40200000
#define ALLOC_BASE_KMALLOC 	0x40100000

static char *next_page = (char*)ALLOC_BASE_PAGE;
static char *next_kmalloc = (char*)ALLOC_BASE_KMALLOC;

phys_addr_t page_to_phys(void *a) {
	return a;
}

phys_addr_t virt_to_phys(const volatile void *x) { return x; };

unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order){
	char *old = next_page;
	next_page += (2 << order) * 4096;
	return old;
}

void *kmalloc(size_t sz, unsigned int d) {
	size_t rounded= (sz + (8 - 1)) & ~(8 - 1);
	char *old = next_kmalloc;
	next_kmalloc += rounded;
	memset(old, 0, rounded);
	return old;
}

void *vmalloc(size_t sz) {
	return kmalloc(sz, 0);
}

void *kzalloc(size_t sz, unsigned int a) {
	return kmalloc(sz, 0);
}

void *__kmalloc(size_t sz, unsigned int d) {
	return kmalloc(sz, d);
}

int printk( const char* format, ... ) {
	va_list args;
	va_start(args, format);
	const int ret = vprintf_(format, args);
	va_end(args);
	return ret;
}

struct arm_smccc_res {
	unsigned long a0;
	unsigned long a1;
	unsigned long a2;
	unsigned long a3;
};

void __arm_smccc_smc(unsigned long a0, unsigned long a1,
	unsigned long a2, unsigned long a3, unsigned long a4,
	unsigned long a5, unsigned long a6, unsigned long a7,
	struct arm_smccc_res *res){
		register size_t reg0 __asm__("x0") = a0; \
		register size_t reg1 __asm__("x1") = a1; \
		register size_t reg2 __asm__("x2") = a2; \
		register size_t reg3 __asm__("x3") = a3; \
		register size_t reg4 __asm__("x4") = a4;
		register size_t reg5 __asm__("x5") = a5;
		register size_t reg6 __asm__("x6") = a6;
		register size_t reg7 __asm__("x7") = a7;
		size_t ret;
	
		__asm__ volatile ("smc #0x0\n" : "+r"(reg0),
			"+r"(reg1), "+r"(reg2), "+r"(reg3), "+r"(reg4), "+r"(reg5), "+r"(reg6), "+r"(reg7));
		ret = reg0;
		res->a0 = reg0;
		res->a1 = reg1;
}