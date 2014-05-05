#ifndef byte_cast_h__
#define byte_cast_h__

ZTD_NAMESPACE_START;

template<typename T1,typename T2>
forcedinline T1 byte_cast(const T2& value)
{
	static_assert( sizeof( T2 ) == sizeof( T1 ),"size T1 T1 must be same!" );
	static_assert( !(std::is_reference<T1>::value),"T1 can't be ref!" );
	union MixData
	{
		typename std::remove_reference<T2>::type raw;
		T1 result;
	} ac;
	ac.raw = value;
	return ac.result;
};

ZTD_NAMESPACE_END;

#endif // byte_cast_h__
