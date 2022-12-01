#pragma once

#include <stdint.h>

#include "Tpm.h"
#include "TpmTypes.h"
#include "ExecCommand_fp.h"
#include "wolfssl/openssl/bn.h"
#include "wolfssl/openssl/ssl.h"
#include "wolfssl/wolfcrypt/rsa.h"
#include "wolfssl/wolfcrypt/asn_public.h"

/* for attestation request */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define MAX_AUTHTAG_LEN         32
#define AAD_LEN			48
#define MSG_HDR_VER  		1

struct Regs {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
};


/* See SNP spec SNP_GUEST_REQUEST section for the structure */
enum msg_type {
        SNP_MSG_TYPE_INVALID = 0,
        SNP_MSG_CPUID_REQ,
        SNP_MSG_CPUID_RSP,
        SNP_MSG_KEY_REQ,
        SNP_MSG_KEY_RSP,
        SNP_MSG_REPORT_REQ,
        SNP_MSG_REPORT_RSP,
        SNP_MSG_EXPORT_REQ,
        SNP_MSG_EXPORT_RSP,
        SNP_MSG_IMPORT_REQ,
        SNP_MSG_IMPORT_RSP,
        SNP_MSG_ABSORB_REQ,
        SNP_MSG_ABSORB_RSP,
        SNP_MSG_VMRK_REQ,
        SNP_MSG_VMRK_RSP,

        SNP_MSG_TYPE_MAX
};

enum aead_algo {
        SNP_AEAD_INVALID,
        SNP_AEAD_AES_256_GCM,
};

struct snp_guest_msg_hdr {
        u8 authtag[MAX_AUTHTAG_LEN];
        u64 msg_seqno;
        u8 rsvd1[8];
        u8 algo;
        u8 hdr_version;
        u16 hdr_sz;
        u8 msg_type;
        u8 msg_version;
        u16 msg_sz;
        u32 rsvd2;
        u8 msg_vmpck;
        u8 rsvd3[35];
} __attribute__((packed));

struct snp_guest_msg {
        struct snp_guest_msg_hdr hdr;
        u8 payload[4000];
} __attribute__((aligned(4096)));

struct snp_report_req {
        /* user data that should be included in the report */
        u8 user_data[64];

        /* The vmpl level to be included in the report */
        u32 vmpl;

        /* Must be zero filled */
        u8 rsvd[28];
};

struct snp_report_resp {
        /* response data, see SEV-SNP spec for the format */
        u8 data[4000];
};

union tcb_version {
	struct {
		uint8_t boot_loader;
		uint8_t tee;
		uint8_t reserved[4];
		uint8_t snp;
		uint8_t microcode;
	};
	uint64_t raw;
};

struct signature {
	uint8_t r[72];
	uint8_t s[72];
	uint8_t reserved[512-144];
} __attribute__((packed));

struct attestation_report {
	uint32_t          version;			/* 0x000 */
	uint32_t          guest_svn;			/* 0x004 */
	uint64_t          policy;			/* 0x008 */
	uint8_t           family_id[16];		/* 0x010 */
	uint8_t           image_id[16];			/* 0x020 */
	uint32_t          vmpl;				/* 0x030 */
	uint32_t          signature_algo;		/* 0x034 */
	union tcb_version platform_version;		/* 0x038 */
	uint64_t          platform_info;		/* 0x040 */
	uint32_t          flags;			/* 0x048 */
	uint32_t          reserved0;			/* 0x04C */
	uint8_t           report_data[64];		/* 0x050 */
	uint8_t           measurement[48];		/* 0x090 */
	uint8_t           host_data[32];		/* 0x0C0 */
	uint8_t           id_key_digest[48];		/* 0x0E0 */
	uint8_t           author_key_digest[48];	/* 0x110 */
	uint8_t           report_id[32];		/* 0x140 */
	uint8_t           report_id_ma[32];		/* 0x160 */
	union tcb_version reported_tcb;			/* 0x180 */
	uint8_t           reserved1[24];		/* 0x188 */
	uint8_t           chip_id[64];			/* 0x1A0 */
	uint8_t           reserved2[192];		/* 0x1E0 */
	struct signature  signature;			/* 0x2A0 */
} __attribute__((packed));


struct msg_report_resp {
	uint32_t status;
	uint32_t report_size;
	uint8_t  reserved[24];
	struct attestation_report report;
} __attribute__((aligned(4096)));
