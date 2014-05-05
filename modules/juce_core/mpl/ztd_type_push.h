#ifndef type_container_h__type_push
#define type_container_h__type_push

ZTD_NAMESPACE_START;

template<typename T,typename typeToPush>
class type_push
{
public:
	FUNCTION_CLASS(type_push);
	typedef type_queue<
		typename T::fristType,
		typename T::left_type,
		typeToPush
	> result;
};

ZTD_NAMESPACE_END;

#endif // type_container_h__
