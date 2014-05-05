#ifndef ZInterlockedExchange_h__
#define ZInterlockedExchange_h__


NAMESPACE_START(helper);

#if JUCE_GCC
forcedinline long InterlockedExchange32(volatile long* Target,long Value) { return __sync_lock_test_and_set(Target,Value); }
forcedinline long InterlockedExchangeAdd32(volatile long* Addend,long Value) { return __sync_fetch_and_add(Addend,Value); }
forcedinline long InterlockedCompareExchangeValue32(volatile long* Destination,long Comperand,long Exchange) { return __sync_val_compare_and_swap(Destination,Comperand,Exchange); }
forcedinline int64 InterlockedCompareExchangeValue64(volatile int64* Destination,int64 Comperand,int64 Exchange) { return __sync_val_compare_and_swap(Destination,Comperand,Exchange); }
forcedinline bool InterlockedCompareExchangeBool32(volatile long* Destination,long Comperand,long Exchange) { return __sync_bool_compare_and_swap(Destination,Comperand,Exchange); }
forcedinline bool InterlockedCompareExchangeBool64(volatile int64* Destination,int64 Comperand,int64 Exchange) { return __sync_bool_compare_and_swap(Destination,Comperand,Exchange); }
#elif JUCE_MSVC
forcedinline long InterlockedExchange32(volatile long* Target,long Value) { return _InterlockedExchange(Target,Value); }
forcedinline long InterlockedExchangeAdd32(volatile long* Addend,long Value) { return _InterlockedExchangeAdd(Addend,Value); }
forcedinline long InterlockedCompareExchangeValue32(volatile long* Destination,long Comperand,long Exchange) { return _InterlockedCompareExchange(Destination,Exchange,Comperand); }
forcedinline int64 InterlockedCompareExchangeValue64(volatile int64* Destination,int64 Comperand,int64 Exchange) { return _InterlockedCompareExchange64(Destination,Exchange,Comperand); }
forcedinline bool InterlockedCompareExchangeBool32(volatile long* Destination,long Comperand,long Exchange) { return InterlockedCompareExchangeValue32(Destination,Comperand,Exchange)==Comperand; }
forcedinline bool InterlockedCompareExchangeBool64(volatile int64* Destination,int64 Comperand,int64 Exchange) { return InterlockedCompareExchangeValue64(Destination,Comperand,Exchange)==Comperand; }
#endif

forcedinline long InterlockedLoad32(volatile long* Target) { return InterlockedExchangeAdd32(Target,0l); }
forcedinline void InterlockedStore32(volatile long* Target,long value) { InterlockedExchange32(Target,value); }

#if JUCE_64BIT
#	if JUCE_MSVC
		forcedinline int64 InterlockedExchange64(volatile int64* Target,int64 Value) { return _InterlockedExchange64(Target,Value); }
		forcedinline int64 InterlockedExchangeAdd64(volatile int64* Addend,int64 Value) { return _InterlockedExchangeAdd64(Addend,Value); }
#	elif JUCE_GCC
		forcedinline int64 InterlockedExchange64(volatile int64* Target,int64 Value) { return __sync_lock_test_and_set(Target,Value); }
		forcedinline int64 InterlockedExchangeAdd64(volatile int64* Addend,int64 Value) { return __sync_fetch_and_add(Addend,Value); }
#	endif
	forcedinline int64 InterlockedLoad64(volatile int64* Target) { return InterlockedExchangeAdd64(Target,0ll); }
	forcedinline void InterlockedStore64(volatile int64* Target,int64 value) { InterlockedExchange64(Target,value); }
#else //x86,32bit
	forcedinline int64 InterlockedLoad64(volatile int64* Target) { return InterlockedCompareExchangeValue64(Target,0ll,0ll); }
	forcedinline void InterlockedStore64(volatile int64* Target,int64 value)
	{
		int64 temp =InterlockedLoad64(Target);
		int64 k = 0ll;
		for(;;) {
			k = InterlockedCompareExchangeValue64(Target,temp,value);
			likely_if(k == temp) break;
			temp = k;
		}
	}
	forcedinline int64 InterlockedExchange64(volatile int64* Target,int64 Value)
	{
		int64 temp =InterlockedLoad64(Target);
		int64 k=0ll;
		for(;;) {
			k=InterlockedCompareExchangeValue64(Target,temp,Value);
			likely_if(k==temp) break;
			temp=k;
		}
		return k;
	}
	forcedinline int64 InterlockedExchangeAdd64(volatile int64* Addend,int64 Value)
	{
		int64 temp =InterlockedLoad64(Addend);
		int64 k = 0ll;
		for(;;) {
			k = InterlockedCompareExchangeValue64(Addend,temp,temp+Value);
			likely_if(k == temp) break;
			temp = k;
		}
		return k;
	}
#endif

//下面是CAS的:--------------------------------------------------------------
#if JUCE_64BIT

struct int128_t
{
	int64 lo;
	int64 hi;
};

typedef aligned_type<int128_t,16>::type int128;

#	if JUCE_MSVC
forcedinline bool InterlockedCompareExchangeBool128(volatile int128* Destination,int128 Comperand,int128 Exchange)
{
	checkPtrSIMD(Destination,16); //必须对齐16byte!
	int64* cmpPtr = (int64*)&Comperand; //这里一定小心,valueToCmp和其他的不一样,必须不是const的!
	int64* valuePtr = (int64*)&Exchange;
	return _InterlockedCompareExchange128(
		reinterpret_cast<volatile __int64*>( Destination ),
		valuePtr[1],
		valuePtr[0],
		cmpPtr
	)!=0;
};
forcedinline int128 InterlockedCompareExchangeValue128(volatile int128* Destination,int128 Comperand,int128 Exchange)
{
	checkPtrSIMD(Destination,16); //必须对齐16byte!
	int64* cmpPtr = (int64*)&Comperand; //这里一定小心,valueToCmp和其他的不一样,必须不是const的!
	int64* valuePtr = (int64*)&Exchange;
	_InterlockedCompareExchange128(
		reinterpret_cast<volatile __int64*>( Destination ),
		valuePtr[1],
		valuePtr[0],
		cmpPtr
		);
	return Comperand;
};
#	elif JUCE_GCC
forcedinline bool InterlockedCompareExchangeBool128(volatile int128* Destination,int128 Comperand,int128 Exchange)
{
	checkPtrSIMD(Destination,16); //必须对齐16byte!
	bool result;
	__asm__ __volatile__(
		"lock cmpxchg16b %1;"
		"setz %0"
		: "=q" ( result ),"+m" ( *Destination ),"+d" ( Comperand.hi ),"+a" ( Comperand.lo )
		: "c" ( Exchange.hi ),"b" ( Exchange.lo )
		: "cc"
		);
	return result;
};
forcedinline int128 InterlockedCompareExchangeValue128(volatile int128* Destination,int128 Comperand,int128 Exchange)
{
	checkPtrSIMD(Destination,16); //必须对齐16byte!
	__asm__ __volatile__(
		"lock cmpxchg16b %0"
		: "+m" ( *Destination ),"+d" ( Comperand.hi ),"+a" ( Comperand.lo )
		: "c" ( Exchange.hi ),"b" ( Exchange.lo )
		: "cc"
		);
	return Comperand;
};
#	endif //GCC N MSVC
forcedinline int128 InterlockedLoad128(volatile int128* Target) { int128 temp; temp.hi=0ll;temp.lo=0ll; return InterlockedCompareExchangeValue128(Target,temp,temp); }
forcedinline void InterlockedStore128(volatile int128* Target,int128 value)
{
	int128 temp=InterlockedLoad128(Target);
	int128 k; k.hi=0ll; k.lo=0ll;
	for(;;) {
		k = InterlockedCompareExchangeValue128(Target,temp,value);
		likely_if( (k.hi == temp.hi) && (k.lo==temp.lo) ) break;
		temp = k;
	}
}
forcedinline int128 InterlockedExchange128(volatile int128* Target,int128 Value)
{
	int128 temp=InterlockedLoad128(Target);
	int128 k; k.hi = 0ll; k.lo = 0ll;
	for(;;) {
		k = InterlockedCompareExchangeValue128(Target,temp,Value);
		likely_if( (k.hi == temp.hi) && (k.lo==temp.lo) ) break;
		temp = k;
	}
	return k;
}
forcedinline int64 InterlockedExchangeAdd128(volatile int128*,int128)
{
	jassertfalse; //TODO
	return 0ll;
}
#endif //64bit

NAMESPACE_END;


#endif // ZInterlockedExchange_h__
