#include "stdafx.h"
#include "CppUnitTest.h"

#include "util/string_util.h"
#include "../indicator.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DjinnTest {
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