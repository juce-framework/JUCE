#ifndef type_container_h__
#define type_container_h__

ZTD_NAMESPACE_START;	


template<typename T,typename... Ts>
class type_queue
{
public:
	FUNCTION_CLASS(type_queue);
	typedef T fristType;
	typedef type_queue<Ts...> left_type;
	typedef tuple<T,Ts...> tuple_type;
	enum { 
		numType = 1 + sizeof...( Ts ),
		maxSizeof = value_max<sizeof(fristType),left_type::maxSizeof>::value,
		minSizeof = value_min<sizeof(fristType),left_type::minSizeof>::value,
		maxAlignof = value_max<ALIGNOF(fristType),left_type::maxAlignof>::value
	};
};

template<typename T>
class type_queue<T>
{
public:
	FUNCTION_CLASS(type_queue);
	typedef T fristType;
	typedef type_queue<void> left_type;
	typedef tuple<T> tuple_type;
	enum { 
		numType = 1 ,
		maxSizeof = sizeof(T),
		minSizeof = sizeof(T),
		maxAlignof = __alignof(T)
	};
};

ZTD_NAMESPACE_END;

#endif // type_container_h__
