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

#include <custom_tclap/advanced_tclap_output.h>

#include <sstream>

namespace CustomTclap
{
  AdvancedTclapOutput::AdvancedTclapOutput(const std::vector<std::ostream*>& output_streams, int max_width)
    : output_streams_(output_streams)
    , max_width_(max_width)
  {}

  AdvancedTclapOutput::AdvancedTclapOutput(std::ostream* output_stream, int max_width)
    : AdvancedTclapOutput(std::vector<std::ostream*>{output_stream}, max_width)
  {}

  void AdvancedTclapOutput::version(TCLAP::CmdLineInterface &cmd)
  {
    std::string progName = cmd.getProgramName();
    std::string xversion = cmd.getVersion();

    std::stringstream ss;

    // Create string
    ss << std::endl << progName << "  version: "
      << xversion << std::endl << std::endl;


    // publish string to all output streams
    for (std::ostream* output_stream : output_streams_)
    {
      (*output_stream) << ss.str();
    }
  }

  void AdvancedTclapOutput::usage(TCLAP::CmdLineInterface &cmd)
  {
    std::stringstream ss;

    // Create string
    ss << std::endl << "USAGE: " << std::endl << std::endl;
    shortUsage(cmd, ss);
    ss << std::endl << std::endl << "Where: " << std::endl << std::endl;
    longUsage(cmd, ss);
    ss << std::endl;

    // publish string to all output streams
    for (std::ostream* output_stream : output_streams_)
    {
      (*output_stream) << ss.str();
    }
  }

  void AdvancedTclapOutput::failure(TCLAP::CmdLineInterface &cmd, TCLAP::ArgException &e)
  {
    std::stringstream ss;

    // Create string

    std::string progName = cmd.getProgramName();

    ss << "PARSE ERROR: " << e.argId() << std::endl
      << "             " << e.error() << std::endl << std::endl;

    if (cmd.hasHelpAndVersion())
    {
      ss << "Brief USAGE: " << std::endl;

      shortUsage(cmd, ss);

      ss << std::endl << "For complete USAGE and HELP type: "
        << std::endl << "   " << progName << " --help"
        << std::endl << std::endl;
    }
    else
    {
      usage(cmd);
    }

    // publish string to all output streams
    for (std::ostream* output_stream : output_streams_)
    {
      (*output_stream) << ss.str();
    }


    //throw TCLAP::ExitException(1);

  }

  void AdvancedTclapOutput::setArgumentHidden(TCLAP::Arg* argument, bool hidden)
  {
    if (hidden)
      hidden_arguments_.emplace(argument);
    else
      hidden_arguments_.erase(argument);
  }

  void AdvancedTclapOutput::shortUsage(TCLAP::CmdLineInterface& cmd, std::ostream& os) const
  {
    std::list<TCLAP::Arg*>                arg_list = cmd.getArgList();
    std::string                           prog_name = cmd.getProgramName();
    TCLAP::XorHandler                     xor_handler = cmd.getXorHandler();
    std::vector<std::vector<TCLAP::Arg*>> xor_list = xor_handler.getXorList();

    std::string s = prog_name + " ";

    // first the xor
    xor_list = createXorListWithoutHiddenArgs(xor_list);
    for (int i = 0; static_cast<size_t>(i) < xor_list.size(); i++)
    {
      s += " {";
      for (TCLAP::ArgVectorIterator it = xor_list[i].begin(); it != xor_list[i].end(); ++it)
      {
        s += (*it)->shortID() + "|";
      }

      s[s.length() - 1] = '}';
    }

    // then the rest
    for (TCLAP::ArgListIterator it = arg_list.begin(); it != arg_list.end(); ++it)
    {
      if (!xor_handler.contains((*it))
        && (hidden_arguments_.find(*it) == hidden_arguments_.end()))
      {
        s += " " + (*it)->shortID();
      }
    }

    // if the program name is too long, then adjust the second line offset 
    int second_line_offset = static_cast<int>(prog_name.length()) + 2;
    if (second_line_offset > max_width_ / 3)
    {
      second_line_offset = static_cast<int>(max_width_ / 3);
    }

    spacePrint(os, s, max_width_, 3, second_line_offset);
  }

  void AdvancedTclapOutput::longUsage(TCLAP::CmdLineInterface& cmd, std::ostream& os) const
  {
    std::list<TCLAP::Arg*>                arg_list = cmd.getArgList();
    std::string                           message = cmd.getMessage();
    TCLAP::XorHandler                     xor_handler = cmd.getXorHandler();
    std::vector<std::vector<TCLAP::Arg*>> xor_list = xor_handler.getXorList();

    xor_list = createXorListWithoutHiddenArgs(xor_list);

    // first the xor 
    for (int i = 0; static_cast<unsigned int>(i) < xor_list.size(); i++)
    {
      for (TCLAP::ArgVectorIterator it = xor_list[i].begin(); it != xor_list[i].end(); ++it)
      {
        spacePrint(os, (*it)->longID(), max_width_, 3, 3);
        spacePrint(os, (*it)->getDescription(), max_width_, 5, 0);

        if (it + 1 != xor_list[i].end())
          spacePrint(os, "-- OR --", max_width_, 9, 0);
      }
      os << std::endl << std::endl;
    }

    // then the rest
    for (TCLAP::ArgListIterator it = arg_list.begin(); it != arg_list.end(); ++it)
    {
      if (!xor_handler.contains((*it))
        && (hidden_arguments_.find(*it) == hidden_arguments_.end()))
      {
        spacePrint(os, (*it)->longID(), max_width_, 3, 3);
        spacePrint(os, (*it)->getDescription(), max_width_, 5, 0);
        os << std::endl;
      }
    }

    os << std::endl;

    spacePrint(os, message, max_width_, 3, 0);
  }

  std::vector<std::vector<TCLAP::Arg*>> AdvancedTclapOutput::createXorListWithoutHiddenArgs(const std::vector<std::vector<TCLAP::Arg*>>& xor_list) const
  {
    std::vector<std::vector<TCLAP::Arg*>> cleaned_xor_list;

    // Remove hidden arguments from XOR list
    for (size_t i = 0; i < xor_list.size(); i++)
    {
      std::vector<TCLAP::Arg*> arg_list;
      for (size_t j = 0; j < xor_list[i].size(); j++)
      {
        if (hidden_arguments_.find(xor_list[i][j]) == hidden_arguments_.end())
        {
          arg_list.push_back(xor_list[i][j]);
        }
      }

      if (!arg_list.empty())
      {
        cleaned_xor_list.push_back(std::move(arg_list));
      }
    }

    return cleaned_xor_list;
  }
}