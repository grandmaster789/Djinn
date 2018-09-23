#include "stdafx.h"
#include "CppUnitTest.h"

#include "util/string_util.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace {
    struct Indicator {
        Indicator() = default; // -> m_Val == 1

        Indicator(const Indicator& ):
            m_Val(2)
        {
        }

        Indicator& operator = (const Indicator& ) {
            m_Val = 3;
            return *this;
        }

        Indicator(Indicator&& i) noexcept:
            m_Val(4)
        {
            i.m_Val = 5;
        }

        Indicator& operator = (Indicator&& i) noexcept {
            m_Val = 6;
            i.m_Val = 5;
            return *this;
        }

        bool isDefaultConstructed() const noexcept { return m_Val == 1; }
        bool isCopyConstructed()    const noexcept { return m_Val == 2; }
        bool isCopyAssigned()       const noexcept { return m_Val == 3; }
        bool isMoveConstructed()    const noexcept { return m_Val == 4; }
        bool isMovedFrom()          const noexcept { return m_Val == 5; }
        bool isMoveAssigned()       const noexcept { return m_Val == 6; }
        
        int m_Val = 1;
    };

    std::ostream& operator << (std::ostream& os, const Indicator& i) {
        os << i.m_Val;
        return os;
    }
}

namespace DjinnTest{
    TEST_CLASS(StringUtil) {
    public:
        TEST_METHOD(stringify) {
            using djinn::util::stringify;

            {
                auto s = stringify(1, 2, 3);
                Assert::IsTrue(s == "123");
            }

            {
                Indicator a;
                Assert::IsTrue(a.isDefaultConstructed());

                auto s1 = stringify(a, Indicator());
                Assert::IsTrue(s1 == "11");
                Assert::IsTrue(a.isDefaultConstructed());

                auto b = a; // equivalent to: auto b(a);
                Assert::IsTrue(b.isCopyConstructed());

                auto c = std::move(a); // equivalent to auto c(std::move(a));
                Assert::IsTrue(c.isMoveConstructed());
                Assert::IsTrue(a.isMovedFrom());

                auto s2 = stringify(a, b, c);
                Assert::IsTrue(s2 == "524");

                b = a;
                Assert::IsTrue(b.isCopyAssigned());

                c = std::move(a);
                Assert::IsTrue(c.isMoveAssigned());
                
                auto s3 = stringify(a, b, c);
                Assert::IsTrue(s3 == "536");
            }
        }
    };
}