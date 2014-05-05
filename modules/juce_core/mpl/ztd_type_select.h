#ifndef type_select_h__
#define type_select_h__



template<int i,typename... Ts>
class type_selector
{
private:
	FUNCTION_CLASS(type_selector);
	typedef type_queue<Ts...> m_typeQueue;
public:
	typedef typename search_type<m_typeQueue,i>::result type;
};



#endif // type_select_h__
