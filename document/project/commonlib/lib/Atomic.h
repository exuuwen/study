#include <stdint.h>
#include "Base.h"

template<typename T>
class AtomicIntegerT : Noncopyable
{
public:
	AtomicIntegerT(int data = 0) : value_(data)
	{
	}

	AtomicIntegerT(const AtomicIntegerT& that) : value_(that.read())
	{
	}

	AtomicIntegerT& operator=(const AtomicIntegerT& that)
	{
		set(that.read());
		return *this;
	}

	T read() const
	{
		return __sync_val_compare_and_swap(const_cast<volatile T*>(&value_), 0, 0);
	}
	

	T add(T x)
	{
		return __sync_add_and_fetch(&value_, x);
	}

	T sub(T x)
	{
		return __sync_sub_and_fetch(&value_, x);
	}

	T inc()
	{
		return __sync_add_and_fetch(&value_, 1);
	}

	T dec()
	{
		return __sync_sub_and_fetch(&value_, 1);
	}

	

	T addReturn(T x)
	{
		return __sync_fetch_and_add(&value_, x);
	}

	T subReturn(T x)
	{
		return __sync_fetch_and_sub(&value_, x);
	}

	T incReturn()
	{
		return __sync_fetch_and_add(&value_, 1);
	}

	T decReturn()
	{
		return __sync_fetch_and_sub(&value_, 1);
	}
	


	T set(T newValue)
	{
		return __sync_lock_test_and_set(&value_, newValue);
	}

private:
	volatile T value_;
};

typedef AtomicIntegerT<int8_t> AtomicInt8;
typedef AtomicIntegerT<int16_t> AtomicInt16;
typedef AtomicIntegerT<int32_t> AtomicInt32;
typedef AtomicIntegerT<int64_t> AtomicInt64;

/*
typedef AtomicIntegerT<uint8_t> AtomicUInt8;
typedef AtomicIntegerT<uint16_t> AtomicUInt16;
typedef AtomicIntegerT<uint32_t> AtomicUInt32;
typedef AtomicIntegerT<uint64_t> AtomicUInt64;
*/


