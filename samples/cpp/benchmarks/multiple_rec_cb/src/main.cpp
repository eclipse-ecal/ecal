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

#define MULTIPLE_REC        1
#define MULTIPLE_REC_VECTOR 0
#define MULTIPLE_REC_PERSON 0

#include "multiple_rec_cb.h"
#include "multiple_rec_cb_vector.h"
#include "multiple_rec_cb_person.h"

int main(int argc, char **argv)
{
#if MULTIPLE_REC
  MultipleRec(argc, argv);
#endif

#if MULTIPLE_REC_VECTOR
  MultipleRecVector(argc, argv);
#endif

#if MULTIPLE_REC_PERSON
  MultipleRecPerson(argc, argv);
#endif
  return(0);
}
