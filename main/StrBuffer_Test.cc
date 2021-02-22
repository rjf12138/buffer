#include "str_buffer.h"
#include "gtest/gtest.h"

using namespace my_utils;

namespace my {
namespace project {
namespace {

class StrBuffer_Test : public ::testing::Test {
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

TEST_F(StrBuffer_Test, split)
{
    StrBuffer sbuf("1, 2, 3, 4,");

    auto subs = sbuf.split(", ");
    cout << "SubLength: " << subs.size() << endl;

    cout << "==============================" << endl;
    for (std::size_t i = 0; i < subs.size(); ++i) {
        cout << subs[i] << endl;
    }
    
    ASSERT_EQ(sbuf.remove_substr(", "), "1234,");
    ASSERT_EQ(sbuf.remove_substr(","), "1234");
}

}
}
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}