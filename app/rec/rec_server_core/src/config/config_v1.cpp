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

#include <config/config_v1.h>

#include <tinyxml2.h>

#include <algorithm>
#include <regex>
#include <clocale>

#include <rec_client_core/ecal_rec_logger.h>

namespace eCAL
{
  namespace rec_server
  {
    namespace 
    {
      void ReplaceLegacyEcalParserFunctions(std::string& string)
      {
        const std::map<std::string, std::string> legacy_to_new
        {
          { "$(HOST_NAME)",    "${HOSTNAME}" },
          { "*[HOST_NAME]",    "${HOSTNAME}" },
          { "$(TIME)",         "${TIME %F-%H-%M-%S-%.m}" },
          { "*[TIME]",         "${TIME %F-%H-%M-%S-%.m}" },
          { "$(TIME_SECONDS)", "${TIME %F-%H-%M-%S}" },
          { "*[TIME_SECONDS]", "${TIME %F-%H-%M-%S}" },
        };

        for (const auto& legacy_new_pair : legacy_to_new)
        {
          for(size_t index = string.find(legacy_new_pair.first, 0); index != std::string::npos; index = string.find(legacy_new_pair.first, index))
          {
            string.replace(index, legacy_new_pair.first.length(), legacy_new_pair.second);
            index += legacy_new_pair.second.size();
          }
        }
      }
    }

    namespace config_v1
    {
      eCAL::rec_server::RecServerConfig readConfig(const tinyxml2::XMLElement* const main_config_element)
      {
        char decimal_point = std::localeconv()->decimal_point[0]; // decimal point for std::to_string de-localization
        eCAL::rec_server::RecServerConfig config_output;

        {
          // root_dir
          auto root_dir_element = main_config_element->FirstChildElement(ELEMENT_NAME_ROOT_DIR);
          if ((root_dir_element != nullptr)
            && (root_dir_element->GetText() != nullptr))
          {
            config_output.root_dir_ = root_dir_element->GetText();
            
            // The old recorder only supported Windows paths and used a _very_
            // ugly hack to also work on linux: the measurement root dir was
            // just hardcoded to some arbitrary directory. We replicate that
            // behaviour to improve config compatibility.
            std::regex win_drive_with_slash("^[a-zA-Z]\\:[/\\\\]");
            if(std::regex_search(config_output.root_dir_, win_drive_with_slash))
            {
              config_output.root_dir_ = "$TARGET{OSSELECT WIN \"" + config_output.root_dir_ + "\" LINUX \"/tmp/\"}";
            }
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Root directory element is missing");
          }
        }
        {
          // meas_name (1)
          auto meas_name_element = main_config_element->FirstChildElement(ELEMENT_NAME_MEAS_NAME_1);
          if ((meas_name_element != nullptr)
            && (meas_name_element->GetText() != nullptr))
          {
            std::string meas_name_part = meas_name_element->GetText();
            ReplaceLegacyEcalParserFunctions(meas_name_part);
            config_output.meas_name_ += meas_name_part;
          }
        }
        {
          // meas_name (2)
          auto meas_name_element = main_config_element->FirstChildElement(ELEMENT_NAME_MEAS_NAME_2);
          if ((meas_name_element != nullptr)
            && (meas_name_element->GetText() != nullptr))
          {
            std::string meas_name_part = meas_name_element->GetText();
            ReplaceLegacyEcalParserFunctions(meas_name_part);
            config_output.meas_name_ += meas_name_part;
          }
        }
        {
          // meas_name (3)
          auto meas_name_element = main_config_element->FirstChildElement(ELEMENT_NAME_MEAS_NAME_3);
          if ((meas_name_element != nullptr)
            && (meas_name_element->GetText() != nullptr))
          {
            std::string meas_name_part = meas_name_element->GetText();
            ReplaceLegacyEcalParserFunctions(meas_name_part);
            config_output.meas_name_ += meas_name_part;
          }
        }
        {
          // description
          auto description_element = main_config_element->FirstChildElement(ELEMENT_NAME_DESCRIPTION);
          if ((description_element != nullptr)
            && (description_element->GetText() != nullptr))

          {
            config_output.description_ = description_element->GetText();
            ReplaceLegacyEcalParserFunctions(config_output.description_);
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Description element is missing");
          }
        }
        {
          // max_file_size
          auto max_file_size_element = main_config_element->FirstChildElement(ELEMENT_NAME_MAX_FILE_SIZE_MIB);
          if ((max_file_size_element != nullptr)
            && (max_file_size_element->GetText() != nullptr))
          {
            std::string max_file_size_string = max_file_size_element->GetText();
            unsigned long long max_file_size(0);

            try
            {
              max_file_size = std::stoull(max_file_size_string);
            }
            catch(std::exception& e)
            {
              eCAL::rec::EcalRecLogger::Instance()->warn(std::string("Error reading max file size: ") + e.what());
            }
            if (max_file_size > std::numeric_limits<unsigned int>::max())
            {
              eCAL::rec::EcalRecLogger::Instance()->error(std::string("Error reading max file size: Number it too large."));
            }
            else
            {
              config_output.max_file_size_ = static_cast<unsigned int>(max_file_size);
            }
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Max file size element is missing");
          }
        }
        {
          // pre_buffer_length
          auto pre_buffer_length_element = main_config_element->FirstChildElement(ELEMENT_NAME_PRE_BUFFER_LENGTH_SECS);
          if ((pre_buffer_length_element != nullptr)
            && (pre_buffer_length_element->GetText() != nullptr))
          {
            std::string pre_buffer_length_secs_string = pre_buffer_length_element->GetText();
            std::replace(pre_buffer_length_secs_string.begin(), pre_buffer_length_secs_string.end(), '.', decimal_point);

            double pre_buffer_length_secs = 0.0;
            try
            {
              pre_buffer_length_secs = std::stod(pre_buffer_length_secs_string);
            }
            catch(std::exception& e)
            {
              eCAL::rec::EcalRecLogger::Instance()->warn(std::string("Error reading pre-buffer-length: ") + e.what());
            }
            config_output.pre_buffer_length_ = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(pre_buffer_length_secs));
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Pre-buffer-length element is missing");
          }
        }
        {
          // local host filter
          bool local_host_filter = false;
          auto local_host_filter_element = main_config_element->FirstChildElement(ELEMENT_NAME_LOCAL_HOSTFILTER);
          if ((local_host_filter_element != nullptr)
            && (local_host_filter_element->GetText() != nullptr))
          {
            std::string local_host_filter_string = local_host_filter_element->GetText();
            if (local_host_filter_string == "1")
              local_host_filter = true;
            else if (local_host_filter_string == "0")
              local_host_filter = false;
            else
              eCAL::rec::EcalRecLogger::Instance()->warn("Unable to parse local host filter value");
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Local host filter element is missing");
          }

          // enabled_clients_config
          auto enabled_clients_element = main_config_element->FirstChildElement(ELEMENT_NAME_ENABLED_RECORDERS);
          if (enabled_clients_element != nullptr)
          {
            for(auto client = enabled_clients_element->FirstChildElement(); client != nullptr; client = client->NextSiblingElement())
            {
              if ((std::string(client->Name()) != ELEMENT_NAME_ENABLED_RECORDER_ENTRY)
                || (client->GetText() == nullptr)
                || (client->GetText()[0] == '\0'))
              {
                continue;
              }
              std::string           client_name = client->GetText();
              std::set<std::string> host_filter;

              if (local_host_filter)
                host_filter.emplace(client_name);

              eCAL::rec_server::ClientConfig client_settings;
              client_settings.host_filter_    = std::move(host_filter);
              config_output.enabled_clients_config_.emplace(client_name, std::move(client_settings));
            }
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Recorders element is missing");
          }
        }
        {
          // topic_list

          std::set<std::string> blacklist;
          std::set<std::string> whitelist;
          bool blacklist_enabled = false;

          auto topic_list_element = main_config_element->FirstChildElement(ELEMENT_NAME_TOPIC_LIST);
          if (topic_list_element != nullptr)
          {
            {
              // Blacklist enabled
              auto blacklist_enabled_element = topic_list_element->FirstChildElement(ELEMENT_NAME_TOPIC_LIST_BLACKLIST_ENABLED);
              if ((blacklist_enabled_element != nullptr)
                && (blacklist_enabled_element->GetText() != nullptr))
              {
                std::string blacklist_enabled_string = blacklist_enabled_element->GetText();

                if (blacklist_enabled_string == "1")
                  blacklist_enabled = true;
                else if (blacklist_enabled_string == "0")
                  blacklist_enabled = false;
                else
                  eCAL::rec::EcalRecLogger::Instance()->warn("Unable to parse blacklist enabled value");
              }
              else
              {
                eCAL::rec::EcalRecLogger::Instance()->warn("Blacklist enabled element is missing");
              }
            }
            {
              // Blacklist
              auto blacklist_element = topic_list_element->FirstChildElement(ELEMENT_NAME_TOPIC_LIST_BLACKLIST);
              if (blacklist_element != nullptr)
              {
                // Actual topic list (there may be multiple from multiple hosts, but we don't care about the host)
                for(auto actual_topic_list_element = blacklist_element->FirstChildElement(); actual_topic_list_element != nullptr; actual_topic_list_element = actual_topic_list_element->NextSiblingElement())
                {
                  if (std::string(actual_topic_list_element->Name()) != ELEMENT_NAME_TOPIC_LIST_ACTUAL_LIST)
                  {
                    continue;
                  }
                  for(auto topic_list_entry_element = actual_topic_list_element->FirstChildElement(); topic_list_entry_element != nullptr; topic_list_entry_element = topic_list_entry_element->NextSiblingElement())
                  {
                    // Blacklist entry
                    if ((std::string(topic_list_entry_element->Name()) != ELEMENT_NAME_TOPIC_LIST_ENTRY)
                      || (topic_list_entry_element->GetText() == nullptr)
                      || (topic_list_entry_element->GetText()[0] == '\0'))
                    {
                      continue;
                    }
                    blacklist.emplace(topic_list_entry_element->GetText());
                  }
                }
              }
            }
            {
              // Whitelist
              auto whitelist_element = topic_list_element->FirstChildElement(ELEMENT_NAME_TOPIC_LIST_WHITELIST);
              if (whitelist_element != nullptr)
              {
                // Actual topic list (there may be multiple from multiple hosts, but we don't care about the host)
                for(auto actual_topic_list_element = whitelist_element->FirstChildElement(); actual_topic_list_element != nullptr; actual_topic_list_element = actual_topic_list_element->NextSiblingElement())
                {
                  if (std::string(actual_topic_list_element->Name()) != ELEMENT_NAME_TOPIC_LIST_ACTUAL_LIST)
                  {
                    continue;
                  }
                  for(auto topic_list_entry_element = actual_topic_list_element->FirstChildElement(); topic_list_entry_element != nullptr; topic_list_entry_element = topic_list_entry_element->NextSiblingElement())
                  {
                    // Whitelist entry
                    if ((std::string(topic_list_entry_element->Name()) != ELEMENT_NAME_TOPIC_LIST_ENTRY)
                      || (topic_list_entry_element->GetText() == nullptr)
                      || (topic_list_entry_element->GetText()[0] == '\0'))
                    {
                      continue;
                    }
                    whitelist.emplace(topic_list_entry_element->GetText());
                  }
                }
              }
            }
          }

          if (blacklist_enabled)
          {
            if (whitelist.empty())
            {
              config_output.record_mode_ = eCAL::rec::RecordMode::Blacklist;
              config_output.listed_topics_ = blacklist;
            }
            else
            {
              // Remove the topics from the whitelist that are also blacklisted
              for (const std::string& blacklisted_topic : blacklist)
              {
                whitelist.erase(blacklisted_topic);
              }

              config_output.record_mode_ = eCAL::rec::RecordMode::Whitelist;
              config_output.listed_topics_ = whitelist;
            }
          }
          else if (!whitelist.empty())
          {
            config_output.record_mode_   = eCAL::rec::RecordMode::Whitelist;
            config_output.listed_topics_ = whitelist;
          }
          else
          {
            config_output.record_mode_ = eCAL::rec::RecordMode::All;
          }
        }

        return config_output;
      }
    }
  }
}
