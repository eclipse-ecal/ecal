/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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
 * eCALSys config file parser
**/

#include "esys_cfg_parser.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

#include "tinyxml2.h"

#include <ecal_utils/string.h>
#include <ecal_utils/filesystem.h>

#ifdef WIN32
#include <ecal_utils/str_convert.h>
#endif // WIN32


namespace eCAL
{
  namespace Sys
  {
    namespace Config
    {
      /**
      * @brief                  Add a XML child element to a parent
      *
      * @param parent_element   The parent element of the element to be added
      * @param name             Name of the element to be added
      * @param content          Content of the element to be added
      *
      **/
      void AddChildElement(tinyxml2::XMLDocument& xml, tinyxml2::XMLElement& parent_element, const std::string& name, const std::string& content)
      {
        auto element = xml.NewElement(name.c_str());
        element->SetValue(name.c_str());
        element->SetText(content.c_str());

        parent_element.InsertEndChild(element);
      }

      /**
      * @brief                  Add a runner specified by name if not found in the runners list
      *
      * @param runners          Runners list
      * @param runner_name      Runner name
      *
      * @return                 True if the runner was added to the runners list, false otherwise
      **/
      bool AddRunnerIfNotFound(CConfiguration::RunnerList& runners, const std::string& runner_name)
      {
        bool runner_found = false;
        for (const auto& runner_it : runners)
        {
          if (runner_it.name_ == runner_name)
          {
            runner_found = true;
            break;
          }
        }
        if (!runner_found)
        {
          CConfiguration::Runner runner;
          runner.name_ = runner_name;
          runners.push_back(runner);
          return true;
        }
        return false;
      }

      /**
      * @brief              Lexicographical task comparator between two tasks by name   
      *
      * @param lhs          First task to compare
      * @param rhs          Second task to compare
      *
      * @return             True if the first task compares lexicographically less than the second, false otherwise
      **/
      bool LexicographicalTaskComparator(const eCAL::Sys::Config::CConfiguration::Task& lhs, const eCAL::Sys::Config::CConfiguration::Task& rhs)
      {
        return std::lexicographical_compare(lhs.name_.begin(), lhs.name_.end(), rhs.name_.begin(), rhs.name_.end());
      }

      /**
      * @brief                  Find a XML element specified by name
      *
      * @param xml              XML document
      * @param str              Element name
      *
      * @return                 The element if it was found, nullptr otherwise
      **/
      tinyxml2::XMLElement* FindElementByName(tinyxml2::XMLDocument& xml, const std::string& str)
      {
        if (xml.ErrorID() != tinyxml2::XML_SUCCESS)
          return nullptr;

        auto element = xml.FirstChildElement();
        while (element)
        {
          if (!std::string(element->Value()).compare(str)) return element;
          if (element->FirstChildElement())
            element = element->FirstChildElement();
          else
            if (element->NextSiblingElement())
              element = element->NextSiblingElement();
            else
            {
              while (element && element->Parent() && !element->Parent()->NextSiblingElement())
                element = element->Parent()->ToElement();
              if (element && element->Parent() && element->Parent()->NextSiblingElement())
                element = element->Parent()->NextSiblingElement();
              else
                break;
            }
        }

        return nullptr;
      }

      /**
      * @brief                  Search in a list of runners for an identical runner with the given runner 
      *
      * @param runner           Runner to be searched
      * @param runners          List of runners
      * @param runner_map       Map which associates searched runner ID with the ID of the found runner
      *
      * @return                 True if the identical runner was found, false otherwise
      **/
      bool FindRunner(const CConfiguration::Runner& runner, const CConfiguration::RunnerList& runners, std::unordered_map<unsigned int, unsigned int>& runner_map)
      {
        for (const auto& runner_it : runners)
        {
          if (runner.name_ == runner_it.name_ &&
            runner.path_ == runner_it.path_ &&
            runner.default_algo_dir_ == runner_it.default_algo_dir_ &&
            runner.load_cmd_argument_ == runner_it.load_cmd_argument_)
          {
            runner_map[runner.GetId()] = runner_it.GetId();
            return true;
          }
        }
        return false;
      }

      /**
      * @brief                  Search in a list of tasks for an identical task with the given task
      *
      * @param task             Task to be searched
      * @param tasks            List of tasks
      * @param task_map         Map which associates searched task ID with the ID of the found task
      *
      * @return                 True if the identical task was found, false otherwise
      **/
      bool FindTask(const CConfiguration::Task& task, const CConfiguration::TaskList& tasks, std::unordered_map<unsigned int, unsigned int>& task_map)
      {
        for (const auto& task_it : tasks)
        {
          if (task.name_ == task_it.name_ &&
            task.start_stop_.additional_cmd_line_args_ == task_it.start_stop_.additional_cmd_line_args_ &&
            task.start_stop_.algo_ == task_it.start_stop_.algo_ &&
            task.start_stop_.working_dir_ == task_it.start_stop_.working_dir_ &&
            task.start_stop_.launch_order_ == task_it.start_stop_.launch_order_ &&
            task.start_stop_.runner_id_ == task_it.start_stop_.runner_id_ &&
            task.start_stop_.target_ == task_it.start_stop_.target_ &&
            task.start_stop_.timeout_ == task_it.start_stop_.timeout_ &&
            task.start_stop_.visibility_ == task_it.start_stop_.visibility_ &&
            task.start_stop_.do_monitor_ == task_it.start_stop_.do_monitor_ &&
            task.monitoring_.restart_by_beaconing_ == task_it.monitoring_.restart_by_beaconing_ &&
            task.monitoring_.max_beacon_response_time_ == task_it.monitoring_.max_beacon_response_time_ &&
            task.monitoring_.restart_by_severity_ == task_it.monitoring_.restart_by_severity_ &&
            task.monitoring_.restart_below_severity_ == task_it.monitoring_.restart_below_severity_ &&
            task.monitoring_.restart_below_severity_level_ == task_it.monitoring_.restart_below_severity_level_)
          {
            task_map[task.GetId()] = task_it.GetId();
            return true;
          }
        }
        return false;
      }

      /**
      * @brief                  Search in a list of functions for an identical function with the given function
      *
      * @param function         Function to be searched
      * @param functions        List of functions
      *
      * @return                 True if the identical function was found, false otherwise
      **/
      bool FindFunction(CConfiguration::Function& function, CConfiguration::FunctionList& functions)
      {
        for (const auto& function_it : functions)
        {
          if (function.name_ == function_it.name_ &&
            function.tasks_.size() == function_it.tasks_.size() &&
            function.states_.size() == function_it.states_.size())
          {
            return true;
          }
        }
        return false;
      }

      /**
      * @brief                  Read configuration from the given path
      *
      * @param path             Path of file to be read
      * @param configuration    Configuration filled with the read data
      * @param import           True if the configuration is imported, false otherwise
      *
      * @return                 True if the operation was successful, false otherwise
      **/
      bool ReadConfig(const std::string& path, CConfiguration& configuration, bool import = false)
      {
        FILE* xml_file;
#ifdef WIN32
        std::wstring w_path = EcalUtils::StrConvert::Utf8ToWide(path);
        xml_file = _wfopen(w_path.c_str(), L"rb");
#else
        xml_file = fopen(path.c_str(), "rb");
#endif // WIN32

        if (xml_file == nullptr)
        {
          return false;
        }

        tinyxml2::XMLDocument src;
        tinyxml2::XMLError errorcode = src.LoadFile(xml_file);

        fclose(xml_file);

        try
        {
          auto root_element = src.FirstChildElement();
          if (root_element == nullptr || std::string(root_element->Value()).compare("eCALSys") != 0)
          {
            return false;
          }

          // Get imported configurations
          if (!import) 
          {
            auto import_element = FindElementByName(src, "import");
            if (import_element != nullptr)
            {
              for (auto config_element = import_element->FirstChildElement(); config_element != nullptr; config_element = config_element->NextSiblingElement())
              {
                std::string imported_path = EcalUtils::String::Trim(config_element->GetText());

                // Quick and dirty solution to strip the filename from the path
                std::string base_path = EcalUtils::Filesystem::CleanPath(path + "/..", EcalUtils::Filesystem::Current);

                EcalUtils::Filesystem::AbsolutePath(base_path, imported_path);
                configuration.AddImportedConfig(imported_path);
              }
            }

            // Get Targets
            auto targets_element = FindElementByName(src, "targets");
            if (targets_element != nullptr)
            {
              for (auto target_el = targets_element->FirstChildElement(); target_el != nullptr; target_el = target_el->NextSiblingElement())
              {
                std::string name = "", ip = "";
                auto name_var = target_el->Attribute("name");
                if (name_var != nullptr)
                {
                  name = name_var;
                }

                auto ip_var = target_el->Attribute("ip");
                if (ip_var != nullptr)
                {
                  ip = ip_var;
                }

                configuration.targets_.push_back(CConfiguration::Target(name, ip));
              }
            }
          }

          // Get Runners
          std::unordered_map<unsigned int, unsigned int> runner_map;
          auto runners_element = FindElementByName(src, "runners");
          if (runners_element != nullptr)
          {
            for (auto runner_el = runners_element->FirstChildElement(); runner_el != nullptr; runner_el = runner_el->NextSiblingElement())
            {
              unsigned int id = 0;
              auto id_var = runner_el->Attribute("id");
              if (id_var != nullptr)
              {
                id = std::stoul(id_var);
              }

              std::string name;
              auto name_var = runner_el->Attribute("name");
              if (name_var != nullptr)
              {
                name = name_var;
              }

              CConfiguration::Runner runner(id);
              runner.name_ = name;

              for (auto element = runner_el->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
              {
                std::string element_name = element->Value();
                std::string element_content;

                if (element->GetText() != nullptr)
                  element_content = EcalUtils::String::Trim(element->GetText());

                if (element_name.compare("path") == 0)
                {
                  runner.path_ = element_content;
                }
                if (element_name.compare("default_algo_dir") == 0)
                {
                  runner.default_algo_dir_ = element_content;
                }
                if (element_name.compare("load_cmd_argument") == 0)
                {
                  runner.load_cmd_argument_ = element_content;
                }
              }
              runner.imported_ = import;
              if (!import || !FindRunner(runner, configuration.runners_, runner_map))
              {
                configuration.runners_.push_back(runner);
              }
            }
            AddRunnerIfNotFound(configuration.runners_, EXE_RUNNER);
            AddRunnerIfNotFound(configuration.runners_, BAT_RUNNER);
          }

          // Get Tasks
          std::unordered_map<unsigned int, unsigned int> task_map;
          auto tasks_element = FindElementByName(src, "tasks");
          if (tasks_element != nullptr)
          {
            for (auto task_el = tasks_element->FirstChildElement(); task_el != nullptr; task_el = task_el->NextSiblingElement())
            {
              unsigned int id = 0;
              auto id_var = task_el->Attribute("id");
              if (id_var != nullptr)
              {
                id = std::stoul(id_var);
              }

              std::string name = "";
              auto name_var = task_el->Attribute("name");
              if (name_var != nullptr)
              {
                name = name_var;
              }

              CConfiguration::Task task(id);
              task.name_ = name;

              for (auto task_component = task_el->FirstChildElement(); task_component != nullptr; task_component = task_component->NextSiblingElement())
              {
                std::string task_component_name = task_component->Value();

                if (task_component_name.compare("start_stop") == 0)
                {
                  for (auto element = task_component->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
                  {
                    std::string element_name = element->Value();
                    std::string element_content;

                    if (element->GetText() != nullptr)
                      element_content = EcalUtils::String::Trim(element->GetText());

                    if (element_name.compare("target") == 0)
                    {
                      task.start_stop_.target_ = element_content;
                    }
                    if (element_name.compare("runner_id") == 0)
                    {
                      task.start_stop_.runner_id_ = std::stoul(element_content);
                    }
                    if (element_name.compare("algo") == 0)
                    {
                      task.start_stop_.algo_ = element_content;
                    }
                    if (element_name.compare("working_dir") == 0)
                    {
                      task.start_stop_.working_dir_ = element_content;
                    }
                    if (element_name.compare("launch_order") == 0)
                    {
                      task.start_stop_.launch_order_ = std::stoi(element_content);
                    }
                    if (element_name.compare("timeout") == 0)
                    {
                      task.start_stop_.timeout_ = std::chrono::milliseconds(std::stoll(element_content));
                    }
                    if (element_name.compare("visibility") == 0)
                    {
                      task.start_stop_.visibility_ = element_content;
                    }
                    if (element_name.compare("additional_cmd_line_args") == 0)
                    {
                      task.start_stop_.additional_cmd_line_args_ = element_content;
                    }
                    if (element_name.compare("do_monitor") == 0)
                    {
                      task.start_stop_.do_monitor_ = std::stoi(element_content) > 0;
                    }
                  }
                }

                if (task_component_name.compare("monitoring") == 0)
                {
                  for (auto element = task_component->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
                  {
                    std::string element_name = element->Value();
                    std::string element_content;
                    if (element->GetText() != nullptr)
                      element_content = EcalUtils::String::Trim(element->GetText());

                    if (element_name.compare("restart") == 0)
                    {
                      task.monitoring_.restart_by_beaconing_ = false;
                      task.monitoring_.restart_by_severity_ = std::stoi(element_content) > 0;
                    }
                    if (element_name.compare("restart_by_beaconing") == 0)
                    {
                      task.monitoring_.restart_by_beaconing_ = std::stoi(element_content) > 0;
                    }
                    if (element_name.compare("max_beacon_response_time") == 0)
                    {
                      task.monitoring_.max_beacon_response_time_ = std::chrono::microseconds(std::stoll(element_content));
                    }
                    if (element_name.compare("restart_by_severity") == 0)
                    {
                      task.monitoring_.restart_by_severity_ = std::stoi(element_content) > 0;
                    }
                    if (element_name.compare("restart_below_severity") == 0)
                    {
                      task.monitoring_.restart_below_severity_ = element_content;
                    }
                    if (element_name.compare("restart_below_severity_level") == 0)
                    {
                      task.monitoring_.restart_below_severity_level_ = element_content;
                    }
                  }
                }
              }
              task.imported_ = import;
              if (import && runner_map[task.start_stop_.runner_id_])
              {
                task.start_stop_.runner_id_ = runner_map[task.start_stop_.runner_id_];
              }
              if (!import || !FindTask(task, configuration.tasks_, task_map))
              {
                configuration.tasks_.push_back(task);
              }
            }
          }
          
          // Sort by launch_order, using default operator<
          //configuration.tasks_.sort(); // Sorting caused non-deterministic behaviour. We rather sort beforehand and fix this later along with a new configuration format.

          // Get Functions
          auto functions_element = FindElementByName(src, "functions");
          if (functions_element != nullptr)
          {
            for (auto function_el = functions_element->FirstChildElement(); function_el != nullptr; function_el = function_el->NextSiblingElement())
            {
              std::string name;
              auto name_var = function_el->Attribute("name");
              if (name_var != nullptr)
              {
                name = name_var;
              }

              CConfiguration::Function function;
              function.name_ = name;

              for (auto function_component = function_el->FirstChildElement(); function_component != nullptr; function_component = function_component->NextSiblingElement())
              {
                std::string function_component_name = function_component->Value();

                if (function_component_name.compare("tasks") == 0)
                {
                  for (auto task_el = function_component->FirstChildElement(); task_el != nullptr; task_el = task_el->NextSiblingElement())
                  {
                    unsigned long task_id = std::stoul(task_el->Attribute("id"));
                    if (import && task_map[task_id])
                    {
                      task_id = task_map[task_id];
                    }
                    function.tasks_.push_back(CConfiguration::Function::Task(task_id));
                  }
                }

                if (function_component_name.compare("state") == 0)
                {
                  CConfiguration::Function::State state;
                  state.colour_ = function_component->Attribute("colour");
                  state.name_ = function_component->Attribute("name");
                  state.prio_ = std::stoul(function_component->Attribute("prio"));

                  for (auto task_el = function_component->FirstChildElement(); task_el != nullptr; task_el = task_el->NextSiblingElement())
                  {
                    unsigned long task_id = std::stoul(task_el->Attribute("id"));
                    if (import && task_map[task_id])
                    {
                      task_id = task_map[task_id];
                    }
                    state.tasks_.push_back(CConfiguration::Function::State::Task(task_id,
                      task_el->Attribute("severity"),
                      task_el->Attribute("severity_level")));
                  }
                  function.states_.push_back(state);
                }
              }
              function.imported_ = import;
              if (!import || !FindFunction(function, configuration.functions_))
              {
                configuration.functions_.push_back(function);
              }
            }
          }

          if (!import)
          {
            // Get options
            eCAL::Sys::Config::CConfiguration::Options options;
            auto opt = FindElementByName(src, "options");
            if (opt != nullptr)
            {
              options.use_only_local_host_  = std::stoi(opt->FirstChildElement("only_local_host")->GetText()) > 0;
              options.use_all_on_this_host_ = std::stoi(opt->FirstChildElement("all_on_this_host")->GetText()) > 0;

              if (opt->FirstChildElement("all_targets_reachable") != nullptr)
                options.all_targets_reachable_ = std::stoi(opt->FirstChildElement("all_targets_reachable")->GetText()) > 0;

              // try to find option "stop_all_on_close"  in the root (for old versions of xml) - to be deleted at some point
              auto stop_all_element = FindElementByName(src, "stop_all_on_close");
              if (stop_all_element != nullptr)
              {
                options.stop_all_on_close_ = std::stoi(stop_all_element->GetText()) > 0;
              }
              else
              {
                // TODO(zepa) : keep only the line below and delete the above for option "stop_all_on_close"
                options.stop_all_on_close_ = std::stoi(opt->FirstChildElement("stop_all_on_close")->GetText()) > 0;
              }

              configuration.SetOptions(options);
            }

            // Get layout
            //eCAL::Sys::Config::Size win_size(0, 0);
            //auto size_element = FindElementByName(src, "window_size");
            //if (size_element != nullptr)
            //{
            //  win_size.SetHeight(std::stoi(size_element->FirstChildElement("height")->GetText()));
            //  win_size.SetWidth(std::stoi(size_element->FirstChildElement("width")->GetText()));
            //}

            //eCAL::Sys::Config::Point win_pos(0, 0);
            //auto pos_element = FindElementByName(src, "window_position");
            //if (pos_element != nullptr)
            //{
            //  win_pos.x = static_cast<int>(std::stof(pos_element->FirstChildElement("x")->GetText()));
            //  win_pos.y = static_cast<int>(std::stof(pos_element->FirstChildElement("y")->GetText()));
            //}

            //bool win_maximized = false;
            //auto max_element = FindElementByName(src, "window_maximized");
            //if (max_element != nullptr)
            //{
            //  win_maximized = std::stoi(max_element->GetText()) != 0;
            //}

            //std::string layout;
            //auto layout_element = FindElementByName(src, "layout");
            //if (layout_element != nullptr)
            //{
            //  if (layout_element->GetText() != nullptr)
            //    layout = EcalUtils::String::Trim(layout_element->GetText());
            //}

            //configuration.SetLayout(layout, win_size, win_pos, win_maximized);
          }
        }
        catch (...)
        {
          throw;
        }

        return true;
      }

      bool Open(const std::string& path, CConfiguration& configuration, std::string& exception_msg, bool import /* = false */)
      {
        std::string abspath = EcalUtils::Filesystem::AbsolutePath(path, EcalUtils::Filesystem::OsStyle::Current);

        if (ReadConfig(abspath, configuration, import))
        {
          for (const auto& imported_path : configuration.GetImportedConfigs())
          {
            if (!ReadConfig(imported_path, configuration, true))
            {
              exception_msg = "Imported configuration file " + imported_path + " could not be opened.\n";
            }
          }
          return true;
        }
        exception_msg = "Configuration file " + abspath + " could not be opened.\n";
        return false;
      }

      bool Save(const std::string& path, CConfiguration& configuration)
      {
        std::string abspath = EcalUtils::Filesystem::AbsolutePath(path, EcalUtils::Filesystem::OsStyle::Current);

        tinyxml2::XMLDocument doc;

        auto root_element = doc.NewElement("eCALSys");
        root_element->SetValue("eCALSys");
        doc.InsertEndChild(root_element);

        auto version_element = doc.NewElement("version");
        version_element->SetValue("version");
        version_element->SetText("1.0");
        root_element->InsertEndChild(version_element);

        // Set Imported configurations
        auto import_element = doc.NewElement("import");
        import_element->SetValue("import");

        for (const auto& config : configuration.GetImportedConfigs())
        {
          auto config_element = doc.NewElement("config");
          config_element->SetValue("config");
          config_element->SetText(EcalUtils::Filesystem::RelativePath(abspath, config).c_str());
          import_element->InsertEndChild(config_element);
        }
        root_element->InsertEndChild(import_element);

        // Set Targets
        auto targets_element = doc.NewElement("targets");
        targets_element->SetValue("targets");
        //configuration.targets_.sort();  // Sorting caused non-deterministic behaviour. We rather sort beforehand and fix this later along with a new configuration format.

        for (const auto& target : configuration.targets_)
        {
          auto target_element = doc.NewElement("target");
          target_element->SetValue("target");
          target_element->SetAttribute("name", target.name_.c_str());
          target_element->SetAttribute("ip", target.ip_.c_str());
          targets_element->InsertEndChild(target_element);
        }
        root_element->InsertEndChild(targets_element);

        // Set Runners
        auto runners_element = doc.NewElement("runners");
        runners_element->SetValue("runners");

        //configuration.runners_.sort();  // Sorting caused non-deterministic behaviour. We rather sort beforehand and fix this later along with a new configuration format.

        for (const auto& runner: configuration.runners_)
        {
          if (!runner.imported_)
          {
            auto runner_element = doc.NewElement("runner");
            runner_element->SetValue("runner");
            runner_element->SetAttribute("id", std::to_string(runner.GetId()).c_str());
            runner_element->SetAttribute("name", runner.name_.c_str());

            AddChildElement(doc, *runner_element, "path", runner.path_);
            AddChildElement(doc, *runner_element, "default_algo_dir", runner.default_algo_dir_);
            AddChildElement(doc, *runner_element, "load_cmd_argument", runner.load_cmd_argument_);

            runners_element->InsertEndChild(runner_element);
          }
        }
        root_element->InsertEndChild(runners_element);

        // Set Tasks
        auto tasks_element = doc.NewElement("tasks");
        tasks_element->SetValue("tasks");
        //configuration.tasks_.sort(LexicographicalTaskComparator);  // Sorting caused non-deterministic behaviour. We rather sort beforehand and fix this later along with a new configuration format.

        for (const auto& task : configuration.tasks_)
        {
          if (!task.imported_)
          {
            auto task_element = doc.NewElement("task");
            task_element->SetValue("task");
            task_element->SetAttribute("id", std::to_string(task.GetId()).c_str());
            task_element->SetAttribute("name", task.name_.c_str());

            auto start_stop_element = doc.NewElement("start_stop");
            start_stop_element->SetValue("start_stop");
            AddChildElement(doc, *start_stop_element, "target", task.start_stop_.target_);
            AddChildElement(doc, *start_stop_element, "runner_id", std::to_string(task.start_stop_.runner_id_));
            AddChildElement(doc, *start_stop_element, "algo", task.start_stop_.algo_);
            AddChildElement(doc, *start_stop_element, "working_dir", task.start_stop_.working_dir_);
            AddChildElement(doc, *start_stop_element, "launch_order", std::to_string(task.start_stop_.launch_order_));
            AddChildElement(doc, *start_stop_element, "timeout", std::to_string(task.start_stop_.timeout_.count()));
            AddChildElement(doc, *start_stop_element, "visibility", task.start_stop_.visibility_);
            AddChildElement(doc, *start_stop_element, "additional_cmd_line_args", task.start_stop_.additional_cmd_line_args_);
            AddChildElement(doc, *start_stop_element, "do_monitor", task.start_stop_.do_monitor_ == true ? "1" : "0");
            task_element->InsertEndChild(start_stop_element);

            auto monitoring_element = doc.NewElement("monitoring");
            monitoring_element->SetValue("monitoring");
            AddChildElement(doc, *monitoring_element, "restart_by_beaconing", task.monitoring_.restart_by_beaconing_ == true ? "1" : "0");
            AddChildElement(doc, *monitoring_element, "max_beacon_response_time", std::to_string(task.monitoring_.max_beacon_response_time_.count()));
            AddChildElement(doc, *monitoring_element, "restart_by_severity", task.monitoring_.restart_by_severity_ == true ? "1" : "0");
            AddChildElement(doc, *monitoring_element, "restart_below_severity", task.monitoring_.restart_below_severity_);
            AddChildElement(doc, *monitoring_element, "restart_below_severity_level", task.monitoring_.restart_below_severity_level_);
            task_element->InsertEndChild(monitoring_element);

            tasks_element->InsertEndChild(task_element);
          }
        }
        root_element->InsertEndChild(tasks_element);

        // Set Functions
        auto functions_element = doc.NewElement("functions");
        functions_element->SetValue("functions");
        //configuration.functions_.sort();  // Sorting caused non-deterministic behaviour. We rather sort beforehand and fix this later along with a new configuration format.

        for (const auto& function : configuration.functions_)
        {
          if (!function.imported_)
          {
            auto function_element = doc.NewElement("function");
            function_element->SetValue("function");
            function_element->SetAttribute("name", function.name_.c_str());

            auto function_tasks_element = doc.NewElement("tasks");
            function_tasks_element->SetValue("tasks");

            for (const auto& task_it : function.tasks_)
            {
              auto task_element = doc.NewElement("task");
              task_element->SetValue("task");
              task_element->SetAttribute("id", std::to_string(task_it.GetId()).c_str());

              function_tasks_element->InsertEndChild(task_element);
            }

            function_element->InsertEndChild(function_tasks_element);

            for (const auto& state : function.states_)
            {
              auto state_element = doc.NewElement("state");
              state_element->SetValue("state");
              state_element->SetAttribute("name", state.name_.c_str());
              state_element->SetAttribute("prio", std::to_string(state.prio_).c_str());
              state_element->SetAttribute("colour", state.colour_.c_str());

              for (const auto& task : state.tasks_)
              {
                auto state_task_element = doc.NewElement("task");
                state_task_element->SetValue("task");
                state_task_element->SetAttribute("id", std::to_string(task.GetId()).c_str());
                state_task_element->SetAttribute("severity", task.severity_.c_str());
                state_task_element->SetAttribute("severity_level", task.severity_level_.c_str());

                state_element->InsertEndChild(state_task_element);
              }
              function_element->InsertEndChild(state_element);
            }
            functions_element->InsertEndChild(function_element);
          }
        }
        root_element->InsertEndChild(functions_element);

        // Set options
        auto opt_element = doc.NewElement("options");
        opt_element->SetValue("options");
        auto opt = configuration.GetOptions();
        AddChildElement(doc, *opt_element, "all_targets_reachable", opt.all_targets_reachable_ ? "1" : "0");
        AddChildElement(doc, *opt_element, "only_local_host", opt.use_only_local_host_ ? "1" : "0");
        AddChildElement(doc, *opt_element, "all_on_this_host", opt.use_all_on_this_host_ ? "1" : "0");
        AddChildElement(doc, *opt_element, "stop_all_on_close", opt.stop_all_on_close_ ? "1" : "0");
        root_element->InsertEndChild(opt_element);

        // Set layout
        auto size_element = doc.NewElement("window_size");
        size_element->SetValue("window_size");
        eCAL::Sys::Config::Size win_size = configuration.GetWindowSize();
        AddChildElement(doc, *size_element, "height", std::to_string(win_size.GetHeight()));
        AddChildElement(doc, *size_element, "width", std::to_string(win_size.GetWidth()));
        root_element->InsertEndChild(size_element);

        auto pos_element = doc.NewElement("window_position");
        pos_element->SetValue("window_position");
        eCAL::Sys::Config::Point win_pos = configuration.GetWindowPosition();
        AddChildElement(doc, *pos_element, "x", std::to_string(win_pos.x));
        AddChildElement(doc, *pos_element, "y", std::to_string(win_pos.y));
        root_element->InsertEndChild(pos_element);

        AddChildElement(doc, *root_element, "window_maximized", std::to_string(static_cast<int>(configuration.GetIsWindowMaximized())));

        AddChildElement(doc, *root_element, "layout", configuration.GetLayout());

        FILE* xml_file;
#ifdef WIN32
        std::wstring w_path = EcalUtils::StrConvert::Utf8ToWide(path);
        xml_file = _wfopen(w_path.c_str(), L"w");
#else
        xml_file = fopen(path.c_str(), "w");
#endif // WIN32

        if (xml_file == nullptr)
        {
          return false;
        }

        tinyxml2::XMLError errorcode = doc.SaveFile(xml_file, false);
        fclose(xml_file);

        if (errorcode == tinyxml2::XML_SUCCESS)
        {
          return true;
        }
        else
        {
          return false;
        }
      }
    }  //  namespace Config
  }  //  namespace Sys
}  //  namespace eCAL
