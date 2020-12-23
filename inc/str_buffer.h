#ifndef __STR_UTIL_H__
#define __STR_UTIL_H__

#include "basic_head.h"

namespace my_utils {

class StrBuffer {
public:
    StrBuffer(const string &str);
    ~StrBuffer(void);

    vector<string> split_str(const string& separator);
    string remove_str_from_buffer(const string& str2); // WRANNING
private:
    string str_buffer_;
    vector<string> split_strs_;
};

}

#endif