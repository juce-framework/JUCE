#ifndef type_container_h__type_push
#define type_container_h__type_push



template<typename T,typename typeToPush>
class type_push
{
public:
	FUNCTION_CLASS(type_push);
	typedef type_queue< typename T::fristType , typename T::left_type , typeToPush > result;
};



#endif // type_container_h__
