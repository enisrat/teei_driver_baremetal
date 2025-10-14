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
}

void kmtest_optee() {
    void *ctx = soter_fake_ctx("bta_loader");
    struct tee_ioctl_open_session_arg arg = {0};
    struct tee_param params[4] = {{0}};
    
    uuid_to_octets(arg.uuid, &uuid_ta);
    arg.clnt_login = TEEC_LOGIN_PUBLIC;
    arg.num_params = 4;

    soter_open_session(ctx, &arg, params);

    // Configure command (0x48)
    struct tee_shm *shm_input_config = isee_shm_alloc(ctx, 0x5000, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
    
    //uint8_t payload_config[] = {0x03,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}; // PASSED

    uint8_t payload_config[] = 
    {0xc0, 0xd4, 0x01, 0x00,  // OS Version: 120000 (Android 12)
    0xa2, 0x16, 0x03, 0x00  // Patch Level: 202402 (February 2024)
    }; // PASSED

    memcpy(shm_input_config->kaddr, payload_config, sizeof(payload_config));

    struct tee_shm *shm_output_config = isee_shm_alloc(ctx, 0x5000, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
    struct tee_shm *shm_msg_config = isee_shm_alloc(ctx, 0x1000, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
    struct optee_msg_arg *msg_config = shm_msg_config->kaddr;

    msg_config->cmd = OPTEE_MSG_CMD_INVOKE_COMMAND;
    msg_config->func = 0x48;
    msg_config->session = arg.session;
    msg_config->cancel_id = arg.cancel_id;
    msg_config->num_params = 4;

    msg_config->params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INOUT;
    msg_config->params[0].u.value.a = 1;
    msg_config->params[0].u.value.b = 0;
    msg_config->params[0].u.value.c = 0;

    msg_config->params[1].attr = OPTEE_MSG_ATTR_TYPE_TMEM_INPUT;
    uint64_t pa_input_config = 0;
    isee_shm_get_pa(shm_input_config, 0, &pa_input_config);
    msg_config->params[1].u.tmem.buf_ptr = pa_input_config;
    msg_config->params[1].u.tmem.size = sizeof(payload_config);
    msg_config->params[1].u.tmem.shm_ref = (uint64_t)shm_input_config;

    msg_config->params[2].attr = OPTEE_MSG_ATTR_TYPE_TMEM_OUTPUT;
    uint64_t pa_output_config = 0;
    isee_shm_get_pa(shm_output_config, 0, &pa_output_config);
    msg_config->params[2].u.tmem.buf_ptr = pa_output_config;
    msg_config->params[2].u.tmem.size = 0x5000;
    msg_config->params[2].u.tmem.shm_ref = (uint64_t)shm_output_config;

    msg_config->params[3].attr = OPTEE_MSG_ATTR_TYPE_NONE;
    memset(&msg_config->params[3].u, 0, sizeof(msg_config->params[3].u));

    soter_do_call_with_arg(ctx, msg_config);

    // Original command (0xC0)
    struct tee_shm *shm_input = isee_shm_alloc(ctx, 0x5000, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);

    uint8_t payload[] = {
    0x00, 0x00, 0x00, 0x00, // Command ID: generateKey (0x00)
    0x07, 0x00, 0x00, 0x00, // Tag count
    0x39, 0x00, 0x00, 0x00, // Algorithm (RSA=0x39)
    0x03, 0x00, 0x00, 0x30, // Key size (3072 bits = 0x3000)
    0x00, 0x01, 0x00, 0x00, // Purpose (encrypt=0x1)
    0x02, 0x00, 0x00, 0x10, // Digest (SHA256=0x2)
    0x03, 0x00, 0x00, 0x00, // Padding (none=0x0)
    0x0a, 0x00, 0x00, 0x10, // Block mode (ECB=0xa)
    0x01, 0x00, 0x00, 0x00, // Caller nonce
    0x01, 0x00, 0x00, 0x20, // Min MAC length
    0x02, 0x00, 0x00, 0x00, // EC curve (unused for RSA)
    0x05, 0x00, 0x00, 0x20, // RSA public exponent (65537=0x20005)
    0x04, 0x00, 0x00, 0x00, // Include attestation
    0xf7, 0x01, 0x00, 0x70, // Attestation challenge (32-byte nonce)
    0x01, 0xbd, 0x02, 0x00, // Unique ID
    0x60, 0x50, 0xc5, 0x81, // Nonce data (example)
    0x15, 0x8e, 0x01, 0x00, // More nonce data
    0x00, 0x00, 0x00, 0x00  // Padding/alignment
    };
    
    memcpy(shm_input->kaddr, payload, sizeof(payload));

    struct tee_shm *shm_output = isee_shm_alloc(ctx, 0x5000, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
    struct tee_shm *shm_msg = isee_shm_alloc(ctx, 0x1000, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
    struct optee_msg_arg *msg = shm_msg->kaddr;

    msg->cmd = OPTEE_MSG_CMD_INVOKE_COMMAND;
    msg->func = 0x00;
    msg->session = arg.session;
    msg->cancel_id = arg.cancel_id;
    msg->num_params = 4;

    msg->params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INOUT;
    msg->params[0].u.value.a = 1;
    msg->params[0].u.value.b = 0;
    msg->params[0].u.value.c = 0;

    msg->params[1].attr = OPTEE_MSG_ATTR_TYPE_TMEM_INPUT;
    uint64_t pa_input = 0;
    isee_shm_get_pa(shm_input, 0, &pa_input);
    msg->params[1].u.tmem.buf_ptr = pa_input;
    msg->params[1].u.tmem.size = sizeof(payload);
    msg->params[1].u.tmem.shm_ref = (uint64_t)shm_input;

    msg->params[2].attr = OPTEE_MSG_ATTR_TYPE_TMEM_OUTPUT;
    uint64_t pa_output = 0;
    isee_shm_get_pa(shm_output, 0, &pa_output);
    msg->params[2].u.tmem.buf_ptr = pa_output;
    msg->params[2].u.tmem.size = 0x5000;
    msg->params[2].u.tmem.shm_ref = (uint64_t)shm_output;

    msg->params[3].attr = OPTEE_MSG_ATTR_TYPE_NONE;
    memset(&msg->params[3].u, 0, sizeof(msg->params[3].u));

    soter_do_call_with_arg(ctx, msg);
}