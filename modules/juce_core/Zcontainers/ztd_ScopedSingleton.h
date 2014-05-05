#ifndef ztd_ZscopedSingleton_h__
#define ztd_ZscopedSingleton_h__



/**********************************************************
*   智能单例，在main()之前构造，在程序结束时析构
	使用单例类必须符合以下几个条件:

	1.main函数之前,程序必须是单线程的
	2.单例类A的构造函数中可以使用单例B而不用担心构造顺序,析构顺序和构造顺序相反.
	  因此,我们不能这样使用单例,即先在A()中使用了B(则B比A先构造),然后又在~B()中使用A,因为~A()将先于~B()执行.
	3.ScopedSingleton<A,true>和ScopedSingleton<A,false>不是一个单例! 因为同一个单例不可能既是立即构造的又是new的
	4.单例类的构造函数中不得引用自己.
	5.避免循环引用

	@T: 需要变成单例的类型,请注意保证类型本身的多线程访问,ScopedSingleton只保证单例类的创建和销毁,以及单例之间的构造顺序的线程安全,不保证T本身访问的安全
	@DirectConstruction: 当为true时,在static变量位置直接创建单例本身,当为false时,则static变量只是一个智能指针,对象T创建在堆上.
************************************************************/
template<typename T,bool DirectConstruction = true>
class ScopedSingleton
{
private:
	//! 当false==DirectConstruction时,InstanceCreator负责创建单例本身(使用new)
	class InstanceCreator {
	public:
		forcedinline InstanceCreator() :m_ptr(new T) {};
		forcedinline ~InstanceCreator() { delete m_ptr; };
		forcedinline operator T&() noexcept { return *m_ptr;}
	private:
		T* RESTRICT const m_ptr;
		NONCOPYABLE_CLASS(InstanceCreator);
	};
	typedef typename type_if<DirectConstruction,T,InstanceCreator>::type CreateType; //此处判断是直接创建static变量还是使用new创建
public:
	static forcedinline T& getInstance()
	{
		static CreateType m_instancePtr;
		m_dummyUser.DoNothing(); //此处似乎是模板的一个bug,加了这一句才能保证main之前m_instancePtr一定被构造.
		return m_instancePtr;
	};
private:
	forcedinline ScopedSingleton(){ getInstance(); }
	forcedinline ~ScopedSingleton(){ getInstance(); }
	class DummyInstanceUser
	{
	public:
		forcedinline DummyInstanceUser(){ ScopedSingleton::getInstance(); }
		forcedinline ~DummyInstanceUser() { ScopedSingleton::getInstance(); }
		NONCOPYABLE_CLASS(DummyInstanceUser);
		void DoNothing(){}
	};
private:
	static DummyInstanceUser m_dummyUser;
	NONCOPYABLE_CLASS(ScopedSingleton);
};


template<typename T,bool DirectConstruction>
SELECT_ANY typename ScopedSingleton<T,DirectConstruction>::DummyInstanceUser ScopedSingleton<T,DirectConstruction>::m_dummyUser; 



#endif // ztd_ZscopedSingleton_h__
