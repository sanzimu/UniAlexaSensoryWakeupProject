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

#include "SensoryWakeWordEngine.h"
#include <iostream>
#include "Logger.h"
#include "WakeWordException.h"
#include <stdio.h>
#define PRINT_RESULT(fp,beign,end,word)do {fprintf(fp,"%.5f\t%.5f\t%s\n",begin,end,word);}while(0)
const std::string ALEXA_TASK_VERSION = "~0.7.0";
const std::string MODEL_FILE = "../models/spot-alexa-rpi-31000.snsr";
FILE* result_file_pointer=NULL;
namespace AlexaWakeWord {

// Callback for whenever audio samples have been read from the input stream
// and are about to be processed.
// This callback exists to allow sensory to gracefully exit when we wish
// to end the thread.
SnsrRC samplesReadySensoryCallback(SnsrSession s, const char* key,
                                   void* userData) {
  SensoryWakeWordEngine* engine = static_cast<SensoryWakeWordEngine*>(userData);
  if (!engine->isRunning()) {
    return SNSR_RC_INTERRUPTED;
  }
  return SNSR_RC_OK;
}

// Callback for when the wakeword is detected
SnsrRC wakeWordDetectedSensoryCallback(SnsrSession s, const char* key,
                                       void* userData) {
  log(Logger::DEBUG, " *** Wakeword Detected ***");
  //SensoryWakeWordEngine* engine = static_cast<SensoryWakeWordEngine*>(userData);
  double begin;
  double end;
  const char* wakeword = NULL;
  snsrGetDouble(s, SNSR_RES_BEGIN_MS, &begin);
  snsrGetDouble(s, SNSR_RES_END_MS, &end);
  snsrGetString(s, SNSR_RES_TEXT, &wakeword);
  /*std::cout << begin / 1000 << "\t" << end / 1000 << "\t" << wakeword
            << std::endl;*/
  begin=begin/1000;
  end=end/1000;
  PRINT_RESULT(result_file_pointer,begin,end,wakeword);
  return SNSR_RC_OK;
}

// Returns information about the ongoing sensory session.
// Primarily used to populate error messages.
std::string getSensoryDetails(SnsrSession session, SnsrRC result) {
  std::string message;

  if (session) {
    message = snsrErrorDetail(session);
  } else {
    message = snsrRCMessage(result);
  }
  if ("" == message) {
    message = "Unrecognized error";
  }
  return message;
}

SensoryWakeWordEngine::SensoryWakeWordEngine(const char* filename)
    : m_isRunning{false}, m_session{nullptr} {
  try {
    input_wavefile=filename;
    char file_path[256]={0};
    sprintf(file_path,"%s_result.wkr",filename);
    result_file_pointer=fopen(file_path,"w");
    init();
    start();
    fclose(result_file_pointer);
    result_file_pointer=NULL;
  } catch (std::bad_alloc& e) {
    log(Logger::ERROR, "SensoryWakeWordEngine: Failed to allocate memory");
    throw;
  } catch (WakeWordException& e) {
    log(Logger::ERROR,
        std::string("SensoryWakeWordEngine: Initialization error:") + e.what());
    throw;
  }
}

SensoryWakeWordEngine::~SensoryWakeWordEngine() {
  stop();
  snsrRelease(m_session);
  m_session = nullptr;
}

void SensoryWakeWordEngine::pause() {
  log(Logger::INFO, "SensoryWakeWordEngine: handling pause");
  stop();
}

void SensoryWakeWordEngine::resume() {
  log(Logger::INFO, "SensoryWakeWordEngine: handling resume");

  try {
    start();
  } catch (std::bad_alloc& e) {
    log(Logger::ERROR, "SensoryWakeWordEngine: Failed to allocate memory");
    throw;
  } catch (WakeWordException& e) {
    log(Logger::ERROR,
        std::string("SensoryWakeWordEngine: Initialization error:") + e.what());
    throw;
  }
}

bool SensoryWakeWordEngine::isRunning() { return m_isRunning; }

// void SensoryWakeWordEngine::callWakeWordDetected() { wakeWordDetected(); }

void SensoryWakeWordEngine::init() {
  if (m_session) {
    log(Logger::INFO, "SensoryWakeWordEngine already initialized");
    return;
  }

  log(Logger::INFO, std::string("Initializing Sensory library") +
                        " | library name: " + SNSR_NAME +
                        " | library version: " + SNSR_VERSION +
                        " | model file: " + MODEL_FILE);

  // Allocate the Sensory library handle
  SnsrRC result = snsrNew(&m_session);
  if (result != SNSR_RC_OK) {
    throw WakeWordException("Could not allocation a new Sensory session: " +
                            getSensoryDetails(m_session, result));
  }

  // Get the expiration date of the library
  const char* info = nullptr;
  result = snsrGetString(m_session, SNSR_LICENSE_EXPIRES, &info);
  if (result == SNSR_RC_OK && info) {
    log(Logger::WARNING, "Library expires on: " + std::string(info));
  } else {
    log(Logger::DEBUG, "Library does not expire");
  }

  // Check if the expiration date is near, then we should display a warning
  result = snsrGetString(m_session, SNSR_LICENSE_WARNING, &info);
  if (result == SNSR_RC_OK && info) {
    log(Logger::WARNING, "Library expires in: " + std::string(info));
  } else {
    log(Logger::DEBUG, "Library expiration is valid for at least 60 days.");
  }

  if (snsrLoad(m_session, snsrStreamFromFileName(MODEL_FILE.c_str(), "r")) !=
          SNSR_RC_OK ||
      snsrRequire(m_session, SNSR_TASK_TYPE, SNSR_PHRASESPOT) != SNSR_RC_OK ||
      snsrRequire(m_session, SNSR_TASK_VERSION, ALEXA_TASK_VERSION.c_str()) !=
          SNSR_RC_OK) {
    throw WakeWordException("Could not load and configure Sensory model: " +
                            getSensoryDetails(m_session, result));
  }

  result = snsrSetHandler(m_session, SNSR_SAMPLES_EVENT,
                          snsrCallback(samplesReadySensoryCallback, nullptr,
                                       reinterpret_cast<void*>(this)));
  if (result != SNSR_RC_OK) {
    throw WakeWordException("Could not set audio samples callback: " +
                            getSensoryDetails(m_session, result));
  }
  result = snsrSetHandler(m_session, SNSR_RESULT_EVENT,
                          snsrCallback(wakeWordDetectedSensoryCallback, nullptr,
                                       reinterpret_cast<void*>(this)));
  if (result != SNSR_RC_OK) {
    throw WakeWordException("Could not set wake word detected callback: " +
                            getSensoryDetails(m_session, result));
  }
}

void SensoryWakeWordEngine::start() {
  if (m_isRunning) {
    log(Logger::INFO, "SensoryWakeWordEngine already started");
    return;
  }
  log(Logger::DEBUG, "SensoryWakeWordEngine: starting");

  // Associate the microphone input with Sensory session.
  SnsrRC result = snsrSetStream(m_session, SNSR_SOURCE_AUDIO_PCM,
                                snsrStreamFromFileName(input_wavefile, "r"));

  if (result != SNSR_RC_OK) {
    throw WakeWordException("Could not set audio stream from default mic: " +
                            getSensoryDetails(m_session, result));
  }

  m_isRunning = true;
  log(Logger::INFO, "SensoryWakeWordEngine: mainLoop thread started");

  result = snsrRun(m_session);
  if (result != SNSR_RC_OK && result != SNSR_RC_STREAM_END) {
    log(Logger::ERROR, "An error happened in the mainLoop of SensoryWakeWord " +
                           getSensoryDetails(m_session, result));
  }

  log(Logger::INFO, "SensoryWakeWordEngine: mainLoop thread ended");
}

void SensoryWakeWordEngine::stop() {
  if (!m_isRunning) {
    log(Logger::INFO, "SensoryWakeWordEngine already stopped");
    return;
  }
  log(Logger::INFO, " *** THREAD JOINING: Sensory ***");

  m_isRunning = false;
  // m_thread->join();
  snsrClearRC(m_session);
  snsrSetStream(m_session, SNSR_SOURCE_AUDIO_PCM, nullptr);
}

// void SensoryWakeWordEngine::mainLoop() {}

}  // namespace AlexaWakeWord
