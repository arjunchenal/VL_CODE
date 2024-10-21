#include "VarLogger.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <sstream>
#include <ArduinoJson.h>
#include <SD.h>
#include <SPI.h>
#include <mutex>

int VarLogger::buffer_select = 1;
int VarLogger::save_buffer = 0;
int VarLogger::buffer_index = 0;
unsigned long VarLogger::created_timestamp = millis();
unsigned long VarLogger::time_to_write = 0;
std::map<int, std::vector<std::pair<unsigned long, int>>> VarLogger::data_dict;
std::map<std::string, int> VarLogger::_vardict;
std::map<std::string, int> VarLogger::_thread_map;
std::map<std::string, std::string> VarLogger::threads_info;

std::vector<std::pair<int, unsigned long>> VarLogger::data1(TRACE_LENGTH, {0, 0});
std::vector<std::pair<int, unsigned long>> VarLogger::data2(TRACE_LENGTH, {0, 0});

int VarLogger::prev1_event = -1;
int VarLogger::prev2_event = -1;
unsigned long VarLogger::prev1_time = 0;
unsigned long VarLogger::prev2_time = 0;
int VarLogger::_write_count = 0;
std::string VarLogger::write_name = "log0";
std::string VarLogger::trace_name = "trace0";
int VarLogger::cur_file = 0;

std::mutex buffer_mutex;
const unsigned long flushInterval = 5000;
unsigned long last_flush_time = millis();
bool VarLogger::sdInitialized = false;

bool VarLogger::initializeSDCard(int csPin) {
    if (!SD.begin(csPin)) {
        Serial.println("SD Card initialization failed!");
        sdInitialized = false;
        return false;
    }
    Serial.println("SD Card initialized successfully.");
    sdInitialized = true;
    return true;
}

void VarLogger::log(std::string var, std::string fun, std::string clas, std::string th, int val, bool save) {
    unsigned long log_time = millis() - created_timestamp - time_to_write;
    int event_num = _var2int(th + "-" + clas + "-" + fun + "-" + var);

    std::lock_guard<std::mutex> lock(buffer_mutex);

    if (prev1_event != event_num) {
        logSeq(event_num, log_time);
        _write_count++;
    }

    if (_write_count >= TRACE_LENGTH && save != false) {
        _write_count = 0;
        unsigned long start_time = millis();
        writeData();
        time_to_write += millis() - start_time;
        data1.clear();
    }

    if(millis() - last_flush_time >= flushInterval) {
      flush();
      last_flush_time = millis();
    }

    prev2_event = prev1_event;
    prev1_event = event_num;
    prev2_time = prev1_time;
    prev1_time = log_time;
}

int VarLogger::_var2int(std::string var) {
    if (_vardict.find(var) == _vardict.end()) {
        _vardict[var] = _vardict.size();
    }
    return _vardict[var];
}

std::string VarLogger::_int2var(int num) {
    for (auto &it : _vardict) {
        if (it.second == num) {
            return it.first;
        }
    }
    return "";
}

void VarLogger::logSeq(int event, unsigned long log_time) {
    if (buffer_select == 1) {
        if (buffer_index == TRACE_LENGTH - 1) {
            data1[buffer_index] = {event, log_time};
            buffer_select = 2;
            save_buffer = 1;
            buffer_index = 0;
        } else {
            data1[buffer_index++] = {event, log_time};
        }
    } else {
        if (buffer_index == TRACE_LENGTH - 1) {
            data2[buffer_index] = {event, log_time};
            buffer_select = 1;
            save_buffer = 2;
            buffer_index = 0;
        } else {
            data2[buffer_index++] = {event, log_time};
        }
    }
}

void VarLogger::generateFileNames() {
    while (SD.exists("/trace" + String(cur_file) + ".txt")) {
        cur_file++;
    }
    trace_name = "trace" + String(cur_file);
    write_name = "log" + String(cur_file);
}


void VarLogger::writeData() {
    if (!sdInitialized) {
        Serial.println("SD Card not initialized!");
        return;
    }

    int retries = 3;  
    bool writeSuccess = false;

    std::lock_guard<std::mutex> lock(buffer_mutex);

    while(retries > 0 && !writeSuccess) {
      File traceFile = SD.open("/" + trace_name + ".txt", FILE_WRITE);
      if (traceFile) {
          StaticJsonDocument<1024> jsonDoc;
          if (save_buffer == 1) {
              for (auto &entry : data1) {
                  JsonArray array = jsonDoc.createNestedArray();
                  array.add(entry.first);
                  array.add(entry.second);
              }
              serializeJson(jsonDoc, traceFile);
              save_buffer = 0;
          } else if (save_buffer == 2) {
              for (auto &entry : data2) {
                  JsonArray array = jsonDoc.createNestedArray();
                  array.add(entry.first);
                  array.add(entry.second);
              }
              serializeJson(jsonDoc, traceFile);
              save_buffer = 0;
          }
          traceFile.close();
          writeSuccess = true;
      } else {
        Serial.println("Error writing trace file, retrying...");
        retries--;
        delay(100);
      }
    }

    if(!writeSuccess) {
      Serial.println("Failed to write trace file after 3 attempts");
      return;
    }

    retries = 3;
    writeSuccess = false;

    while(retries > 0 && !writeSuccess) {
      File varFile = SD.open("/varlist" + std::string(cur_file) + ".txt", FILE_WRITE);
      if (varFile) {
          StaticJsonDocument<1024> jsonDoc;
          for (auto &entry : _vardict) {
              jsonDoc[entry.first] = entry.second;
          }
          serializeJson(jsonDoc, varFile);
          varFile.close();
          writeSuccess = true;
      } else {
        Serial.println("Error writing varlist file, retrying...");
        retries--;
        delay(100);
      }
    }

    if(!writeSuccess) {
      Serial.println("Failed to write varlist file after 3 attempts.");
      return;
    }
    cur_file++;
    generateFileNames(); //For getting the next trace and log file names
}

void VarLogger::flush() {
  writeData();
}

void VarLogger::save() {
    writeData();
}

void VarLogger::threadStatus(std::string thread_id, std::string status) {
    if (status != "" && thread_id != "") {
        threads_info[thread_id] = status;
        if (_thread_map.find(thread_id) == _thread_map.end()) {
            _thread_map[thread_id] = _thread_map.size();
        }
    }
}

int VarLogger::mapThread(std::string thread_id) {
    if (_thread_map.find(thread_id) == _thread_map.end()) {
        return -1; 
    }
    return _thread_map[thread_id];
}

void VarLogger::traceback(std::string exc) {
    if (!SD.begin()) {
        return;
    }

    File traceFile = SD.open("/traceback.txt", FILE_APPEND);
    if (traceFile) {
        traceFile.println(std::string(millis()) + ": Exception - " + exc);
        traceFile.close();
    }
}

