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

#include <config/config_v2to3.h>

#include <tinyxml2.h>
#include <limits>
#include <clocale>

#include <rec_client_core/ecal_rec_logger.h>

namespace eCAL
{
  namespace rec_server
  {
    namespace config_v2to3
    {
      bool writeConfigFile(const eCAL::rec_server::RecServerImpl& rec_server, const std::string& path)
      {
        char decimal_point = std::localeconv()->decimal_point[0]; // decimal point for std::to_string de-localization
        
        tinyxml2::XMLDocument document;

        {
          // Main config element
          auto main_config_element = document.NewElement(ELEMENT_NAME_MAIN_CONFIG);
          main_config_element->SetAttribute(ATTRIBUTE_NAME_MAIN_CONFIG_VERSION, NATIVE_CONFIG_VERSION);
          document.InsertEndChild(main_config_element);

          {
            // root directory
            auto root_directory_element = document.NewElement(ELEMENT_NAME_ROOT_DIR);
            root_directory_element->SetText(rec_server.GetMeasRootDir().c_str());
            main_config_element->InsertEndChild(root_directory_element);
          }
          {
            // measurement name
            auto meas_name_element = document.NewElement(ELEMENT_NAME_MEAS_NAME);
            meas_name_element->SetText(rec_server.GetMeasName().c_str());
            main_config_element->InsertEndChild(meas_name_element);
          }
          {
            // max file size
            auto max_file_size_element = document.NewElement(ELEMENT_NAME_MAX_FILE_SIZE_MIB);
            max_file_size_element->SetText(std::to_string(rec_server.GetMaxFileSizeMib()).c_str());
            main_config_element->InsertEndChild(max_file_size_element);
          }
          {
            // description
            auto description_element = document.NewElement(ELEMENT_NAME_DESCRIPTION);
            description_element->SetText(rec_server.GetDescription().c_str());
            main_config_element->InsertEndChild(description_element);
          }
          {
            // enabled recorders
            auto enabled_recorders = document.NewElement(ELEMENT_NAME_ENABLED_RECORDERS);
            main_config_element->InsertEndChild(enabled_recorders);

            for (const auto& enabled_client : rec_server.GetEnabledRecClients())
            {
              // recorder entry
              auto recorder_entry = document.NewElement(ELEMENT_NAME_ENABLED_RECORDER_ENTRY);

              {
                // recorder entry name
                auto recorder_entry_name = document.NewElement(ELEMENT_NAME_ENABLED_RECORDER_ENTRY_NAME);
                recorder_entry_name->SetText(enabled_client.first.c_str());
                recorder_entry->InsertEndChild(recorder_entry_name);
              }

              {
                // recorder entry hostFilter
                auto recorder_entry_hostfilter = document.NewElement(ELEMENT_NAME_ENABLED_RECORDER_ENTRY_HOSTFILTER);

                for (const std::string& host : enabled_client.second.host_filter_)
                {
                  // hostfilter entry
                  auto hostfilter_entry = document.NewElement(ELEMENT_NAME_ENABLED_RECORDER_ENTRY_HOSTFILTER_ENTRY);
                  hostfilter_entry->SetText(host.c_str());
                  recorder_entry_hostfilter->InsertEndChild(hostfilter_entry);
                }
                recorder_entry->InsertEndChild(recorder_entry_hostfilter);
              }

              {
                // enabled addons
                auto enabled_addons_element = document.NewElement(ELEMENT_NAME_ENABLED_RECORDER_ENTRY_ENABLED_ADDONS);

                for (const std::string& addon_id : enabled_client.second.enabled_addons_)
                {
                  // addon entry
                  auto addon_entry = document.NewElement(ELEMENT_NAME_ENABLED_RECORDER_ENTRY_ENABLED_ADDON_ENTRY);
                  addon_entry->SetText(addon_id.c_str());
                  enabled_addons_element->InsertEndChild(addon_entry);
                }
                recorder_entry->InsertEndChild(enabled_addons_element);
              }

              enabled_recorders->InsertEndChild(recorder_entry);
            }
          }
          {
            // pre buffer enabled
            auto pre_buffer_enabled = document.NewElement(ELEMENT_NAME_PRE_BUFFER_ENABLED);
            pre_buffer_enabled->SetText(rec_server.GetPreBufferingEnabled() ? "true" : "false");
            main_config_element->InsertEndChild(pre_buffer_enabled);
          }
          {
            // pre buffer length
            auto pre_buffer_length = document.NewElement(ELEMENT_NAME_PRE_BUFFER_LENGTH_SECS);
            std::string pre_buffer_length_string = std::to_string(std::chrono::duration_cast<std::chrono::duration<double>>(rec_server.GetMaxPreBufferLength()).count());
            std::replace(pre_buffer_length_string.begin(), pre_buffer_length_string.end(), decimal_point, '.');
            pre_buffer_length->SetText(pre_buffer_length_string.c_str());
            main_config_element->InsertEndChild(pre_buffer_length);
          }
          {
            // use built-in recorder
            auto built_in_recorder_enabled_element = document.NewElement(ELEMENT_NAME_BUILT_IN_RECORDER_ENABLED);
            built_in_recorder_enabled_element->SetText(rec_server.IsUsingBuiltInRecorderEnabled() ? "true" : "false");
            main_config_element->InsertEndChild(built_in_recorder_enabled_element);
          }
          {
            // topic list
            auto topic_list = document.NewElement(ELEMENT_NAME_TOPIC_LIST);
            if (rec_server.GetRecordMode() != eCAL::rec::RecordMode::All)
            {
              topic_list->SetAttribute(ATTRIBUTE_NAME_TOPIC_LIST_TYPE, rec_server.GetRecordMode() == eCAL::rec::RecordMode::Blacklist ? "blacklist" : "whitelist");
              for (const std::string& topic : rec_server.GetListedTopics())
              {
                // topic list entry
                auto topic_list_entry = document.NewElement(ELEMENT_NAME_TOPIC_LIST_ENTRY);
                topic_list_entry->SetText(topic.c_str());
                topic_list->InsertEndChild(topic_list_entry);
              }
            }
            main_config_element->InsertEndChild(topic_list);
          }
          {
            // upload config (added in v3)
            auto upload_config_element = document.NewElement(ELEMENT_NAME_UPLOAD_CONFIG);
            auto upload_config = rec_server.GetUploadConfig();

            if (upload_config.type_ == UploadConfig::Type::FTP)
            {
              upload_config_element->SetAttribute(ATTRIBUTE_NAME_UPLOAD_PROTOCOL, "ftp");

              auto upload_config_host_element     = document.NewElement(ELEMENT_NAME_UPLOAD_CONFIG_HOST);
              auto upload_config_port_element     = document.NewElement(ELEMENT_NAME_UPLOAD_CONFIG_PORT);
              auto upload_config_username_element = document.NewElement(ELEMENT_NAME_UPLOAD_CONFIG_USER);
              auto upload_config_password_element = document.NewElement(ELEMENT_NAME_UPLOAD_CONFIG_PASSWORD);
              auto upload_config_root_dir_element = document.NewElement(ELEMENT_NAME_UPLOAD_CONFIG_ROOT_DIR);

              upload_config_host_element    ->SetText(upload_config.host_.c_str());
              upload_config_port_element    ->SetText(std::to_string(upload_config.port_).c_str());
              upload_config_username_element->SetText(upload_config.username_.c_str());
              upload_config_password_element->SetText(upload_config.password_.c_str());
              upload_config_root_dir_element->SetText(upload_config.root_path_.c_str());

              upload_config_element->InsertEndChild(upload_config_host_element);
              upload_config_element->InsertEndChild(upload_config_port_element);
              upload_config_element->InsertEndChild(upload_config_username_element);
              upload_config_element->InsertEndChild(upload_config_password_element);
              upload_config_element->InsertEndChild(upload_config_root_dir_element);
            }
            else
            {
              upload_config_element->SetAttribute(ATTRIBUTE_NAME_UPLOAD_PROTOCOL, "internal");
            }

            auto upload_config_delete_after_upload_element = document.NewElement(ELEMENT_NAME_UPLOAD_CONFIG_DELETE_AFTER_UPLOAD);
            upload_config_delete_after_upload_element->SetText(upload_config.delete_after_upload_ ? "true" : "false");
            upload_config_element->InsertEndChild(upload_config_delete_after_upload_element);

            main_config_element->InsertEndChild(upload_config_element);
          }
        }

        tinyxml2::XMLError errorcode = document.SaveFile(path.c_str(), false);

        if (errorcode == tinyxml2::XML_SUCCESS)
        {
          return true;
        }
        else
        {
#if TINYXML2_MAJOR_VERSION >= 6
          eCAL::rec::EcalRecLogger::Instance()->error(std::string("Error saving config to \"") + path + "\": " + document.ErrorStr());
#else // TINYXML2_MAJOR_VERSION
            eCAL::rec::EcalRecLogger::Instance()->error(std::string("Error saving config to \"") + path + "\": " + document.GetErrorStr1());
#endif // TINYXML2_MAJOR_VERSION
          return false;
        }
      }

      eCAL::rec_server::RecServerConfig readConfig(const tinyxml2::XMLElement* const main_config_element)
      {
        char decimal_point = std::localeconv()->decimal_point[0]; // decimal point for std::to_string de-localization
        eCAL::rec_server::RecServerConfig config_output;

        // root_dir
        {
          auto root_dir_element = main_config_element->FirstChildElement(ELEMENT_NAME_ROOT_DIR);
          if ((root_dir_element != nullptr)
            && (root_dir_element->GetText() != nullptr))
          {
            config_output.root_dir_ = root_dir_element->GetText();
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Root directory element is missing");
          }
        }
        
        // meas_name
        {
          auto meas_name_element = main_config_element->FirstChildElement(ELEMENT_NAME_MEAS_NAME);
          if ((meas_name_element != nullptr)
            && (meas_name_element->GetText() != nullptr))
          {
            config_output.meas_name_ = meas_name_element->GetText();
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Measurement name element is missing");
          }
        }
        
        // max_file_size
        {
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
        
        // description
        {
          auto description_element = main_config_element->FirstChildElement(ELEMENT_NAME_DESCRIPTION);
          if ((description_element != nullptr)
            && (description_element->GetText() != nullptr))

          {
            config_output.description_ = description_element->GetText();
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Description name element is missing");
          }
        }
        
        // enabled_clients_config
        {
          auto enabled_clients_element = main_config_element->FirstChildElement(ELEMENT_NAME_ENABLED_RECORDERS);
          if (enabled_clients_element != nullptr)
          {
            
            for(auto client = enabled_clients_element->FirstChildElement(); client != nullptr; client = client->NextSiblingElement())
            {
              if (std::string(client->Name()) != ELEMENT_NAME_ENABLED_RECORDER_ENTRY)
              {
                continue;
              }
              std::string           client_name ("");
              std::set<std::string> host_filter;
              std::set<std::string> enabled_addons;

              {
                // Client name
                auto client_name_element = client->FirstChildElement(ELEMENT_NAME_ENABLED_RECORDER_ENTRY_NAME);
                if ((client_name_element == nullptr)
                  || (client_name_element->GetText() == nullptr)
                  || (client_name_element->GetText()[0] == '\0'))
                {
                  eCAL::rec::EcalRecLogger::Instance()->warn("Encountered recorder client without name");
                  continue;
                }
                client_name = client_name_element->GetText();
              }
              {
                // Client host filter
                auto host_filter_element = client->FirstChildElement(ELEMENT_NAME_ENABLED_RECORDER_ENTRY_HOSTFILTER);
                if (host_filter_element != nullptr)
                {
                  for(auto host_filter_entry = host_filter_element->FirstChildElement(); host_filter_entry != nullptr; host_filter_entry = host_filter_entry->NextSiblingElement())
                  {
                    // Host Filter entry
                    if ((std::string(host_filter_entry->Name()) != ELEMENT_NAME_ENABLED_RECORDER_ENTRY_HOSTFILTER_ENTRY)
                      || (host_filter_entry->GetText() == nullptr)
                      || (host_filter_entry->GetText()[0] == '\0'))
                    {
                      continue;
                    }
                    host_filter.emplace(host_filter_entry->GetText());
                  }
                }
              }
              {
                // Enabled addons
                auto enabled_addons_element = client->FirstChildElement(ELEMENT_NAME_ENABLED_RECORDER_ENTRY_ENABLED_ADDONS);
                if (enabled_addons_element != nullptr)
                {
                  for(auto enabled_addon_entry = enabled_addons_element->FirstChildElement(); enabled_addon_entry != nullptr; enabled_addon_entry = enabled_addon_entry->NextSiblingElement())
                  {
                    // Enabled addon entry
                    if ((std::string(enabled_addon_entry->Name()) != ELEMENT_NAME_ENABLED_RECORDER_ENTRY_ENABLED_ADDON_ENTRY)
                      || (enabled_addon_entry->GetText() == nullptr)
                      || (enabled_addon_entry->GetText()[0] == '\0'))
                    {
                      continue;
                    }
                    enabled_addons.emplace(enabled_addon_entry->GetText());
                  }
                }
              }

              eCAL::rec_server::ClientConfig client_settings;
              client_settings.host_filter_    = std::move(host_filter);
              client_settings.enabled_addons_ = std::move(enabled_addons);
              config_output.enabled_clients_config_.emplace(client_name, std::move(client_settings));
            }
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Recorders element is missing");
          }
        }
        
        // pre_buffer_enabled
        {
          auto pre_buffer_enabled_element = main_config_element->FirstChildElement(ELEMENT_NAME_PRE_BUFFER_ENABLED);
          if ((pre_buffer_enabled_element != nullptr)
            && (pre_buffer_enabled_element->GetText() != nullptr))
          {
            std::string pre_buffer_enabled_string = pre_buffer_enabled_element->GetText();
            std::transform(pre_buffer_enabled_string.begin(), pre_buffer_enabled_string.end(), pre_buffer_enabled_string.begin(),
              [](unsigned char c){ return static_cast<unsigned char>(std::tolower(c)); });
            if (pre_buffer_enabled_string == "true")
            {
              config_output.pre_buffer_enabled_ = true;
            }
            else if (pre_buffer_enabled_string == "false")
            {
              config_output.pre_buffer_enabled_ = false;
            }
            else
            {
              eCAL::rec::EcalRecLogger::Instance()->warn("Error parsing pre-buffer-enabled setting");
            }
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Pre-buffer-enabled element is missing");
          }
        }
        
        // pre_buffer_length
        {
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
        
        // use built-in recorder
        {
          auto use_built_in_recorder_enabled_element = main_config_element->FirstChildElement(ELEMENT_NAME_BUILT_IN_RECORDER_ENABLED);
          if ((use_built_in_recorder_enabled_element != nullptr)
            && (use_built_in_recorder_enabled_element->GetText() != nullptr))
          {
            std::string use_built_in_recorder_enabled_string = use_built_in_recorder_enabled_element->GetText();
            std::transform(use_built_in_recorder_enabled_string.begin(), use_built_in_recorder_enabled_string.end(), use_built_in_recorder_enabled_string.begin(),
              [](unsigned char c){ return static_cast<unsigned char>(std::tolower(c)); });
            if (use_built_in_recorder_enabled_string == "true")
            {
              config_output.built_in_recorder_enabled_ = true;
            }
            else if (use_built_in_recorder_enabled_string == "false")
            {
              config_output.built_in_recorder_enabled_ = false;
            }
            else
            {
              eCAL::rec::EcalRecLogger::Instance()->warn("Error parsing use-built-in-recorder setting");
            }
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Use-built-in-recorder element is missing");
          }
        }
        
        // topic_list
        {
          auto topic_list_element = main_config_element->FirstChildElement(ELEMENT_NAME_TOPIC_LIST);
          if (topic_list_element != nullptr)
          {
            const char* record_mode_char_p = topic_list_element->Attribute(ATTRIBUTE_NAME_TOPIC_LIST_TYPE);
            if (record_mode_char_p != nullptr)
            {
              std::string record_mode_string = record_mode_char_p;
              std::transform(record_mode_string.begin(), record_mode_string.end(), record_mode_string.begin(),
                [](unsigned char c){ return static_cast<unsigned char>(std::tolower(c)); });
              if (record_mode_string == "whitelist")
              {
                config_output.record_mode_ = eCAL::rec::RecordMode::Whitelist;
              }
              else if (record_mode_string == "blacklist")
              {
                config_output.record_mode_ = eCAL::rec::RecordMode::Blacklist;
              }
              else if (record_mode_string == "all")
              {
                config_output.record_mode_ = eCAL::rec::RecordMode::All;
              }
              else
              {
                config_output.record_mode_ = eCAL::rec::RecordMode::All;
                eCAL::rec::EcalRecLogger::Instance()->warn("Invalid record mode detected: " + record_mode_string);
              }

              // topic_list
              for(auto topic_list_entry_element = topic_list_element->FirstChildElement(); topic_list_entry_element != nullptr; topic_list_entry_element = topic_list_entry_element->NextSiblingElement())
              {
                if ((std::string(topic_list_entry_element->Name()) != ELEMENT_NAME_TOPIC_LIST_ENTRY)
                  || (topic_list_entry_element->GetText() == nullptr)
                  || (topic_list_entry_element->GetText()[0] == '\0'))
                {
                  continue;
                }
                config_output.listed_topics_.emplace(topic_list_entry_element->GetText());
              }
            }
          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Topic-list element is missing");
          }
        }

        // upload_config (added in v3)
        {
          auto upload_config_element = main_config_element->FirstChildElement(ELEMENT_NAME_UPLOAD_CONFIG);
          if (upload_config_element != nullptr)
          {
            // upload protocol
            const char* protocol_char_p = upload_config_element->Attribute(ATTRIBUTE_NAME_UPLOAD_PROTOCOL);
            if (protocol_char_p != nullptr)
            {
              std::string protocol_string = protocol_char_p;
              std::transform(protocol_string.begin(), protocol_string.end(), protocol_string.begin(),
                              [](unsigned char c){ return static_cast<unsigned char>(std::tolower(c)); });
              if (protocol_string == "ftp")
              {
                config_output.upload_config_.type_ = eCAL::rec_server::UploadConfig::Type::FTP;
                
                // TODO: e.g. the password may legally be empty. I have to check if I get a nullpointer in that case and thus throw a warning.

                // host
                {
                  auto host_element     = upload_config_element->FirstChildElement(ELEMENT_NAME_UPLOAD_CONFIG_HOST);
                  if ((host_element != nullptr) && (host_element->GetText() != nullptr))
                    config_output.upload_config_.host_ = std::string(host_element->GetText());
                  else
                    eCAL::rec::EcalRecLogger::Instance()->warn("Upload hostname element is missing");
                }

                // port
                {
                  auto port_element     = upload_config_element->FirstChildElement(ELEMENT_NAME_UPLOAD_CONFIG_PORT);

                  if ((port_element != nullptr) && (port_element->GetText() != nullptr))
                  {
                    std::string port_string(port_element->GetText());
                    unsigned long port_long = config_output.upload_config_.port_;
                    try
                    {
                      port_long = std::stoul(port_string);
                    }
                    catch(std::exception& e)
                    {
                      eCAL::rec::EcalRecLogger::Instance()->warn(std::string("Error reading port: ") + e.what());
                    }

                    if ((port_long < std::numeric_limits<decltype(config_output.upload_config_.port_)>::max())
                      && (port_long > std::numeric_limits<decltype(config_output.upload_config_.port_)>::min()))
                    {
                      config_output.upload_config_.port_ = static_cast<decltype(config_output.upload_config_.port_)>(port_long);
                    }
                    else
                    {
                      eCAL::rec::EcalRecLogger::Instance()->warn(std::string("Error reading port: ") + std::to_string(port_long) + " is not a valid port.");
                    }
                  }
                  else
                  {
                    eCAL::rec::EcalRecLogger::Instance()->warn("Upload port element is missing");
                  }
                }

                // username
                {
                  auto username_element = upload_config_element->FirstChildElement(ELEMENT_NAME_UPLOAD_CONFIG_USER);
                  if ((username_element != nullptr) && (username_element->GetText() != nullptr))
                    config_output.upload_config_.username_ = std::string(username_element->GetText());
                  else
                    eCAL::rec::EcalRecLogger::Instance()->warn("Upload username element is missing");
                }

                // password
                {
                  auto password_element = upload_config_element->FirstChildElement(ELEMENT_NAME_UPLOAD_CONFIG_PASSWORD);
                  if ((password_element != nullptr) && (password_element->GetText() != nullptr))
                    config_output.upload_config_.password_ = std::string(password_element->GetText());
                  else
                    eCAL::rec::EcalRecLogger::Instance()->warn("Upload password element is missing");
                }

                // root dir
                {
                  auto root_dir_element = upload_config_element->FirstChildElement(ELEMENT_NAME_UPLOAD_CONFIG_ROOT_DIR);
                  if ((root_dir_element != nullptr) && (root_dir_element->GetText() != nullptr))
                    config_output.upload_config_.root_path_ = std::string(root_dir_element->GetText());
                  else
                    eCAL::rec::EcalRecLogger::Instance()->warn("Upload root dir element is missing");
                }

              }
              else if (protocol_string == "internal")
              {
                config_output.upload_config_.type_ = eCAL::rec_server::UploadConfig::Type::INTERNAL_FTP;
              }
              else
              {
                eCAL::rec::EcalRecLogger::Instance()->warn("Unrecognized upload protocol: \"" + protocol_string + "\"");
              }

            }

            // delete after upload
            {
              auto delete_after_upload_element = upload_config_element->FirstChildElement(ELEMENT_NAME_UPLOAD_CONFIG_DELETE_AFTER_UPLOAD);
              if ((delete_after_upload_element != nullptr) && (delete_after_upload_element->GetText() != nullptr))
              {
                std::string delete_after_upload_string = std::string(delete_after_upload_element->GetText());

                std::transform(delete_after_upload_string.begin(), delete_after_upload_string.end(), delete_after_upload_string.begin(),
                              [](unsigned char c){ return static_cast<unsigned char>(std::tolower(c)); });
                if (delete_after_upload_string == "true")
                {
                  config_output.upload_config_.delete_after_upload_ = true;
                }
                else if (delete_after_upload_string == "false")
                {
                  config_output.upload_config_.delete_after_upload_ = false;
                }
                else
                {
                  eCAL::rec::EcalRecLogger::Instance()->warn("Error parsing delete-after-upload setting");
                }

              }
              else
              {
                eCAL::rec::EcalRecLogger::Instance()->warn("Delete-after-upload element is missing");
              }
            }

          }
          else
          {
            eCAL::rec::EcalRecLogger::Instance()->warn("Upload-config element is missing");
          }
        }

        return config_output;
      }
    }
  }
}
