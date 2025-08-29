#include <linux/types.h>
#include <tee.h>
#include <tee_client_api.h>
#include <tee_drv.h>


struct tee_device;

/**
 * struct tee_shm - shared memory object
 * @teedev:	device used to allocate the object
 * @ctx:	context using the object, if NULL the context is gone
 * @link	link element
 * @paddr:	physical address of the shared memory
 * @kaddr:	virtual address of the shared memory
 * @size:	size of shared memory
 * @dmabuf:	dmabuf used to for exporting to user space
 * @flags:	defined by TEE_SHM_* in tee_drv.h
 * @id:		unique id of a shared memory object on this device
 */
struct tee_shm {
	struct tee_device *teedev;
	struct tee_context *ctx;
	struct list_head link;
	phys_addr_t paddr;
	void *kaddr;
	size_t size;
	char *dmabuf;
	u32 flags;
	int id;
};