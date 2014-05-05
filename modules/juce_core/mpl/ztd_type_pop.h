#ifndef type_container_h__type_pop
#define type_container_h__type_pop

ZTD_NAMESPACE_START;

template<typename T>
class type_pop
{
public:
	FUNCTION_CLASS(type_pop);
	typedef typename T::fristType pop_type;
	typedef typename T::left_type left_type;
};

ZTD_NAMESPACE_END;

#endif // type_container_h__
