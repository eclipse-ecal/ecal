/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.9.1 */

#ifndef PB_ECAL_PB_MONITORING_NPB_H_INCLUDED
#define PB_ECAL_PB_MONITORING_NPB_H_INCLUDED
#include <pb.h>
#include "ecal/core/pb/host.npb.h"
#include "ecal/core/pb/process.npb.h"
#include "ecal/core/pb/service.npb.h"
#include "ecal/core/pb/topic.npb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _eCAL_pb_Monitoring {
    pb_callback_t hosts; /* hosts */
    pb_callback_t processes; /* processes */
    pb_callback_t services; /* services */
    pb_callback_t topics; /* topics */
    pb_callback_t clients; /* clients */
} eCAL_pb_Monitoring;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define eCAL_pb_Monitoring_init_default          {{{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}}
#define eCAL_pb_Monitoring_init_zero             {{{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define eCAL_pb_Monitoring_hosts_tag             1
#define eCAL_pb_Monitoring_processes_tag         2
#define eCAL_pb_Monitoring_services_tag          3
#define eCAL_pb_Monitoring_topics_tag            4
#define eCAL_pb_Monitoring_clients_tag           5

/* Struct field encoding specification for nanopb */
#define eCAL_pb_Monitoring_FIELDLIST(X, a) \
X(a, CALLBACK, REPEATED, MESSAGE,  hosts,             1) \
X(a, CALLBACK, REPEATED, MESSAGE,  processes,         2) \
X(a, CALLBACK, REPEATED, MESSAGE,  services,          3) \
X(a, CALLBACK, REPEATED, MESSAGE,  topics,            4) \
X(a, CALLBACK, REPEATED, MESSAGE,  clients,           5)
#define eCAL_pb_Monitoring_CALLBACK pb_default_field_callback
#define eCAL_pb_Monitoring_DEFAULT NULL
#define eCAL_pb_Monitoring_hosts_MSGTYPE eCAL_pb_Host
#define eCAL_pb_Monitoring_processes_MSGTYPE eCAL_pb_Process
#define eCAL_pb_Monitoring_services_MSGTYPE eCAL_pb_Service
#define eCAL_pb_Monitoring_topics_MSGTYPE eCAL_pb_Topic
#define eCAL_pb_Monitoring_clients_MSGTYPE eCAL_pb_Client

extern const pb_msgdesc_t eCAL_pb_Monitoring_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define eCAL_pb_Monitoring_fields &eCAL_pb_Monitoring_msg

/* Maximum encoded size of messages (where known) */
/* eCAL_pb_Monitoring_size depends on runtime parameters */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif