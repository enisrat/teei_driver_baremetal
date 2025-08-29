#include <linux/types.h>
#include <tee.h>
#include <tee_client_api.h>
#include <tee_drv.h>
#include <soter_smc.h>
#include "tee_private.h"

#define KM_COMMAND_MAGIC 'X'

//c09c9c5daa504b78b0e46eda61556c3a
static struct TEEC_UUID uuid_ta = { 0xc09c9c5d, 0xaa50, 0x4b78,
	{ 0xb0, 0xe4, 0x6e, 0xda, 0x61, 0x55, 0x6c, 0x3a } };

//8b22aba81ef0ccbfd9f5f4b634127e15
//static struct TEEC_UUID uuid_ta2 = { 0x8b22aba8, 0x1ef0, 0xccbf,
//	{ 0xd9, 0xf5, 0xf4, 0xb6, 0x34, 0x12, 0x7e, 0x15 } };

static void uuid_to_octets(uint8_t d[TEE_IOCTL_UUID_LEN],
		const struct TEEC_UUID *s)
{
	d[0] = s->timeLow >> 24;
	d[1] = s->timeLow >> 16;
	d[2] = s->timeLow >> 8;
	d[3] = s->timeLow;
	d[4] = s->timeMid >> 8;
	d[5] = s->timeMid;
	d[6] = s->timeHiAndVersion >> 8;
	d[7] = s->timeHiAndVersion;
	memcpy(d + 8, s->clockSeqAndNode, sizeof(s->clockSeqAndNode));
}

void kmtest() {

	void *ctx = soter_fake_ctx("bta_loader");
	
	struct tee_ioctl_open_session_arg arg = {0};
	struct tee_param params[4] = {{0}};

	uuid_to_octets(arg.uuid, &uuid_ta);
	arg.clnt_login = TEEC_LOGIN_PUBLIC;
	arg.num_params = 4;

	soter_open_session(ctx, &arg, params);

	struct tee_shm *shm = isee_shm_alloc(ctx, 0x11800, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);

	memcpy(shm->kaddr, "ZZZZZZZZ", 8);

	struct tee_ioctl_invoke_arg argi = {0};
	argi.func = 0x0;
	argi.session = arg.session;
	argi.num_params = 4;
	params[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INOUT;
	params[2].u.memref.shm = shm;
	params[2].u.memref.size = 0x11800;
	params[2].u.memref.shm_offs = 0;
	params[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	params[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	params[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	soter_invoke_func(ctx, &argi, params);

	printf("DONE\n");
}

void kmtest_optee() {

	void *ctx = soter_fake_ctx("bta_loader");
	
	struct tee_ioctl_open_session_arg arg = {0};
	struct tee_param params[4] = {{0}};

	uuid_to_octets(arg.uuid, &uuid_ta);
	arg.clnt_login = TEEC_LOGIN_PUBLIC;
	arg.num_params = 4;

	soter_open_session(ctx, &arg, params);

	struct tee_shm *shm = isee_shm_alloc(ctx, 0x11800, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
	memcpy(shm->kaddr, "ZZZZZZZZ", 8);

	struct tee_shm *shm_msg = isee_shm_alloc(ctx, 0x1000, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
	struct optee_msg_arg *msg = shm_msg->kaddr;

	msg->cmd = OPTEE_MSG_CMD_INVOKE_COMMAND;
	msg->func = 0xC0;
	msg->session = arg.session;
	msg->cancel_id = arg.cancel_id;
	msg->num_params = 4;

	msg->params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_OUTPUT;
	msg->params[1].attr = OPTEE_MSG_ATTR_TYPE_RMEM_OUTPUT;
	msg->params[1].u.rmem.offs = 0;//shm->kaddr - isee_shm_base();
	msg->params[1].u.rmem.size = 0x11800;
	msg->params[1].u.rmem.shm_ref = shm->kaddr;

	printf("param1 addr: %llx\n", msg->params[1].u.rmem.shm_ref);

	soter_do_call_with_arg(ctx, msg);
	printf("OPTEE_MSG_CMD_INVOKE_COMMAND RET: %d %d\n", msg->ret, msg->ret_origin);

}