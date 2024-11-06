#ifndef GAPBUFFER_H
#define GAPBUFFER_H

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ostream>
#include <string_view>
#include <type_traits>
#include <utility>

// Gap buffer data structure implementation: https://en.wikipedia.org/wiki/Gap_buffer
class Gapbuffer {
    public: 
        // member types
        using value_type = char;
        using allocator_type = std::allocator<char>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = char&;
        using const_reference = const char&;
        using pointer = std::allocator_traits<std::allocator<char>>::pointer;
        using const_pointer = std::allocator_traits<std::allocator<char>>::const_pointer;

    private:
        // Iterator built from: https://medium.com/p/fc5b994462c6#90c4
        // https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
        // https://medium.com/@joao_vaz/c-iterators-and-implementing-your-own-custom-one-a-primer-72f1506e5d71

        template <typename pointer_type>
        class IteratorTemplate {
            public:
                static const bool is_const = std::is_const_v<std::remove_pointer_t<pointer_type>>;

                using value_type = 
                    typename std::conditional<is_const, const char, char>::type;
                using gapbuffer_ptr_type = 
                    typename std::conditional<is_const, const Gapbuffer*, Gapbuffer*>::type;
                using difference_type   = std::ptrdiff_t;
                using pointer           = pointer_type;
                using reference         = value_type&;
                using iterator_category = std::random_access_iterator_tag;

                explicit IteratorTemplate() = default;
                explicit IteratorTemplate(pointer_type input_ptr, gapbuffer_ptr_type gb_ptr) 
                    : ptr(input_ptr), gb(gb_ptr) {}

                // reference operator*() const { return *ptr; }
                // pointer operator->() const { return ptr; }
                // reference operator[](difference_type n) const { return *(ptr + n); }

                IteratorTemplate& operator++() {}

                // pre-increment
                IteratorTemplate operator++(int) {}

                IteratorTemplate& operator--() {}

                // pre-increment
                IteratorTemplate operator--(int) {}

                IteratorTemplate operator+(const int val) {}
                IteratorTemplate operator+(const difference_type other) const {}
                friend IteratorTemplate operator+(
                    const difference_type value,
                    const IteratorTemplate& other) {}

                IteratorTemplate operator-(const int val) {}
                difference_type operator-(const IteratorTemplate& other) const {}
                IteratorTemplate operator-(const difference_type other) const {}

                IteratorTemplate& operator+=(difference_type n) {}
                IteratorTemplate& operator-=(difference_type n) {}

                auto operator<=>(const IteratorTemplate&) const = default;

            private:
                pointer_type ptr;
                gapbuffer_ptr_type gb;
        };

    public:
        // Iterator member types
        using iterator = IteratorTemplate<pointer>;
        using const_iterator = IteratorTemplate<const_pointer>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // Constructors

        constexpr explicit Gapbuffer() {
            bufferStart = allocator_type().allocate(32);
            bufferEnd = std::uninitialized_value_construct_n(bufferStart, 32);
            gapStart = bufferStart;
            gapEnd = bufferEnd;
        }

        constexpr explicit Gapbuffer(const size_type length) {
            bufferStart = allocator_type().allocate(length);
            bufferEnd = std::uninitialized_value_construct_n(bufferStart, length);
            gapStart = bufferStart;
            gapEnd = bufferEnd;
        }

        constexpr explicit Gapbuffer(std::string_view str) {
            bufferStart = allocator_type().allocate(str.size() + 8);

            const auto &[_, final_destination] = std::uninitialized_move_n(str.begin(), str.size(), bufferStart);
            gapStart = final_destination;
            gapEnd = gapStart + 8;
            bufferEnd = gapEnd;
        }

        template <typename InputIt>
        constexpr explicit Gapbuffer(InputIt begin, InputIt end) {
            const unsigned int len = std::distance(begin, end);

            bufferStart = allocator_type().allocate(len + 8);

            auto &[_, final_destination] = std::uninitialized_move_n(begin, len, bufferStart);
            gapStart = final_destination;
            gapEnd = gapStart + 8;
            bufferEnd = gapEnd;
        }
        
        constexpr explicit Gapbuffer(std::initializer_list<char> lst) {
            bufferStart = allocator_type().allocate(lst.size());
            const auto &[_, final_destination] = std::uninitialized_move_n(
                    lst.begin(),
                    lst.size(),
                    bufferStart
                    );
            gapStart = final_destination;
            gapEnd = gapStart + 8;
            bufferEnd = gapEnd;
        }

        // Copy Constructor
        constexpr Gapbuffer(const Gapbuffer& other) {
            bufferStart = allocator_type().allocate(other.capacity());
            gapStart = std::uninitialized_copy_n(other.bufferStart, other.capacity(), bufferStart);

            gapEnd = bufferStart + (other.gapEnd - other.bufferStart);
            bufferEnd = bufferStart + other.capacity();
        }

        // Copy Assignment
        constexpr Gapbuffer& operator=(const Gapbuffer& other) {
            if (this != &other) {
                bufferStart = allocator_type().allocate(other.capacity());

                gapStart = std::uninitialized_copy_n(other.bufferStart, other.capacity(), bufferStart);

                gapEnd = bufferStart + (other.gapEnd - other.bufferStart);
                bufferEnd = bufferStart + other.capacity();
            }
            return *this;
        }

        // Move Constructor
        constexpr Gapbuffer(Gapbuffer&& other)
            : bufferStart(other.bufferStart),
              gapStart(other.gapStart),
              gapEnd(other.gapEnd),
              bufferEnd(other.bufferEnd) {

            allocator_type().deallocate(other.bufferStart, other.capacity());
        }

        // Move Assignment Operator
        constexpr Gapbuffer& operator=(Gapbuffer&& other) {
            if (this != &other) {
                std::destroy_n(bufferStart, capacity());
                allocator_type().deallocate(bufferStart, capacity());

                bufferStart = std::exchange(other.bufferStart, {});
                gapStart = std::exchange(other.gapStart, {});
                gapEnd = std::exchange(other.gapEnd, {});
                bufferEnd = std::exchange(other.bufferEnd, {});
            }
            return *this;
        }

        ~Gapbuffer() {
            std::destroy_n(bufferStart, capacity());
            allocator_type().deallocate(bufferStart, capacity());
        }

        // Operator Overloads
        // Element Access
        // Iterators
        // Capacity
        [[nodiscard]] constexpr bool empty() const noexcept { return bufferStart == gapStart; }
        [[nodiscard]] constexpr size_type size() const noexcept {
            return bufferEnd - bufferStart - (gapEnd - gapStart);
        }

        [[nodiscard]] constexpr size_type capacity() const noexcept { return bufferEnd - bufferStart; }

        constexpr void reserve(int new_cap) {
            if (new_cap > capacity()) {
                pointer new_mem = allocator_type().allocate(new_cap);
                pointer new_end = std::uninitialized_value_construct_n(bufferStart, new_cap);

                const auto &[_, lhs_buf] = std::uninitialized_move_n(
                        bufferStart,
                        gapStart - bufferStart,
                        new_mem
                        );
                gapStart = lhs_buf;

                std::size_t rhs_size = bufferEnd - gapEnd;
                std::uninitialized_move_n(gapEnd, rhs_size, new_end - rhs_size);

                bufferStart = new_mem;
                bufferEnd = new_end;
                gapStart = lhs_buf;
                gapEnd = bufferEnd - rhs_size;
            }
        }

        [[nodiscard]] constexpr unsigned int line_count() const noexcept {
            if (bufferStart == gapStart && bufferEnd == gapEnd) {
                return 0;
            }
            int newlines = std::count(begin(), end(), '\n');
            if (back() != '\n') {
                newlines++;
            }
            return newlines;
        }

        // Modifiers

    private:
        pointer bufferStart;
        pointer gapStart; // One space past the last value in the left half
        pointer gapEnd; // Pointing to the first value in the right half
        pointer bufferEnd; // One space past the last value in the right half
};

#endif // GAPBUFFER_H
