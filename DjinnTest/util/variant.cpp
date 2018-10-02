#include "stdafx.h"
#include "CppUnitTest.h"

#include "util/variant.h"
#include "../indicator.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DjinnTest {
    TEST_CLASS(Variant) {
    public:
        TEST_METHOD(overload_set) {
            auto oset = djinn::util::OverloadSet(
                [](int)      { return 123; },
                [](double)   { return 456; },
                [](uint64_t) { return 789; }
            );

            auto xn = oset(2);
            auto yn = oset(2.0);
            auto zn = oset(2ull);

            Assert::IsTrue(xn == 123);
            Assert::IsTrue(yn == 456);
            Assert::IsTrue(zn == 789);
        }

        TEST_METHOD(matcher) {
            auto matcher = djinn::util::match(
                [](int)      { return 123; },
                [](double)   { return 456; },
                [](uint64_t) { return 789; }
            );

            using VarType = std::variant<int, double, uint64_t>;

            VarType var;

            var = 2;
            Assert::IsTrue(matcher(var) == 123);

            var = 2.0;
            Assert::IsTrue(matcher(var) == 456);

            var = 2ull;
            Assert::IsTrue(matcher(var) == 789);
        }
    };
}