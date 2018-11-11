#include "stdafx.h"
#include "CppUnitTest.h"
#include <sstream>
#include <string>

#include "util/variant.h"

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

		TEST_METHOD(multi_match) {
			int state = 0;

			using VarType = std::variant<int, double, uint64_t>;

			auto matcher = djinn::util::match(
				[&](int,      int)      { state = 1; },
				[&](double,   int)      { state = 2; },
				[&](uint64_t, int)      { state = 3; },

				[&](int,      double)   { state = 4; },
				[&](double,   double)   { state = 5; },
				[&](uint64_t, double)   { state = 6; },

				[&](int,      uint64_t) { state = 7; },
				[&](double,   uint64_t) { state = 8; },
				[&](uint64_t, uint64_t) { state = 9; }
			);

			VarType v1 = 1;
			VarType v2 = 2.0;
			VarType v3 = 3ull;

			matcher(v1, v1); Assert::IsTrue(state == 1);
			matcher(v2, v1); Assert::IsTrue(state == 2);
			matcher(v3, v1); Assert::IsTrue(state == 3);

			matcher(v1, v2); Assert::IsTrue(state == 4);
			matcher(v2, v2); Assert::IsTrue(state == 5);
			matcher(v3, v2); Assert::IsTrue(state == 6);

			matcher(v1, v3); Assert::IsTrue(state == 7);
			matcher(v2, v3); Assert::IsTrue(state == 8);
			matcher(v3, v3); Assert::IsTrue(state == 9);
		}
    };
}