#ifndef __STR_UTIL_H__
#define __STR_UTIL_H__

#include "basic_head.h"
#include "byte_buffer.h"

namespace my_utils {

class StrBuffer {
public:
    StrBuffer(ByteBuffer &buff);
    ~StrBuffer(void);

    vector<ByteBuffer> split(ByteBuffer &buff);
    ByteBuffer remove_substr(ByteBuffer &buff);

private:
    std::pair<ByteBuffer_Iterator, ByteBuffer_Iterator> match_sub(ByteBuffer_Iterator start_pos, ByteBuffer &buff);
private:
    ByteBuffer buffer_;
};

}

#endif