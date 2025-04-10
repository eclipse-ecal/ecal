/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.9.1 */

#ifndef PB_ECAL_PB_HOST_NPB_H_INCLUDED
#define PB_ECAL_PB_HOST_NPB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _eCAL_pb_OSInfo {
    pb_callback_t name; /* name */
} eCAL_pb_OSInfo;

typedef struct _eCAL_pb_Host {
    pb_callback_t name; /* host name */
    bool has_os;
    eCAL_pb_OSInfo os; /* operating system details */
} eCAL_pb_Host;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define eCAL_pb_OSInfo_init_default              {{{NULL}, NULL}}
#define eCAL_pb_Host_init_default                {{{NULL}, NULL}, false, eCAL_pb_OSInfo_init_default}
#define eCAL_pb_OSInfo_init_zero                 {{{NULL}, NULL}}
#define eCAL_pb_Host_init_zero                   {{{NULL}, NULL}, false, eCAL_pb_OSInfo_init_zero}

/* Field tags (for use in manual encoding/decoding) */
#define eCAL_pb_OSInfo_name_tag                  1
#define eCAL_pb_Host_name_tag                    1
#define eCAL_pb_Host_os_tag                      2

/* Struct field encoding specification for nanopb */
#define eCAL_pb_OSInfo_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, STRING,   name,              1)
#define eCAL_pb_OSInfo_CALLBACK pb_default_field_callback
#define eCAL_pb_OSInfo_DEFAULT NULL

#define eCAL_pb_Host_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, STRING,   name,              1) \
X(a, STATIC,   OPTIONAL, MESSAGE,  os,                2)
#define eCAL_pb_Host_CALLBACK pb_default_field_callback
#define eCAL_pb_Host_DEFAULT NULL
#define eCAL_pb_Host_os_MSGTYPE eCAL_pb_OSInfo

extern const pb_msgdesc_t eCAL_pb_OSInfo_msg;
extern const pb_msgdesc_t eCAL_pb_Host_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define eCAL_pb_OSInfo_fields &eCAL_pb_OSInfo_msg
#define eCAL_pb_Host_fields &eCAL_pb_Host_msg

/* Maximum encoded size of messages (where known) */
/* eCAL_pb_OSInfo_size depends on runtime parameters */
/* eCAL_pb_Host_size depends on runtime parameters */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
