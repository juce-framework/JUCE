#ifndef byte_cast_h__fdsaaaaaaaaaaaa
#define byte_cast_h__fdsaaaaaaaaaaaa

ZTD_NAMESPACE_START;

template<typename T2,typename T1>
forcedinline T2& byte_ref_cast(T1& value)
{
	return reinterpret_cast<T2&>( value );
};

ZTD_NAMESPACE_END;

#endif // byte_cast_h__
