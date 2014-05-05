#ifndef ____header__dfix
#define ____header__dfix

class fixed 
{
public:
           fixed     (                              )=default;
	       fixed     ( int integer , double decimal ) ;
	       fixed     ( const fixed&                 )=default;
	fixed& operator= ( const fixed&                 )=default;
	       ~fixed    (                              )=default;
public:
	size_t m_integer;
	double m_decimal;
};


#endif //____header__dfix