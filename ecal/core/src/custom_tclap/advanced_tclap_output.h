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

#pragma once

#include <tclap/StdOutput.h>

#include <ostream>
#include <vector>
#include <set>

namespace CustomTclap
{
  /**
   * @brief The AdvancedTclapOutput is similar to the TCLAP::StdOutput but provides some additional options.
   *
   * The AdvancedTclapOutput generates output similar to the TCLAP::StdOutput.
   * It however exposes some internal parameters to make them user-defined and
   * adds additional functionality.
   *
   * The features are:
   *
   *     - The maximum line-width can be set instead of being fixed to 75
   *
   *     - The output stream (or multiple streams) can be set. When just
   *       providing &std::cout, it behaves similar to the TCLAP::StdOutput.
   *       When providing e.g. an std::stringstream, the string output of this
   *       class (when printing help or version) can be used by the user code
   *       and e.g. displayed in a GUI.
   *
   *     - When printing the --help or --version, the programm is not killed any
   *       more but continues running. Of course the user code can still kill
   *       the program if desired.
   *
   *     - Arguments can be hidden from the output, e.g. in cases where
   *       dreprecated parameters are still supposed to exist but shall not
   *       appear in the help any more.
   */
  class AdvancedTclapOutput : public TCLAP::StdOutput
  {
  public:
    /**
     * @brief Creates an AdvancedTclapOutput that will publish its output on multiple streams
     * @param output_streams A vector of streams to publish the output to
     * @param max_width      The maximum line width of the output
     */
    AdvancedTclapOutput(const std::vector<std::ostream*>& output_streams, int max_width);

    /**
     * @brief Creates an AdvancedTclapOutput that will publish its output to the given stream
     * @param output_stream The stream to publish the output to (e.g. &std::cout)
     * @param max_width     The maximum line width of the output
     */
    AdvancedTclapOutput(std::ostream* output_stream, int max_width);

    virtual void version(TCLAP::CmdLineInterface &cmd) override;
    virtual void usage(TCLAP::CmdLineInterface &cmd) override;
    virtual void failure(TCLAP::CmdLineInterface &cmd, TCLAP::ArgException &e) override;

    /**
     * @brief Hides / un-hides an argument
     *
     * Hidden arguments do not appear in the help or usage output.
     *
     * @param argument The argument to hide
     * @param hidden   Whether the argument shall be hidden
     */
    void setArgumentHidden(TCLAP::Arg* argument, bool hidden);

  protected:
    virtual void shortUsage(TCLAP::CmdLineInterface& cmd, std::ostream& os) const;
    virtual void longUsage(TCLAP::CmdLineInterface& cmd, std::ostream& os) const;

  private:
    std::vector<std::vector<TCLAP::Arg*>> createXorListWithoutHiddenArgs(const std::vector<std::vector<TCLAP::Arg*>>& xor_list) const;

  private:
    std::vector<std::ostream*> output_streams_;
    int max_width_;
    std::set<TCLAP::Arg*> hidden_arguments_;
  };
}
