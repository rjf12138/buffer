###ByteBuffer
####功能：
1. 以特定格式将数据临时缓冲(int8, int16 int32 int64,string, bytes)。
2. 支持多线程对同一个buff进行读写。
3. 支持迭代器遍历每个字节。

####注意事项：
1.  使用write_bytes写的是包含类成员的结构时，确保该类成员内不含指针
    思考这样一种情况：
    ```
    struct test { string str; } a = {"hello"}, b;
    buff.write_bytes(&a, sizeof(test));
    buff.read_bytes(b, sizeof(test))；
    ```
    a 和 b 相同, a 和 b 内部的指针指向同一个位置，程序结束时会对同一
    个指针释放两次，从而报错double free or corruption (fasttop)

2. 能分配的大小取决于vector的大小, 但往往在内存不够用时报出异常中断程序
   所以要注意不要分配的空间大于可用内存。

3. 当多个线程读写同一个buff_时， 要使用带锁的读写函数，不然会出现异常情况。