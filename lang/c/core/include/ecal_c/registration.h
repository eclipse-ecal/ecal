/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

/**
 * @file   registration.h
 * @brief  eCAL registration c interface
**/

#ifndef ecal_c_registration_h_included
#define ecal_c_registration_h_included

#include <ecal_c/export.h>
#include <ecal_c/pubsub/types.h>
#include <ecal_c/types.h>

#include <stddef.h>

struct eCAL_Registration_SServiceMethod
{
  const char* service_name;
  const char* method_name;
};

typedef size_t eCAL_Registration_CallbackToken;

enum eCAL_Registration_RegistrationEventType
{
  eCAL_Registration_RegistrationEventType_new_entity,     //!< Represents a new entity registration
  eCAL_Registration_RegistrationEventType_deleted_entity  //!< Represents a deletion of an entity
};

typedef void (*eCAL_Registration_TopicEventCallbackT)(const struct eCAL_STopicId*, enum eCAL_Registration_RegistrationEventType);

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Get complete snapshot of all known publishers.
   *
   * @param[out] topic_ids_        Returned array of topics ids. Must point to NULL and needs to be released by eCAL_Free().
   * @param[out] topic_ids_length_ Returned length of the array. Must point to zero.
   * 
   * @return Zero if succeeded, non-zero otherwise
  **/
  ECALC_API int eCAL_Registration_GetPublisherIDs(struct eCAL_STopicId** topic_ids_, size_t* topic_ids_length_);

  /**
   * @brief Get data type information with quality for specific publisher.
   *
   * @param topic_id_        Publisher topic id.
   * @param[out] topic_info_ Returned data type information. Must point to NULL and needs to be released by eCAL_Free().
   *
   * @return Zero if succeeded, non-zero otherwise
  **/
  ECALC_API int eCAL_Registration_GetPublisherInfo(const struct eCAL_STopicId* topic_id_, struct eCAL_SDataTypeInformation** topic_info_);

  /**
   * @brief Get complete snapshot of all known subscribers.
   *
   * @param[out] topic_ids_        Returned array of topic ids. Must point to NULL and needs to be released by eCAL_Free().
   * @param[out] topic_ids_length_ Returned length of the array. Must point to zero.
   *
   * @return Zero if succeeded, non-zero otherwise
  **/
  ECALC_API int eCAL_Registration_GetSubscriberIDs(struct eCAL_STopicId** topic_ids_, size_t* topic_ids_length_);

  /**
   * @brief Get data type information with quality for specific subscriber.
   *
   * @param topic_id_        Topic id.
   * @param[out] topic_info_ Returned data type information. Must point to NULL and needs to be released by eCAL_Free().
   *
   * @return Zero if succeeded, non-zero otherwise
  **/
  ECALC_API int eCAL_Registration_GetSubscriberInfo(const struct eCAL_STopicId* topic_id_, struct eCAL_SDataTypeInformation** topic_info_);

  /**
   * @brief Get complete snapshot of all known servers.
   *
   * @param[out] service_ids_      Returned array of service ids. Must point to NULL and needs to be released by eCAL_Free().
   * @param[out] topic_ids_length_ Returned length of the array. Must point to zero.
   *
   * @return Zero if succeeded, non-zero otherwise
  **/
  ECALC_API int eCAL_Registration_GetServerIDs(struct eCAL_SServiceId** service_ids_, size_t* service_ids_length_);

  /**
   * @brief Get service method information with quality for specific server.
   *
   * @param service_id_                       Service id.
   * @param[out] service_method_info_         Returned array of service method information. Must point to NULL and needs to be released by eCAL_Free().
   * @param[out] service_method_info_length_  Returned length of the array. Must point to zero.
   * 
   * @return Zero if succeeded, non-zero otherwise
  **/
  ECALC_API int eCAL_Registration_GetServerInfo(const struct eCAL_SServiceId* service_id_, struct eCAL_SServiceMethodInformation** service_method_info_, size_t* service_method_info_length_);

  /**
   * @brief Get complete snapshot of all known clients.
   *
   * @param[out] service_ids_      Returned array of service ids. Must point to NULL and needs to be released by eCAL_Free().
   * @param[out] topic_ids_length_ Returned length of the array. Must point to zero.
   *
   * @return Zero if succeeded, non-zero otherwise
  **/
  ECALC_API int eCAL_Registration_GetClientIDs(struct eCAL_SServiceId** service_ids_, size_t* service_ids_length_);

  /**
   * @brief Get service method information with quality for specific client.
   *
   * @param service_id_                       Service id.
   * @param[out] service_method_info_         Returned array of service method information. Must point to NULL and needs to be released by eCAL_Free().
   * @param[out] service_method_info_length_  Returned length of the array. Must point to zero.
   *
   * @return Zero if succeeded, non-zero otherwise
  **/
  ECALC_API int eCAL_Registration_GetClientInfo(const struct eCAL_SServiceId* service_id_, struct eCAL_SServiceMethodInformation** service_method_info_, size_t* service_method_info_length_);

  /**
   * @brief Get all names of topics that are being published.
   *        This is a convenience function. 
   *        It calls GetPublisherIDs() and filters by name
   * 
   * @param[out] topic_names_         Returned pointer array of null-terminated strings. Must point to NULL and needs to be released by eCAL_Free().
   * @param[out] topic_names_length_  Returned length of the array. Must point to zero.
   *
   * @return Zero if succeeded, non-zero otherwise
  **/
  ECALC_API int eCAL_Registration_GetPublishedTopicNames(char*** topic_names_, size_t* topic_names_length_);

  /**
   * @brief Get all names of topics that are being subscribed.
   *        This is a convenience function.
   *        It calls GetSubscriberIDs() and filters by name
   *
   * @param[out] topic_names_         Returned pointer array of null-terminated strings. Must point to NULL and needs to be released by eCAL_Free().
   * @param[out] topic_names_length_  Returned length of the array. Must point to zero.
   *
   * @return Zero if succeeded, non-zero otherwise
  **/
  ECALC_API int eCAL_Registration_GetSubscribedTopicNames(char*** topic_names_, size_t* topic_names_length_);

  /**
   * @brief Get the pairs of service name / method name of all eCAL Servers.
   *
   * @param[out] server_method_names_        Returned array of server method names. Must point to NULL and needs to be released by eCAL_Free().
   * @param[out] server_method_names_length_ Returned length of the array. Must point to zero.
   *
   * @return Zero if succeeded, non-zero otherwise
  **/
  ECALC_API int eCAL_Registration_GetServerMethodNames(struct eCAL_Registration_SServiceMethod** server_method_names_, size_t* server_method_names_length_);

  /**
   * @brief Get the pairs of service name / method name of all eCAL Clients.
   *
   * @param[out] client_method_names_        Returned array of client method names. Must point to NULL and needs to be released by eCAL_Free().
   * @param[out] client_method_names_length_ Returned length of the array. Must point to zero.
   *
   * @return Zero if succeeded, non-zero otherwise
  **/
  ECALC_API int eCAL_Registration_GetClientMethodNames(struct eCAL_Registration_SServiceMethod** client_method_names_, size_t* client_method_names_length_);

  /**
   * @brief Register a callback function to be notified when a new publisher becomes available.
   *
   * @param callback_       The callback function to be called with the STopicId of the new publisher.
   *                        The callback function must not be blocked for a longer period of time,
   *                        otherwise timeout mechanisms of the eCAL registration would be triggered.
   *
   * @return CallbackToken that can be used to unregister the callback.
  **/
  ECALC_API eCAL_Registration_CallbackToken eCAL_Registration_AddPublisherEventCallback(eCAL_Registration_TopicEventCallbackT callback_);
  
  /**
   * @brief Unregister the publisher callback using the provided token.
   *
   * @param token_  The token returned by eCAL_Registration_AddPublisherEventCallback().
  **/
  ECALC_API void eCAL_Registration_RemPublisherEventCallback(eCAL_Registration_CallbackToken token_);

  /**
   * @brief Register a callback function to be notified when a new subscriber becomes available.
   *
   * @param callback_       The callback function to be called with the STopicId of the new subscriber.
   *                        The callback function must not be blocked for a longer period of time,
   *                        otherwise timeout mechanisms of the eCAL registration would be triggered.
   *
   * @return CallbackToken that can be used to unregister the callback.
  **/
  ECALC_API eCAL_Registration_CallbackToken eCAL_Registration_AddSubscriberEventCallback(eCAL_Registration_TopicEventCallbackT callback_);
  
  /**
   * @brief Unregister the subscriber callback using the provided token.
   *
   * @param token  The token returned by eCAL_Registration_AddSubscriberEventCallback().
  **/
  ECALC_API void eCAL_Registration_RemSubscriberEventCallback(eCAL_Registration_CallbackToken token_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_registration_h_included*/