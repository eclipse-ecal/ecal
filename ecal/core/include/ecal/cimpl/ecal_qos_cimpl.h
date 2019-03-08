/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
 * @file   ecal_qos_cimpl.h
 * @brief  eCAL quality of service settings
**/

#ifndef ecal_qos_cimpl_h_included
#define ecal_qos_cimpl_h_included

/**
 * @brief eCAL QOS history kind mode.
**/
enum eQOSPolicy_HistoryKindC
{
  keep_last_history_qos,          /*!< Keep only a number of samples, default value.                     */
  keep_all_history_qos,           /*!< Keep all samples until the ResourceLimitsQosPolicy are exhausted. */
};

/**
 * @brief eCAL QOS reliability mode.
**/
enum eQOSPolicy_ReliabilityC
{
  best_effort_reliability_qos,    /*!< Best Effort reliability (default for Subscribers). */
  reliable_reliability_qos,       /*!< Reliable reliability (default for Publishers).     */
};

/**
 * @brief eCAL data writer QOS settings.
**/
struct SWriterQOSC
{
  enum eQOSPolicy_HistoryKindC  history_kind;            /*!< qos history kind mode       */
  int                           history_kind_depth;      /*!< qos history kind mode depth */
  enum eQOSPolicy_ReliabilityC  reliability;             /*!< qos reliability mode        */
};

/**
 * @brief eCAL data reader QOS settings.
**/
struct SReaderQOSC
{
  enum eQOSPolicy_HistoryKindC  history_kind;            /*!< qos history kind mode       */
  int                           history_kind_depth;      /*!< qos history kind mode depth */
  enum eQOSPolicy_ReliabilityC  reliability;             /*!< qos reliability mode        */
};

#endif /*ecal_qos_cimpl_h_included*/
