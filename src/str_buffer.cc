#include "str_buffer.h"

namespace my_util {

StrBuffer::StrBuffer(const string &str)
    :str_buffer_(str)
{}
StrBuffer::~StrBuffer(void)
{}

vector<string> 
StrBuffer::split_str(const string& separator)
{
    string store_str(str_buffer_);
    
    string::size_type pos;
    uint32_t next_search_start_pos = 0;
    while ((pos = store_str.find(separator)) != string::npos) {      // 在 store_str 中找 separator 首次出现的位置
        string tmp_str = string(store_str, 0, pos);                  // 获取 store_str 开始位置到 separator 首次出现中间的字符串
        if (tmp_str.size() > 0) {                                    // 排除 separator 出现在 store_str 开始位置的情况
            split_strs_.push_back(tmp_str);
        }
        next_search_start_pos = pos + separator.size();
        // 去除已经获取的中间字符串和上次查找的 p_separator
        if (next_search_start_pos < store_str.size()) {
            store_str = string(store_str, next_search_start_pos, store_str.size() - next_search_start_pos);
        } else {
            // 当 next_search_start_pos 大于 store_str 的长度时， 说明 store_str 中已经没有可以用的字符串了，所以将它设为空， 防止触发之后的赋值
            store_str = "";
            break;
        }
    }

    if (store_str.size() > 0) {   // 将最后剩余的字符串放入返回结果
        split_strs_.push_back(store_str);
    }

    return split_strs_;
}

string 
StrBuffer::remove_str_from_buffer(const string& p_str2)
{
    auto l_pos = str_buffer_.find(p_str2);
    if (l_pos == string::npos) {
        throw runtime_error("Can't find " + p_str2 + " in " + str_buffer_);
    }

    return string(str_buffer_, 0, l_pos);
}

}