#include "byte_buffer.h"
#include "gtest/gtest.h"

using namespace my_utils;

namespace my {
namespace project {
namespace {

#define MUTITHREAD 1
#define TEST_SPCE_INCREASE 26

#define TEST_THREAD_NUM 1000
#define TEST_COUNT 8000

////////////////////////////测试工具函数//////////////////////////////////
vector<ByteBuffer>
random_str(ByteBuffer &src, ByteBuffer &patten, int gap)
{
    ByteBuffer front_str, back_str;
    vector<ByteBuffer> ret;

    int j = 0;
    for (auto iter = src.begin(); iter != src.end(); ++iter) {
        front_str.write_int8(*iter);
        back_str.write_int8(*iter);

        if (j >= gap) {
            ByteBuffer_Iterator last_iter = front_str.last_data();
            front_str.insert_front(last_iter, patten);

            last_iter = back_str.last_data();
            back_str.insert_back(last_iter, patten);
            j = 0;
            continue;
        }
        ++j;
    }

    ret.push_back(front_str);
    ret.push_back(back_str);

    return ret;
}


////////////////////////////////////////////////////////////////////////////////

//#define TEST_WRITE_ALL_BUFF 

class ByteBuffer_Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }
};

// 读写指定字节数据
struct test_stru {
    int8_t i8;
    int16_t i16;
    char str[34];
    char buf[16];
};

// 对非锁的读写函数循环测试，功能测试
TEST_F(ByteBuffer_Test, ByteBuff_none_lock_read_write) 
{
    int buff_size = 64;
    int test_cnt = 900000;
    ByteBuffer buff(buff_size);
    // 读写8位数据
    for (int i = 0; i < test_cnt; ++i) {
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(buff.write_int8('a'), 1);
        ASSERT_EQ(buff.data_size(), 1);
        int8_t val_8;
        ASSERT_EQ(buff.read_int8(val_8), 1);
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(val_8, 'a');
    }
    // 读写16位数据
    for (int i = 0; i < test_cnt; ++i) {
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(buff.write_int16(6536), 2);
        ASSERT_EQ(buff.data_size(), 2);
        int16_t val_16;
        ASSERT_EQ(buff.read_int16(val_16), 2);
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(val_16, 6536);
    }
    // 读写32位数据
    for (int i = 0; i < test_cnt; ++i) {
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(buff.write_int32(655536), 4);
        ASSERT_EQ(buff.data_size(), 4);
        int32_t val_32;
        ASSERT_EQ(buff.read_int32(val_32), 4);
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(val_32, 655536);
    }
    // 读写64位数据
    for (int i = 0; i < test_cnt; ++i) {
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(buff.write_int64(65566536), 8);
        ASSERT_EQ(buff.data_size(), 8);
        int64_t val_64;
        ASSERT_EQ(buff.read_int64(val_64), 8);
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(val_64, 65566536);
    }
    // 读写字符串
    string str = "Hello, world";
    for (int i = 0; i < test_cnt; ++i) {
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ((std::size_t)buff.write_string(str), str.length());
        ASSERT_EQ((std::size_t)buff.data_size(), str.length());
        string val_str;
        ASSERT_EQ((std::size_t)buff.read_string(val_str), str.length());
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(val_str, str);
    }

    struct test_stru stru = {'b', 12345, "Nice to meet you", "hello"};
    for (int i = 0; i < test_cnt; ++i) {
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ((std::size_t)buff.write_bytes((void*)&stru, sizeof(stru)) , sizeof(stru));
        ASSERT_EQ((std::size_t)buff.data_size(), sizeof(stru));
        struct test_stru val_stru;
        ASSERT_EQ((std::size_t)buff.read_bytes(&val_stru, sizeof(stru)), sizeof(stru));
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(val_stru.i8, stru.i8);
        ASSERT_EQ(val_stru.i16, stru.i16);
        ASSERT_EQ(strcmp(val_stru.str, stru.str), 0);
        ASSERT_EQ(strcmp(val_stru.buf, stru.buf), 0);
    }
}

// 测试 buffer 空间增长，压力测试
TEST_F(ByteBuffer_Test, ByteBuffer_increase)
{
    ByteBuffer buff(1);
    BUFSIZE_T n = 2;
    for (int i = 0; i < TEST_SPCE_INCREASE; ++i) {
        for (BUFSIZE_T j = 0; j < n / 2; ++j) {
            buff.write_int8('a');
        }
        
        ASSERT_EQ(buff.data_size(), n / 2);
        int8_t out;
        for (BUFSIZE_T j = 0; j < n / 2; ++j) {
            buff.read_int8(out);
            ASSERT_EQ(buff.data_size(), n / 2 - j - 1);
        }

        ASSERT_EQ(buff.empty(), true);
        n = n * 2;
    }

#ifdef TEST_WRITE_ALL_BUFF
    // 将空间写满,再读出来
    BUFSIZE_T write_cnt = 0, read_cnt = 0;
    ByteBuffer max_buff;
    // 写 8 位数据
    int8_t data8 = 'b';
    while(true) {
        BUFSIZE_T ret = max_buff.write_int8(data8);
        if (ret != sizeof(int8_t)) {
            break;
        }
        write_cnt++;
    }
    ASSERT_EQ(max_buff.data_size(), MAX_DATA_SIZE);
    ASSERT_EQ(max_buff.idle_size(), 0);

    // 读 8 位数据
    while(true) {
        int8_t data;
        BUFSIZE_T ret = max_buff.read_int8(data);
        if (ret != sizeof(int8_t) || data != data8) {
            break;
        }
        read_cnt++;
    }
    ASSERT_EQ(max_buff.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(max_buff.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);
    // 同一缓存再次写满
    while(true) {
        BUFSIZE_T ret = max_buff.write_int8(data8);
        if (ret != sizeof(int8_t)) {
            break;
        }
        write_cnt++;
    }
    ASSERT_EQ(max_buff.data_size(), MAX_DATA_SIZE);
    ASSERT_EQ(max_buff.idle_size(), 0);

    // 读 8 位数据
    while(true) {
        int8_t data;
        BUFSIZE_T ret = max_buff.read_int8(data);
        if (ret != sizeof(int8_t) || data != data8) {
            break;
        }
        read_cnt++;
    }
    ASSERT_EQ(max_buff.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(max_buff.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);

//////////////////////////////////////// 16 /////////////////////////////////////
    write_cnt = 0, read_cnt = 0;
    ByteBuffer buff16;
    // 写 16 位数据
    int16_t data16 = 23456;
    while(true) {
        BUFSIZE_T ret = buff16.write_int16(data16);
        if (ret != sizeof(int16_t)) {
            break;
        }
        write_cnt++;
    }
    
    ASSERT_GT((BUFSIZE_T)sizeof(int16_t), MAX_DATA_SIZE - buff16.data_size());
    ASSERT_LE(buff16.idle_size(), (BUFSIZE_T)sizeof(int16_t));

    // 读 16 位数据
    while(true) {
        int16_t data;
        BUFSIZE_T ret = buff16.read_int16(data);
        if (ret != sizeof(int16_t) || data != data16) {
            break;
        }
        read_cnt++;
    }
    ASSERT_EQ(buff16.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(buff16.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);
    write_cnt = read_cnt = 0;
    // 再次写满
    while(true) {
        BUFSIZE_T ret = buff16.write_int16(data16);
        if (ret != sizeof(int16_t)) {
            break;
        }
        write_cnt++;
    }
    ASSERT_GT((BUFSIZE_T)sizeof(int16_t), MAX_DATA_SIZE - buff16.data_size());
    ASSERT_LE(buff16.idle_size(), (BUFSIZE_T)sizeof(int16_t));
    // 读 16 位数据
    while(true) {
        int16_t data;
        BUFSIZE_T ret = buff16.read_int16(data);
        if (ret != sizeof(int16_t) || data != data16) {
            break;
        }
        read_cnt++;
    }
    ASSERT_EQ(buff16.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(buff16.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);
    ////////////////////////////////// 16 ////////////////////////////////////////
    // 读 string 数据
    write_cnt = 0, read_cnt = 0;
    ByteBuffer buffbyte;

    // 写 string 数据
    string str = "sadjklfafks78934729374^*&^&%^&$^%$^%";
    while(true) {
        UNBUFSIZE_T ret = buffbyte.write_string(str);
        if (ret != str.length()) {
            break;
        }
        write_cnt++;
    }
    ASSERT_GT((BUFSIZE_T)str.length(), MAX_DATA_SIZE - buffbyte.data_size());
    ASSERT_LE(buffbyte.idle_size(), (BUFSIZE_T)str.length());

    while(true) {
        string read_str;
        UNBUFSIZE_T ret = buffbyte.read_string(read_str, str.length());
        if (ret != str.length() || read_str != str) {
            break;
        }
        read_cnt++;
    }

    ASSERT_EQ(buffbyte.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(buffbyte.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);

    write_cnt = 0, read_cnt = 0;
    // 再次写满
    while(true) {
        UNBUFSIZE_T ret = buffbyte.write_string(str);
        if (ret != str.length()) {
            break;
        }
        write_cnt++;
    }
    ASSERT_GT((BUFSIZE_T)str.length(), MAX_DATA_SIZE - buffbyte.data_size());
    ASSERT_LE(buffbyte.idle_size(), (BUFSIZE_T)str.length());


    // 再次读 string 位数据
    while(true) {
        string read_str;
        UNBUFSIZE_T ret = buffbyte.read_string(read_str, str.length());
        if (ret != str.length() || str != read_str) {
            break;
        }
        read_cnt++;
    }
    ASSERT_EQ(buffbyte.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(buffbyte.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);

    //////////////////////////////////// 多字节读写 ////////////////////////////////////////////
    // 读 多字节 数据
    write_cnt = 0, read_cnt = 0;
    buffbyte.clear();

    // 写 多字节 数据
    struct test_stru stru = {'b', 12345, "Nice to meet you", "hello"};
    BUFSIZE_T data_size = sizeof(stru);
    while(true) {
        BUFSIZE_T ret = buffbyte.write_bytes(&stru, data_size);
        if (ret != data_size) {
            break;
        }
        write_cnt++;
    }
    ASSERT_GT(data_size, MAX_DATA_SIZE - buffbyte.data_size());
    ASSERT_LE(buffbyte.idle_size(), data_size);

    while(true) {
        struct test_stru read_stru = {'a', 0, "", ""};
        BUFSIZE_T ret = buffbyte.read_bytes(&read_stru, data_size);
        if (ret != data_size || memcmp(&stru, &read_stru, sizeof(stru)) != 0) {
            break;
        }
        read_cnt++;
    }

    ASSERT_EQ(buffbyte.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(buffbyte.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);

    write_cnt = 0, read_cnt = 0;
    // 再次写满
    while(true) {
        BUFSIZE_T ret = buffbyte.write_bytes(&stru, data_size);
        if (ret != data_size) {
            break;
        }
        write_cnt++;
    }
    ASSERT_GT(data_size, MAX_DATA_SIZE - buffbyte.data_size());
    ASSERT_LE(buffbyte.idle_size(), data_size);


    // 再次读 多字节 位数据
    while(true) {
        struct test_stru read_stru;
        BUFSIZE_T ret = buffbyte.read_bytes(&read_stru, data_size);
        if (ret != data_size || memcmp(&stru, &read_stru, sizeof(stru)) != 0) {
            break;
        }
        read_cnt++;
    }

    ASSERT_EQ(buffbyte.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(buffbyte.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);
#endif
}

TEST_F(ByteBuffer_Test, boundary_test)
{
    ByteBuffer buff(-2);
    ASSERT_EQ(buff.data_size(), 0);
    ASSERT_EQ(buff.empty(), true);
    ASSERT_GE(buff.idle_size(), 0);

    test_stru test;
    buff.write_bytes((void*)&test, sizeof(test));
    ASSERT_EQ((std::size_t)buff.data_size(), sizeof(test));
    ASSERT_EQ(buff.empty(), false);
    
    buff.clear();
    ASSERT_EQ(buff.data_size(), 0);
    ASSERT_EQ(buff.empty(), true);
}

TEST_F(ByteBuffer_Test, copy_test)
{
    ByteBuffer src, dest;
    int start_size = 1000, end_size = 10000;
    for (int i = start_size;i < end_size; i += 100) {
        for (int j = 0;j < i; ++j) {
            src.write_int8(j % 256);
        }
        dest = src;
        ASSERT_EQ(src, dest);
        dest.clear();
        src.clear();
    }
}

TEST_F(ByteBuffer_Test, iterator)
{
    ByteBuffer buff;

    string str = "Hello, world! Everyone";
    string str2= "Good Morning! Everyone...";

    // 两个字符串交叉写
    for (int i = 0; i < 5000; ++i) {
        if (i % 2 == 0) {
            buff.write_string(str);
        } else {
            buff.write_string(str2);
        }
    }

    UNBUFSIZE_T read_cnt = 0;
    bool choose_read_str = false;
    ByteBuffer_Iterator iter = buff.begin();
    for (UNBUFSIZE_T i = 0; i < (UNBUFSIZE_T)buff.data_size(); ++i) {
        if (choose_read_str == false) {
            ASSERT_EQ(*(iter + i), str[read_cnt]);
            read_cnt++;
            if (read_cnt == str.length()) {
                read_cnt = 0;
                choose_read_str = true;
            }
        } else {
            ASSERT_EQ(*(iter + i), str2[read_cnt]);
            read_cnt++;
            if (read_cnt == str2.length()) {
                read_cnt = 0;
                choose_read_str = false;
            }
        }
    }

    string read_str;
    read_cnt = str.length();
    choose_read_str = false;
    for (auto iter = buff.begin(); iter != buff.end(); ++iter) {
        read_str += *iter;
        read_cnt--;
        if (read_cnt == 0) {
            if (choose_read_str == false) {
                ASSERT_EQ(read_str, str);
                read_cnt = str2.length();
                choose_read_str = true;
                read_str = "";
            } else {
                ASSERT_EQ(read_str, str2);
                read_cnt = str.length();
                choose_read_str = false;
                read_str = "";
            }
        }
    }
    buff.read_string(read_str);

    // 单个字符串循环写
    for (int i = 0;i < 36000; ++i) {
        read_str = "";
        buff.write_string(str);
        for (auto iter = buff.begin(); iter != buff.end(); ++iter) {
            read_str += *iter;
        }
        ASSERT_EQ(str, read_str);
        buff.read_string(read_str);
    }

    // 测试判等
    bool throw_error = false;
    ByteBuffer buff1, buff2;
    ByteBuffer_Iterator iter1, iter2;

    iter1 = buff1.begin();
    iter2 = buff2.begin();

    ASSERT_NE(iter1, iter2);
    iter2 = iter1;
    ASSERT_EQ(iter1, iter2);

    //测试异常抛出
    throw_error = false;
    try {
        char ch = *iter1;
        ch = ch + 1;
    } catch (runtime_error &e) {
        throw_error = true;
    }
    ASSERT_EQ(throw_error, true);

    // 测试前置，后置加减
    ByteBuffer buff_plus;
    buff_plus.write_string("helxo world");

    iter = buff_plus.begin();
    ASSERT_EQ(*iter, 'h');
    iter++;
    ASSERT_EQ(*iter, 'e');
    ByteBuffer_Iterator iter_back_plus = iter++;
    ASSERT_EQ(*iter_back_plus, 'e');
    ASSERT_EQ(*iter, 'l');
    ByteBuffer_Iterator iter_front_plus = ++iter;
    ASSERT_EQ(*iter, 'x');
    ASSERT_EQ(*iter_front_plus, 'x');

    ByteBuffer_Iterator iter_back_des = iter--;
    ASSERT_EQ(*iter_back_des, 'x');
    ASSERT_EQ(*iter, 'l');
    ByteBuffer_Iterator iter_front_des = --iter;
    ASSERT_EQ(*iter, 'e');
    ASSERT_EQ(*iter_front_des, 'e');

}

#define RANDOM_RANGE 256)
//#define RANDOM_RANGE (('z' - 'a')) + 'a')
TEST_F(ByteBuffer_Test, operate_buffer)
{
    // 测试 remove 
    ByteBuffer data("abcabc");
    data.remove(ByteBuffer("ca"));
    ASSERT_EQ(data, ByteBuffer("abbc"));

    data.remove(ByteBuffer(""));
    ASSERT_EQ(data, ByteBuffer("abbc"));
    // 测试 insert_front 和 insert_back
    ByteBuffer src_buf("Hello");
    vector<string> ins_bufs = {"", "1", "12", "12345"};
    
    auto beg_iter = src_buf.begin();
    src_buf.insert_front(beg_iter, ByteBuffer("12"));
    ASSERT_EQ(src_buf, ByteBuffer("12Hello"));
    
    beg_iter = src_buf.begin();
    src_buf.insert_back(beg_iter, ByteBuffer("12"));
    ASSERT_EQ(src_buf, ByteBuffer("1122Hello"));

    src_buf.clear();
    src_buf.write_string("Hello");
    beg_iter = src_buf.begin() + 2;
    src_buf.insert_front(beg_iter, ByteBuffer("12"));
    ASSERT_EQ(src_buf, ByteBuffer("He12llo"));

    beg_iter = src_buf.begin() + 2;
    src_buf.insert_back(beg_iter, ByteBuffer("12"));
    ASSERT_EQ(src_buf, ByteBuffer("He1122llo"));

    return ;
    // 测试分割和替换
    int max_str_len = 50;
    for (int i = 0; i < 1000; ++i) {
        int patten_len = rand() % max_str_len;
        int src_len = rand() % max_str_len;
        int replace_len = rand() % max_str_len;

        ByteBuffer patten, src, replace_str;
        for (int j = 0;j < patten_len; ++j) {
            BUFFER_TYPE value = (rand() % RANDOM_RANGE;
            patten.write_int8(value);
        }

        for (int j = 0;j < src_len; ++j) {
            BUFFER_TYPE value = (rand() % RANDOM_RANGE;
            src.write_int8(value);
        }

        for (int j = 0;j < replace_len; ++j) {
            BUFFER_TYPE value = (rand() % RANDOM_RANGE;
            replace_str.write_int8(value);
        }

        if (src.find(patten).size() > 0) { // 确保src中不存在符合patten的子串
            src.remove(patten);
        }

        if (src.find(replace_str).size() > 0) { // 确保src中不存在符合replace_str的子串
            src.remove(replace_str);
        }

        for (int gap = 1; gap < 10; ++gap) {
            vector<ByteBuffer> ret = random_str(src, patten, gap);
            vector<ByteBuffer> split_1 = ret[0].split(patten);
            vector<ByteBuffer> split_2 = ret[1].split(patten);

            BUFSIZE_T split1_len = 0, split2_len = 0;
            for (std::size_t is = 0; is < split_1.size(); ++is) {
                split1_len += split_1[is].data_size();
            }
            for (std::size_t is = 0; is < split_2.size(); ++is) {
                split2_len += split_2[is].data_size();
            }
            //cout << "i: " << i << " gap: " << gap << endl;
            ASSERT_EQ(split1_len, src.data_size());
            ASSERT_EQ(split2_len, src.data_size()); // 计算分割后总的字符串是否与源字符串等长

            auto iter = src.begin(); // 比较分割后的内容与原内容是否一致
            for (std::size_t ib = 0; ib < split_1.size(); ++ib) {
                for (BUFSIZE_T is = 0; is < split_1[ib].data_size(); ++is) {
                    ASSERT_EQ(split_1[ib][is], *iter);
                    ++iter; 
                }
            }
            
            iter = src.begin();
            for (std::size_t ib = 0; ib < split_2.size(); ++ib) {
                for (BUFSIZE_T is = 0; is < split_2[ib].data_size(); ++is) {
                    ASSERT_EQ(split_2[ib][is], *iter);
                    ++iter; 
                }
            }

            // 测试替换
            ByteBuffer rep1 = ret[0].replace(patten, replace_str);
            ByteBuffer rep2 = ret[1].replace(patten, replace_str);

            split_1 = rep1.split(replace_str);
            split_2 = rep2.split(replace_str);
            
            split1_len = 0, split2_len = 0;
            for (std::size_t is = 0; is < split_1.size(); ++is) {
                split1_len += split_1[is].data_size();
            }
            for (std::size_t is = 0; is < split_2.size(); ++is) {
                split2_len += split_2[is].data_size();
            }
            ASSERT_EQ(split1_len, src.data_size());
            ASSERT_EQ(split2_len, src.data_size()); // 计算分割后总的字符串是否与源字符串等长
            
            iter = src.begin(); // 比较分割后的内容与原内容是否一致
            for (std::size_t ib = 0; ib < split_1.size(); ++ib) {
                for (BUFSIZE_T is = 0; is < split_1[ib].data_size(); ++is) {
                    ASSERT_EQ(split_1[ib][is], *iter);
                    ++iter; 
                }
            }
            
            iter = src.begin();
            for (std::size_t ib = 0; ib < split_2.size(); ++ib) {
                for (BUFSIZE_T is = 0; is < split_2[ib].data_size(); ++is) {
                    ASSERT_EQ(split_2[ib][is], *iter);
                    ++iter; 
                }
            }

        }
    }

    // string pattern = "a", data = "ababacastababacad::vecababacatoababacar<ByteBuffeababacar_Iteraababacator>ababacaabdddd";
    // vector<int> out;
    // ByteBuffer buff, patten;

    // patten.write_string(pattern);
    // patten.kmp_compute_prefix(out);
    // for (std::size_t i = 0; i < out.size(); ++i) {
    //     cout << out[i] << " ";
    // }
    // cout << endl;
    // buff.write_string(data);
    // vector<ByteBuffer_Iterator> res = buff.find(patten);
    // for (std::size_t i = 0; i < res.size(); ++i) {
    //     std::cout << *res[i] << std::endl;
    // }
    // cout << endl << "======== split ========" << endl;

    // string str;
    // vector<ByteBuffer> ret = buff.split(patten);
    // cout << "size: " << ret.size() << endl;
    // for (std::size_t i = 0;i < ret.size(); ++i) {
    //     ret[i].read_string(str);
    //     std::cout << str << std::endl;
    // }

    // patten.clear();
    // patten.write_string("ab");
    // ByteBuffer result = buff.remove(patten);
    // result.read_string(str);
    // std::cout << str << std::endl;

    // result = buff.remove(patten, 1);
    // result.read_string(str);
    // std::cout << str << std::endl;
    
    // result = buff.remove(patten, 3);
    // result.read_string(str);
    // std::cout << str << std::endl;


    // 正则表达式测试
    // buff.clear();
    // patten.clear();
    // patten.write_string("<(.*)>(.*)</(\\1)>");
    // buff.write_string("123<xml>value</xml>456<widget>center</widget>hahaha<vertical>window</vertical>the end");

    // ret = buff.match(patten);
    // ASSERT_EQ(ret.size(), 3);
    // ASSERT_EQ(ret[0].str(), std::string("<xml>value</xml>"));
    // ASSERT_EQ(ret[1].str(), std::string("<widget>center</widget>"));
    // ASSERT_EQ(ret[2].str(), std::string("<vertical>window</vertical>"));
}

}  // namespace
}  // namespace project
}  // namespace my

// ./test1 -h 查看参数说明
// ./test1 --gtest_list_tests 查看用例
// ./test1 运行所有用例
// ./test1 --gtest_filter=TestClass.Testname1 运行指定用例
// ./test1 --gtest_filter='TestClass.*' 使用通配符
// ./test1 --gtest_filter=-TestClass.Testname1 排除指定用例

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    // int total_size = ::testing::UnitTest::GetInstance()->total_test_suite_count();
    // ::testing::UnitTest::GetInstance() ;
    // return 0;
    return RUN_ALL_TESTS();
}