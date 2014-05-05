#ifndef type_container_h__search_index
#define type_container_h__search_index



template<typename T,typename TypeToSearch>
class search_index
{
private:
	typedef typename type_pop<T>::pop_type fristTypeToSearch;
	typedef typename type_pop<T>::left_type LeftTypeToSearch;
public:
	FUNCTION_CLASS(search_index);
	enum {
		result = value_if<
			is_same<fristTypeToSearch,TypeToSearch>::value,
			0,
			search_index<LeftTypeToSearch,TypeToSearch>::result + 1
		>::value
	};
};

template<typename TypeToSearch>
class search_index<type_queue<void>,TypeToSearch>
{
public:
	FUNCTION_CLASS(search_index);
	enum {
		result = -INT_MAX
	};
};

#endif // type_container_h__
