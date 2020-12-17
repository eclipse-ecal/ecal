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
 * eCALSys XML types definition
**/

#pragma once

#include <algorithm>
#include <climits>
#include <list>
#include <random>
#include <string>
#include <chrono>

typedef std::list<std::string> StringList;

namespace eCAL
{
  namespace Sys
  {
    namespace Config
    {
      const std::string EXE_RUNNER = "exe";
      const std::string BAT_RUNNER = "bat";

      class Point
      {
       public:
        int x, y;

        Point() : x(0), y(0) { }
        Point(const Point& point) : x(point.x), y(point.y) {}
        Point(int x_, int y_) : x(x_), y(y_) { }
        Point &operator =(const Point& point)
        {
          if (this != &point)
          {
            x = point.x; y = point.y;
          }
          return *this;
        }
      };

      class Size
      {
       public:
        Size() : x(0), y(0) { }
        Size &operator =(const Size& size) { x = size.x; y = size.y; return *this; }
        Size(const Size& size) : x(size.x), y(size.y) {}
        Size(int width, int height) : x(width), y(height) { }

        void Set(int width, int height) { x = width; y = height; }
        void SetWidth(int w) { x = w; }
        void SetHeight(int h) { y = h; }

        int GetWidth() const { return x; }
        int GetHeight() const { return y; }
       private:
        int x, y;
      };

      class CConfiguration
      {
       public:

        class Target
        {
         public:
          Target() {}
          Target &operator =(const Target& target) { name_ = target.name_; ip_ = target.ip_; return *this; }
          explicit Target(const std::string& name, const std::string& ip) : name_(name), ip_(ip) {}
          explicit Target(const Target& target) : name_(target.name_), ip_(target.ip_) {}

          // overloaded operator "<" for sorting targets by name
          bool operator<(const CConfiguration::Target& rhs) const
          {
            return std::lexicographical_compare(name_.begin(), name_.end(), rhs.name_.begin(), rhs.name_.end());
          }

          std::string name_;
          std::string ip_;
        };

        class Runner
        {
         public:
          explicit Runner(unsigned int id) 
            : id_(id) {}
          Runner()
          {
            std::random_device rd;
            std::mt19937 generator(rd());
            std::uniform_int_distribution<unsigned int> distribution(1, UINT_MAX);
            id_ = distribution(generator);
          }
          Runner(const Runner& runner)
            : name_(runner.name_)
            , path_(runner.path_)
            , default_algo_dir_(runner.default_algo_dir_)
            , load_cmd_argument_(runner.load_cmd_argument_)
            , imported_(runner.imported_)
            , id_(runner.id_) {}

          // overloaded operator "<" for sorting runners by name
          bool operator<(const Runner& rhs) const
          {
            return std::lexicographical_compare(name_.begin(), name_.end(), rhs.name_.begin(), rhs.name_.end());
          }

          // overloaded operator "==" . 2 runners are considered to be equal if they have the same fields
          bool operator==(const Runner& runner_b) const
          {
            return (this->GetId() == runner_b.GetId() &&
                    this->name_ == runner_b.name_ &&
                    this->path_ == runner_b.path_ &&
                    this->default_algo_dir_ == runner_b.default_algo_dir_ &&
                    this->load_cmd_argument_ == runner_b.load_cmd_argument_ &&
                    this->imported_ == runner_b.imported_);
          }

          bool operator==(const unsigned int id) const
          {
            return (this->GetId() == id);
          }

          // overloaded operator "=". Update all fields
          void operator=(const Runner& runner_b)
          {
            this->id_ = runner_b.GetId();
            this->name_ = runner_b.name_;
            this->path_ = runner_b.path_;
            this->default_algo_dir_ = runner_b.default_algo_dir_;
            this->load_cmd_argument_ = runner_b.load_cmd_argument_;
            this->imported_ = runner_b.imported_;
          }

          std::string name_;
          std::string path_;
          std::string default_algo_dir_;
          std::string load_cmd_argument_;
          bool imported_ = false;

          unsigned int GetId() const { return id_; }

         private:
          unsigned int id_;
        };

        class Task
        {
         public:
          class CStartStop
          {
           public:
            CStartStop() : runner_id_(0), launch_order_(0), timeout_(0), do_monitor_(true) {}
            CStartStop(const std::string& target, const unsigned int runner_id, const std::string& algo, const std::string& working_dir, size_t launch_order, std::chrono::milliseconds timeout, const std::string& visibility, const std::string& additional_cmd_line_args, bool do_monitor)
              : target_(target)
              , runner_id_(runner_id)
              , algo_(algo)
              , working_dir_(working_dir)
              , launch_order_(launch_order)
              , timeout_(timeout)
              , visibility_(visibility)
              , additional_cmd_line_args_(additional_cmd_line_args)
              , do_monitor_(do_monitor){}

            std::string target_;
            unsigned int runner_id_;
            std::string algo_;
            std::string working_dir_;
            size_t launch_order_;
            std::chrono::milliseconds timeout_;
            std::string visibility_;
            std::string additional_cmd_line_args_;
            bool do_monitor_;
          };

          class CMonitoring
          {
           public:
            CMonitoring() : restart_by_beaconing_(0), max_beacon_response_time_(0), restart_by_severity_(0) {}
            CMonitoring(bool restart_by_beaconing, std::chrono::microseconds max_beacon_response_time, bool restart_by_severity, std::string restart_below_severity, std::string restart_below_severity_level)
              : restart_by_beaconing_(restart_by_beaconing)
              , max_beacon_response_time_(max_beacon_response_time)
              , restart_by_severity_(restart_by_severity)
              , restart_below_severity_(restart_below_severity)
              , restart_below_severity_level_(restart_below_severity_level) {}

            bool restart_by_beaconing_;
            std::chrono::microseconds max_beacon_response_time_;
            bool restart_by_severity_;
            std::string restart_below_severity_;
            std::string restart_below_severity_level_;
          };

          explicit Task(unsigned int id) 
            : id_(id) {}
          Task()
          {
            std::random_device rd;
            std::mt19937 generator(rd());
            std::uniform_int_distribution<unsigned int> distribution(1, UINT_MAX);
            id_ = distribution(generator);
          }
          Task(const Task& task)
            : name_(task.name_)
            , start_stop_(task.start_stop_.target_, task.start_stop_.runner_id_, task.start_stop_.algo_, task.start_stop_.working_dir_, task.start_stop_.launch_order_, task.start_stop_.timeout_, task.start_stop_.visibility_, task.start_stop_.additional_cmd_line_args_, task.start_stop_.do_monitor_)
            , monitoring_(task.monitoring_.restart_by_beaconing_, task.monitoring_.max_beacon_response_time_, task.monitoring_.restart_by_severity_, task.monitoring_.restart_below_severity_, task.monitoring_.restart_below_severity_level_)
            , imported_(task.imported_)
            , id_(task.id_) {}

          // overloaded operator "<" for sorting taskslist with launch_order key
          bool operator<(const Task& task_b) const
          {
            if (start_stop_.launch_order_ == task_b.start_stop_.launch_order_)
              return name_ < task_b.name_;
            else
              return start_stop_.launch_order_ < task_b.start_stop_.launch_order_;
          }

          // overloaded operator "==" . 2 tasks are considered to be equal if they have the same fields
          bool operator==(const Task& task_b) const
          {
            return (this->GetId() == task_b.GetId() &&
                    this->name_ == task_b.name_ &&
                    this->start_stop_.additional_cmd_line_args_ == task_b.start_stop_.additional_cmd_line_args_ &&
                    this->start_stop_.algo_ == task_b.start_stop_.algo_ &&
                    this->start_stop_.working_dir_ == task_b.start_stop_.working_dir_ &&
                    this->start_stop_.launch_order_ == task_b.start_stop_.launch_order_ &&
                    this->start_stop_.runner_id_ == task_b.start_stop_.runner_id_ &&
                    this->start_stop_.target_ == task_b.start_stop_.target_ &&
                    this->start_stop_.timeout_ == task_b.start_stop_.timeout_ &&
                    this->start_stop_.visibility_ == task_b.start_stop_.visibility_ &&
                    this->start_stop_.do_monitor_ == task_b.start_stop_.do_monitor_ &&
                    this->monitoring_.restart_by_beaconing_ == task_b.monitoring_.restart_by_beaconing_ &&
                    this->monitoring_.max_beacon_response_time_ == task_b.monitoring_.max_beacon_response_time_ &&
                    this->monitoring_.restart_by_severity_ == task_b.monitoring_.restart_by_severity_ &&
                    this->monitoring_.restart_below_severity_ == task_b.monitoring_.restart_below_severity_ &&
                    this->monitoring_.restart_below_severity_level_ == task_b.monitoring_.restart_below_severity_level_ &&
                    this->imported_ == task_b.imported_);
          }

          // overloaded operator "="
          void operator=(const Task& task_b)
          {
            this->id_ = task_b.id_;
            this->name_ = task_b.name_;
            this->start_stop_.additional_cmd_line_args_ = task_b.start_stop_.additional_cmd_line_args_;
            this->start_stop_.algo_ = task_b.start_stop_.algo_;
            this->start_stop_.working_dir_ = task_b.start_stop_.working_dir_;
            this->start_stop_.launch_order_ = task_b.start_stop_.launch_order_;
            this->start_stop_.runner_id_ = task_b.start_stop_.runner_id_;
            this->start_stop_.target_ = task_b.start_stop_.target_;
            this->start_stop_.timeout_ = task_b.start_stop_.timeout_;
            this->start_stop_.visibility_ = task_b.start_stop_.visibility_;
            this->start_stop_.do_monitor_ = task_b.start_stop_.do_monitor_;
            this->monitoring_.restart_by_beaconing_ = task_b.monitoring_.restart_by_beaconing_;
            this->monitoring_.max_beacon_response_time_ = task_b.monitoring_.max_beacon_response_time_;
            this->monitoring_.restart_by_severity_ = task_b.monitoring_.restart_by_severity_;
            this->monitoring_.restart_below_severity_ = task_b.monitoring_.restart_below_severity_;
            this->monitoring_.restart_below_severity_level_ = task_b.monitoring_.restart_below_severity_level_;
            this->imported_ = task_b.imported_;
          }

          std::string name_;
          CStartStop  start_stop_;
          CMonitoring monitoring_;
          bool imported_ = false;

          unsigned int GetId() const { return id_; }

         private:
          unsigned int id_;
        };

        class Function
        {
         public:
           class Task
           {
            public:
              Task() : id_(0) {}
              explicit Task(unsigned int id) : id_(id) {}

              bool operator==(const Task& task_b) const
              {
                return (this->GetId() == task_b.GetId());
              }

              bool operator<(const Task& task_b) const
              {
                return (this->GetId() < task_b.GetId());
              }

              unsigned int GetId() const { return id_; }

            private:
              unsigned int id_;
           };
          class State
          {
           public:
            class Task
            {
             public:
              Task() : id_(0) {}
              Task(unsigned int id, const std::string& severity, const std::string& severity_level)
                : severity_(severity)
                , severity_level_(severity_level)
                , id_(id){}

              bool operator==(const Task& task_b) const
              {
                return (this->GetId() == task_b.GetId() &&
                        this->severity_ == task_b.severity_ &&
                        this->severity_level_ == task_b.severity_level_);
              }

              bool operator<(const Task& task_b) const
              {
                return (this->GetId() < task_b.GetId());
              }

              std::string severity_;
              std::string severity_level_;
              unsigned int GetId() const { return id_; }

             private:
              unsigned int id_;
            };

            typedef std::list<Task> TaskList;

            explicit State(unsigned int id) : prio_(0), id_(id) {}
            State()
            {
              std::random_device rd;
              std::mt19937 generator(rd());
              std::uniform_int_distribution<unsigned int> distribution(1, UINT_MAX);
              id_ = distribution(generator);
              prio_ = 0;
            }
            State(const State& state)
              : name_(state.name_)
              , prio_(state.prio_)
              , colour_(state.colour_)
              , tasks_(state.tasks_)
              , id_(state.id_) {}

            bool operator==(const State& state_b) const
            {
              return (this->GetId() == state_b.GetId() &&
                      this->name_ == state_b.name_ &&
                      this->prio_ == state_b.prio_ &&
                      this->colour_ == state_b.colour_ &&
                      this->tasks_.size() == state_b.tasks_.size());
            }

            void operator=(const State& state_b)
            {
              if (this == &state_b) return;
              this->id_ = state_b.GetId();
              this->name_ = state_b.name_;
              this->prio_ = state_b.prio_;
              this->colour_ = state_b.colour_;
              this->tasks_.clear();
              for (const auto& task : state_b.tasks_)
              {
                this->tasks_.push_back(task);
              }
            }

            // overloaded operator "<" for sorting state list with priority key
            bool operator<(const State& state_b) const
            {
              return (this->prio_ < state_b.prio_);
            }

            std::string name_;
            unsigned int prio_;
            std::string colour_;
            TaskList tasks_;
            unsigned int GetId() const { return id_; }

           private:
            unsigned int id_;
          };

          typedef std::list<Task> TaskList;
          typedef std::list<State> StateList;

          explicit Function(unsigned int id) 
            : id_(id) {}
          Function()
          {
            std::random_device rd;
            std::mt19937 generator(rd());
            std::uniform_int_distribution<unsigned int> distribution(1, UINT_MAX);
            id_ = distribution(generator);
          }
          Function(const Function& function)
            : name_(function.name_)
            , tasks_(function.tasks_)
            , states_(function.states_)
            , imported_(function.imported_)
            , id_(function.id_) {}

          bool operator<(const Function& rhs) const
          {
            return std::lexicographical_compare(name_.begin(), name_.end(), rhs.name_.begin(), rhs.name_.end());
          }

          bool operator==(const Function& function_b) const
          {
            return (this->GetId() == function_b.GetId() &&
                    this->name_ == function_b.name_ &&
                    this->tasks_.size() == function_b.tasks_.size() &&
                    this->states_.size() == function_b.states_.size() &&
                    this->imported_ == function_b.imported_);
          }

          void operator=(const Function& function_b)
          {
            if (this == &function_b) return;
            this->id_ = function_b.id_;
            this->name_ = function_b.name_;
            this->tasks_.clear();
            for (const auto& task : function_b.tasks_)
            {
              this->tasks_.push_back(task);
            }
            this->states_.clear();
            for (const auto& state : function_b.states_)
            {
              this->states_.push_back(state);
            }
            this->imported_ = function_b.imported_;
          }

          std::string name_;
          TaskList tasks_;
          StateList states_;
          bool imported_ = false;

          unsigned int GetId() const { return id_; }

         private:
          unsigned int id_;
        };

        class Options
        {
         public:
           Options() {}
           Options(bool all_targets_reachable, bool use_only_local, bool use_all_on_this_host, bool stop_all_on_close)
             : all_targets_reachable_(all_targets_reachable)
             , use_only_local_host_(use_only_local)
             , use_all_on_this_host_(use_all_on_this_host)
             , stop_all_on_close_(stop_all_on_close) {}

           bool all_targets_reachable_ = true;
           bool use_only_local_host_ = false;
           bool use_all_on_this_host_ = false;
           bool stop_all_on_close_ = false;
        };

        typedef std::list<Target>   TargetList;
        typedef std::list<Runner>   RunnerList;
        typedef std::list<Task>     TaskList;
        typedef std::list<Function> FunctionList;

        TargetList    targets_;
        RunnerList    runners_;
        TaskList      tasks_;
        FunctionList  functions_;

        void SetLayout(const std::string& layout, const Size& window_size, const Point& window_position, bool window_maximized)
        {
          layout_ = layout;
          window_position_ = window_position;
          window_size_ = window_size;
          window_maximized_ = window_maximized;
        }

        void SetOptions(const Options& options) { options_ = options; }

        std::string GetLayout() const { return layout_; }
        Size        GetWindowSize() const { return window_size_; }
        Point       GetWindowPosition() const { return window_position_; }
        bool        GetIsWindowMaximized() const { return window_maximized_; }
        Options     GetOptions() const { return options_; }

        StringList  GetImportedConfigs() const { return imported_configs_; }
        void        AddImportedConfig(const std::string& config_path) { imported_configs_.push_back(config_path); }

       private:
        std::string layout_;
        Size        window_size_;
        Point       window_position_;
        bool        window_maximized_;
        Options     options_;
        StringList  imported_configs_;
      };
    }   // namespace Config
  }   // namespace Sys
}  // namespace eCAL
