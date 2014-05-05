#ifndef type_select_h__
#define type_select_h__

ZTD_NAMESPACE_START;

template<int i,typename... Ts>
class type_selector
{
private:
	FUNCTION_CLASS(type_selector);
	typedef type_queue<Ts...> m_typeQueue;
public:
	typedef typename search_type<m_typeQueue,i>::result type;
};

ZTD_NAMESPACE_END;

#endif // type_select_h__
