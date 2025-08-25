#include <linux/types.h>
#include <page.h>
#include <stdarg.h>

#define ALLOC_BASE_PAGE=0x4C800000
#define ALLOC_BASE_KMALLOC=0x4CC00000

static char *next_page = (char*)ALLOC_BASE_PAGE;
static char *next_kmalloc = (char*)ALLOC_BASE_KMALLOC;

phys_addr_t page_to_phys(void *a) {
	return a;
}

void *virt_to_phys(void * a){
	return a;
}

unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order){
	char *old = next_page;
	next_page += (2 << order) * PAGE_SIZE;
	return old;
}

void *kmalloc(size_t sz, unsigned int d) {
	size_t rounded= (sz + (8 - 1)) & ~(8 - 1);
	char *old = next_kmalloc;
	next_kmalloc += rounded;
	return old;
}

void *vmalloc(size_t sz) {
	return kmalloc(sz, 0);
}

void *kzalloc(size_t sz, unsigned int a) {
	return kmalloc(sz, 0);
}

int printk( const char* format, ... ) {
	va_list args;
	va_start(args, format);
	const int ret = vprintf_(format, args);
	va_end(args);
	return ret;
}

