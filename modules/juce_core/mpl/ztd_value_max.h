#ifndef value_max_h__
#define value_max_h__

template<int V1,int V2>
class value_max
{
public:
	enum { value = value_if< V1 >= V2,V1,V2 >::value };
};


#endif // value_min_h__
