#include <linux/types.h>
#include <tee.h>
#include <tee_client_api.h>
#include <tee_drv.h>
#include <soter_smc.h>
#include "tee_private.h"

#define KM_COMMAND_MAGIC 'X'

//93feffccd8ca11e796c7c7a21acb4932.ta
static struct TEEC_UUID uuid_ta = { 0x93feffcc, 0xd8ca, 0x11e7,
	{ 0x96, 0xc7, 0xc7, 0xa2, 0x1a, 0xcb, 0x49, 0x32 } };

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


void test_my_ta_c1() {
    void *ctx = soter_fake_ctx("bta_loader");

    struct tee_ioctl_open_session_arg arg = {0};
    struct tee_param params[4] = {{0}};  

    uuid_to_octets(arg.uuid, &uuid_ta);
    arg.clnt_login = TEEC_LOGIN_PUBLIC;
    arg.num_params = 0; 

    soter_open_session(ctx, &arg, params);  

   
    struct tee_shm *shm = isee_shm_alloc(ctx, 0x11800, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
    memset(shm->kaddr, 0, 0x11800);  

    struct tee_shm *shm_msg = isee_shm_alloc(ctx, 0x1000, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
    struct optee_msg_arg *msg = shm_msg->kaddr;

    msg->cmd = OPTEE_MSG_CMD_INVOKE_COMMAND;
    msg->func = 0xC0;  // command ID
    msg->session = arg.session;
    msg->cancel_id = arg.cancel_id;
    msg->num_params = 2;  

    msg->params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_OUTPUT;
    msg->params[0].u.value.a = 0;  
    msg->params[0].u.value.b = 0;
    msg->params[0].u.value.c = 0;

    msg->params[1].attr = OPTEE_MSG_ATTR_TYPE_RMEM_OUTPUT;  
    msg->params[1].u.rmem.offs = 0;
    msg->params[1].u.rmem.size = 0x11800;
    msg->params[1].u.rmem.shm_ref = shm->id;  

    printf("param1 shm_ref: %llx\n", msg->params[1].u.rmem.shm_ref); 

    soter_do_call_with_arg(ctx, msg);
    printf("OPTEE_MSG_CMD_INVOKE_COMMAND RET: %d %d\n", msg->ret, msg->ret_origin);

    // if needed, add input param example:
    msg->num_params = 3;
    msg->params[2].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
    msg->params[2].u.value.a = 6;

    // cleanup: isee_shm_free(shm); etc.
}
