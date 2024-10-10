#ifndef VARLOGGER_H
#define VARLOGGER_H

#include <Arduino.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>

class VarLogger {
public:
    static const int TRACE_LENGTH = 500;
    static int buffer_select;                                                               // 1 for data1, 2 for data2
    static int save_buffer;                                                                 // 1 for data1, 2 for data2, 0 for none
    static int buffer_index;
    static std::vector<std::pair<int, unsigned long>> buffer1;
    static std::vector<std::pair<int, unsigned long>> buffer2;

    static unsigned long created_timestamp;
    static unsigned long time_to_write;                                                     // Time to write them into Flash memory
    static int _write_count;
    static int prev1_event, prev2_event;
    static unsigned long prev1_time, prev2_time;

    static int data1[TRACE_LENGTH][2];                                                      // 2D array to store variable sequence as [event, time]
    static int data2[TRACE_LENGTH][2];

    static std::unordered_map<std::string, int> _vardict; // Simplified dictionary for mapping variables
    static int num_vars;

    static void init();
    static void log(const char* var, const char* fun, const char* clas, const char* th, int val = 0, bool save = false);
    static void print_buffers();


private:
    static int var2int(const std::string& var);
    static void log_seq(int event, unsigned long log_time);
    static void write_data();
    static std::string int2var(int num);

};

#endif
