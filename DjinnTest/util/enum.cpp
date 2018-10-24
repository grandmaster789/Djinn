#include "stdafx.h"
#include "CppUnitTest.h"

#include "util/enum.h"
#include <sstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace {
    enum class FooBar {
        aaa,
        bbb,
        ccc,
        ddd,
        eee,
        fff
    };

    std::ostream& operator << (std::ostream& os, const FooBar& fb) {
        switch (fb) {
        case FooBar::aaa: os << "aaa"; break;
        case FooBar::bbb: os << "bbb"; break;
        case FooBar::ccc: os << "ccc"; break;
        case FooBar::ddd: os << "ddd"; break;
        case FooBar::eee: os << "eee"; break;
        case FooBar::fff: os << "fff"; break;
        }

        return os;
    }

    using FooBarIterator = djinn::util::EnumIterator<FooBar, FooBar::aaa, FooBar::fff>;
}

namespace DjinnTest {
    TEST_CLASS(Enum) {
    public:
        TEST_METHOD(enum_iterator) {
            {
                std::stringstream sstr;

                FooBar fb = FooBar::ddd;

                for (auto it = FooBarIterator(fb); it != FooBarIterator::end(); ++it)
                    sstr << *it;
                
                std::string result   = sstr.str();
                std::string expected = "dddeeefff";

                Assert::IsTrue(result == expected);
            }

            {
                std::stringstream sstr;

                for (auto val : FooBarIterator())
                    sstr << val;

                std::string result = sstr.str();
                std::string expected = "aaabbbcccdddeeefff";

                Assert::IsTrue(result == expected);
            }
        }
    };
}