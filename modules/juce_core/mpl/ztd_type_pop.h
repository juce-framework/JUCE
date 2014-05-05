#ifndef type_container_h__type_pop
#define type_container_h__type_pop



template<typename T>
class type_pop
{
public:
	FUNCTION_CLASS(type_pop);
	typedef typename T::fristType pop_type;
	typedef typename T::left_type left_type;
};



#endif // type_container_h__
