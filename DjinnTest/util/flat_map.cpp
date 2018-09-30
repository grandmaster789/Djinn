#include "stdafx.h"
#include "CppUnitTest.h"

#include "util/flat_map.h"
#include "../indicator.h"

#include <sstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DjinnTest {
    TEST_CLASS(FlatMap) {
    public:
        TEST_METHOD(insert) {
            using djinn::util::FlatMap;

            FlatMap<int, int> fm;

            fm.assign_or_insert(1, 2);
            fm.assign_or_insert(3, 4);
            fm.assign_or_insert(5, 6);
            fm.assign_or_insert(7, 8);
            fm.assign_or_insert(9, 0);

            fm.assign_or_insert(2, 3);
            fm.assign_or_insert(4, 5);
            fm.assign_or_insert(6, 7);
            fm.assign_or_insert(8, 9);

            Assert::IsTrue(*fm[4] == 5);
            Assert::IsTrue(fm[999] == nullptr);

            Assert::IsTrue(std::is_sorted(
                fm.getKeys().cbegin(),
                fm.getKeys().cend())
            );
        }

        TEST_METHOD(assign_size) {
            using djinn::util::FlatMap;

            FlatMap<int, int> fm;

            fm.assign_or_insert(1, 2);
            fm.assign_or_insert(3, 4);

            Assert::IsTrue(*fm[1] == 2);
            Assert::IsTrue(fm.size() == 2);

            fm.assign_or_insert(1, 123);
            Assert::IsTrue(*fm[1] == 123);
            Assert::IsTrue(fm.size() == 2);
        }

        TEST_METHOD(erase_contains_clear) {
            using djinn::util::FlatMap;

            FlatMap<int, int> fm;

            fm.assign_or_insert(1, 2);
            fm.assign_or_insert(3, 4);

            Assert::IsTrue(fm.contains(3));
            Assert::IsFalse(fm.contains(666));

            fm.erase(3);
            Assert::IsFalse(fm.contains(3));
            
            Assert::IsTrue(fm.size() > 0);

            fm.clear();
            Assert::IsTrue(fm.size() == 0);
        }

        TEST_METHOD(foreach) {
            using djinn::util::FlatMap;

            FlatMap<int, double> fm;

            fm.assign_or_insert(1, 2.3);
            fm.assign_or_insert(2, 4.5);
            fm.assign_or_insert(3, 6.7);
            fm.assign_or_insert(4, 8.9);
            
            int key_sum = 0;
            double value_sum = 0.0;
            
            fm.foreach([&](auto key, auto val) {
                key_sum += key;
                value_sum += val;
            });

            Assert::IsTrue(key_sum = 1 + 2 + 3 + 4);
            Assert::IsTrue(value_sum = 2.3 + 4.5 + 6.7 + 8.9);
        }
    };
}