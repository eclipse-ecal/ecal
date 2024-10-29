/* =========================== LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * =========================== LICENSE =================================
 */

#pragma once

#include <functional>

namespace eCAL
{
    template<typename T>
    class Setting
    {
        private:
            enum class Setting_state
            {
                initialized,
                modified,
                valid,
                invalid
            };

            T                         m_value;
            Setting_state             m_state;
            std::function<bool(void)> m_validation;
        
        public:
            Setting() = delete;
            Setting(const T& value_) : m_value(value_), m_state(Setting_state::initialized) {}

            Setting& operator=(const Setting& other_)
            {
                switch (other_.m_state)
                {
                    case Setting_state::initialized:
                        break;
                    case Setting_state::modified:
                    {
                        m_value = other_.m_value;
                        m_state = Setting_state::modified;
                        break;
                    }
                    case Setting_state::valid:
                        throw std::runtime_error("Trying to override already validated setting.");
                        break;
                    default:
                        break;
                }
                return *this;
            }

            Setting& operator=(const T& value_)
            {
                switch (m_state)
                {
                    case Setting_state::initialized:
                        m_state = Setting_state::modified;
                    case Setting_state::modified:
                    case Setting_state::invalid:
                        m_value = value_;
                        break;
                    case Setting_state::valid:
                        throw std::runtime_error("Trying to override already validated setting.");
                        break;
                    default:
                        break;

                }
                return *this;
            }

            operator T() const 
            {
                return m_value;
            }

            friend std::ostream& operator<<(std::ostream& os_, const Setting<T>& setting_)
            {
                os_ << setting_.m_value;
                return os_;
            }

            void setValidationFunction(std::function<bool()> validationFunc)
            {
                m_validation = validationFunc;
            }

            bool validate()
            {
                if (m_validation)
                {
                  if (m_validation())
                  {
                    m_state = Setting_state::valid;
                    return true;
                  }
                }

                m_state = Setting_state::invalid;
                return false;
            }

            bool isValid() 
            {
                return m_state == Setting_state::valid ? true : false;
            }
    };
}