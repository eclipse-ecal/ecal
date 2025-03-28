/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.9.1 */

#ifndef PB_ECAL_PB_LOGGING_NPB_H_INCLUDED
#define PB_ECAL_PB_LOGGING_NPB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _eCAL_pb_LogMessage {
    int64_t time; /* time */
    pb_callback_t host_name; /* host name */
    int32_t process_id; /* process id */
    pb_callback_t process_name; /* process name */
    pb_callback_t unit_name; /* unit name */
    int32_t level; /* message level */
    pb_callback_t content; /* message content */
} eCAL_pb_LogMessage;

typedef struct _eCAL_pb_LogMessageList {
    pb_callback_t log_messages; /* log messages */
} eCAL_pb_LogMessageList;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define eCAL_pb_LogMessage_init_default          {0, {{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}, 0, {{NULL}, NULL}}
#define eCAL_pb_LogMessageList_init_default      {{{NULL}, NULL}}
#define eCAL_pb_LogMessage_init_zero             {0, {{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}, 0, {{NULL}, NULL}}
#define eCAL_pb_LogMessageList_init_zero         {{{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define eCAL_pb_LogMessage_time_tag              1
#define eCAL_pb_LogMessage_host_name_tag         2
#define eCAL_pb_LogMessage_process_id_tag        3
#define eCAL_pb_LogMessage_process_name_tag      4
#define eCAL_pb_LogMessage_unit_name_tag         5
#define eCAL_pb_LogMessage_level_tag             6
#define eCAL_pb_LogMessage_content_tag           7
#define eCAL_pb_LogMessageList_log_messages_tag  1

/* Struct field encoding specification for nanopb */
#define eCAL_pb_LogMessage_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT64,    time,              1) \
X(a, CALLBACK, SINGULAR, STRING,   host_name,         2) \
X(a, STATIC,   SINGULAR, INT32,    process_id,        3) \
X(a, CALLBACK, SINGULAR, STRING,   process_name,      4) \
X(a, CALLBACK, SINGULAR, STRING,   unit_name,         5) \
X(a, STATIC,   SINGULAR, INT32,    level,             6) \
X(a, CALLBACK, SINGULAR, STRING,   content,           7)
#define eCAL_pb_LogMessage_CALLBACK pb_default_field_callback
#define eCAL_pb_LogMessage_DEFAULT NULL

#define eCAL_pb_LogMessageList_FIELDLIST(X, a) \
X(a, CALLBACK, REPEATED, MESSAGE,  log_messages,      1)
#define eCAL_pb_LogMessageList_CALLBACK pb_default_field_callback
#define eCAL_pb_LogMessageList_DEFAULT NULL
#define eCAL_pb_LogMessageList_log_messages_MSGTYPE eCAL_pb_LogMessage

extern const pb_msgdesc_t eCAL_pb_LogMessage_msg;
extern const pb_msgdesc_t eCAL_pb_LogMessageList_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define eCAL_pb_LogMessage_fields &eCAL_pb_LogMessage_msg
#define eCAL_pb_LogMessageList_fields &eCAL_pb_LogMessageList_msg

/* Maximum encoded size of messages (where known) */
/* eCAL_pb_LogMessage_size depends on runtime parameters */
/* eCAL_pb_LogMessageList_size depends on runtime parameters */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
