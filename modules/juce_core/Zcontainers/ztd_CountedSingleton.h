#ifndef ztd_ZcountedSingleton_h__
#define ztd_ZcountedSingleton_h__

ZTD_NAMESPACE_START;

/*****************************************************************************************************************
线程安全的引用计数单例，在第一个User创建时创建实例，在最后一个User析构时销毁实例.
只有第一个当User被构造时,单例类会被构造,无论在任何线程,其他线程会自动等待第一个
User构造完毕.使用User有以下限制:

1.任何User实例必须在main函数之后被创建.
2.User不得是静态对象,例如 static User a;
3.User的最大数量不得超过INT_MAX

CountedSingleton的使用与ScopedSingleton不同,其实例在第一个User创建时创建,在最后一个User析构时析构,CountedSingleton
不能直接getInstance(),但User可以getInstance().getInstance()仅相当于一次指针访问,但User的
构造函数可能会产生锁等待.假如我们有类T需要作为单例,有类X需要访问,则直接让X类private继承CountedSingleton<T>::User
即可.
******************************************************************************************************************/
template<typename T>
class CountedSingleton
{
public:
	friend class User;
	class User
	{
	public:
		NONCOPYABLE_CLASS(User);
		inline User()
		{
			if ( unlikely(++ CountedSingleton::m_counter == 1) ) {
				CountedSingleton::m_instancePtr = new T;
			} else {
				int i=0;
				while ( unlikely(CountedSingleton::m_instancePtr.get()==nullptr) ) {
					if(++i==40) { 
						i=0;
						Thread::sleep(20);
					}
				}
			}
		}
		inline virtual ~User()
		{
			if ( unlikely(--CountedSingleton::m_counter ==0) ) {
				T*const k = CountedSingleton::m_instancePtr.get();
				CountedSingleton::m_instancePtr = nullptr;
				delete k;
			}
		}
		forcedinline T& getInstance() noexcept
		{
			return *CountedSingleton::m_instancePtr.get();
		}
	};
protected:
	NONCOPYABLE_CLASS(CountedSingleton);
	CountedSingleton()=default;
	~CountedSingleton() { jassert(m_counter.get()==0); }
private:
	static Atomic<T*> m_instancePtr;
	static Atomic<int> m_counter;
};


template<typename T>
SELECT_ANY Atomic<T*> CountedSingleton<T>::m_instancePtr; //Atomic默认构造成0

template<typename T>
SELECT_ANY Atomic<int> CountedSingleton<T>::m_counter;

ZTD_NAMESPACE_END;

#endif // ztd_ZcountedSingleton_h__
