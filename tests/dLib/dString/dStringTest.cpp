#include "dLib/aTest/TestHarness.h"

#include <iostream>
#include <limits>
#include <iomanip>
#include <stdexcept>
#include <ciso646>

#include "dLib/dString/dString.h"
#include "utilities/Math.h"

SUITE(dString)


class StringUtf8Setup : public TestSetup
{
public:
    void setup()
    {
        s1 = "ที่รัก";
        s2 = "Beautiful ";
        s3 = "World!";
        s213 = "Beautiful ที่รักWorld!";
        s5 = "กWorld!";
    }
    void teardown() {}
protected:
    FTS::String s1, s2, s3, s213,s5;
};

TEST_INSUITE_WITHSETUP(dString, StringUtf8, Utf8)
{
    CHECK_EQUAL(6, s1.len());
    CHECK_EQUAL(18, s1.byteCount());
    CHECK_EQUAL(s2.len(),s2.byteCount());
    CHECK_EQUAL(10, s2.len());
    CHECK_EQUAL(10, s2.byteCount());
    FTS::String left2 = s1.left(2);
    CHECK_EQUAL(2, left2.len());
    CHECK_EQUAL(6, left2.byteCount());

    CHECK_EQUAL("Hello,ที่รัก World!", FTS::String("Fucking ello,ที่รัก World!").replaceStr(0, 8, "H"));
    CHECK_EQUAL("Hello, World!", FTS::String("Hที่รักello, World!").replaceStr(0, 7, FTS::String("H")));
    FTS::String s = s2 + s1 + s3 ;
    CHECK_EQUAL(s213, s);
    FTS::String sresult = s.mid(s2.len(), s3.len());
    CHECK_EQUAL(s1, sresult);
    sresult = s.right(s3.len() + 1);
    CHECK_EQUAL(s5,sresult);

    /// TODO: find, replace and more.
}


TEST_INSUITE(dString, ConstructionSimple)
{
    /* A whole bunch of constructors. */
    CHECK_EQUAL(std::string(""), FTS::String().str());
    CHECK_EQUAL(std::string(""), FTS::String("").str());
    CHECK_EQUAL(std::string(""), FTS::String((const char *)NULL).str());
    CHECK_EQUAL(std::string("hello, world"), FTS::String("hello, world").str());
    CHECK_EQUAL(std::string("hello"), FTS::String(FTS::String("hello")).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(std::string("hello")).str());
#ifdef D_STRING_CEGUI
    CHECK_EQUAL(std::string("hello"), FTS::String(CEGUI::String("hello")).str());
#endif
    CHECK_EQUAL(std::string("héllô ที่รัก"), FTS::String(std::string("héllô ที่รัก")).str());
}

TEST_INSUITE(dString, ConstructionLen)
{
#ifdef D_STRING_CEGUI
    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("héllô"), 0, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("héllô"), 0, -1).str());
    CHECK_EQUAL(std::string("h"), FTS::String(CEGUI::String("héllô"), 0, 1).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(CEGUI::String("héllô"), 0, 5).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(CEGUI::String("héllô"), 0, 9).str());

    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("héllô"), -1).str());
    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("héllô"), 5).str());
    CHECK_EQUAL(std::string("ô"), FTS::String(CEGUI::String("héllô"), 4).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(CEGUI::String("héllô"), 3).str());
    CHECK_EQUAL(std::string("éllô"), FTS::String(CEGUI::String("héllô"), 1).str());

    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("héllô"), -1, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("héllô"), 5, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("héllô"), 3, 0).str());
    CHECK_EQUAL(std::string("l"), FTS::String(CEGUI::String("héllô"), 3, 1).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(CEGUI::String("héllô"), 3, 2).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(CEGUI::String("héllô"), 3, 3).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(CEGUI::String("héllô"), 3, -1).str());
#endif

    CHECK_EQUAL(std::string(""), FTS::String("héllô", 0, 0).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String("héllô", 0, -1).str());
    CHECK_EQUAL(std::string("h"), FTS::String("héllô", 0, 1).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String("héllô", 0, 5).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String("héllô", 0, 9).str());

    CHECK_EQUAL(std::string(""), FTS::String("héllô", -1).str());
    CHECK_EQUAL(std::string(""), FTS::String("héllô", 5).str());
    CHECK_EQUAL(std::string("ô"), FTS::String("héllô", 4).str());
    CHECK_EQUAL(std::string("lô"), FTS::String("héllô", 3).str());
    CHECK_EQUAL(std::string("éllô"), FTS::String("héllô", 1).str());

    CHECK_EQUAL(std::string(""), FTS::String("héllô", -1, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String("héllô", 5, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String("héllô", 3, 0).str());
    CHECK_EQUAL(std::string("l"), FTS::String("héllô", 3, 1).str());
    CHECK_EQUAL(std::string("lô"), FTS::String("héllô", 3, 2).str());
    CHECK_EQUAL(std::string("lô"), FTS::String("héllô", 3, 3).str());
    CHECK_EQUAL(std::string("lô"), FTS::String("héllô", 3, -1).str());

    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 0, 0).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 0, -1).str());
    CHECK_EQUAL(std::string("h"), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 0, 1).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 0, 5).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 0, 9).str());

    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const int8_t*>("héllô"), -1).str());
    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 5).str());
    CHECK_EQUAL(std::string("ô"), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 4).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 3).str());
    CHECK_EQUAL(std::string("éllô"), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 1).str());

    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const int8_t*>("héllô"), -1, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 5, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 3, 0).str());
    CHECK_EQUAL(std::string("l"), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 3, 1).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 3, 2).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 3, 3).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(reinterpret_cast<const int8_t*>("héllô"), 3, -1).str());

    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 0, 0).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 0, -1).str());
    CHECK_EQUAL(std::string("h"), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 0, 1).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 0, 5).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 0, 9).str());

    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), -1).str());
    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 5).str());
    CHECK_EQUAL(std::string("ô"), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 4).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 3).str());
    CHECK_EQUAL(std::string("éllô"), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 1).str());

    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), -1, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 5, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 3, 0).str());
    CHECK_EQUAL(std::string("l"), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 3, 1).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 3, 2).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 3, 3).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(reinterpret_cast<const uint8_t*>("héllô"), 3, -1).str());

    CHECK_EQUAL(std::string(""), FTS::String(FTS::String("héllô"), 0, 0).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(FTS::String("héllô"), 0, -1).str());
    CHECK_EQUAL(std::string("h"), FTS::String(FTS::String("héllô"), 0, 1).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(FTS::String("héllô"), 0, 5).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(FTS::String("héllô"), 0, 9).str());

    CHECK_EQUAL(std::string(""), FTS::String(FTS::String("héllô"), -1).str());
    CHECK_EQUAL(std::string(""), FTS::String(FTS::String("héllô"), 5).str());
    CHECK_EQUAL(std::string("ô"), FTS::String(FTS::String("héllô"), 4).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(FTS::String("héllô"), 3).str());
    CHECK_EQUAL(std::string("éllô"), FTS::String(FTS::String("héllô"), 1).str());

    CHECK_EQUAL(std::string(""), FTS::String(FTS::String("héllô"), -1, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(FTS::String("héllô"), 5, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(FTS::String("héllô"), 3, 0).str());
    CHECK_EQUAL(std::string("l"), FTS::String(FTS::String("héllô"), 3, 1).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(FTS::String("héllô"), 3, 2).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(FTS::String("héllô"), 3, 3).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(FTS::String("héllô"), 3, -1).str());

    CHECK_EQUAL(std::string(""), FTS::String(std::string("héllô"), 0, 0).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(std::string("héllô"), 0, -1).str());
    CHECK_EQUAL(std::string("h"), FTS::String(std::string("héllô"), 0, 1).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(std::string("héllô"), 0, 5).str());
    CHECK_EQUAL(std::string("héllô"), FTS::String(std::string("héllô"), 0, 9).str());

    CHECK_EQUAL(std::string(""), FTS::String(std::string("héllô"), -1).str());
    CHECK_EQUAL(std::string(""), FTS::String(std::string("héllô"), 5).str());
    CHECK_EQUAL(std::string("ô"), FTS::String(std::string("héllô"), 4).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(std::string("héllô"), 3).str());
    CHECK_EQUAL(std::string("éllô"), FTS::String(std::string("héllô"), 1).str());

    CHECK_EQUAL(std::string(""), FTS::String(std::string("héllô"), -1, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(std::string("héllô"), 5, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(std::string("héllô"), 3, 0).str());
    CHECK_EQUAL(std::string("l"), FTS::String(std::string("héllô"), 3, 1).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(std::string("héllô"), 3, 2).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(std::string("héllô"), 3, 3).str());
    CHECK_EQUAL(std::string("lô"), FTS::String(std::string("héllô"), 3, -1).str());
}

TEST_INSUITE(dString, ConstructionFancy)
{
    // Constructors from other types.
    CHECK_EQUAL("0", FTS::String::chr('0').str());

    CHECK_EQUAL("127", FTS::String::nr(static_cast<int8_t>(127)).str());
    CHECK_EQUAL("127", FTS::String::nr(static_cast<int8_t>(127), 1).str());
    CHECK_EQUAL("aa127", FTS::String::nr(static_cast<int8_t>(127), 5, 'a').str());
    CHECK_EQUAL("aaa7f", FTS::String::nr(static_cast<int8_t>(127), 5, 'a', std::ios::hex).str());

    CHECK_EQUAL("255", FTS::String::nr(static_cast<uint8_t>(-1)).str());
    CHECK_EQUAL("255", FTS::String::nr(static_cast<uint8_t>(-1), 1).str());
    CHECK_EQUAL("aa255", FTS::String::nr(static_cast<uint8_t>(-1), 5, 'a').str());
    CHECK_EQUAL("aaaff", FTS::String::nr(static_cast<uint8_t>(-1), 5, 'a', std::ios::hex).str());

    CHECK_EQUAL("32767", FTS::String::nr(static_cast<int16_t>(32767)).str());
    CHECK_EQUAL("32767", FTS::String::nr(static_cast<int16_t>(32767), 1).str());
    CHECK_EQUAL("aa32767", FTS::String::nr(static_cast<int16_t>(32767), 7, 'a').str());
    CHECK_EQUAL("aaa7fff", FTS::String::nr(static_cast<int16_t>(32767), 7, 'a', std::ios::hex).str());

    CHECK_EQUAL("65535", FTS::String::nr(static_cast<uint16_t>(-1)).str());
    CHECK_EQUAL("65535", FTS::String::nr(static_cast<uint16_t>(-1), 1).str());
    CHECK_EQUAL("aa65535", FTS::String::nr(static_cast<uint16_t>(-1), 7, 'a').str());
    CHECK_EQUAL("aaaffff", FTS::String::nr(static_cast<uint16_t>(-1), 7, 'a', std::ios::hex).str());

    CHECK_EQUAL("2147483647", FTS::String::nr(static_cast<int32_t>(2147483647)).str());
    CHECK_EQUAL("2147483647", FTS::String::nr(static_cast<int32_t>(2147483647), 1).str());
    CHECK_EQUAL("aa2147483647", FTS::String::nr(static_cast<int32_t>(2147483647), 12, 'a').str());
    CHECK_EQUAL("aaaa7fffffff", FTS::String::nr(static_cast<int32_t>(2147483647), 12, 'a', std::ios::hex).str());

    CHECK_EQUAL("4294967295", FTS::String::nr(static_cast<uint32_t>(-1)).str());
    CHECK_EQUAL("4294967295", FTS::String::nr(static_cast<uint32_t>(-1), 1).str());
    CHECK_EQUAL("aa4294967295", FTS::String::nr(static_cast<uint32_t>(-1), 12, 'a').str());
    CHECK_EQUAL("aaaaffffffff", FTS::String::nr(static_cast<uint32_t>(-1), 12, 'a', std::ios::hex).str());

    CHECK_EQUAL("9223372036854775807", FTS::String::nr(static_cast<int64_t>(9223372036854775807UL)).str());
    CHECK_EQUAL("9223372036854775807", FTS::String::nr(static_cast<int64_t>(9223372036854775807UL), 1).str());
    CHECK_EQUAL("aaa9223372036854775807", FTS::String::nr(static_cast<int64_t>(9223372036854775807UL), 22, 'a').str());
    CHECK_EQUAL("aaaaaa7fffffffffffffff", FTS::String::nr(static_cast<int64_t>(9223372036854775807UL), 22, 'a', std::ios::hex).str());

    CHECK_EQUAL("18446744073709551615", FTS::String::nr(static_cast<uint64_t>(-1)).str());
    CHECK_EQUAL("18446744073709551615", FTS::String::nr(static_cast<uint64_t>(-1), 1).str());
    CHECK_EQUAL("aa18446744073709551615", FTS::String::nr(static_cast<uint64_t>(-1), 22, 'a').str());
    CHECK_EQUAL("aaaaaaffffffffffffffff", FTS::String::nr(static_cast<uint64_t>(-1), 22, 'a', std::ios::hex).str());

    CHECK_EQUAL("2.000000", FTS::String::nr(2.0f).str());
    CHECK_EQUAL("2", FTS::String::nr(2.0f, 0).str());
    CHECK_EQUAL("2.5", FTS::String::nr(2.5f, 1).str());
    CHECK_EQUAL("2.1", FTS::String::nr(2.051f, 1).str());
    CHECK_EQUAL("2.0", FTS::String::nr(2.05f, 1).str());
    CHECK_EQUAL("2.0", FTS::String::nr(2.f, 1).str());

    CHECK_EQUAL("\342\210\236", FTS::String::nr(std::numeric_limits<float>::infinity()).str());
    CHECK_EQUAL("NaN", FTS::String::nr(std::numeric_limits<float>::signaling_NaN()).str());
    CHECK_EQUAL("NaN", FTS::String::nr(std::numeric_limits<float>::quiet_NaN()).str());

    CHECK_EQUAL("2.000000", FTS::String::nr(2.0).str());
    CHECK_EQUAL("2", FTS::String::nr(2.0, 0).str());
    CHECK_EQUAL("2.5", FTS::String::nr(2.5, 1).str());
    CHECK_EQUAL("2.1", FTS::String::nr(2.051, 1).str());
    CHECK_EQUAL("2.0", FTS::String::nr(2.05, 1).str());
    CHECK_EQUAL("2.0", FTS::String::nr(2., 1).str());
    CHECK_EQUAL("2.0", FTS::String::nr(2., 1).str());

    CHECK_EQUAL("\342\210\236", FTS::String::nr(std::numeric_limits<double>::infinity()).str());
    CHECK_EQUAL("NaN", FTS::String::nr(std::numeric_limits<double>::signaling_NaN()).str());
    CHECK_EQUAL("NaN", FTS::String::nr(std::numeric_limits<double>::quiet_NaN()).str());

    CHECK_EQUAL("True", FTS::String::b(true).str());
    CHECK_EQUAL("False", FTS::String::b(false).str());
    CHECK_EQUAL("True", FTS::String::b(!0).str());
    CHECK_EQUAL("False", FTS::String::b(!1).str());

    CHECK_EQUAL(6, FTS::String::random("######", FTS::random<int>).len());
    CHECK_EQUAL(3, FTS::String::random("#\\##", FTS::random<int>).len());
    CHECK_EQUAL('#', FTS::String::random("#\\##", FTS::random<int>).getCharAt(1));
    CHECK_EQUAL('\\', FTS::String::random("#\\\\##", FTS::random<int>).getCharAt(1));
    CHECK_EQUAL(4, FTS::String::random("#\\\\##", FTS::random<int>).len());
}

TEST_INSUITE(dString, LeftRightMid)
{
    CHECK_EQUAL("Héllô", FTS::String("Héllô, nice ที่รัก!").left(5).str());
    CHECK_EQUAL("ที่รัก!", FTS::String("Héllô, nice ที่รัก!").right(7).str());
    CHECK_EQUAL("nice", FTS::String("Héllô, nice ที่รัก!").mid(7,8).str());
    CHECK_EQUAL("ที่รัก", FTS::String("Héllô, nice ที่รัก!").mid(12,1).str());

    CHECK_EQUAL("", FTS::String("Héllô, nice ที่รัก!").left(0).str());
    CHECK_EQUAL("H", FTS::String("Héllô, nice ที่รัก!").left(1).str());
    CHECK_EQUAL("Héllô, nice ที่รัก!", FTS::String("Héllô, nice ที่รัก!").left(19).str());
    CHECK_EQUAL("Héllô, nice ที่รัก!", FTS::String("Héllô, nice ที่รัก!").left(20).str());
    CHECK_EQUAL("Héllô, nice ที่รัก!", FTS::String("Héllô, nice ที่รัก!").left(100).str());

    CHECK_EQUAL("", FTS::String("Héllô, nice ที่รัก!").right(0).str());
    CHECK_EQUAL("!", FTS::String("Héllô, nice ที่รัก!").right(1).str());
    CHECK_EQUAL("Héllô, nice ที่รัก!", FTS::String("Héllô, nice ที่รัก!").right(19).str());
    CHECK_EQUAL("Héllô, nice ที่รัก!", FTS::String("Héllô, nice ที่รัก!").right(20).str());
    CHECK_EQUAL("Héllô, nice ที่รัก!", FTS::String("Héllô, nice ที่รัก!").right(100).str());

    CHECK_EQUAL("Héllô, nice ที่รัก!", FTS::String("Héllô, nice ที่รัก!").mid(0,0).str());
    CHECK_EQUAL("Héllô, nice ที่รัก", FTS::String("Héllô, nice ที่รัก!").mid(0,1).str());
    CHECK_EQUAL("éllô, nice ที่รัก!", FTS::String("Héllô, nice ที่รัก!").mid(1,0).str());
    CHECK_EQUAL("éllô, nice ที่รัก", FTS::String("Héllô, nice ที่รัก!").mid(1,1).str());
    CHECK_EQUAL("", FTS::String("Héllô, nice ที่รัก!").mid(19,0).str());
    CHECK_EQUAL("", FTS::String("Héllô, nice ที่รัก!").mid(19,1).str());
    CHECK_EQUAL("", FTS::String("Héllô, nice ที่รัก!").mid(20,0).str());
    CHECK_EQUAL("", FTS::String("Héllô, nice ที่รัก!").mid(20,1).str());
    CHECK_EQUAL("", FTS::String("Héllô, nice ที่รัก!").mid(0,19).str());
    CHECK_EQUAL("", FTS::String("Héllô, nice ที่รัก!").mid(1,19).str());
    CHECK_EQUAL("", FTS::String("Héllô, nice ที่รัก!").mid(0,20).str());
    CHECK_EQUAL("", FTS::String("Héllô, nice ที่รัก!").mid(1,20).str());
}

TEST_INSUITE(dString, HexData)
{
    for(uint8_t byte=0 ; byte!=255 ; ++byte) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)byte;
        std::string s = ss.str();
        CHECK_EQUAL(s, FTS::String::sfromHex(byte).str());
    }

    for(uint8_t byte=0 ; byte!=255 ; ++byte) {
        std::stringstream ss;
        ss << std::hex << std::nouppercase << std::setw(2) << std::setfill('0') << (int)byte;
        std::string s = ss.str();
        CHECK_EQUAL(s, FTS::String::sfromHex(byte, false).str());
    }

    for(uint8_t byte=0 ; byte!=255 ; ++byte) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)byte;
        std::string s = ss.str();
        bool bSuccess = false;
        CHECK_EQUAL((int)byte, (int)FTS::String::byteFromHex(s, bSuccess));
        CHECK(bSuccess);
    }
    for(uint8_t byte=0 ; byte!=16 ; ++byte) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << (int)byte;
        std::string s = ss.str();
        bool bSuccess = false;
        CHECK_EQUAL((int)byte, (int)FTS::String::byteFromHex(s, bSuccess));
        CHECK(bSuccess);
    }

    for(uint8_t byte=0 ; byte!=255 ; ++byte) {
        std::stringstream ss;
        ss << std::hex << std::nouppercase << std::setw(2) << std::setfill('0') << (int)byte;
        std::string s = ss.str();
        bool bSuccess = false;
        CHECK_EQUAL((int)byte, (int)FTS::String::byteFromHex(s, bSuccess));
        CHECK(bSuccess);
    }
    for(uint8_t byte=0 ; byte!=16 ; ++byte) {
        std::stringstream ss;
        ss << std::hex << std::nouppercase << (int)byte;
        std::string s = ss.str();
        bool bSuccess = false;
        CHECK_EQUAL((int)byte, (int)FTS::String::byteFromHex(s, bSuccess));
        CHECK(bSuccess);
    }

    bool bSuccess = false;
    CHECK_EQUAL(0, FTS::String::byteFromHex("", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("xyz", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("xy", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("0g", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("g", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("g0", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("XYZ", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("XY", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("0G", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("G", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("G0", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex(FTS::String::chr('0'-1), bSuccess));
    CHECK(!bSuccess);

    // For those, let's just make some little tests.

    CHECK_EQUAL("000102030405060708090A0B0C0D0E0F10FF", FTS::String::hexFromData("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\xFF",18));
    CHECK_EQUAL("000102030405060708090a0b0c0d0e0f10ff", FTS::String::hexFromData("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\xFF",18, false));

    // Hmmm...
    uint8_t* testData1 = FTS::String("000102030405060708090A0B0C0D0E0F10FF").dataFromHex();
    uint8_t* testData2 = FTS::String("000102030405060708090a0b0c0d0e0f10ff").dataFromHex();
    uint8_t* testData3 = FTS::String("000102030g05060708090a0b0c0d0e0f10ff").dataFromHex();
    if(testData1) {
        CHECK_EQUAL(0, memcmp(testData1, "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\xFF", 18));
        delete [] testData1;
    } else {
        FAIL("testData1 = NULL");
    }
    if(testData2) {
        CHECK_EQUAL(0, memcmp(testData2, "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\xFF", 18));
        delete [] testData2;
    } else {
        FAIL("testData2 = NULL");
    }
    CHECK_EQUAL(0, testData3);
}

TEST_INSUITE(dString, Fmt)
{
    CHECK_EQUAL("", FTS::String("").fmt(""));
    CHECK_EQUAL("abที่รักdefghi{10}", FTS::String("{1}{2}{3}{4}{5}{6}{7}{8}{9}{10}").fmt("a", "b", "ที่รัก", "d", "e", "f", "g", "h", "i"));
    CHECK_EQUAL("abcที่รักefghijklmnopqrs{10}u", FTS::String("a{1}c{2}e{3}g{4}i{5}k{6}m{7}o{8}q{9}s{10}u").fmt("b", "ที่รัก", "f", "h", "j", "l", "n", "p", "r"));
    CHECK_EQUAL("", FTS::String("").fmtRemoveEmpty(""));
    CHECK_EQUAL("abที่รักdefg{10}", FTS::String("{1}{2}{3}{4}{5}{6}{7}{8}{9}{10}").fmtRemoveEmpty("a", "b", "ที่รัก", "d", "e", "f", "g"));
    CHECK_EQUAL("abcที่รักefghijklmnoqs{10}u", FTS::String("a{1}c{2}e{3}g{4}i{5}k{6}m{7}o{8}q{9}s{10}u").fmtRemoveEmpty("b", "ที่รัก", "f", "h", "j", "l", "n"));
}

TEST_INSUITE(dString, Trivial)
{
    CHECK(FTS::String("").empty());
    CHECK(FTS::String::EMPTY.empty());

    CHECK_EQUAL(0, FTS::String("").len());
    CHECK_EQUAL(0, FTS::String("").lenInt());
    CHECK_EQUAL(1, FTS::String("a").len());
    CHECK_EQUAL(1, FTS::String("a").lenInt());
    CHECK_EQUAL(1, FTS::String("à").len());
    CHECK_EQUAL(1, FTS::String("à").lenInt());
    CHECK_EQUAL(5, FTS::String("abcdé").len());
    CHECK_EQUAL(5, FTS::String("abcdé").lenInt());
    CHECK_EQUAL(6, FTS::String("ที่รัก").len());
    CHECK_EQUAL(6, FTS::String("ที่รัก").lenInt());

    // Some conversion functions.
    CHECK_EQUAL(std::string(""), std::string(FTS::String("").c_str()));
    CHECK_EQUAL(std::string("abcdé"), std::string(FTS::String("abcdé").c_str()));
    CHECK_EQUAL(std::string(""), FTS::String("").str());
    CHECK_EQUAL(std::string("abcdé"), FTS::String("abcdé").str());
}

TEST_INSUITE(dString, Searching)
{
    CHECK_EQUAL(std::string::npos, FTS::String("").find(""));
    CHECK_EQUAL(std::string::npos, FTS::String("").find(FTS::String("")));
    CHECK_EQUAL(std::string::npos, FTS::String("abc").find(""));
    CHECK_EQUAL(std::string::npos, FTS::String("abc").find(FTS::String("")));
    CHECK_EQUAL(std::string::npos, FTS::String("").find("abc"));
    CHECK_EQUAL(std::string::npos, FTS::String("").find(FTS::String("abc")));
    CHECK_EQUAL(std::string::npos, FTS::String("a").find("a", 1));
    CHECK_EQUAL(std::string::npos, FTS::String("a").find(FTS::String("a"), 1));
    CHECK_EQUAL(std::string::npos, FTS::String("a").find("a", 100));
    CHECK_EQUAL(std::string::npos, FTS::String("a").find(FTS::String("a"), 100));
    CHECK_EQUAL(std::string::npos, FTS::String("a").find("b"));
    CHECK_EQUAL(std::string::npos, FTS::String("a").find(FTS::String("b")));
    CHECK_EQUAL(std::string::npos, FTS::String("bab").find("b", 3));
    CHECK_EQUAL(std::string::npos, FTS::String("bab").find(FTS::String("b"), 3));
    CHECK_EQUAL(4, FTS::String("bababbabbba").find("bbabb"));
    CHECK_EQUAL(4, FTS::String("bababbabbba").find(FTS::String("bbabb")));
    CHECK_EQUAL(4, FTS::String("bababbabbba").find("bbabb", 1));
    CHECK_EQUAL(4, FTS::String("bababbabbba").find(FTS::String("bbabb"), 1));
    CHECK_EQUAL(7, FTS::String("Héllô, ที่รัก!").find("ที่รัก"));
    CHECK_EQUAL(7, FTS::String("Héllô, ที่รัก!").find(FTS::String("ที่รัก")));
    CHECK_EQUAL(14, FTS::String("Héllô, ที่รัก Héllô!").find("Héllô", 1));
    CHECK_EQUAL(14, FTS::String("Héllô, ที่รัก Héllô!").find(FTS::String("Héllô"), 1));

    CHECK(FTS::String("").ieq(""));
    CHECK(FTS::String("").ieq(FTS::String("")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq("!abcdefghijklmnopqrstuvwxyz?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq(FTS::String("!abcdefghijklmnopqrstuvwxyz?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq("!ABCDEFGHIJKLMNOPQRSTUVWXYZ?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq(FTS::String("!ABCDEFGHIJKLMNOPQRSTUVWXYZ?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq("!AbCdEfGhIjKlMnOpQrStUvWxYz?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq(FTS::String("!AbCdEfGhIjKlMnOpQrStUvWxYz?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq("!aBcDeFgHiJkLmNoPqRsTuVwXyZ?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq(FTS::String("!aBcDeFgHiJkLmNoPqRsTuVwXyZ?")));
    CHECK(FTS::String("0123456789").ieq("0123456789"));
    CHECK(FTS::String("0123456789").ieq(FTS::String("0123456789")));
    CHECK(not FTS::String("0123456789").ieq(FTS::String("0123456788")));
    CHECK(not FTS::String("0123456789").ieq(FTS::String("1123456789")));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq("!ABCDEFGHIJKLMNOPQRSTUVWXYZ"));

    CHECK(FTS::String("").neq(""));
    CHECK(FTS::String("").neq(FTS::String("")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq("!abc"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq(FTS::String("!abc")));
    CHECK(not FTS::String("!abc").neq("!abcdefghijklmnopqrstuvwxyz?"));
    CHECK(not FTS::String("!abc").neq(FTS::String("!abcdefghijklmnopqrstuvwxyz?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq("!abcdefghijklmnopqrstuvwxyz?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq(FTS::String("!abcdefghijklmnopqrstuvwxyz?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq("!abcxyz", 0));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq(FTS::String("!abcxyz"), 0));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq("!abcxyz", 1));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq(FTS::String("!abcxyz"), 1));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq("!abcxyz", 2));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq(FTS::String("!abcxyz"), 2));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq("!abcxyz", 3));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq(FTS::String("!abcxyz"), 3));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq("!abcxyz", 4));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq(FTS::String("!abcxyz"), 4));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq("!abcxyz", 5));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").neq(FTS::String("!abcxyz"), 5));

    CHECK(FTS::String("").nieq(""));
    CHECK(FTS::String("").nieq(FTS::String("")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq("!abc"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq("!ABC"));
    CHECK(FTS::String("!aBcdefghijklmnopqrstuvwxyz?").nieq("!abC"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!abc")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!ABC")));
    CHECK(FTS::String("!aBcdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!abC")));
    CHECK(not FTS::String("!abc").nieq("!abcdefghijklmnopqrstuvwxyz?"));
    CHECK(not FTS::String("!aBc").nieq("!abCdefghijklmnopqrstuvwxyz?"));
    CHECK(not FTS::String("!abc").nieq(FTS::String("!abcdefghijklmnopqrstuvwxyz?")));
    CHECK(not FTS::String("!aBc").nieq(FTS::String("!abCdefghijklmnopqrstuvwxyz?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq("!abcdefghijklmnopqrstuvwxyz?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq("!ABCDEFGHIJKLMNOPQRSTUVWXYZ?"));
    CHECK(FTS::String("!aBcDeFgHiJkLmNoPqRsTuVwXyZ?").nieq("!AbCdEfGhIjKlMnOpQrStUvWxYz?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!abcdefghijklmnopqrstuvwxyz?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!ABCDEFGHIJKLMNOPQRSTUVWXYZ?")));
    CHECK(FTS::String("!aBcDeFgHiJkLmNoPqRsTuVwXyZ?").nieq(FTS::String("!AbCdEfGhIjKlMnOpQrStUvWxYz?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq("!abcxyz", 0));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq("!ABCXYZ", 0));
    CHECK(FTS::String("!AbCdefghijklmnopqrstuvwxyz?").nieq("!aBcXyZ", 0));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!abcxyz"), 0));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!ABCXYZ"), 0));
    CHECK(FTS::String("!AbCdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!aBcXyZ"), 0));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq("!abcxyz", 1));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nieq("!aBcxyz", 1));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!abcxyz"), 1));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!aBcxyz"), 1));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq("!abcxyz", 2));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nieq("!aBcxyz", 2));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!abcxyz"), 2));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!aBcxyz"), 2));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq("!abcxyz", 3));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nieq("!aBcxyz", 3));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!abcxyz"), 3));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!aBcxyz"), 3));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq("!abcxyz", 4));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nieq("!aBcxyz", 4));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!abcxyz"), 4));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!aBcxyz"), 4));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq("!abcxyz", 5));
    CHECK(not FTS::String("!abCdefghijklmnopqrstuvwxyz?").nieq("!aBcxyz", 5));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!abcxyz"), 5));
    CHECK(not FTS::String("!abCdefghijklmnopqrstuvwxyz?").nieq(FTS::String("!aBcxyz"), 5));

    CHECK(FTS::String("Hello, World!").contains("Hello"));
    CHECK(FTS::String("Hello, World!").contains("World"));
    CHECK(FTS::String("Hello, World!").contains("!"));
    CHECK(not FTS::String("").contains(""));
    CHECK(not FTS::String("Hello, World!").contains(""));
    CHECK(not FTS::String("").contains("Hello"));
    CHECK(not FTS::String("").contains("H"));
    CHECK(not FTS::String("Hello, World!").contains("n"));
    CHECK(not FTS::String("Hello, World!").contains("noob"));
}

TEST_INSUITE(dString, RemRepl)
{
    CHECK_EQUAL("", FTS::String("!").removeChar(0));
    CHECK_EQUAL("", FTS::String("").removeChar(0));
    CHECK_EQUAL("", FTS::String("").removeChar(1));
    CHECK_EQUAL("", FTS::String("!").removeChar(1));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("!Héllô, ที่รัก!").removeChar(0));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Hé!llô, ที่รัก!").removeChar(2));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก!?").removeChar(14));
    CHECK_EQUAL("Héllô, ที่รัก", FTS::String("Héllô, ที่รัก!").removeChar(13));
    CHECK_EQUAL("Héllô, ที่รัก", FTS::String("Héllô, ที่รัก!").removeChar(14));
    CHECK_EQUAL("Héllô, ที่รัก", FTS::String("Héllô, ที่รัก!").removeChar(-1));

    CHECK_EQUAL("!", FTS::String("").addChar(0, '!'));
    CHECK_EQUAL("!", FTS::String("").addChar(1, '!'));
    CHECK_EQUAL("!", FTS::String("").addChar(10, '!'));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("éllô, ที่รัก!").addChar(0, 'H'));
    CHECK_EQUAL("Hellô, ที่รัก!", FTS::String("Hllô, ที่รัก!").addChar(1, 'e'));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก").addChar(13, '!'));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก").addChar(14, '!'));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก").addChar(130, '!'));

    CHECK_EQUAL("ก", FTS::String("Héllô, ที่รัก").replaceStr(0, 12, ""));
    CHECK_EQUAL("ก", FTS::String("Héllô, ที่รัก").replaceStr(0, 12, FTS::String("")));
    CHECK_EQUAL("H", FTS::String("Héllô, ที่รัก").replaceStr(1, 12, ""));
    CHECK_EQUAL("H", FTS::String("Héllô, ที่รัก").replaceStr(1, 12, FTS::String("")));
    CHECK_EQUAL("Hก", FTS::String("Héllô, ที่รัก").replaceStr(1, 11, ""));
    CHECK_EQUAL("Hก", FTS::String("Héllô, ที่รัก").replaceStr(1, 11, FTS::String("")));
    CHECK_EQUAL("", FTS::String("Héllô, ที่รัก").replaceStr(0, 13, ""));
    CHECK_EQUAL("", FTS::String("Héllô, ที่รัก").replaceStr(0, 13, FTS::String("")));
    CHECK_EQUAL("", FTS::String("Héllô, ที่รัก").replaceStr(0, 120, ""));
    CHECK_EQUAL("", FTS::String("Héllô, ที่รัก").replaceStr(0, 120, FTS::String("")));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("you sücก").replaceStr(0, 8, "Héllô, ที่รัก!"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("you sücก").replaceStr(0, 8, FTS::String("Héllô, ที่รัก!")));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("you sücก").replaceStr(0, 120, "Héllô, ที่รัก!"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("you sücก").replaceStr(0, 120, FTS::String("Héllô, ที่รัก!")));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Méllô, ที่รัก!").replaceStr(0, 1, "H"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Méllô, ที่รัก!").replaceStr(0, 1, FTS::String("H")));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("llô, ที่รัก!").replaceStr(0, 0, "Hé"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("llô, ที่รัก!").replaceStr(0, 0, FTS::String("Hé")));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Fucking éllô, ที่รัก!").replaceStr(0, 8, "H"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Fucking éllô, ที่รัก!").replaceStr(0, 8, FTS::String("H")));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Fucking éllô, ที่รัก!").replaceStr(0, 9, "Hé"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Fucking éllô, ที่รัก!").replaceStr(0, 9, FTS::String("Hé")));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัorz?").replaceStr(12, 4, "ก!"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัorz?").replaceStr(12, 4, FTS::String("ก!")));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัorz?").replaceStr(12, 10, "ก!"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัorz?").replaceStr(12, 10, FTS::String("ก!")));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("you sücก").replaceStr(0, 120, "Héllô, ที่รัก!"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("you sücก").replaceStr(0, 120, FTS::String("Héllô, ที่รัก!")));
    CHECK_EQUAL("you sücHéllô, ที่รัก!", FTS::String("you sücก").replaceStr(7, -1, "Héllô, ที่รัก!"));
    CHECK_EQUAL("you sücHéllô, ที่รัก!", FTS::String("you sücก").replaceStr(7, -1, FTS::String("Héllô, ที่รัก!")));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก!").replaceStr(100, 3, "abc"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก!").replaceStr(100, 3, FTS::String("abc")));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก!").replaceStr(100, 120, "abc"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก!").replaceStr(100, 120, FTS::String("abc")));

    CHECK_EQUAL("", FTS::String("").replaceStr("", ""));
    CHECK_EQUAL("c", FTS::String("c").replaceStr("c", "c"));
    CHECK_EQUAL("c", FTS::String("c").replaceStr("a", "b"));
    CHECK_EQUAL("c", FTS::String("b").replaceStr("b", "c"));
    CHECK_EQUAL("asd", FTS::String("ard").replaceStr("ar", "as"));
    CHECK_EQUAL("dos", FTS::String("dos").replaceStr("dosa", "aaa"));
    CHECK_EQUAL("dos", FTS::String("dos").replaceStr("ados", "aaa"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô,ที่รัก!").replaceStr(",", ", "));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Bye, ที่รัก").replaceStr("Bye, ที่รัก", "Héllô, ที่รัก!"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Bye, ที่รัก!").replaceStr("Bye", "Héllô"));

}

TEST_INSUITE(dString, Trimming)
{
    CHECK_EQUAL("", FTS::String("").trimLeft());
    CHECK_EQUAL("", FTS::String(" \n \r \t ").trimLeft());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก!").trimLeft());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("  Héllô, ที่รัก!").trimLeft());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("\n\r\t Héllô, ที่รัก!").trimLeft());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String(" \n \r \t Héllô, ที่รัก!").trimLeft());
    CHECK_EQUAL("Héllô, ที่รัก!  ", FTS::String("Héllô, ที่รัก!  ").trimLeft());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("!!!?!?!?!11Héllô, ที่รัก!").trimLeft("!?1"));
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("!!!Héllô, ที่รัก!").trimLeft("!!!!"));

    CHECK_EQUAL("", FTS::String("").trimRight());
    CHECK_EQUAL("", FTS::String(" \n \r \t ").trimRight());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก!").trimRight());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก!  ").trimRight());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก!\n\r\t ").trimRight());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก! \n \r \t ").trimRight());
    CHECK_EQUAL("  Héllô, ที่รัก!", FTS::String("  Héllô, ที่รัก!").trimRight());
    CHECK_EQUAL("Héllô, ที่รัก", FTS::String("Héllô, ที่รัก?1!!!!?!?!?!11").trimRight("!?1"));
    CHECK_EQUAL("Héllô, ที่รัก", FTS::String("Héllô, ที่รัก!!!").trimRight("!!!!"));

    CHECK_EQUAL("", FTS::String("").trim());
    CHECK_EQUAL("", FTS::String(" \n \r \t ").trim());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("Héllô, ที่รัก!").trim());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("  Héllô, ที่รัก!  ").trim());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String("\n\r\t Héllô, ที่รัก!\n\r\t ").trim());
    CHECK_EQUAL("Héllô, ที่รัก!", FTS::String(" \n \r \t Héllô, ที่รัก! \n \r \t ").trim());
    CHECK_EQUAL("Héllô, ที่รัก", FTS::String("Héllô, ที่รัก").trim(" ,"));
    CHECK_EQUAL("Héllô, ที่รัก", FTS::String("!?!?!?!11Héllô, ที่รัก?1!!!!?!?!?!11").trim("!?1"));
    CHECK_EQUAL("Héllô, ที่รัก", FTS::String("!!!Héllô, ที่รัก!!!").trim("!!!!"));

    FTS::String hello("  Héllô  ");
    CHECK_EQUAL("Héllô  ", hello.trimLeft());
    CHECK_EQUAL("  Héllô  ", hello);
    CHECK_EQUAL("  Héllô", hello.trimRight());
    CHECK_EQUAL("  Héllô  ", hello);
    CHECK_EQUAL("Héllô", hello.trim());
    CHECK_EQUAL("  Héllô  ", hello);
}

TEST_INSUITE(dString, UpperLower)
{
    CHECK_EQUAL("", FTS::String("").lower());
    CHECK_EQUAL("0123456789", FTS::String("0123456789").lower());
    CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", FTS::String("abcdefghijklmnopqrstuvwxyz").lower());
    CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", FTS::String("ABCDEFGHIJKLMNOPQRSTUVWXYZ").lower());
    CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", FTS::String("AbCdEfGhIjKlMnOpQrStUvWxYz").lower());
    CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", FTS::String("aBcDeFgHiJkLmNoPqRsTuVwXyZ").lower());

    CHECK_EQUAL("", FTS::String("").upper());
    CHECK_EQUAL("0123456789", FTS::String("0123456789").upper());
    CHECK_EQUAL("ABCDEFGHIJKLMNOPQRSTUVWXYZ", FTS::String("ABCDEFGHIJKLMNOPQRSTUVWXYZ").upper());
    CHECK_EQUAL("ABCDEFGHIJKLMNOPQRSTUVWXYZ", FTS::String("abcdefghijklmnopqrstuvwxyz").upper());
    CHECK_EQUAL("ABCDEFGHIJKLMNOPQRSTUVWXYZ", FTS::String("AbCdEfGhIjKlMnOpQrStUvWxYz").upper());
    CHECK_EQUAL("ABCDEFGHIJKLMNOPQRSTUVWXYZ", FTS::String("aBcDeFgHiJkLmNoPqRsTuVwXyZ").upper());

    FTS::String a("AbCd");
    CHECK_EQUAL("abcd", a.lower());
    CHECK_EQUAL("AbCd", a);
    CHECK_EQUAL("ABCD", a.upper());
    CHECK_EQUAL("AbCd", a);
}

TEST_INSUITE(dString, Splitting)
{
    FTS::String out[5] = {"", "", "", "", ""};

    FTS::String("Hello World").split(out, " ");
    CHECK_EQUAL("Hello", out[0]);
    CHECK_EQUAL("World", out[1]);
    CHECK_EQUAL("", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);

    out[0] = ""; out[1] = ""; out[2] = ""; out[3] = ""; out[4] = "";

    FTS::String("Hello World !").split(out, " ");
    CHECK_EQUAL("Hello", out[0]);
    CHECK_EQUAL("World", out[1]);
    CHECK_EQUAL("!", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);

    out[0] = ""; out[1] = ""; out[2] = ""; out[3] = ""; out[4] = "";

    FTS::String(".a.c.").split(out, ".");
    CHECK_EQUAL("", out[0]);
    CHECK_EQUAL("a", out[1]);
    CHECK_EQUAL("c", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);

    out[0] = ""; out[1] = ""; out[2] = ""; out[3] = ""; out[4] = "";

    FTS::String("Hello...World").split(out, "...");
    CHECK_EQUAL("Hello", out[0]);
    CHECK_EQUAL("World", out[1]);
    CHECK_EQUAL("", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);

    out[0] = ""; out[1] = ""; out[2] = ""; out[3] = ""; out[4] = "";

    FTS::String("Hello...World...!").split(out, "...");
    CHECK_EQUAL("Hello", out[0]);
    CHECK_EQUAL("World", out[1]);
    CHECK_EQUAL("!", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);

    out[0] = ""; out[1] = ""; out[2] = ""; out[3] = ""; out[4] = "";

    FTS::String("...a...c...").split(out, "...");
    CHECK_EQUAL("", out[0]);
    CHECK_EQUAL("a", out[1]);
    CHECK_EQUAL("c", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);

    out[0] = ""; out[1] = ""; out[2] = ""; out[3] = ""; out[4] = "";

    FTS::String("Hello, World!").split(out, "x");
    CHECK_EQUAL("Hello, World!", out[0]);
    CHECK_EQUAL("", out[1]);
    CHECK_EQUAL("", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);
}

TEST_INSUITE(dString, PatternMatching)
{
    CHECK(FTS::String("Hello").matchesPattern("*"));
    CHECK(FTS::String("Hello").matchesPattern("*o"));
    CHECK(FTS::String("Hello").matchesPattern("*lo"));
    CHECK(FTS::String("Hello").matchesPattern("H*"));
    CHECK(FTS::String("Hello").matchesPattern("He*"));
    CHECK(FTS::String("Hello").matchesPattern("He*lo"));

    CHECK(FTS::String("Hello").matchesPattern("He?lo"));
#if D_QUEST_CAN_BE_NONE
    CHECK(FTS::String("Hello").matchesPattern("Hello?"));
#else
    CHECK(not FTS::String("Hello").matchesPattern("Hello?"));
#endif
}

class StringOperatorsSetup : public TestSetup
{
public:
    void setup()
    {
        s1 = "Hello, ";
        s2 = "Beautiful ";
        s3 = "World!";
    }
    void teardown() {}
protected:
    FTS::String s1, s2, s3;
};

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorSet)
{
    s1 = s2 = s3 = "Nanana";
    CHECK_EQUAL(s1.str(), s2.str());
    CHECK_EQUAL(s2.str(), s3.str());
    CHECK_EQUAL(s1.str(), s3.str());
    CHECK_EQUAL("Nanana", s1.str());
    CHECK_EQUAL("Nanana", s2.str());
    CHECK_EQUAL("Nanana", s3.str());

    s1 = s2 = s3 = "";
    CHECK(s1.empty());
    CHECK(s2.empty());
    CHECK(s3.empty());

    s1 = s2 = s3 = NULL;
    CHECK(s1.empty());
    CHECK(s2.empty());
    CHECK(s3.empty());

    s1 = "Hello";
    s1 = s1 = s1;
    CHECK_EQUAL("Hello", s1.str());
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorAddSet)
{
    s1 += "";
    CHECK_EQUAL("Hello, ", s1.str());

    s1 += NULL;
    CHECK_EQUAL("Hello, ", s1.str());

    s1 += FTS::String::EMPTY;
    CHECK_EQUAL("Hello, ", s1.str());

    s3 = FTS::String::EMPTY;
    s3 += s1;
    CHECK_EQUAL(s1.str(), s3.str());

    s3 = FTS::String::EMPTY;
    s3 += FTS::String::EMPTY;
    CHECK(s3.empty());

    s3 = FTS::String::EMPTY;
    s3 += "";
    CHECK(s3.empty());

    s3 = FTS::String::EMPTY;
    s3 += NULL;
    CHECK(s3.empty());

    s1 += s2;
    CHECK_EQUAL("Hello, Beautiful ", s1.str());
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorAdd)
{
    CHECK_EQUAL("Hello, Beautiful World!", s1 + s2 + s3);
    CHECK_EQUAL("Hello, ", s1 + "");
    CHECK_EQUAL("Hello, ", s1 + NULL);
    CHECK_EQUAL("Hello, ", s1 + FTS::String::EMPTY);

    CHECK((FTS::String::EMPTY + FTS::String::EMPTY).empty());
    CHECK(("" + FTS::String::EMPTY).empty());
    CHECK((FTS::String::EMPTY + "").empty());
    CHECK((NULL + FTS::String::EMPTY).empty());
    CHECK((FTS::String::EMPTY + NULL).empty());
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorEq)
{
    CHECK(not (s1 == s2));
    CHECK(not (s2 == s1));
    CHECK(not (s1 == FTS::String::EMPTY));
    CHECK(not (FTS::String::EMPTY == s1));
    CHECK(not (s1 == ""));
    CHECK(not ("" == s1));
    CHECK(not (s1 == NULL));
    CHECK(not (NULL == s1));
    CHECK(s1 == s1);
    CHECK(s1 == "Hello, ");
    CHECK(s1 == FTS::String("Hello, "));
    CHECK("Hello, " == s1);
    CHECK(FTS::String("Hello, ") == s1);
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorNotEq)
{
    CHECK(s1 != s2);
    CHECK(s2 != s1);
    CHECK(s1 != FTS::String::EMPTY);
    CHECK(FTS::String::EMPTY != s1);
    CHECK(s1 != "");
    CHECK("" != s1);
    CHECK(s1 != NULL);
    CHECK(NULL != s1);
    CHECK(not (s1 != s1));
    CHECK(not (s1 != "Hello, "));
    CHECK(not (s1 != FTS::String("Hello, ")));
    CHECK(not ("Hello, " != s1));
    CHECK(not (FTS::String("Hello, ") != s1));
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorNot)
{
    CHECK(not !s1);
    CHECK(!FTS::String::EMPTY);
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorLess)
{
    CHECK(not ("" < FTS::String::EMPTY));
    CHECK(not (FTS::String::EMPTY < ""));
    CHECK("" < FTS::String("Hello"));
    CHECK(FTS::String("") < "Hello");
    CHECK(not ("Hello" < FTS::String("")));
    CHECK(not (FTS::String("Hello") < ""));
    CHECK("abcd" < FTS::String("abce"));
    CHECK(FTS::String("abcd") < "abce");
    CHECK("bbcd" < FTS::String("dbce"));
    CHECK(FTS::String("bbcd") < "dbce");
    CHECK(not ("abcd" < FTS::String("abcc")));
    CHECK(not (FTS::String("abcd") < "abcc"));
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorAccessors)
{
    CHECK_EQUAL('H', s1[0]);
    CHECK_EQUAL('e', s1[1]);
    CHECK_EQUAL(' ', s1[6]);
    try {
        s1[7];
        s1[80];
        FAIL("No exception thrown by the string when accessed out-of-bounds");
    } catch(const std::out_of_range&) {
    }
    CHECK_EQUAL('H', s1.getCharAt(0));
    CHECK_EQUAL('e', s1.getCharAt(1));
    CHECK_EQUAL(' ', s1.getCharAt(6));
    CHECK_EQUAL('\0', s1.getCharAt(7));
    CHECK_EQUAL('\0', s1.getCharAt(80));
}

TEST_INSUITE(dString, Empty)
{
    CHECK(FTS::String::EMPTY.empty());
}


