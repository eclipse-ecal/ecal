#include <sys_client_core/task.h>

#include <EcalParser/EcalParser.h>
#include <ecal_utils/filesystem.h>

namespace eCAL
{
  namespace sys_client
  {
    void EvaluateEcalParserFunctions(eCAL::sys_client::Task& task, bool eval_target_functions, std::chrono::system_clock::time_point now)
    {
      task.runner.path             = EcalParser::Evaluate(task.runner.path,             eval_target_functions, now);
      task.runner.arguments        = EcalParser::Evaluate(task.runner.arguments,        eval_target_functions, now);
      task.runner.default_task_dir = EcalParser::Evaluate(task.runner.default_task_dir, eval_target_functions, now);

      task.path        = EcalParser::Evaluate(task.path,        eval_target_functions, now);
      task.arguments   = EcalParser::Evaluate(task.arguments,   eval_target_functions, now);
      task.working_dir = EcalParser::Evaluate(task.working_dir, eval_target_functions, now);
    }

    void MergeRunnerIntoTask(eCAL::sys_client::Task& task, EcalUtils::Filesystem::OsStyle input_path_style)
    {
      std::string executable_path;
      std::string executable_arguments;
      std::string working_dir;

      if (!task.runner.path.empty())
      {
        // If the runner path is not empty, it is the executable
        executable_path = task.runner.path;

        // The task path may now be absolute or relative.
        // If it is relative, the default_task_dir from the runner is used.
        std::string task_path   = task.path;
        bool        is_absolute = EcalUtils::Filesystem::IsAbsolute(task_path, input_path_style);

        std::string task_dir = task.runner.default_task_dir;

        if (!is_absolute && !task_dir.empty())
        {
#ifdef WIN32
          if (task_dir.back() == '/' || task_dir.back() == '\\'
            || (!task_path.empty() && (task_path.front() == '/' || task_path.front() == '\\')))
#else // WIN32
          if (task_dir.back() == '/')
#endif // WIN32
          {
            task_path = task_dir + task_path;
          }
          else
          {
            task_path = task_dir + EcalUtils::Filesystem::NativeSeparator() + task_path;
          }
        }

        // The entire argument string constists of the runner arguments, the task path and the task arguments
        executable_arguments = task.runner.arguments;
        if (!task_path.empty())
        {
          if (!executable_arguments.empty())
            executable_arguments += ' ';
          executable_arguments += "\"" + task_path + "\"";
        }
        if (!task.arguments.empty())
        {
          if (!executable_arguments.empty())
            executable_arguments += ' ';
          executable_arguments += task.arguments;
        }

        // The working dir will evaluate to:
        //  - The task working dir
        //  - The runner task dir (if the task working dir is not set)
        //  - Nothing (if no runner task dir is set)

        if (!task.working_dir.empty())
        {
          working_dir = task.working_dir;
        }
        else
        {
          working_dir = task_dir;
        }
      }
      else
      {
        // If the runner path is empty, the executable is the task path.
        executable_path  = task.path;
        bool is_absolute = EcalUtils::Filesystem::IsAbsolute(executable_path, input_path_style);

        // The executable path may now be absolute or relative.
        // If it is relative, the default_task_dir from the runner is used.
        std::string task_dir = task.runner.default_task_dir;

        if (!is_absolute && !task_dir.empty())
        {
#ifdef WIN32
          if (task_dir.back() == '/' || task_dir.back() == '\\'
            || (!executable_path.empty() && (executable_path.front() == '/' || executable_path.front() == '\\')))
#else // WIN32
          if (task_dir.back() == '/')
#endif // WIN32
          {
            executable_path = task_dir + executable_path;
          }
          else
          {
            executable_path = task_dir + EcalUtils::Filesystem::NativeSeparator() + executable_path;
          }
        }


        // The argument string are the plain task arguments
        executable_arguments = task.arguments;

        // The working dir will evaluate to:
        //  - The task working dir
        //  - The runner task dir (if the task working dir is not set)
        //  - Nothing (if no runner task dir is set)

        if (!task.working_dir.empty())
        {
          working_dir = task.working_dir;
        }
        else
        {
          working_dir = task_dir;
        }
      }

      task.runner      = eCAL::sys_client::Runner();
      task.path        = executable_path;
      task.arguments   = executable_arguments;
      task.working_dir = working_dir;
    }
  }
}