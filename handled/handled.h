#pragma once
#include <iostream>
/*
TODO:
 - check all previous functionality works
 - check that adding pages works
 - allow for sorting vacancies (?)
 - change destroying to only mark as destroying
 - add cleanup function to finalize destruction of all destroying objects
 - figure out what to do about "exception" cases (probably don't actually use exceptions)
 - figure out operator overloading
    - new/delete for Handled? (instead of create()/destroy())
    - dereference for Handle?
 - optional (#if DEBUG?) logging
    - total count of all objects created/destroyed
    - additional (past 0) Pages allocated
*/

template < typename T, size_t COUNT >
struct Handled {
    struct Handle;
    struct Handler {
        struct Page {
            const size_t index;
            T memory[COUNT] = { {0} };
            size_t vacancies[COUNT] = { {0} };
            size_t vacancy_index = 0;
            bool is_full = false;
            Page* page_next = nullptr;
            inline void vacancy_push(size_t index) {
                if (!is_full) ++vacancy_index;
                is_full = false;
                memory[index].active = false;
                vacancies[vacancy_index] = index;
            }
            inline size_t vacancy_pop() {
                if (vacancy_index == 0) {
                    is_full = true;
                    return vacancies[vacancy_index];
                }
                return vacancies[vacancy_index--];
            }
            Page(size_t index_) : index(index_), page_next(nullptr) {
                // init vacancies
                size_t index = COUNT - 1;
                for (size_t i = 0; i < COUNT; ++i) {
                    T* slot = &memory[i];
                    slot->active = false;
                    vacancies[index--] = i;
                }
                vacancy_index = COUNT - 1;
            }
        };
        Page page;
        size_t page_count = 0;
        Page* page_last = nullptr;
        size_t gen_next = 0;
        Handler() : page(Page(0)) {
            page_last = &page;
        }
        T* next() {
            Page* current_page = &page;

            while (current_page->is_full) {
                ++page_count;
                current_page->page_next = new Page(current_page->index + 1);
                // TODO
                current_page = current_page->page_next;
                page_last = current_page;
            }
            size_t vacancy = current_page->vacancy_pop();
            T* next = &(current_page->memory[vacancy]);
            next->active = true;
            next->handle.id = current_page->index * COUNT + vacancy;
            next->handle.gen = gen_next++;
            next->handle.handler = this;
            return next;
        }
        Page* get_page_from_id(size_t id) {
            Page* current_page = &page;
            while (id >= COUNT && current_page != nullptr) {
                id -= COUNT;
                current_page = current_page->page_next;
            }
            if (current_page == nullptr) return nullptr; // EXCEPTION
            return current_page;
        }
        T* get_from_handle(const Handle* handle) {
            size_t id = handle->id;
            Page* current_page = get_page_from_id(id);
            if (current_page == nullptr) return nullptr; // EXCEPTION;
            T* t = &(current_page->memory[id]);
            if (t->handler.gen != handle->gen) return nullptr; // EXCEPTION
            return t;
        }
        void deactivate(size_t id) {
            Page* current_page = get_page_from_id(id);
            if (current_page == nullptr) return; // EXCEPTION;
            current_page->vacancy_push(id % COUNT);
        }
        void iterate(void (T::* method)()) {
            Page* current_page = &page;
            while (current_page != nullptr) {
                T* current_item = current_page->memory;
                while (current_item < current_page->memory + COUNT) {
                    if (current_item->active) (current_item->*method)();
                    ++current_item;
                }
                current_page = current_page->page_next;
            }
        }
        template < typename U >
        void iterate(void (T::* method)(U*), U* ptr) {
            Page* current_page = &page;
            while (current_page != nullptr) {
                T* current_item = current_page->memory;
                while (current_item < current_page->memory + COUNT) {
                    if (current_item->active) (current_item->*method)(ptr);
                    ++current_item;
                }
                current_page = current_page->page_next;
            }
        }
    };
    struct Handle {
        size_t id;
        size_t gen;
        Handler* handler;
        Handle() : id(0), gen(0), handler(nullptr) {}
        Handle(size_t id, size_t gen, Handler* handler) : id(id), gen(gen), handler(handler) {}
        Handle(const Handle& handle) : id(handle.id), gen(handle.gen), handler(handle.handler) {}
        inline bool is_valid() { return handler != nullptr; /* TODO */ }
        Handle& operator=(const Handle& other) {
            if (&other == this) return *this;
            id = other.id;
            gen = other.gen;
            handler = other.handler;
        }
        bool operator==(const Handle& other) {
            return id == other.id && gen == other.gen && handler == other.handler;
        }
        T* operator*() {
            return handler->get_from_handle(this);
        }
    };
    bool active = true;
    Handle handle;
    static inline Handler* handler = nullptr;
    static inline void setup() {
        if (handler != nullptr) delete handler; // TODO: log?
        handler = new Handler();
    }
    static inline T* create() {
        return handler->next();
    }
    void destroy() {
        if (!active) return; // EXCEPTION
        active = false;
        handler->deactivate(handle.id);
    }
    static inline void iterate(void (T::* method)()) {
        handler->iterate(method);
    }
    template < typename U >
    static inline void iterate(void (T::* method)(U*), U* ptr) {
        handler->iterate(method, ptr);
    }
};
