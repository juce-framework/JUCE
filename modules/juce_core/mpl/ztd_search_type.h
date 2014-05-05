#ifndef type_container_h__search_type
#define type_container_h__search_type

ZTD_NAMESPACE_START;

template<typename T,int indexToSearch>
class search_type
{
private:
	typedef typename type_pop<T>::pop_type fristTypeToSearch;
	typedef typename type_pop<T>::left_type LeftTypeToSearch;
public:
	FUNCTION_CLASS(search_type);
	typedef typename type_if<
		indexToSearch == 0,
		fristTypeToSearch,
		typename search_type<LeftTypeToSearch,indexToSearch - 1>::result
	>::type result;
};

template<int indexToSearch>
class search_type<type_queue<void>,indexToSearch>
{
public:
	FUNCTION_CLASS(search_type);
	typedef void result;
};

ZTD_NAMESPACE_END;

#endif // type_container_h__
