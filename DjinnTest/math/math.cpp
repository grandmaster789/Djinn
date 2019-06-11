#include "stdafx.h"
#include "CppUnitTest.h"

#include "math/math.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace djinn::math;

/*
	constexpr auto max(const T& v0, const U& v1);
	constexpr auto max(const T& v0, const Ts&... vs);
	constexpr auto min(const T& v0, const U& v1);
	constexpr auto min(const T& v0, const Ts&... vs);
*/
namespace DjinnTest {
	struct Foo {
		int    i;
		float  f;
		bool   b;
		double d;
		int    j;
	};

#pragma pack(push, 1)
	struct Bar {
		char c[3];
	};
#pragma pack(pop)

	TEST_CLASS(Math) {
	public:
		TEST_METHOD(alignment) {
			constexpr size_t fooSize = sizeof(Foo); // default alignments is 4, so this ends up being 32 bytes
			constexpr size_t barSize = sizeof(Bar); // custom struct layout, 3 bytes

			constexpr size_t smaller = alignToSmaller(sizeof(Foo), sizeof(Bar));
			constexpr size_t larger  = alignToLarger(sizeof(Bar), sizeof(Foo));

			Assert::IsTrue(fooSize == 32);
			Assert::IsTrue(barSize == 3);
			Assert::IsTrue(smaller == 30);
			Assert::IsTrue(larger  == 32);
		}

		TEST_METHOD(rounding) {
			Assert::IsTrue(roundToInt(0.6)  ==  1);
			Assert::IsTrue(roundToInt(0.5)  ==  1);
			Assert::IsTrue(roundToInt(0)    ==  0);
			Assert::IsTrue(roundToInt(-0.4) ==  0);
			Assert::IsTrue(roundToInt(-0.5) == -1);

			for (int i = -100; i < 100; ++i) {
				float f = i * 0.01f - 1.0f;

				Assert::IsTrue(roundToInt(f) == static_cast<int>(std::round(f)));
			}
		}
	
		TEST_METHOD(minmax) {
			Assert::IsTrue(max(0, 1, 2, 3, 4, 5) == 5);
			Assert::IsTrue(min(0, 1, 2, 3, 4, 5) == 0);

			Assert::IsTrue(max(5, 4, 3, 2, 1, 0) == 5);
			Assert::IsTrue(min(5, 4, 3, 2, 1, 0) == 0);

			Assert::IsTrue(max(-0.5f, -0.4f, -0.3f, -0.2f, -0.1f, 0.0f) ==  0.0f);
			Assert::IsTrue(min(-0.5f, -0.4f, -0.3f, -0.2f, -0.1f, 0.0f) == -0.5f);
		}
	};
}