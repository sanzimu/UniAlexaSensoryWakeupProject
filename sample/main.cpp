/**
 * Copyright 2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License"). You may not use
 * this file
 * except in compliance with the License. A copy of the License is located at
 *
 *   http://aws.amazon.com/asl/
 *
 * or in the "license" file accompanying this file. This file is distributed on
 * an "AS IS"
 * BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, express or implied. See
 * the License
 * for the specific language governing permissions and limitations under the
 * License.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <string>
#include "Logger.h"
#include "SensoryWakeWordEngine.h"

using namespace AlexaWakeWord;
using namespace AlexaWakeWord::Logger;

// Change this if you want more trace.
Logger::Level LOG_DEFAULT_LEVEL = Logger::INFO;

// Forward declarations

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Useage:%s input_wavefile\n", argv[0]);
    exit(-1);
  }
  /*FILE* file_pointer = NULL;
  size_t offset = 44;
  file_pointer = fopen(argv[1], "r");
  fseek(file_pointer, 0, SEEK_END);
  size_t len = (ftell(file_pointer) - offset) / 2;
  short* buffer = new short[len];
  fseek(file_pointer, offset, SEEK_SET);
  fread(buffer, sizeof(short), len, file_pointer);
  fclose(file_pointer);
  file_pointer=NULL;*/
  // Setup our signal handling code
  const char* input_wavefile=argv[1];
  log(Logger::DEBUG, "main: Creating Sensory instance");
  SensoryWakeWordEngine Sensory = SensoryWakeWordEngine(input_wavefile);
  /*delete[] buffer;
  return 0;*/
}
