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

#include <EcalParser/Function.h>

namespace EcalParser
{
  class FunctionUsername : public Function
  {
    std::string Evaluate         (const std::string& parameters, std::chrono::system_clock::time_point time) const override;

    std::string Description      () const override;

    std::string ParameterUsage   () const override;
    std::string ParameterExample () const override;

    std::string HtmlDocumentation() const override;
  };
}