#ifndef ZInterlockedExchange_h__999999990000000077777777
#define ZInterlockedExchange_h__999999990000000077777777



NAMESPACE_START(helper);

template<typename T,int size=sizeof(T)>
class ZInterlockedeHelper
{};

template<typename T>
class ZInterlockedeHelper<T,4>
{
public:
	FUNCTION_CLASS(ZInterlockedeHelper);
	static forcedinline T exchange(volatile T& ref,const T newValue)
	{
		return byte_cast<T>( helper::InterlockedExchange32(
			reinterpret_cast<volatile long*>( &ref ),
			byte_cast<long>( newValue )
		));
	}
	static forcedinline T exchangeAdd(volatile T& ref,const T valueToAdd)
	{
		return byte_cast<T>( helper::InterlockedExchangeAdd32(
			reinterpret_cast<volatile long*>( &ref ),
			byte_cast<long>( valueToAdd )
		));
	}
	static forcedinline T fetchAndAdd(volatile T& ref,const long valueToAdd)
	{
		enum { PtrSize = value_if<is_pointer<T>::value,sizeof( typename remove_pointer<T>::type ),1>::value };
		return byte_cast<T>( helper::InterlockedExchangeAdd32(
			reinterpret_cast<volatile long*>( &ref ),
			valueToAdd*PtrSize
			) );
	}
	static forcedinline T addAndFetch(volatile T& ref,const long valueToAdd)
	{
		enum { PtrSize = value_if<is_pointer<T>::value,sizeof( typename remove_pointer<T>::type ),1>::value };
		return byte_cast<T>( helper::InterlockedExchangeAdd32(
			reinterpret_cast<volatile long*>( &ref ),
			valueToAdd*PtrSize
			) + valueToAdd*PtrSize );
	}
	static forcedinline T CompareExchangeValue(volatile T& ref,const T valueToCmp,const T newValue)
	{
		return byte_cast<T>( helper::InterlockedCompareExchangeValue32(
			reinterpret_cast<volatile long*>( &ref ),
			byte_cast<long>( valueToCmp ),
			byte_cast<long>( newValue )
		));
	}
	static forcedinline bool CompareExchangeBool(volatile T& ref,const T valueToCmp,const T newValue)
	{
		return helper::InterlockedCompareExchangeBool32(
			reinterpret_cast<volatile long*>( &ref ),
			byte_cast<long>( valueToCmp ),
			byte_cast<long>( newValue )
		);
	}
	static forcedinline T load(volatile T& ref)
	{
		return byte_cast<T>( helper::InterlockedLoad32(
			reinterpret_cast<volatile long*>( &ref )
			));
	}
	static forcedinline void store(volatile T& ref,T newValue)
	{
		helper::InterlockedStore32(
			reinterpret_cast<volatile long*>( &ref ),
			byte_cast<long>( newValue )
		);
	}
};

template<typename T>
class ZInterlockedeHelper<T,8>
{
public:
	FUNCTION_CLASS(ZInterlockedeHelper);
	static forcedinline T exchange(volatile T& ref,const T newValue)
	{
		return byte_cast<T>( helper::InterlockedExchange64(
			reinterpret_cast<volatile int64*>( &ref ),
			byte_cast<int64>( newValue )
			) );
	}
	static forcedinline T exchangeAdd(volatile T& ref,const T valueToAdd)
	{
		return byte_cast<T>( helper::InterlockedExchangeAdd64(
			reinterpret_cast<volatile int64*>( &ref ),
			byte_cast<int64>( valueToAdd )
			) );
	}
	static forcedinline T fetchAndAdd(volatile T& ref,const int64 valueToAdd)
	{
		enum { PtrSize = value_if<is_pointer<T>::value,sizeof( typename remove_pointer<T>::type ),1>::value };
		return byte_cast<T>( helper::InterlockedExchangeAdd64(
			reinterpret_cast<volatile int64*>( &ref ),
			valueToAdd*PtrSize
		) );
	}
	static forcedinline T addAndFetch(volatile T& ref,const int64 valueToAdd)
	{
		enum { PtrSize = value_if<is_pointer<T>::value,sizeof( typename remove_pointer<T>::type ),1>::value };
		return byte_cast<T>( helper::InterlockedExchangeAdd64(
			reinterpret_cast<volatile int64*>( &ref ),
			valueToAdd*PtrSize
			)+valueToAdd*PtrSize );
	}
	static forcedinline T CompareExchangeValue(volatile T& ref,const T valueToCmp,const T newValue)
	{
		return byte_cast<T>( helper::InterlockedCompareExchangeValue64(
			reinterpret_cast<volatile int64*>( &ref ),
			byte_cast<int64>( valueToCmp ),
			byte_cast<int64>( newValue )
			) );
	}
	static forcedinline bool CompareExchangeBool(volatile T& ref,const T valueToCmp,const T newValue)
	{
		return helper::InterlockedCompareExchangeBool64(
			reinterpret_cast<volatile int64*>( &ref ),
			byte_cast<int64>( valueToCmp ),
			byte_cast<int64>( newValue )
			);
	}
	static forcedinline T load(volatile T& ref)
	{
		return byte_cast<T>( helper::InterlockedLoad64(
			reinterpret_cast<volatile int64*>( &ref )
			) );
	}
	static forcedinline void store(volatile T& ref,T newValue)
	{
		helper::InterlockedStore64(
			reinterpret_cast<volatile int64*>( &ref ),
			byte_cast<int64>( newValue )
		);
	}
};

#if JUCE_64BIT

template<typename T>
class ZInterlockedeHelper<T,16>
{
public:
	FUNCTION_CLASS(ZInterlockedeHelper);
	static forcedinline T exchange(volatile T& ref,const T newValue)
	{
		return byte_cast<T>( helper::InterlockedExchange128(
			reinterpret_cast<volatile int128*>( &ref ),
			byte_cast<int128>( newValue )
			) );
	}
	static forcedinline T exchangeAdd(volatile T&,const T)
	{
        jassertfalse;//the cpu u using do not have interLockExchange for this size;
		return T();
	}
	static forcedinline T exchangeAddNumber(volatile T&,const int64)
	{
        jassertfalse;//the cpu u using do not have interLockExchange for this size;
		return T();
	}
	static forcedinline T CompareExchangeValue(volatile T& ref,const T valueToCmp,const T newValue)
	{
		return byte_cast<T>( helper::InterlockedCompareExchangeValue128(
			reinterpret_cast<volatile int128*>( &ref ),
			byte_cast<int128>( valueToCmp ),
			byte_cast<int128>( newValue )
			) );
	}
	static forcedinline bool CompareExchangeBool(volatile T& ref,const T valueToCmp,const T newValue)
	{
		return helper::InterlockedCompareExchangeBool128(
			reinterpret_cast<volatile int128*>( &ref ),
			byte_cast<int128>( valueToCmp ),
			byte_cast<int128>( newValue )
			);
	}
	static forcedinline T load(volatile T& ref)
	{
		return byte_cast<T>( helper::InterlockedLoad128(
			reinterpret_cast<volatile int128*>( &ref )
			) );
	}
	static forcedinline void store(volatile T& ref,T newValue)
	{
		return helper::InterlockedStore128(
			reinterpret_cast<volatile int128*>( &ref ),
			byte_cast<int128>( newValue )
			);
	}
};

#endif

NAMESPACE_END;

//*************************************************************************
//* 交换ref指向的值为新的值,返回ref的老值
//*************************************************************************
template<typename T>
forcedinline T ZInterlockedExchange(volatile T& ref,const T newValue)
{
	static_assert( IS_DEFAULTCOPYABLE(T),"class T must be default copyable" );
#	if JUCE_64BIT
	static_assert( sizeof( T ) == 4 || sizeof( T ) == 8 || sizeof( T ) == 16,"class T must have size 4,8,16" );
#	else
	static_assert( sizeof( T ) == 4 || sizeof( T ) == 8,"class T must have size 4,8" );
#	endif
	return helper::ZInterlockedeHelper<T>::exchange(ref,newValue);
}

//*************************************************************************
//* 交换ref指向的值为新的值,返回ref的老值,类型T必须有默认的拷贝函数
//*************************************************************************
template<typename T>
forcedinline bool ZInterlockedCompareAndSetBool(volatile T& p,const T valueToCmp,const T newValue)
{
	static_assert(IS_DEFAULTCOPYABLE(T),"class T must be default copyable");
#	if JUCE_64BIT
	static_assert(sizeof(T)==4||sizeof(T)==8||sizeof(T)==16,"class T must have size 4,8,16");
#	else
	static_assert(sizeof(T)==4||sizeof(T)==8,"class T must have size 4,8");
#	endif
	return helper::ZInterlockedeHelper<T>::CompareExchangeBool(p,valueToCmp,newValue);
}

template<typename T>
forcedinline T ZInterlockedCompareAndSetValue(volatile T& p,const T valueToCmp,const T newValue)
{
	static_assert( IS_DEFAULTCOPYABLE(T),"class T must be default copyable" );
#	if JUCE_64BIT
	static_assert( sizeof( T ) == 4 || sizeof( T ) == 8 || sizeof( T ) == 16,"class T must have size 4,8,16" );
#	else
	static_assert( sizeof( T ) == 4 || sizeof( T ) == 8,"class T must have size 4,8" );
#	endif
	return helper::ZInterlockedeHelper<T>::CompareExchangeValue(p,valueToCmp,newValue);
}

template<typename T>
forcedinline T ZInterlockedFetchAndAdd(volatile T& ref,const typename type_if<sizeof( T ) == 4,long,int64>::type valueToAdd)
{
	static_assert( IS_DEFAULTCOPYABLE(T),"class T must be default copyable" );
	static_assert( sizeof( T ) == 4 || sizeof( T ) == 8,"class T must have size 4,8" ); //x64上也没有16byte的
	return helper::ZInterlockedeHelper<T>::fetchAndAdd(ref,valueToAdd);
}

template<typename T>
forcedinline T ZInterlockedFetchAndInc(volatile T& ref)
{
	return ZInterlockedFetchAndAdd(ref,1);
}

template<typename T>
forcedinline T ZInterlockedFetchAndSub(volatile T& ref,const typename type_if<sizeof( T ) == 4,long,int64>::type valueToSub)
{
	return ZInterlockedFetchAndAdd(ref,-valueToSub);
}

template<typename T>
forcedinline T ZInterlockedFetchAndDec(volatile T& ref)
{
	return ZInterlockedFetchAndSub(ref,1);
}

template<typename T>
forcedinline T ZInterlockedAddAndFetch(volatile T& ref,const typename type_if<sizeof( T ) == 4,long,int64>::type valueToAdd)
{
	static_assert( IS_DEFAULTCOPYABLE(T),"class T must be default copyable" );
	static_assert( sizeof( T ) == 4 || sizeof( T ) == 8,"class T must have size 4,8" ); //x64上也没有16byte的
	return helper::ZInterlockedeHelper<T>::addAndFetch(ref,valueToAdd);
}

template<typename T>
forcedinline T ZInterlockedIncAndFetch(volatile T& ref)
{
	static_assert( IS_DEFAULTCOPYABLE(T),"class T must be default copyable" );
	static_assert( sizeof( T ) == 4 || sizeof( T ) == 8,"class T must have size 4,8" ); //x64上也没有16byte的
	return ZInterlockedAddAndFetch(ref,1);
}

template<typename T>
forcedinline T ZInterlockedSubAndFetch(volatile T& ref,const typename type_if<sizeof( T ) == 4,long,int64>::type valueToSub)
{
	static_assert( IS_DEFAULTCOPYABLE(T),"class T must be default copyable" );
	static_assert( sizeof( T ) == 4 || sizeof( T ) == 8,"class T must have size 4,8" ); //x64上也没有16byte的
	return ZInterlockedAddAndFetch(ref,-valueToSub);
}

template<typename T>
forcedinline T ZInterlockedDecAndFetch(volatile T& ref)
{
	static_assert( IS_DEFAULTCOPYABLE(T),"class T must be default copyable" );
	return ZInterlockedSubAndFetch(ref,1);
}



template<typename T>
forcedinline T ZInterlockedLoad(volatile T& ref)
{
	static_assert( IS_DEFAULTCOPYABLE(T),"class T must be default copyable" );
	return helper::ZInterlockedeHelper<T>::load(ref);
}

template<typename T>
forcedinline void ZInterlockedStore(volatile T& ref,const T valueToStore)
{
	static_assert( IS_DEFAULTCOPYABLE(T),"class T must be default copyable" );
	helper::ZInterlockedeHelper<T>::store(ref,valueToStore);
}



#endif // ZInterlockedExchange_h__
