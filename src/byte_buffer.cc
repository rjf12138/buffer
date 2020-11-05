#include "byte_buffer.h"

namespace my_util {

ByteBuffer::ByteBuffer(BUFSIZE_T size)
: start_read_pos_(0), start_write_pos_(0), used_data_size_(0)
{
    if (size <= 0)
    {
        max_buffer_size_ = 0;
        free_data_size_ = 0;
        buffer_ = nullptr;
    }
    else
    {
        max_buffer_size_ = 2 * size;
        if (max_buffer_size_ >= MAX_BUFFER_SIZE) {
            max_buffer_size_ = MAX_BUFFER_SIZE;
        }

        free_data_size_ = max_buffer_size_ - 1;
        buffer_ = new BUFFER_TYPE[max_buffer_size_];
    }
}

ByteBuffer::ByteBuffer(const ByteBuffer &buff)
{
    start_read_pos_ = buff.start_read_pos_;
    start_write_pos_ = buff.start_write_pos_;
    used_data_size_ = buff.used_data_size_;
    free_data_size_ = buff.free_data_size_;
    max_buffer_size_ = buff.max_buffer_size_;

    if (buff.buffer_ != nullptr && buff.max_buffer_size_ > 0) {
        buffer_ = new BUFFER_TYPE[buff.max_buffer_size_];
        memmove(buffer_, buff.buffer_, buff.max_buffer_size_);
    } else {
        buffer_ = nullptr;
        max_buffer_size_ = 0;
    }
}

ByteBuffer::~ByteBuffer()
{
    this->clear();
}

BUFSIZE_T ByteBuffer::clear(void)
{
    if (buffer_ != nullptr) {
        delete[] buffer_;
        buffer_ = nullptr;
    }

    used_data_size_ = 0;
    free_data_size_ = 0;
    start_read_pos_ = 0;
    start_write_pos_ = 0;
    max_buffer_size_ = 0;

    return 0;
}

BUFSIZE_T ByteBuffer::set_extern_buffer(BUFFER_PTR exbuf, int buff_size)
{
    if (exbuf == nullptr || buff_size <= 0) {
        return 0;
    }

    this->clear();
    max_buffer_size_ = buff_size;
    free_data_size_ = max_buffer_size_ - 1;
    buffer_ = exbuf;

    return buff_size;
}

void ByteBuffer::next_read_pos(int offset)
{
    start_read_pos_ = (start_read_pos_ + offset) % max_buffer_size_;
}

void ByteBuffer::next_write_pos(int offset)
{
    
    start_write_pos_ = (start_write_pos_ + offset) % max_buffer_size_;
}

BUFSIZE_T ByteBuffer::data_size(void) const
{
    
    return used_data_size_;
}

BUFSIZE_T ByteBuffer::idle_size() const 
{
    // -1 是为了留出一位，防止写满和为空的时候，
    // start_write和start_read都指向同一个位置，无法辨认
    return free_data_size_;
}

BUFSIZE_T 
ByteBuffer::resize(BUFSIZE_T size)
{
    // 重新分配的空间不能比当前小
    if (size < 0 || size <= max_buffer_size_)
    {
        return 0;
    }

    max_buffer_size_ = 2 * size;
    if (max_buffer_size_ > MAX_BUFFER_SIZE) {
        max_buffer_size_ = MAX_BUFFER_SIZE;
    }

    BUFFER_PTR new_buffer = new BUFFER_TYPE[max_buffer_size_];

    ByteBuffer tmp_buf = *this;
    this->set_extern_buffer(new_buffer, max_buffer_size_);

    BUFSIZE_T copy_size = this->copy_data_to_buffer(tmp_buf.get_read_buffer_ptr(), tmp_buf.get_cont_read_size());
    std::cout << "1-copy_size: " << copy_size << " cont_size: " << tmp_buf.get_cont_read_size() << " data_size: " << used_data_size_ << std::endl;;
    tmp_buf.update_read_pos(copy_size);
    if (tmp_buf.data_size() > 0) {
        copy_size = this->copy_data_to_buffer(tmp_buf.get_read_buffer_ptr(), tmp_buf.get_cont_read_size());
        std::cout << "2-copy_size: " << copy_size << " cont_size: " << tmp_buf.get_cont_read_size() << " data_size: " << used_data_size_ << std::endl;;
        tmp_buf.update_read_pos(copy_size);
    }

    return max_buffer_size_;
}


bool ByteBuffer::empty(void) const
{
    
    return this->data_size() == 0 ? true : false;
}

ByteBuffer_Iterator
ByteBuffer::begin(void) const
{
    ByteBuffer_Iterator tmp(this);
    return tmp.begin();
}

ByteBuffer_Iterator
ByteBuffer::end(void) const
{
    ByteBuffer_Iterator tmp(this);
    return tmp.end();
}

BUFSIZE_T ByteBuffer::copy_data_to_buffer(const void *data, BUFSIZE_T size)
{
    if (data == nullptr || size <= 0) {
        fprintf(stderr, "output buffer(data) is null!");
        return 0;
    }

    if (this->idle_size() <= size) {
        int ret = this->resize(max_buffer_size_ + size);
        if (ret == -1) {
           return 0;
        }
    }

    BUFSIZE_T copy_size = size;
    BUFFER_PTR data_ptr = (BUFFER_PTR)data;
    
    // 检查buff数组后面是否有连续的内存可以写
    while (true)
    {
        BUFSIZE_T write_size = this->get_cont_write_size() > copy_size ? copy_size : this->get_cont_write_size();
        memmove(this->get_write_buffer_ptr(), data_ptr, write_size);
        this->update_write_pos(write_size);
        data_ptr = data_ptr + write_size;

        copy_size -= write_size;
        if (copy_size <= 0 || this->idle_size() == 0) {
            break;
        }
    }
    

    return size - copy_size;
}

BUFSIZE_T ByteBuffer::copy_data_from_buffer(void *data, BUFSIZE_T size)
{
    if (data == nullptr  || size <= 0) {
        fprintf(stderr, "output buffer(data) is null!");
        return 0;
    }
   
    if (this->data_size() < size) {
        fprintf(stderr, "ByteBuffer remain data(%ld) is less than size(%ld)!", this->data_size(), size);
        return 0;
    }

    BUFSIZE_T copy_size = size;
    BUFFER_PTR data_ptr = (BUFFER_PTR)data;
    
    // 检查buff数组后面是否有连续的内存可以读
    while (true)
    {
        BUFSIZE_T read_size = this->get_cont_read_size() > copy_size ? copy_size : this->get_cont_read_size();
        memmove(data_ptr, this->get_read_buffer_ptr(), read_size);
        this->update_read_pos(read_size);
        data_ptr = data_ptr + read_size;
        
        copy_size -= read_size;
        if (copy_size <= 0 || this->data_size() == 0) {
            break;
        }
    }

    return size - copy_size;
}

BUFSIZE_T
ByteBuffer::read_int8(int8_t &val)
{
    return this->copy_data_from_buffer(&val, sizeof(int8_t));
}

BUFSIZE_T
ByteBuffer::read_int16(int16_t &val)
{
    return this->copy_data_from_buffer(&val, sizeof(int16_t));
}

BUFSIZE_T
ByteBuffer::read_int32(int32_t &val)
{
    return this->copy_data_from_buffer(&val, sizeof(int32_t));
}

BUFSIZE_T
ByteBuffer::read_int64(int64_t &val)
{
    return this->copy_data_from_buffer(&val, sizeof(BUFSIZE_T));
}

// 字符串是以 ‘\0’ 结尾的
BUFSIZE_T
ByteBuffer::read_string(string &str, BUFSIZE_T str_size)
{
    if (this->empty()) {
        fprintf(stderr, "ByteBuffer is empty!");
        return 0;
    }

    if (str_size == -1) {
        str_size = this->data_size();
    }

    char *str_ptr = new char[str_size + 1];
    BUFSIZE_T ret =  this->copy_data_from_buffer(str_ptr, str_size);
    if (ret == 0) {
        return 0;
    }
    str_ptr[str_size] = '\0';
    str = str_ptr;
    delete[] str_ptr;

    return str.length();
}

BUFSIZE_T 
ByteBuffer::read_bytes(void *buf, BUFSIZE_T buf_size, bool match)
{
    if (buf == nullptr) {
        fprintf(stderr, "output buffer(data) is null!");
        return 0;
    }

    return this->copy_data_from_buffer(buf, buf_size);
}

BUFSIZE_T
ByteBuffer::write_int8(int8_t val)
{
    return this->copy_data_to_buffer(&val, sizeof(int8_t));
}

BUFSIZE_T
ByteBuffer::write_int16(int16_t val)
{
    return this->copy_data_to_buffer(&val, sizeof(int16_t));
}

BUFSIZE_T
ByteBuffer::write_int32(int32_t val)
{
    return this->copy_data_to_buffer(&val, sizeof(int32_t));
}

BUFSIZE_T
ByteBuffer::write_int64(int64_t val)
{
    return this->copy_data_to_buffer(&val, sizeof(BUFSIZE_T));
}

BUFSIZE_T
ByteBuffer::write_string(string &str, BUFSIZE_T str_size)
{
    return this->copy_data_to_buffer(str.c_str(), str.length());
}

BUFSIZE_T ByteBuffer::write_bytes(const void *buf, BUFSIZE_T buf_size, bool match)
{
    if (buf == NULL) {
        fprintf(stderr, "output buffer(data) is null!");
        return 0;
    }

    return this->copy_data_to_buffer(buf, buf_size);
}

BUFSIZE_T
ByteBuffer::read_int8_lock(int8_t &val)
{
    lock_.lock();
    BUFSIZE_T ret_size = this->read_int8(val);
    lock_.unlock();

    return ret_size;
}

BUFSIZE_T
ByteBuffer::read_int16_lock(int16_t &val)
{
    lock_.lock();
    BUFSIZE_T ret_size = this->read_int16(val);
    lock_.unlock();

    return ret_size;
}

BUFSIZE_T
ByteBuffer::read_int32_lock(int32_t &val)
{
    lock_.lock();
    BUFSIZE_T ret_size = this->read_int32(val);
    lock_.unlock();

    return ret_size;
}

BUFSIZE_T
ByteBuffer::read_int64_lock(int64_t &val)
{
    lock_.lock();
    BUFSIZE_T ret_size = this->read_int64(val);
    lock_.unlock();

    return ret_size;
}

BUFSIZE_T
ByteBuffer::read_string_lock(string &str, BUFSIZE_T str_size)
{
    lock_.lock();
    BUFSIZE_T ret_size = this->read_string(str, str_size);
    lock_.unlock();

    return ret_size;
}

BUFSIZE_T ByteBuffer::read_bytes_lock(void *buf, BUFSIZE_T buf_size, bool match)
{
    lock_.lock();
    BUFSIZE_T ret_size = this->read_bytes(buf, buf_size, match);
    lock_.unlock();

    return ret_size;
}

BUFSIZE_T
ByteBuffer::write_int8_lock(int8_t val)
{
    lock_.lock();
    BUFSIZE_T ret_size = this->write_int8(val);
    lock_.unlock();

    return ret_size;
}

BUFSIZE_T
ByteBuffer::write_int16_lock(int16_t val)
{
    lock_.lock();
    BUFSIZE_T ret_size = this->write_int16(val);
    lock_.unlock();

    return ret_size;
}

BUFSIZE_T
ByteBuffer::write_int32_lock(int32_t val)
{
    lock_.lock();
    BUFSIZE_T ret_size = this->write_int32(val);
    lock_.unlock();

    return ret_size;
}

BUFSIZE_T
ByteBuffer::write_int64_lock(int64_t val)
{
    lock_.lock();
    BUFSIZE_T ret_size = this->write_int64(val);
    lock_.unlock();

    return ret_size;
}

BUFSIZE_T
ByteBuffer::write_string_lock(string &str, BUFSIZE_T str_size)
{
    lock_.lock();
    BUFSIZE_T ret_size = this->write_string(str, str_size);
    lock_.unlock();

    return ret_size;
}
BUFSIZE_T ByteBuffer::write_bytes_lock(const void *buf, BUFSIZE_T buf_size, bool match)
{
    lock_.lock();
    int ret_size = this->write_bytes(buf, buf_size, match);
    lock_.unlock();

    return ret_size;
}

int ByteBuffer::read_int16_ntoh(int16_t &val)
{
    int ret = this->read_int16(val);
    if (ret == 0) {
        return 0;
    }
    val = ntohs(val);

    return ret;
}
int ByteBuffer::read_int32_ntoh(int32_t &val)
{
    int ret = this->read_int32(val);
    if (ret == 0) {
        return 0;
    }
    val = ntohl(val);

    return ret;
}

int ByteBuffer::write_int16_hton(const int16_t &val)
{
    int16_t tmp = val;
    tmp = htons(val);
    int ret = this->write_int16(tmp);

    return ret;
}

int ByteBuffer::write_int32_hton(const int32_t &val)
{
    int32_t tmp = val;
    tmp = htonl(val);
    int  ret = this->write_int32(tmp);

    return ret;
}

//////////////////////// 重载操作符 /////////////////////////

ByteBuffer 
operator+(ByteBuffer &lhs, ByteBuffer &rhs)
{
    // +10 为了提高冗余；
    ByteBuffer out(lhs.data_size() + rhs.data_size());

    for (auto iter = lhs.begin(); iter != lhs.end(); ++iter) {
        out.write_int8(*iter);
    }
    for (auto iter = rhs.begin(); iter != rhs.end(); ++iter) {
        out.write_int8(*iter);
    }

    return out;
}

bool 
operator==(const ByteBuffer &lhs, const ByteBuffer &rhs)
{
    auto lhs_iter = lhs.begin();
    auto rhs_iter = rhs.begin();

    while (true) {
        if (lhs_iter == lhs.end() && rhs_iter == rhs.end()) {
            return true;
        } else if (lhs_iter != lhs.end() && rhs_iter == rhs.end()) {
            return false;
        } if (lhs_iter == lhs.end() && rhs_iter != rhs.end()) {
            return false;
        }

        if (*lhs_iter != *rhs_iter) {
            return false;
        }

        lhs_iter++;
        rhs_iter++;
    }

    return false;
}

bool 
operator!=(const ByteBuffer &lhs, const ByteBuffer &rhs)
{
    if (lhs.data_size() == rhs.data_size()) {
        return false;
    }
    auto lhs_iter = lhs.begin();
    auto rhs_iter = rhs.begin();

    while (true) {
        if (lhs_iter == lhs.end() && rhs_iter == rhs.end()) {
            return false;
        }

        if (*lhs_iter != *rhs_iter) {
            return true;
        }

        lhs_iter++;
        rhs_iter++;
    }
    return false;
}

ByteBuffer& 
ByteBuffer::operator=(const ByteBuffer& src)
{
    this->clear();

    start_read_pos_ = src.start_read_pos_;
    start_write_pos_ = src.start_write_pos_;
    used_data_size_ = src.used_data_size_;
    max_buffer_size_ = src.max_buffer_size_;

    if (src.buffer_ != nullptr && src.max_buffer_size_ > 0) {
        buffer_ = new BUFFER_TYPE[src.max_buffer_size_];
        memmove(buffer_, src.buffer_, src.max_buffer_size_);
    } else {
        buffer_ = nullptr;
    }
    return *this;
}

BUFFER_PTR 
ByteBuffer::get_write_buffer_ptr(void) const
{
    return buffer_ + start_write_pos_;
}

BUFFER_PTR 
ByteBuffer::get_read_buffer_ptr(void) const
{
    return buffer_ + start_read_pos_;
}

BUFSIZE_T 
ByteBuffer::get_cont_write_size(void) const
{
    if (this->idle_size() <= 0) {
        return 0;
    }

    if (start_read_pos_ > start_write_pos_) {
        return this->idle_size();
    } else if (start_read_pos_ <= start_write_pos_) {
        if (start_read_pos_ == 0) {
            return this->idle_size();
        } else {
            return max_buffer_size_ - start_write_pos_;
        }
    }

    return 0;
}

BUFSIZE_T 
ByteBuffer::get_cont_read_size(void) const
{
    if (used_data_size_ <= 0) {
        return 0;
    }

    if (start_read_pos_ > start_write_pos_) {
        return max_buffer_size_ - start_read_pos_;
    } else if (start_write_pos_ > start_read_pos_) {
        return used_data_size_;
    }

    return 0;
}

void 
ByteBuffer::update_write_pos(BUFSIZE_T offset)
{
    if (offset <= 0 || offset > this->idle_size()) {
        return ;
    }

    used_data_size_ += offset;
    free_data_size_ -= offset;
    start_write_pos_ = (start_write_pos_ + offset) % max_buffer_size_;

    return ;
}

void 
ByteBuffer::update_read_pos(BUFSIZE_T offset)
{
    if (offset <= 0 || offset > this->data_size()) {
        return ;
    }

    used_data_size_ -= offset;
    free_data_size_ += offset;
    start_read_pos_ = (start_read_pos_ + offset) % max_buffer_size_;

    return ;
}

}