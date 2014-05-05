#ifndef type_container_h__contain_type
#define type_container_h__contain_type

template<typename T,typename TypeToSearch>
class contain_type;

template<typename TypeToSearch>
class contain_type<type_queue<void>,TypeToSearch>
{
public:
	enum {
		result = 0
	};
};

template<typename T,typename TypeToSearch>
class contain_type
{
private:
	typedef typename type_pop<T>::pop_type fristTypeToSearch;//中文
	typedef typename type_pop<T>::left_type LeftTypeToSearch;
public:
	enum {
		result = value_if <
			std::is_same<TypeToSearch,fristTypeToSearch>::value,
			1,
			value_if<
				std::is_same<LeftTypeToSearch,type_queue<void>>::value,
				0,
				contain_type<LeftTypeToSearch,TypeToSearch>::result
			>::value
		>::value
	};
};



#endif // type_container_h__
