#include <linux/types.h>
#include <tee.h>
#include <tee_client_api.h>
#include <tee_drv.h>
#include <soter_smc.h>
#include "tee_private.h"
#include "payload.h"
#include "hexdump.h"

#define KM_COMMAND_MAGIC 'X'
#define FIXED_SHM_SIZE 0x11800  // fixed size for input/output SHM 
#define MSG_SHM_SIZE 0x1000     // for msg_arg

// Globals
static void *ctx;
static struct tee_ioctl_open_session_arg arg = {0};
static struct tee_param params[4] = {{0}};
static void *KM_CAMP1_INPUT;  
static u32 *KM_CMD;         

extern unsigned char begin_payload_bin[];
extern unsigned int begin_payload_bin_len;
static const unsigned char *begin_payload = begin_payload_bin;
// note: do not keep len hardcoded
static const size_t begin_payload_len = 2008;

// SHM buffers
static struct tee_shm *shm_input;
static struct tee_shm *shm_output;
static struct tee_shm *shm_msg;

// Handle from begin_op (for reuse in update/finish)
static uint64_t op_handle;

// c09c9c5daa504b78b0e46eda61556c3a
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

// init SHM (allocate once, reuse)
void km_init_shm() {
    shm_input = isee_shm_alloc(ctx, FIXED_SHM_SIZE, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
    shm_output = isee_shm_alloc(ctx, FIXED_SHM_SIZE, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
    shm_msg = isee_shm_alloc(ctx, MSG_SHM_SIZE, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);

    // Initialize fuzzing input and command
    KM_CAMP1_INPUT = shm_input->kaddr;
    KM_CMD = &(((struct optee_msg_arg *)shm_msg->kaddr)->func);
}

// cleanup 
void km_cleanup() {
    if (shm_input) isee_shm_free(shm_input);
    if (shm_output) isee_shm_free(shm_output);
    if (shm_msg) isee_shm_free(shm_msg);
}

// init context and open session
void km_init_ctx() {
    ctx = soter_fake_ctx("bta_loader");

    uuid_to_octets(arg.uuid, &uuid_ta);
    arg.clnt_login = TEEC_LOGIN_PUBLIC;
    arg.num_params = 4;

    soter_open_session(ctx, &arg, params);
}

// Configure command (0x48)
int km_configure() {
    uint8_t payload_config[] = {
        0xc0, 0xd4, 0x01, 0x00,  // OS Version: 120000 (Android 12)
        0xa2, 0x16, 0x03, 0x00   // Patch Level: 202402 (February 2024)
    };
    size_t payload_size = sizeof(payload_config);

    // reuse shm; clear and copy data
    memset(shm_input->kaddr, 0, FIXED_SHM_SIZE);
    memcpy(shm_input->kaddr, payload_config, payload_size);
    memset(shm_output->kaddr, 0, FIXED_SHM_SIZE);

    struct optee_msg_arg *msg = shm_msg->kaddr;
    memset(msg, 0, MSG_SHM_SIZE);
    msg->cmd = OPTEE_MSG_CMD_INVOKE_COMMAND;
    msg->func = 0x48;
    msg->session = arg.session;
    msg->cancel_id = arg.cancel_id;
    msg->num_params = 4;

    msg->params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INOUT;
    msg->params[0].u.value.a = 1;

    msg->params[1].attr = OPTEE_MSG_ATTR_TYPE_TMEM_INPUT;
    uint64_t pa_input = 0;
    isee_shm_get_pa(shm_input, 0, &pa_input);
    msg->params[1].u.tmem.buf_ptr = pa_input;
    msg->params[1].u.tmem.size = payload_size;
    msg->params[1].u.tmem.shm_ref = (uint64_t)shm_input;

    msg->params[2].attr = OPTEE_MSG_ATTR_TYPE_TMEM_OUTPUT;
    uint64_t pa_output = 0;
    isee_shm_get_pa(shm_output, 0, &pa_output);
    msg->params[2].u.tmem.buf_ptr = pa_output;
    msg->params[2].u.tmem.size = FIXED_SHM_SIZE;
    msg->params[2].u.tmem.shm_ref = (uint64_t)shm_output;

    msg->params[3].attr = OPTEE_MSG_ATTR_TYPE_NONE;

    soter_do_call_with_arg(ctx, msg);

    if (msg->ret != 0) {
        printf("Configure failed: 0x%08x\n", msg->ret);
        return -1;
    }
    return 0;
}

// Generate key (0x00)
int km_generate_key() {
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
    size_t payload_size = sizeof(payload);

    // reuse shm
    memset(shm_input->kaddr, 0, FIXED_SHM_SIZE);
    memcpy(shm_input->kaddr, payload, payload_size);
    memset(shm_output->kaddr, 0, FIXED_SHM_SIZE);

    struct optee_msg_arg *msg = shm_msg->kaddr;
    memset(msg, 0, MSG_SHM_SIZE);
    msg->cmd = OPTEE_MSG_CMD_INVOKE_COMMAND;
    msg->func = 0x00;
    msg->session = arg.session;
    msg->cancel_id = arg.cancel_id;
    msg->num_params = 4;

    msg->params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INOUT;
    msg->params[0].u.value.a = 1;

    msg->params[1].attr = OPTEE_MSG_ATTR_TYPE_TMEM_INPUT;
    uint64_t pa_input = 0;
    isee_shm_get_pa(shm_input, 0, &pa_input);
    msg->params[1].u.tmem.buf_ptr = pa_input;
    msg->params[1].u.tmem.size = payload_size;
    msg->params[1].u.tmem.shm_ref = (uint64_t)shm_input;

    msg->params[2].attr = OPTEE_MSG_ATTR_TYPE_TMEM_OUTPUT;
    uint64_t pa_output = 0;
    isee_shm_get_pa(shm_output, 0, &pa_output);
    msg->params[2].u.tmem.buf_ptr = pa_output;
    msg->params[2].u.tmem.size = FIXED_SHM_SIZE;
    msg->params[2].u.tmem.shm_ref = (uint64_t)shm_output;

    msg->params[3].attr = OPTEE_MSG_ATTR_TYPE_NONE;

    soter_do_call_with_arg(ctx, msg);

    if (msg->ret != 0) {
        printf("Generate key failed: 0x%08x\n", msg->ret);
        return -1;
    }
    return 0;
}

// Begin (0x04), extract handle
int km_begin_op() {
    // 02 00 00 00 --> keymaster_purpose_t; Value 2 = KM_PURPOSE_SIGN
    // f3 05 00 00 --> Length of keyblob in bytes 0x000005f3 = 1523
    // after that: opaque keyblob
    
    size_t payload_size = begin_payload_len;

    // reuse shm
    memset(shm_input->kaddr, 0, FIXED_SHM_SIZE);
    memcpy(shm_input->kaddr, begin_payload, payload_size);
    memset(shm_output->kaddr, 0, FIXED_SHM_SIZE);

    struct optee_msg_arg *msg = shm_msg->kaddr;
    memset(msg, 0, MSG_SHM_SIZE);

    msg->cmd        = OPTEE_MSG_CMD_INVOKE_COMMAND;
    msg->func       = 0x04;
    msg->session    = arg.session;
    msg->cancel_id  = arg.cancel_id;
    msg->num_params = 4;

    msg->params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INOUT;
    msg->params[0].u.value.a = 1;
    msg->params[0].u.value.b = 0;
    msg->params[0].u.value.c = 0;

    uint64_t pa_input = 0, pa_output = 0;
    isee_shm_get_pa(shm_input,  0, &pa_input);
    isee_shm_get_pa(shm_output, 0, &pa_output);

    msg->params[1].attr = OPTEE_MSG_ATTR_TYPE_TMEM_INPUT;
    msg->params[1].u.tmem.buf_ptr = pa_input;
    msg->params[1].u.tmem.size    = payload_size;
    msg->params[1].u.tmem.shm_ref = (uint64_t)shm_input;

    msg->params[2].attr = OPTEE_MSG_ATTR_TYPE_TMEM_OUTPUT;
    msg->params[2].u.tmem.buf_ptr = pa_output;
    msg->params[2].u.tmem.size    = FIXED_SHM_SIZE;
    msg->params[2].u.tmem.shm_ref = (uint64_t)shm_output;

    msg->params[3].attr = OPTEE_MSG_ATTR_TYPE_NONE;

    printf("Begin shm refs: input=%p, pa_input=%zx, output=%p, pa_output=%zx\n",
           (void *)msg->params[1].u.tmem.shm_ref, pa_input,
           (void *)msg->params[2].u.tmem.shm_ref, pa_output);

    soter_do_call_with_arg(ctx, msg);

    printf("Begin response code: %u\n", msg->ret);
    if (msg->ret != 0) {
        printf("Begin failed: 0x%08x\n", msg->ret);
        return -1;
    }

    size_t out_size = msg->params[2].u.tmem.size;
    printf("Begin output size: %zu bytes\n", out_size);
    hexdump(shm_output->kaddr, out_size > 32 ? 32 : out_size);

    if (out_size >= 12) {
        memcpy(&op_handle, (uint8_t *)shm_output->kaddr + 4, 8);
        printf("REAL HANDLE: 0x%016llx\n", op_handle);
    } else {
        printf("Begin output too small\n");
        return -1;
    }

    return 0;
}

// Update (0x08)
int km_update_op() {
    const size_t CORRECT_SIZE = 41;
    uint8_t update_header[41] = {0};
    memcpy(update_header, &op_handle, 8);  // use extracted handle

    uint32_t input_len = 17;
    memcpy(update_header + 8, &input_len, 4);
    //Hello, Keymaster!
    const uint8_t input_data[] = {
        0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x4b,
        0x65, 0x79, 0x6d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x21
    };
    memcpy(update_header + 12, input_data, 17);

    // empty auth set
    uint32_t zero = 0;
    memcpy(update_header + 29, &zero, 4);
    memcpy(update_header + 33, &zero, 4);
    memcpy(update_header + 37, &zero, 4);

    // reuse shm
    memset(shm_input->kaddr, 0, FIXED_SHM_SIZE);
    memcpy(shm_input->kaddr, update_header, CORRECT_SIZE);
    memset(shm_output->kaddr, 0, FIXED_SHM_SIZE);

    // get pa
    uint64_t pa_input = 0, pa_output = 0;
    isee_shm_get_pa(shm_input,  0, &pa_input);
    isee_shm_get_pa(shm_output, 0, &pa_output);

    struct optee_msg_arg *msg = shm_msg->kaddr;
    memset(msg, 0, MSG_SHM_SIZE);
    msg->cmd = OPTEE_MSG_CMD_INVOKE_COMMAND;
    msg->func = 0x08;
    KM_CMD = &msg->func;  // from template
    msg->session = arg.session;
    msg->num_params = 4;

    // Value IN/OUT 
    msg->params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INOUT;
    msg->params[0].u.value.a = 1;
    msg->params[0].u.value.b = 0;
    msg->params[0].u.value.c = 0;

    // Input buffer (41 bytes) 
    msg->params[1].attr = OPTEE_MSG_ATTR_TYPE_TMEM_INPUT;
    msg->params[1].u.tmem.buf_ptr = pa_input;
    msg->params[1].u.tmem.size    = CORRECT_SIZE;
    msg->params[1].u.tmem.shm_ref = (uint64_t)shm_input;

    // Output buffer (big enough) 
    msg->params[2].attr = OPTEE_MSG_ATTR_TYPE_TMEM_OUTPUT;
    msg->params[2].u.tmem.buf_ptr = pa_output;
    msg->params[2].u.tmem.size    = FIXED_SHM_SIZE;
    msg->params[2].u.tmem.shm_ref = (uint64_t)shm_output;

    msg->params[3].attr = OPTEE_MSG_ATTR_TYPE_NONE;

    printf("Update: Sending EXACTLY %zu bytes\n", CORRECT_SIZE);
    soter_do_call_with_arg(ctx, msg);

    if (msg->ret != 0) {
        printf("Update failed: 0x%08x\n", msg->ret);
        return -1;
    }

    size_t out_size = msg->params[2].u.tmem.size;
    printf("Update output size: %zu bytes\n", out_size);
    hexdump(shm_output->kaddr, out_size > 64 ? 64 : out_size);
    return 0;
}

// Finish (0x0C)
int km_finish_op() {
    const size_t FINISH_SIZE = 32;
    uint8_t finish_header[32] = {0};
    memcpy(finish_header, &op_handle, 8);

    uint32_t zero = 0;
    memcpy(finish_header + 8,  &zero, 4);   // input_len = 0
    uint64_t zero64 = 0;
    memcpy(finish_header + 12, &zero64, 8);
    memcpy(finish_header + 20, &zero, 4);
    memcpy(finish_header + 24, &zero, 4);
    memcpy(finish_header + 28, &zero, 4);

    memset(shm_input->kaddr, 0, FIXED_SHM_SIZE);
    memcpy(shm_input->kaddr, finish_header, FINISH_SIZE);
    memset(shm_output->kaddr, 0, FIXED_SHM_SIZE);

    // get pa
    uint64_t pa_input = 0, pa_output = 0;
    isee_shm_get_pa(shm_input,  0, &pa_input);
    isee_shm_get_pa(shm_output, 0, &pa_output);

    struct optee_msg_arg *msg = shm_msg->kaddr;
    memset(msg, 0, MSG_SHM_SIZE);
    msg->cmd = OPTEE_MSG_CMD_INVOKE_COMMAND;
    msg->func = 0x0C;
    msg->session = arg.session;
    msg->num_params = 4;

    msg->params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INOUT;
    msg->params[0].u.value.a = 1;

    msg->params[1].attr = OPTEE_MSG_ATTR_TYPE_TMEM_INPUT;
    msg->params[1].u.tmem.buf_ptr = pa_input;
    msg->params[1].u.tmem.size    = FINISH_SIZE;
    msg->params[1].u.tmem.shm_ref = (uint64_t)shm_input;

    msg->params[2].attr = OPTEE_MSG_ATTR_TYPE_TMEM_OUTPUT;
    msg->params[2].u.tmem.buf_ptr = pa_output;
    msg->params[2].u.tmem.size    = FIXED_SHM_SIZE;
    msg->params[2].u.tmem.shm_ref = (uint64_t)shm_output;

    msg->params[3].attr = OPTEE_MSG_ATTR_TYPE_NONE;

    printf("Finish: Sending EXACTLY %zu bytes\n", FINISH_SIZE);
    soter_do_call_with_arg(ctx, msg);

    if (msg->ret != 0) {
        printf("Finish failed: 0x%08x\n", msg->ret);
        return -1;
    }

    size_t out_size = msg->params[2].u.tmem.size;
    printf("Finish output size: %zu bytes\n", out_size);
    hexdump(shm_output->kaddr, out_size > 64 ? 64 : out_size);
    return 0;
}

// Warmup: configure -> generate -> begin -> update -> finish
void km_warmup() {
    if (km_configure() != 0) return;
    if (km_generate_key() != 0) return;
    if (km_begin_op() != 0) return;
    if (km_update_op() != 0) return;
    if (km_finish_op() != 0) return;
    printf("Warmup completed successfully\n");
}

// wip
int FuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 5) return 0;   // need command (4) + 1 byte payload

    // Fuzzer controls command
    *KM_CMD = *(const uint32_t *)data;           // e.g. 0x04, 0x08, etc.

    // fuzzer controls input payload
    size_t payload_sz = size - 4;
    if (payload_sz > FIXED_SHM_SIZE) payload_sz = FIXED_SHM_SIZE;
    memcpy(KM_CAMP1_INPUT, data + 4, payload_sz);
    
    // SMC + endfuzz()
    km_camp_1_startfuzz();  

    return 0;
}

void km_camp_1_startfuzz() {
    struct optee_msg_arg *msg = shm_msg->kaddr;
    memset(msg, 0, MSG_SHM_SIZE);

    msg->cmd         = OPTEE_MSG_CMD_INVOKE_COMMAND;
    msg->session     = arg.session;
    msg->cancel_id   = arg.cancel_id;
    msg->num_params  = 4;

    msg->params[0].attr      = OPTEE_MSG_ATTR_TYPE_VALUE_INOUT;
    msg->params[0].u.value.a = 1;
    msg->params[0].u.value.b = 0;
    msg->params[0].u.value.c = 0;

    uint64_t pa_input = 0, pa_output = 0;
    isee_shm_get_pa(shm_input,  0, &pa_input);
    isee_shm_get_pa(shm_output, 0, &pa_output);

    msg->params[1].attr           = OPTEE_MSG_ATTR_TYPE_TMEM_INPUT;
    msg->params[1].u.tmem.buf_ptr = pa_input;
    msg->params[1].u.tmem.size    = FIXED_SHM_SIZE;   // TA reads real size from payload
    msg->params[1].u.tmem.shm_ref = (uint64_t)shm_input;

    msg->params[2].attr           = OPTEE_MSG_ATTR_TYPE_TMEM_OUTPUT;
    msg->params[2].u.tmem.buf_ptr = pa_output;
    msg->params[2].u.tmem.size    = FIXED_SHM_SIZE;
    msg->params[2].u.tmem.shm_ref = (uint64_t)shm_output;

    msg->params[3].attr = OPTEE_MSG_ATTR_TYPE_NONE;

    // Fuzzer already set:
    //   *KM_CMD       → msg->func
    //   KM_CAMP1_INPUT → input buffer

    // → SMC
    soter_do_call_with_arg(ctx, msg);

    printf("KM startfuzz done\n");

    km_camp_1_endfuzz();  // bp
}

void km_camp_1_endfuzz() {
    // second breakpoint
    printf("campaign-1 END :)\n");
}


int test_km() {
    km_init_ctx();
    km_init_shm();

    km_warmup();

    printf("Full flow OK!\n");
    km_cleanup();
    return 0;
}