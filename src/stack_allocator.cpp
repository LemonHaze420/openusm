#include "stack_allocator.h"

#include "bitmath.h"
#include "common.h"
#include "log.h"
#include "memory.h"
#include "trace.h"

#include <cassert>

VALIDATE_SIZE(stack_allocator, 0x10);

VALIDATE_OFFSET(stack_allocator, segment_size_bytes, 0x0);
VALIDATE_OFFSET(stack_allocator, alignment, 0x4);
VALIDATE_OFFSET(stack_allocator, segment, 0x8);
VALIDATE_OFFSET(stack_allocator, current, 0xC);

bool stack_allocator::allocate(int size, int alignment_arg, int external_alignment_arg)
{
    TRACE("stack_allocator::allocate");

    assert(bitmath::is_power_of_2(alignment_arg));

    assert(bitmath::is_power_of_2(external_alignment_arg));

    assert(external_alignment_arg >= alignment_arg);

    this->alignment = alignment_arg;
    this->segment = static_cast<char *>(arch_memalign(external_alignment_arg, size));
    this->current = this->segment;
    this->segment_size_bytes = size;
    assert((long(current) & (alignment - 1)) == 0);

    auto status = this->segment != nullptr;
    return status;
}

void *stack_allocator::push(int size_bytes)
{
    TRACE("stack_allocator::push");

    assert((alignment >= 2) && bitmath::is_power_of_2(alignment));

    auto size = ~(this->alignment - 1) & (size_bytes + this->alignment - 1);
    assert((size >= size_bytes) && (size & (alignment - 1)) == 0);

    this->current += size;

    assert(current <= segment + segment_size_bytes);
    assert((long(current) & (alignment - 1)) == 0);

    return &this->current[-size];
}

int stack_allocator::get_total_allocated_bytes()
{
    assert(current >= segment);
    return this->current - this->segment;
}

void stack_allocator::reset()
{
    this->current = this->segment;
}

void stack_allocator::pop(void *pointer, int size_bytes)
{
    assert((alignment >= 2) && bitmath::is_power_of_2(alignment));

    assert(current <= segment + segment_size_bytes);

    assert(size_bytes >= 1);

    auto size = ~(this->alignment - 1) & (size_bytes + this->alignment - 1);
    assert((size >= size_bytes) && (size & (alignment - 1)) == 0);

    this->current -= size;

    assert("Stack allocator failed to pop a pointer -- mismatched alloc/pop pair" && (current == (char *)pointer));

    assert((int)current >= 0 && "Stack allocator underflow.");

    assert((long(current) & (alignment - 1)) == 0);
}

void stack_allocator::free()
{
    mem_freealign(this->segment);
    this->segment = nullptr;
    this->current = nullptr;
}

void stack_allocator::print() const
{
    sp_log("segment_size_bytes = %d, alignment = %d, segment = %d, current = %d",
           this->segment_size_bytes,
           this->alignment,
           this->segment,
           this->current);
}
