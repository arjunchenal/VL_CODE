#ifndef VARLOGGER_H
#define VARLOGGER_H

#include <Arduino.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <SD.h>
#include <SPI.h>
#include <map>
#include <mutex>

class VarLogger {
public:
    static const int TRACE_LENGTH = 500;
    static int buffer_select;                                                               // 1 for data1, 2 for data2
    static int save_buffer;                                                                 // 1 for data1, 2 for data2, 0 for none
    static int buffer_index;
    static unsigned long created_timestamp;
    static unsigned long time_to_write;                                                     // Time to write them into Flash memory

    static std::map<int, std::vector<std::pair<unsigned long, int>>> data_dict;
    static std::map<std::string, int> _vardict;
    static std::map<std::string, int> _thread_map;
    static std::map<std::string, std::string> threads_info;    

    static std::vector<std::pair<int, unsigned long>> data1;
    static std::vector<std::pair<int, unsigned long>> data2;

    static int prev1_event, prev2_event;
    static unsigned long prev1_time, prev2_time;

    static int _write_count;
    static std::string write_name;
    static std::string trace_name;
    static int cur_file;
    static bool sdInitialized;

    static bool sd_initialized(int csPin);
    static void init();
    static void log(const char* var, const char* fun, const char* clas, const char* th, int val, bool save);
    static void save();

    //For thread
    static void traceback(std::string exc);
    static void threadStatus(std::string thread_id, std::string status);
    static int mapThread(std::string thread_id);


private:
    static int _var2int(const std::string& var);
    static void log_seq(int event, unsigned long log_time);
    static void write_data();
    static std::string int2var(int num);
    static void generateFileNames();
    static void flush();

};

#endif
