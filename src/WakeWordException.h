/**
 * Copyright 2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License"). You may not use this file 
 * except in compliance with the License. A copy of the License is located at
 *
 *   http://aws.amazon.com/asl/
 *
 * or in the "license" file accompanying this file. This file is distributed on an "AS IS" 
 * BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, express or implied. See the License 
 * for the specific language governing permissions and limitations under the License.
 */

#ifndef ALEXA_VS_WAKE_WORD_WAKEWORD_EXCEPTION_H_
#define ALEXA_VS_WAKE_WORD_WAKEWORD_EXCEPTION_H_

#include <string>

namespace AlexaWakeWord {

// Our project-specific exception class.
class WakeWordException: public std::exception {

public:

  explicit WakeWordException(const std::string& msg);
  virtual const char* what() const throw ();
  ~WakeWordException()throw(){
  }

private:
  std::string m_msg;
};

} /* namespace AlexaWakeWord */

#endif /* ALEXA_VS_WAKE_WORD_WAKEWORD_EXCEPTION_H_ */
