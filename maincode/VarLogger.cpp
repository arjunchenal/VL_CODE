#include "VarLogger.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <cstring>


int VarLogger::buffer_select = 1;
int VarLogger::save_buffer = 0;
int VarLogger::buffer_index = 0;
std::vector<std::pair<int, unsigned long>> VarLogger::buffer1;
std::vector<std::pair<int, unsigned long>> VarLogger::buffer2;
unsigned long VarLogger::created_timestamp = 0;
unsigned long VarLogger::time_to_write = 0;
int VarLogger::prev1_event = -1;
int VarLogger::prev2_event = -1;
unsigned long VarLogger::prev1_time = 0;
unsigned long VarLogger::prev2_time = 0;
int VarLogger::data1[TRACE_LENGTH][2] = {0};
int VarLogger::data2[TRACE_LENGTH][2] = {0};
std::unordered_map<std::string, int> VarLogger::_vardict;
int VarLogger::num_vars = 0;
int VarLogger::_write_count = 0;

void VarLogger::init() {
    created_timestamp = millis();
    time_to_write = 0;
    buffer_select = 1;
    save_buffer = 0;
    buffer_index = 0;
    _write_count = 0;
    prev1_event = -1;
    prev2_event = -1;
    prev1_time = 0;
    prev2_time = 0;
    num_vars = 0;
    _vardict.clear();
}

void VarLogger::log(const char* var, const char* fun, const char* clas, const char* th, int val, bool save) {

    std::string thread_str(th);
    std::string class_str(clas);
    std::string func_str(fun);
    std::string var_str(var);

    std::stringstream event_stream;
    event_stream << thread_str << "-" << class_str << "-" << func_str << "-" << var_str;
    std::string event = event_stream.str();

    int event_num = var2int(event);
    unsigned long log_time = millis() - created_timestamp - time_to_write;

    //log the sequence to trace file, but only unique events and avoid duplicates
    if (prev1_event != event_num) {
        log_seq(event_num, log_time);
        _write_count++;
    }

    if (_write_count >= TRACE_LENGTH && !save) {
          _write_count = 0;
          //write_data();
          unsigned long start_time = millis();
          time_to_write += millis() - start_time;
          Serial.print("Write time: ");
          Serial.println(time_to_write);
          memset(data1, 0, sizeof(data1));  //garbage collection by clearing buffers setting to empty
          memset(data2, 0, sizeof(data2));

    }

    prev2_event = prev1_event;
    prev1_event = event_num;
    prev2_time = prev1_time;
    prev1_time = log_time;
}

int VarLogger::var2int(const std::string& var) {
    if(_vardict.find(var) == _vardict.end()) {      //When the item is not present in dictionary, we add them to dictionary
      _vardict[var] = _vardict.size();
    }
    return _vardict[var];
}

std::string VarLogger::int2var(int num) {
  for(const auto& val : _vardict) {
    if(val.second == num) {
      return val.first;
    }
  }
  return "";  //empty string returned if no match found
}

void VarLogger::log_seq(int event, unsigned long log_time) {
    // Serial.print("Logging event: ");
    // Serial.print(int2var(event).c_str());
    // Serial.print(" at time: ");
    // Serial.println(log_time);
    if (buffer_select == 1) {
        if (buffer_index == TRACE_LENGTH - 1) {       //Checking if buffer is full, if full then switch to buffer 2
            data1[buffer_index][0] = event;
            data1[buffer_index][1] = log_time;
            buffer_select = 2;
            save_buffer = 1;
            buffer_index = 0;
        } 
        else if (buffer_index == 0) {                 //Initializing the buffer if empty
            std::memset(data1, 0, sizeof(data1));     //allocating zeros to buffer
            size_t used_memory = sizeof(data1);
            std::cout << "Memory used by buffer1: " << used_memory << " bytes" << std::endl;
            data1[0][0] = event;
            data1[0][1] = log_time;
            buffer_index++;
        } 
        else {
            data1[buffer_index][0] = event;
            data1[buffer_index][1] = log_time;
            buffer_index++;
        }

    } else if (buffer_select == 2) {
        if (buffer_index == TRACE_LENGTH - 1) {     //Checking if buffer is full, if yes switch to buffer 1
            data2[buffer_index][0] = event;
            data2[buffer_index][1] = log_time;
            buffer_select = 1;
            save_buffer = 2;
            buffer_index = 0;
        } 
        else if (buffer_index == 0) {
            std::memset(data2, 0, sizeof(data2));
            size_t used_memory = sizeof(data2);
            std::cout << "Memory used by buffer2: " << used_memory << " bytes" << std::endl;
            data2[0][0] = event;
            data2[0][1] = log_time;
            buffer_index++;
        } 
        else {
            data2[buffer_index][0] = event;
            data2[buffer_index][1] = log_time;
            buffer_index++;
        }
    }
}

void VarLogger::print_buffers() {
    Serial.println("Data1 Buffer Contents:");
    for (int i = 0; i < TRACE_LENGTH; i++) {
        if (data1[i][0] != 0 || data1[i][1] != 0) { 
            Serial.print("Index ");
            Serial.print(i);
            Serial.print(": (");
            Serial.print(data1[i][0]);
            Serial.print(", ");
            Serial.print(data1[i][1]);
            Serial.println(")");
        }
    }

    Serial.println("Data2 Buffer Contents:");
    for (int i = 0; i < TRACE_LENGTH; i++) {
        if (data2[i][0] != 0 || data2[i][1] != 0) {  
            Serial.print("Index ");
            Serial.print(i);
            Serial.print(": (");
            Serial.print(data2[i][0]);
            Serial.print(", ");
            Serial.print(data2[i][1]);
            Serial.println(")");
        }
    }
}



// void VarLogger::write_data() {
//     // Write the data to flash
//     Serial.println("Writing data to flash...");
//     for (int i = 0; i < TRACE_LENGTH; i++) {
//         Serial.print("Data1[");
//         Serial.print(i);
//         Serial.print("]: (");
//         Serial.print(data1[i][0]);
//         Serial.print(", ");
//         Serial.println(data1[i][1]);
//     }
// }
