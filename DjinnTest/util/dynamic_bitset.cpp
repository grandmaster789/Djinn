#include "stdafx.h"
#include "CppUnitTest.h"

#include "util/dynamic_bitset.h"
#include <sstream>
#include <vector>
#include <map>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace djinn::util;

namespace DjinnTest {
    TEST_CLASS(TestDynamicBitset) {
    public:
        TEST_METHOD(setSingle) {
            DynamicBitset db(20);

            Assert::IsTrue(db.size() == 20);

            for (size_t i = 0; i < 20; ++i)
                Assert::IsFalse(db.test(i));

            Assert::IsTrue(db.count(false) == 20);
            Assert::IsTrue(db.count(true)  == 0);

            db.set(2);
            db.set(14);
            db.set(19);

            Assert::IsTrue(db.count(true) == 3);

            Assert::IsTrue(db.test(2));
            Assert::IsTrue(db.test(14));
            Assert::IsTrue(db.test(19));

            db.set(2, false);
            db.set(14, false);
            db.set(19, false);

            for (size_t i = 0; i < 20; ++i)
                Assert::IsFalse(db.test(i));

            DynamicBitset cd(16);

            for (size_t i = 0; i < 20; ++i)
                Assert::IsFalse(cd.test(i));

            cd.set(0);
            cd.set(2);
            cd.set(14);
            cd.set(15);

            Assert::IsTrue(cd.test(0));
            Assert::IsTrue(cd.test(2));
            Assert::IsTrue(cd.test(14));
            Assert::IsTrue(cd.test(15));

            cd.set(0, false);
            cd.set(2, false);
            cd.set(14, false);
            cd.set(15, false);

            for (size_t i = 0; i < 20; ++i)
                Assert::IsFalse(cd.test(i));
        }

        TEST_METHOD(setRange) {
            DynamicBitset db(20);

            db.set(2, 4, true);
            
            Assert::IsTrue(db.test(2));
            Assert::IsTrue(db.test(3));
            Assert::IsFalse(db.test(4));

            db.set(0, 16, true);
            for (size_t i = 0; i < 16; ++i)
                Assert::IsTrue(db.test(i));

            db.set(15, 20, true);
            for (size_t i = 0; i < 20; ++i)
                Assert::IsTrue(db.test(i));

            db.set(0, 20, false);
            for (size_t i = 0; i < 20; ++i)
                Assert::IsFalse(db.test(i));

            // --------------
            DynamicBitset cb(16);

            cb.set(2, 4, true);

            Assert::IsTrue(cb.test(2));
            Assert::IsTrue(cb.test(3));
            Assert::IsFalse(cb.test(4));

            cb.set(0, 8, true);
            for (size_t i = 0; i < 8; ++i)
                Assert::IsTrue(cb.test(i));

            cb.set(8, 16, true);
            for (size_t i = 0; i < 16; ++i)
                Assert::IsTrue(cb.test(i));

            cb.set(0, 16, false);
            for (size_t i = 0; i < 16; ++i)
                Assert::IsFalse(cb.test(i));

        }

        TEST_METHOD(findOccupied) {
            DynamicBitset db(20);

            db.set(4);
            db.set(16);
            db.set(19);

            Assert::IsTrue(db.findNext(0, 20, true) == 4);
            Assert::IsTrue(db.findNext(4, 20, true) == 4);
            Assert::IsTrue(db.findNext(5, 20, true) == 16);
            Assert::IsTrue(db.findNext(16, 20, true) == 16);
            Assert::IsTrue(db.findNext(17, 20, true) == 19);
            Assert::IsTrue(db.findNext(19, 20, true) == 19);

            db.set(19, false);

            Assert::IsTrue(db.findNext(17, 20, true) == 20); // not found
        }

        TEST_METHOD(findFree) {
            DynamicBitset db(20);

            db.set(8, 16, true);

            Assert::IsTrue(db.findNext(0, 20, false) == 0);
            Assert::IsTrue(db.findNext(8, 20, false) == 16);
            Assert::IsTrue(db.findNext(12, 20, false) == 16);
        }
    };
}