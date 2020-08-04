#include "pch.h"
#include "CppUnitTest.h"
#include "handled.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace handled
{
#define PAGE_SIZE 3
	struct Foo : Handled<Foo, PAGE_SIZE> {
		static inline int iteration = 0;
		static void reset_iteration() {
			iteration = 0;
		}
		int i = 0;
		void without_arg() {
			++iteration;
			Logger::WriteMessage("Iterating without arg ");
			Logger::WriteMessage(std::to_string(handle.id).c_str());
			Logger::WriteMessage("\n");
		}
		void with_arg(int* arg) {
			iteration += *arg;
		}
	};

	TEST_CLASS(handled) {
	public:
		
		TEST_METHOD(Setup) {
			Foo::setup();
			Foo::Handler::Page* first_page = &Foo::handler->page;
			Assert::AreEqual((size_t)0, first_page->index);
			Assert::IsTrue(&Foo::handler->page == Foo::handler->page_last);
			for (int i = 0; i < 3; ++i) {
				Assert::AreEqual(false, first_page->memory[i].active);
			}
		}

		TEST_METHOD(FillPage) {
			Foo::setup();
			for (int i = 0; i < PAGE_SIZE; ++i) Foo::create();
			Assert::IsTrue(Foo::handler->page.is_full);
			Assert::IsTrue(&Foo::handler->page == Foo::handler->page_last);
		}

		TEST_METHOD(AddPage) {
			Foo::setup();
			for (int i = 0; i < PAGE_SIZE; ++i) Foo::create();
			Foo::Handler::Page* first_page = &Foo::handler->page;
			Assert::IsTrue(first_page->is_full);
			Foo::create();
			Assert::IsTrue(&Foo::handler->page != Foo::handler->page_last);
			Assert::IsTrue(first_page->page_next == Foo::handler->page_last);
			Assert::IsTrue(Foo::handler->page_last->index == 1);
			Assert::AreEqual((size_t)(PAGE_SIZE), Foo::handler->page_last->memory[0].handle.id);
		}

		TEST_METHOD(IterateWithoutArg) {
			Foo::setup();
			for (int i = 0; i < PAGE_SIZE; ++i) Foo::create();
			Foo::reset_iteration();
			Foo::iterate(&Foo::without_arg);
			Assert::AreEqual(PAGE_SIZE, Foo::iteration);
		}

		TEST_METHOD(IterateWithArg) {
			int num = 2;
			Foo::setup();
			for (int i = 0; i < PAGE_SIZE; ++i) Foo::create();
			Foo::reset_iteration();
			Foo::iterate(&Foo::with_arg, &num);
			Assert::AreEqual(PAGE_SIZE * num, Foo::iteration);
		}

		TEST_METHOD(IterateWithoutArgTwoPages) {
			Foo::setup();
			for (int i = 0; i < PAGE_SIZE + 1; ++i) Foo::create();
			Foo::reset_iteration();
			Foo::iterate(&Foo::without_arg);
			Assert::AreEqual(PAGE_SIZE + 1, Foo::iteration);
		}

		TEST_METHOD(IterateWithArgTwoPages) {
			int num = 2;
			Foo::setup();
			for (int i = 0; i < PAGE_SIZE + 1; ++i) Foo::create();
			Foo::reset_iteration();
			Foo::iterate(&Foo::with_arg, &num);
			Assert::AreEqual((PAGE_SIZE + 1) * num, Foo::iteration);
		}
	};
}
