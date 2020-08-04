#include "pch.h"
#include "CppUnitTest.h"
#include "handled.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define PAGE_SIZE 3

namespace handled
{
	struct Foo : Handled<Foo, PAGE_SIZE> {
		static inline int iteration = 0;
		static void reset_iteration() {
			iteration = 0;
		}
		int i = 0;
		void without_arg() {
			++iteration;
			Logger::WriteMessage("Iterating #");
			Logger::WriteMessage(std::to_string(handle.id).c_str());
			Logger::WriteMessage(" without arg\n");
		}
		void with_arg(int* arg) {
			iteration += *arg;
			Logger::WriteMessage("Iterating #");
			Logger::WriteMessage(std::to_string(handle.id).c_str());
			Logger::WriteMessage(" with arg: ");
			Logger::WriteMessage(std::to_string(*arg).c_str());
			Logger::WriteMessage("\n");
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
			Assert::AreEqual((size_t)1, Foo::handler->page_last->index);
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

		TEST_METHOD(Destroy) {
			Foo::setup();
			for (int i = 0; i < PAGE_SIZE - 2; ++i) Foo::create();
			Foo* to_destroy = Foo::create();
			Foo::create();
			to_destroy->destroy();
			Assert::IsTrue(Foo::handler->page.is_full); // page should be full because obj is destroying but not yet removed
			Foo::reset_iteration();
			Foo::iterate(&Foo::without_arg);
			Assert::AreEqual(2, Foo::iteration); // destroyed obj is skipped for iteration
			Foo::cleanup();
			Assert::IsFalse(Foo::handler->page.is_full); // page should NOT be full since we just cleaned up destroying obj
			Foo::create();
			Assert::IsTrue(Foo::handler->page.is_full); // page should be full once again
		}
	};
}
