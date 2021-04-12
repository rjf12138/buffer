#include "byte_buffer.h"

namespace my_utils {

ByteBuffer::ByteBuffer(BUFSIZE_T size)
: buffer_(nullptr),
  start_read_pos_(0), 
  start_write_pos_(0), 
  used_data_size_(0),
  free_data_size_(0),
  max_buffer_size_(0)
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
: buffer_(nullptr),
  start_read_pos_(0), 
  start_write_pos_(0), 
  used_data_size_(0),
  free_data_size_(0),
  max_buffer_size_(0)
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
        this->clear();
    }
}

ByteBuffer::ByteBuffer(const std::string &str)
: buffer_(nullptr),
  start_read_pos_(0), 
  start_write_pos_(0), 
  used_data_size_(0),
  free_data_size_(0),
  max_buffer_size_(0)
{
    this->write_string(str);
}

ByteBuffer::ByteBuffer(const BUFFER_PTR data, BUFSIZE_T size)
: buffer_(nullptr),
  start_read_pos_(0), 
  start_write_pos_(0), 
  used_data_size_(0),
  free_data_size_(0),
  max_buffer_size_(0)
{
    this->write_bytes(data, size);
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

    BUFSIZE_T new_size = 2 * size;
    if (new_size > MAX_BUFFER_SIZE) {
        new_size = MAX_BUFFER_SIZE;
    }

    BUFSIZE_T tmp_buffer_size = this->data_size();
    BUFFER_PTR tmp_buffer = nullptr;
    if (tmp_buffer_size > 0) {
        tmp_buffer = new BUFFER_TYPE[tmp_buffer_size];
        this->read_bytes(tmp_buffer, tmp_buffer_size);
    }

    BUFFER_PTR new_buffer = new BUFFER_TYPE[new_size];
    this->set_extern_buffer(new_buffer, new_size);
    if (tmp_buffer_size > 0) {
        this->write_bytes(tmp_buffer, tmp_buffer_size);
        delete[] tmp_buffer;
    }

    return max_buffer_size_;
}


bool ByteBuffer::empty(void) const
{
    
    return this->data_size() == 0 ? true : false;
}

ByteBuffer_Iterator
ByteBuffer::begin(void)
{
    ByteBuffer_Iterator tmp(this);
    return tmp.begin();
}

ByteBuffer_Iterator
ByteBuffer::end(void)
{
    ByteBuffer_Iterator tmp(this);
    return tmp.end();
}

ByteBuffer_Iterator 
ByteBuffer::last_data(void)
{
    ByteBuffer_Iterator tmp(this);
    if (this->data_size() <= 0) {
        return tmp.end();
    }

    return (tmp.begin() + (this->data_size() - 1));
}

ByteBuffer_Iterator 
ByteBuffer::cbegin(void) const
{
    ByteBuffer_Iterator tmp(this);
    return tmp.begin();
}

ByteBuffer_Iterator 
ByteBuffer::cend(void) const
{
    ByteBuffer_Iterator tmp(this);
    return tmp.end();
}

ByteBuffer_Iterator 
ByteBuffer::clast_data(void) const
{
    ByteBuffer_Iterator tmp(this);
    return tmp.end();
}

BUFSIZE_T ByteBuffer::copy_data_to_buffer(const void *data, BUFSIZE_T size)
{
    if (data == nullptr || size <= 0) {
        return 0;
    }

    if (this->idle_size() <= size) {
        int ret = this->resize(max_buffer_size_ + size);
        if (ret == -1) {
           return 0;
        }
    }

    if (this->idle_size() < size) {
        fprintf(stderr, "ByteBuffer remain idle space(%ld) is less than size(%ld)!", this->idle_size(), size);
        return 0;
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
        memmove(data_ptr, this->get_read_buffer_ptr(), (size_t)read_size);
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
        return 0;
    }

    return this->copy_data_from_buffer(buf, buf_size);
}

std::string 
ByteBuffer::str()
{
    std::string str;
    this->read_string(str);

    return str;
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
ByteBuffer::write_string(const string &str, BUFSIZE_T str_size)
{
    return this->copy_data_to_buffer(str.c_str(), str.length());
}

BUFSIZE_T ByteBuffer::write_bytes(const void *buf, BUFSIZE_T buf_size, bool match)
{
    if (buf == NULL) {
        return 0;
    }

    return this->copy_data_to_buffer(buf, buf_size);
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

BUFSIZE_T
ByteBuffer::get_data(ByteBuffer &out, ByteBuffer_Iterator &copy_start, BUFSIZE_T copy_size)
{
    out.clear();
    if (this->buffer_ == nullptr || copy_size <= 0) {
        return 0;
    }

    if (copy_start.buff_->buffer_ != this->buffer_) {
        return 0;
    }

    BUFSIZE_T i = 0;
    ByteBuffer_Iterator tmp = copy_start;
    for (; i < copy_size && tmp != copy_start.end(); ++i) {
        out.write_int8(*tmp);
        ++tmp;
    }

    return i;
}

//////////////////////// 重载操作符 /////////////////////////

ByteBuffer& 
ByteBuffer::operator+(const ByteBuffer &rhs)
{
    BUFSIZE_T rhs_read_size = rhs.get_cont_read_size();
    BUFFER_PTR rhs_read_ptr = rhs.get_read_buffer_ptr();

    this->copy_data_to_buffer(rhs_read_ptr, rhs_read_size);
    if (rhs_read_size < rhs.data_size()) {
        rhs_read_size = rhs.get_cont_read_size();
        rhs_read_ptr = rhs.get_read_buffer_ptr();

        this->copy_data_to_buffer(rhs_read_ptr, rhs_read_size);
    }

    return *this;
}

ByteBuffer& 
ByteBuffer::operator+=(const ByteBuffer &rhs)
{
    BUFSIZE_T rhs_read_size = rhs.get_cont_read_size();
    BUFFER_PTR rhs_read_ptr = rhs.get_read_buffer_ptr();

    this->copy_data_to_buffer(rhs_read_ptr, rhs_read_size);
    if (rhs_read_size < rhs.data_size()) {
        rhs_read_size = rhs.get_cont_read_size();
        rhs_read_ptr = rhs.get_read_buffer_ptr();

        this->copy_data_to_buffer(rhs_read_ptr, rhs_read_size);
    }

    return *this;
}

bool 
ByteBuffer::operator==(const ByteBuffer &rhs) const
{
    if (this->data_size() != rhs.data_size()) {
        return false;
    }

    auto lhs_iter = this->cbegin();
    auto rhs_iter = rhs.cbegin();

    while (true) {
        if (lhs_iter == this->cend() && rhs_iter == rhs.cend()) {
            return true;
        } else if (lhs_iter != this->cend() && rhs_iter == rhs.cend()) {
            return false;
        } else if (lhs_iter == this->cend() && rhs_iter != rhs.cend()) {
            return false;
        }

        if (*lhs_iter != *rhs_iter) {
            return false;
        }

        ++lhs_iter;
        ++rhs_iter;
    }

    return false;
}

bool 
ByteBuffer::operator!=(const ByteBuffer &rhs) const
{
    return !(*this == rhs);
}

ByteBuffer& 
ByteBuffer::operator=(const ByteBuffer& src)
{
    if (src == *this) { // 当赋值对象是自己时，直接返回
        return *this;
    }
    this->clear();

    start_read_pos_ = src.start_read_pos_;
    start_write_pos_ = src.start_write_pos_;
    used_data_size_ = src.used_data_size_;
    free_data_size_ = src.free_data_size_;
    max_buffer_size_ = src.max_buffer_size_;

    if (src.buffer_ != nullptr && src.max_buffer_size_ > 0) {
        buffer_ = new BUFFER_TYPE[src.max_buffer_size_];
        memmove(buffer_, src.buffer_, src.max_buffer_size_);
    } else {
        this->clear();
    }
    return *this;
}

BUFFER_TYPE& 
ByteBuffer::operator[](BUFSIZE_T index)
{
    BUFSIZE_T size = this->data_size();
    if (size <= 0 || index >= size) {
        ostringstream ostr;
        ostr << "Line: " << __LINE__ << " out of range.";
        throw runtime_error(ostr.str());
    }

    index = (this->start_read_pos_ + index) %  max_buffer_size_;

    return buffer_[index];
}

bool 
ByteBuffer::bytecmp(ByteBuffer_Iterator &iter, ByteBuffer &patten, BUFSIZE_T size)
{
    if (iter.buff_->buffer_ != this->buffer_ || iter == this->end())
    {
        return false;
    }

    BUFSIZE_T max_size = (size == -1 ? patten.data_size() : size);
    BUFSIZE_T index = 0;
    for (auto tmp_iter = iter; tmp_iter != this->end() && index < max_size; ++tmp_iter, ++index) {
        if (*tmp_iter != patten[index]) {
            return false;
        }
    }

    if (index == max_size) {
        return true;
    }

    return false;
}

BUFFER_PTR 
ByteBuffer::get_write_buffer_ptr(void) const
{
    return buffer_ != nullptr ? buffer_ + start_write_pos_ : nullptr;
}

BUFFER_PTR 
ByteBuffer::get_read_buffer_ptr(void) const
{
    return buffer_ != nullptr ? buffer_ + start_read_pos_ : nullptr;
}

BUFSIZE_T 
ByteBuffer::get_cont_write_size(void) const
{
    if (free_data_size_ <= 0) {
        return 0;
    }

    if (start_read_pos_ > start_write_pos_) {
        return free_data_size_;
    } else if (start_read_pos_ <= start_write_pos_) {
        if (start_read_pos_ == 0) {
            return free_data_size_;
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
    if (offset <= 0 || offset > free_data_size_) {
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
    if (offset <= 0 || offset > used_data_size_) {
        return ;
    }

    used_data_size_ -= offset;
    free_data_size_ += offset;
    start_read_pos_ = (start_read_pos_ + offset) % max_buffer_size_;

    return ;
}

///////////////////////////// 操作 ByteBuffer /////////////////////////////
std::vector<ByteBuffer_Iterator>
ByteBuffer::find(ByteBuffer patten)
{
    std::vector<ByteBuffer_Iterator> result;
    if (patten.data_size() == 0 || this->data_size() == 0) {
        return result;
    }

    int patten_index = 0;
    for (int i = 0; i < this->data_size(); ++i) {
        if (patten[patten_index] == (*this)[i]) {
            for (int j = i; j < this->data_size();) {
                if (patten_index < patten.data_size() && patten[patten_index] == (*this)[j]) {
                    ++patten_index;
                    ++j;
                } else {
                    break;
                }
            }
            
            if (patten_index == patten.data_size()) {
                result.push_back(this->begin() + i);
                i += patten.data_size();
            }
            patten_index = 0;
        }
    }

    return result;
}

std::vector<ByteBuffer> 
ByteBuffer::split(ByteBuffer buff)
{
    std::vector<ByteBuffer> result;
    if (buff.data_size() <= 0 || this->data_size() <= 0) {
        result.push_back(*this);
        return result;
    }

    std::vector<ByteBuffer_Iterator> find_buff = this->find(buff);

    ByteBuffer tmp;
    BUFSIZE_T copy_size;
    ByteBuffer_Iterator start_copy_pos = this->begin();
    for (std::size_t i = 0; i < find_buff.size(); ++i) {
        copy_size = find_buff[i] - start_copy_pos;

        this->get_data(tmp, start_copy_pos, copy_size);
        start_copy_pos = find_buff[i] + buff.data_size();
        
        if (tmp.data_size() <= 0) {
            continue;
        }

        result.push_back(tmp);
    }

    copy_size = (this->last_data() - start_copy_pos) + 1; // 保存剩余的字符
    if (copy_size > 0) {
        this->get_data(tmp, start_copy_pos, copy_size);
        if (tmp.data_size() > 0) {
            result.push_back(tmp);
        }
    }

    return result;
}


ByteBuffer 
ByteBuffer::replace(ByteBuffer buf1, ByteBuffer buf2, BUFSIZE_T index)
{
    if (buf1.data_size() <= 0 || this->data_size() <= 0) {
        return *this;
    }

    BUFSIZE_T copy_size = 0;
    ByteBuffer result, tmp;
    ByteBuffer_Iterator copy_pos_iter = this->begin();
    std::vector<ByteBuffer_Iterator> find_buff = this->find(buf1);
    if (find_buff.size() == 0) {
        return *this;
    }

    if (index < 0 || index >= (BUFSIZE_T)find_buff.size()) { // 替换所有
        for (std::size_t i = 0; i < find_buff.size(); ++i) {
            copy_size = find_buff[i] - copy_pos_iter;
            if (copy_size > 0) {
                this->get_data(tmp, copy_pos_iter, copy_size);
                result = result + tmp;
                result = result + buf2;
                copy_pos_iter = find_buff[i] + buf1.data_size();
            }
        }

        copy_size = this->last_data() - copy_pos_iter + 1; // 保存剩余的字符
        if (copy_size > 0) {
            this->get_data(tmp, copy_pos_iter, copy_size);
            result = result + tmp;
        }
    } else {
        copy_size = find_buff[index] - copy_pos_iter;
        if (copy_size > 0) {
            this->get_data(tmp, copy_pos_iter, copy_size);
            result = result + tmp;
            result = result + buf2;
            copy_pos_iter = find_buff[index] + buf1.data_size();
        }
    }

    return result;
}


ByteBuffer 
ByteBuffer::remove(ByteBuffer buff, BUFSIZE_T index)
{
    if (buff.data_size() <= 0 || this->data_size() <= 0) {
        return *this;
    }
    
    ByteBuffer tmp_buf;
    std::vector<ByteBuffer_Iterator> find_buff = this->find(buff);
    if (index < 0 || index >= (BUFSIZE_T)find_buff.size()) {
        index = -1;
    }

    if (index == -1) {
        std::vector<ByteBuffer> ret = this->split(buff);
        for (std::size_t i = 0; i < ret.size(); ++i) {
            tmp_buf = tmp_buf + ret[i];
        }
    } else {
        ByteBuffer_Iterator begin_iter = this->begin();
        BUFSIZE_T copy_size = find_buff[index] - begin_iter;

        ByteBuffer out;
        this->get_data(out, begin_iter, copy_size);
        tmp_buf = tmp_buf + out;

        find_buff[index] = find_buff[index] + buff.data_size();
        copy_size = this->last_data() - find_buff[index];
        this->get_data(out, find_buff[index], copy_size);
        tmp_buf = tmp_buf + out;
    }

    *this = tmp_buf;

    return tmp_buf;
}

BUFSIZE_T 
ByteBuffer::insert_front(ByteBuffer_Iterator &insert_iter, ByteBuffer buff)
{
    ByteBuffer tmp_buf, result;
    if (!(insert_iter >= this->begin() && 
            insert_iter <= this->last_data())) {
        return -1;
    }

    ByteBuffer_Iterator begin_iter = this->begin();
    BUFSIZE_T copy_front_size = insert_iter - begin_iter;
    this->get_data(result, begin_iter, copy_front_size);
    
    result = result + buff;
    copy_front_size = this->last_data() - insert_iter + 1;
    this->get_data(tmp_buf, insert_iter, copy_front_size);
    result = result + tmp_buf;

    *this = result;

    return 0;
}

BUFSIZE_T 
ByteBuffer::insert_back(ByteBuffer_Iterator &insert_iter, ByteBuffer buff)
{
    ByteBuffer tmp_buf, result;
    if (!(insert_iter >= this->begin() && 
            insert_iter <= this->last_data())) {
        return -1;
    }

    ByteBuffer_Iterator begin_iter = this->begin();
    BUFSIZE_T copy_front_size = insert_iter - begin_iter + 1;
    this->get_data(result, begin_iter, copy_front_size);
    
    result = result + buff;
    copy_front_size = this->last_data() - insert_iter;
    insert_iter++;
    this->get_data(tmp_buf, insert_iter, copy_front_size);
    result = result + tmp_buf;

    *this = result;

    return 0;
}

    // 返回符合模式 regex 的子串(使用正则表达式)
vector<ByteBuffer> 
ByteBuffer::match(ByteBuffer regex_str)
{
    vector<ByteBuffer> ret_match_str;
    std::regex reg(regex_str.str());
    std::string content(this->str());
    std::smatch m;

    auto pos = content.cbegin();
    auto end = content.cend();
    for (; std::regex_search(pos, end, m, reg); pos = m.suffix().first)
    {
        ByteBuffer match_substr(m.str());
        ret_match_str.push_back(match_substr);
    }

    return ret_match_str;
}

}