#include "stdafx.h"
#include "CppUnitTest.h"

#include "util/reflect.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace djinn::util::reflect;

namespace {
	struct Foo {
		int    i;
		bool   b;
		double d;
	};

	struct Bar {
		int   i  = 234;
		bool  b  = false;
		float f  = 56.7f;
		int*  ip = nullptr;
	};

	struct Baz {
		std::unique_ptr<int>    up = std::make_unique<int>(678);
		std::shared_ptr<double> sp = std::make_shared<double>(7.89);
	};

	struct Frob {
	private:
		int    i = 111;
		double d = 22.2;
	};

	struct Niz {
		Niz() = default;
		Niz(const Niz&) = delete;
	private:
		int   i = 333;
		float f = 4.44f;
	};
}

namespace DjinnTest {
	TEST_CLASS(Reflect) {
	public:
		TEST_METHOD(is_brace_constructible) noexcept {
			// Foo has no constructor generated, but still should be aggregate
			// initializable with 0-3 arguments
			static_assert( detail::is_brace_constructible<Foo, 0>());
			static_assert( detail::is_brace_constructible<Foo, 1>());
			static_assert( detail::is_brace_constructible<Foo, 2>());
			static_assert( detail::is_brace_constructible<Foo, 3>());
			static_assert(!detail::is_brace_constructible<Foo, 4>());

			// Bar provides default arguments for everything, so 0-4 arguments should work
			static_assert( detail::is_brace_constructible<Bar, 0>());
			static_assert( detail::is_brace_constructible<Bar, 1>());
			static_assert( detail::is_brace_constructible<Bar, 2>());
			static_assert( detail::is_brace_constructible<Bar, 3>());
			static_assert( detail::is_brace_constructible<Bar, 4>());
			static_assert(!detail::is_brace_constructible<Bar, 5>());

			// Baz provides default constructors, just with somewhat complicated types
			// so 0-2 arguments should work
			static_assert( detail::is_brace_constructible<Baz, 0>());
			static_assert( detail::is_brace_constructible<Baz, 1>());
			static_assert( detail::is_brace_constructible<Baz, 2>());
			static_assert(!detail::is_brace_constructible<Baz, 3>());

			// Frob only has private members, but they are default constructed
			// so 0 arguments should work, and for 1 argument the generated
			// copy constructor should be valid
			static_assert( detail::is_brace_constructible<Frob, 0>());
			static_assert( detail::is_brace_constructible<Frob, 1>());
			static_assert(!detail::is_brace_constructible<Frob, 2>());

			// Niz has deleted the copy constructor, but is otherwise pretty much
			// identical to Frob. Here only the default constructor should work.
			static_assert( detail::is_brace_constructible<Niz, 0>());
			static_assert(!detail::is_brace_constructible<Niz, 1>());
			static_assert(!detail::is_brace_constructible<Niz, 2>());
		}
		
		TEST_METHOD(fieldCount) noexcept {
            Foo foo = {};
            Bar bar = {};
            Baz baz = {};
            Frob frob = {};
            Niz niz = {};

			static_assert(getFieldCount(foo).value == 3);
			static_assert(getFieldCount(bar).value == 4);
			static_assert(getFieldCount(baz).value == 2);

			// for these two, getFieldCount will report erroneous values, 
			// because the fields are private
			static_assert(getFieldCount(frob).value == 1);
			static_assert(getFieldCount(niz).value == 0);
		}

		TEST_METHOD(eachField) {
			Foo foo = { 1, true, 3.4 };

			std::stringstream sstr;
			sstr << std::boolalpha;

			forEachField(foo, [&](auto field) { sstr << field << " "; });

			Assert::IsTrue(sstr.str() == "1 true 3.4 ");
		}
	};
}