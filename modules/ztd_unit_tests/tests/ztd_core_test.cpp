
#include "ztd_core.h"

ZTD_NAMESPACE_START;

class CoreTest: public UnitTest
{
public:
	CoreTest() : UnitTest("ztd_core test") {}
	void runTest()
	{
		beginTest("ztd_aligned_malloc test");
		expect(aligned_mallocTest());
		beginTest("Zatomic test1");
		expect(AtomicTest1());
		beginTest("Zatomic test2");
		expect(AtomicTest2());
		beginTest("Zatomic test3");
		expect(AtomicTest3());
		beginTest("Zatomic test4");
		expect(AtomicTest4());
		beginTest("Zatomic test5");
		expect(AtomicTest5());
		beginTest("ScopedSingleton test");
		expect(ScopedSingletonTest());
		beginTest("CountedSingleton test");
		expect(CountedSingletonTest());
		beginTest("ScopedDenormalFlag test");
		expect(ScopedDenormalFlagTest());
		beginTest("AlignedHeapBlock test");
		expect(AlignedHeapBlockTest());
		beginTest("ReadWriteLock test");
		expect(ReadWriteLockTest());
		beginTest("CircularBuffer test");
		expect(CircularBufferTest());
		beginTest("LockfreeCircularBuffer test");
		expect(LockfreeCircularBufferTest());
		//beginTest("LockfreeCircularBuffer test2");
		//expect(LockfreeCircularBufferTest2());
		beginTest("ZthreadPool test");
		expect(ZthreadPoolTest());
		beginTest("ZthreadPool test2");
		expect(ZthreadPoolTest2());
		beginTest("ObjPool test");
		expect(ObjPoolTest());
	}
private:
	bool aligned_mallocTest()
	{
		for(int go = 1; go < 10; go+=1) {
			for(int i = 1; i < 655; i+=1) {
				int* p=(int*)aligned_malloc(i*sizeof(int),1u<<go);
				checkPtrSIMD_nonNullptr(p,1u<<go);
				zeromem(p,i*sizeof(int));
				aligned_free(p);
			}
		}
		return true;
	}
	bool AtomicTest1()
	{
		Zatomic<int> a(10);
		ZASSERT(a.exchange(100)==10);
		ZASSERT(a.get()==100);
		ZASSERT(!a.compareAndSetBool(-1,100));
		ZASSERT(a.get()==100);
		ZASSERT(a.compareAndSetBool(100,1000));
		ZASSERT(!a.compareAndSetBool(-1,100));
		ZASSERT(a.get()==1000);
		ZASSERT(a.compareAndSetValue(1000,-1)==1000);
		ZASSERT(a.get()==-1);
		//--------------------
		a=10;
		ZASSERT(a.get()==10);
		ZASSERT(a++==10);
		ZASSERT(a.get()==11);
		ZASSERT(++a==12);
		ZASSERT(a.get()==12);
		//--------------------
		a = 10;
		ZASSERT(a-- == 10);
		ZASSERT(a.get() == 9);
		ZASSERT(--a == 8);
		ZASSERT(a.get() == 8);
		//--------------------
		a = 10;
		ZASSERT(a.fetch_add(2)==10);
		ZASSERT(a.get() ==12);
		ZASSERT(a.fetch_sub(3) == 12);
		ZASSERT(a.get() == 9);
		ZASSERT(a.add_fetch(5) == 14);
		ZASSERT(a.get() == 14);
		ZASSERT(a.sub_fetch(3) == 11);
		ZASSERT(a.get() == 11);
		return true;
	}
	bool AtomicTest2()
	{
		struct K
		{
		int16 m_data1;
		int16 m_data2;
		};
		Zatomic<K> a;
		K temp;
		temp.m_data1=100;
		temp.m_data2=-10;
		a=temp;
		K temp2;
		temp2.m_data1=200;
		temp2.m_data2=101;
		K g=a.exchange(temp2);
		ZASSERT(g.m_data1==100&&g.m_data2==-10);
		ZASSERT(a.get().m_data1==200&&a.get().m_data2==101);
		a=temp;
		ZASSERT(a.compareAndSetBool(temp,temp2));
		ZASSERT(a.get().m_data1==200&&a.get().m_data2==101);
		ZASSERT(!a.compareAndSetBool(temp,temp2));
		K gg=a.compareAndSetValue(temp2,temp);
		ZASSERT(gg.m_data1==200&&gg.m_data2==101);
		ZASSERT(a.get().m_data1==100&&a.get().m_data2==-10);
		return true;
	}
	bool AtomicTest3()
	{
		Zatomic<float*> a(nullptr);
		ZASSERT(a.exchange((float*)100) == nullptr);
		ZASSERT(a.get() == (float*)100);
		ZASSERT(!a.compareAndSetBool((float*)-1,(float*)100));
		ZASSERT(a.get() == (float*)100);
		ZASSERT(a.compareAndSetBool((float*)100,(float*)1000));
		ZASSERT(!a.compareAndSetBool((float*)-1,(float*)100));
		ZASSERT(a.get() == (float*)1000);
		ZASSERT(a.compareAndSetValue((float*)1000,(float*)-1) == (float*)1000);
		ZASSERT(a.get() == (float*)-1);
		//--------------------
		a = (float*)10;
		ZASSERT(a.get() == (float*)10);
		ZASSERT(a++ == (float*)10);
		ZASSERT(a.get() == (float*)(10+sizeof(float)));
		ZASSERT(++a == (float*)(10+sizeof(float)*2));
		ZASSERT(a.get() == (float*)(10+sizeof(float)*2));
		//--------------------
		a = (float*)10;
		ZASSERT(a-- == (float*)10);
		ZASSERT(a.get() == (float*)(10-sizeof(float)));
		ZASSERT(--a == (float*)(10-sizeof(float)*2));
		ZASSERT(a.get() == (float*)(10-sizeof(float)*2));
		//--------------------
		a = (float*)10;
		ZASSERT(a.fetch_add(2) == (float*)(10));
		ZASSERT(a.get() == (float*)(10+sizeof(float)*2));
		ZASSERT(a.fetch_sub(3) == (float*)(10+sizeof(float)*2));
		ZASSERT(a.get() == (float*)(10-sizeof(float)));
		ZASSERT(a.add_fetch(5) == (float*)(10+sizeof(float)*4));
		ZASSERT(a.get() == (float*)(10+sizeof(float)*4));
		ZASSERT(a.sub_fetch(3) == (float*)(10+sizeof(float)));
		ZASSERT(a.get() == (float*)(10+sizeof(float)));
		return true;
	}
	bool AtomicTest4()
	{
		struct K
		{
			int32 m_data1;
			int32 m_data2;
		};
		Zatomic<K> a;
		K temp;
		temp.m_data1 = 100;
		temp.m_data2 = -10;
		a = temp;
		K temp2;
		temp2.m_data1 = 200;
		temp2.m_data2 = 101;
		K g = a.exchange(temp2);
		ZASSERT(g.m_data1 == 100 && g.m_data2 == -10);
		ZASSERT(a.get().m_data1 == 200 && a.get().m_data2 == 101);
		a = temp;
		ZASSERT(a.compareAndSetBool(temp,temp2));
		ZASSERT(a.get().m_data1 == 200 && a.get().m_data2 == 101);
		ZASSERT(!a.compareAndSetBool(temp,temp2));
		K gg = a.compareAndSetValue(temp2,temp);
		ZASSERT(gg.m_data1 == 200 && gg.m_data2 == 101);
		ZASSERT(a.get().m_data1 == 100 && a.get().m_data2 == -10);
		return true;
	}
	bool AtomicTest5()
	{
#	if JUCE_64BIT
		struct K
		{
			int64 m_data1;
			int64 m_data2;
		};
		//ZASSERT(false);
		Zatomic<K> a;
		K temp;
		temp.m_data1 = 100ll;
		temp.m_data2 = -10ll;
		a = temp;
		K temp2;
		temp2.m_data1 = 200ll;
		temp2.m_data2 = 101ll;
		K g = a.exchange(temp2);
		ZASSERT(g.m_data1 == 100ll && g.m_data2 == -10ll);
		ZASSERT(a.get().m_data1 == 200ll && a.get().m_data2 == 101ll);
		a = temp;
		ZASSERT(a.compareAndSetBool(temp,temp2));
		ZASSERT(a.get().m_data1 == 200ll && a.get().m_data2 == 101ll);
		ZASSERT(!a.compareAndSetBool(temp,temp2));
		K gg = a.compareAndSetValue(temp2,temp);
		ZASSERT(gg.m_data1 == 200ll && gg.m_data2 == 101ll);
		ZASSERT(a.get().m_data1 == 100ll && a.get().m_data2 == -10ll);
#	endif
		return true;
	}
	bool ScopedSingletonTest()
	{
		class A
		{
		public:
			A(){}
			Zatomic<int> m_data;
			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(A);
		};
		class TestThread : public juce::Thread
		{
		public:
			TestThread():Thread("ScopedSingletonTest thread"){}
			~TestThread(){}
			virtual void run() { ++ScopedSingleton<A,false>::getInstance().m_data; --ScopedSingleton<A,true>::getInstance().m_data; }
		};
		TestThread a,b,c,d,e;
		ScopedSingleton<A,true>::getInstance().m_data.m_data=0;
		ScopedSingleton<A,false>::getInstance().m_data.m_data=0;
		a.startThread();
		b.startThread();
		c.startThread();
		d.startThread();
		e.startThread();
		a.stopThread(-1);
		b.stopThread(-1);
		c.stopThread(-1);
		d.stopThread(-1);
		e.stopThread(-1);
		typedef ScopedSingleton<A,false> AA;
		typedef ScopedSingleton<A,true> B;
		ZASSERT( AA ::getInstance().m_data.load()==5);
		ZASSERT( B ::getInstance().m_data.load()==-5);
		return true;
	}
	bool CountedSingletonTest()
	{
		class A
		{
		public:
			A():m_data(0){}
			Zatomic<int> m_data;
			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(A);
		};
		class TestThread: public juce::Thread
			            , private CountedSingleton<A>::User
		{
		public:
			TestThread() :Thread("CountedSingletonTest thread"){}
			~TestThread(){}
			virtual void run()
			{
				++this->getInstance().m_data;
			}
		};
		TestThread a,b,c,d,e;
		a.startThread();
		b.startThread();
		c.startThread();
		d.startThread();
		e.startThread();
		a.stopThread(-1);
		b.stopThread(-1);
		c.stopThread(-1);
		d.stopThread(-1);
		e.stopThread(-1);
		typedef CountedSingleton<A> AA;
		AA::User user;
		ZASSERT(user.getInstance().m_data.load() == 5);
		return true;
	}
	bool ScopedDenormalFlagTest()
	{
		{
			ScopedDenormalFlag k;
		//	__m128 h=_mm_set1_ps(FLOAT_REAL_MIN); 这里,编译器可能将它优化掉,所以这个测试难以进行
		//	h=_mm_add_ps(h,_mm_setzero_ps());
		//	ZASSERT(isDenormalFloat(_mm_cvtss_f32(h)));
		}
		{
		//	__m128 h = _mm_set1_ps(FLOAT_REAL_MIN);
		//	h=_mm_add_ps(h,_mm_setzero_ps());
		//	ZASSERT( ! isDenormalFloat(_mm_cvtss_f32(h)));
		}
		return true;
	}
	bool AlignedHeapBlockTest()
	{
		for(int go = 1; go < 10; go += 1) {
			for(int i = 1; i < 655; i += 1) {
				AlignedHeapBlock<int> p;
				p.malloc(i,1u<<go);
				checkPtrSIMD_nonNullptr(p.getData(),1u << go);
				int* k=p;
				zeromem(k,i*sizeof( int ));
			}
		}
		for(int go = 1; go < 10; go += 1) {
			for(int i = 1; i < 655; i += 1) {
				AlignedHeapBlock<int> p;
				p.calloc(i,1u << go);
				checkPtrSIMD_nonNullptr(p.getData(),1u << go);
				for(int a = 0; a < i; a++) ZASSERT(p[a]==0);
			}
		}
		return true;
	}
	bool ReadWriteLockTest()
	{
		ReadWriteLock lock;
		lock.EnterWrite();
		ZASSERT(lock.EnterWriteAny()==-1);
		lock.ExitWrite();
		lock.EnterRead();
		lock.EnterRead();
		{
			ReadWriteLock::ScopedReadLock e(lock);
		}
		lock.ExitRead();
		lock.ExitRead();
		ReadWriteLock::ScopedWriteLock e2(lock);
		return true;
	}
	bool CircularBufferTest()
	{
		CircularBuffer<int> a;
		int k;
		ZASSERT(!a.pop(k));
		a.push(3);
		ZASSERT(a.pop(k));
		ZASSERT(k==3);
		class A
		{
		public:
			A(){}
			int m_data;
		private:
			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(A);
		};
		CircularBuffer<A*> b(1);
		for (int i=0;i<9000;++i) b.push(new A);
		for (A* k=nullptr;b.pop(k);delete k) k->m_data=1;
		ZASSERT(b.isEmpty());
		for(int i = 0; i < 90; ++i) b.push(new A);
		A* ka=nullptr;
		for(int c=0;c<10;++c) {
			ZASSERT(b.pop(ka));
			ka->m_data = 1;
			delete ka;
		}
		for(int i = 0; i < 90; ++i) b.push(new A);
		for(A* k = nullptr; b.pop(k); delete k) k->m_data = 1;
		ZASSERT(b.isEmpty());
		return true;
	}
	bool LockfreeCircularBufferTest()
	{
		LockfreeCircularBuffer<int> a;
		int k;
		ZASSERT(!a.pop(k));
		a.push(3);
		ZASSERT(a.pop(k));
		ZASSERT(k == 3);
		class A
		{
		public:
			A(){}
			int m_data;
		private:
			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(A);
		};
		LockfreeCircularBuffer<A*> b(1);
		for(int i = 0; i < 9000; ++i) b.push(new A);
		for(A* k = nullptr; b.pop(k); delete k) k->m_data = 1;
		ZASSERT(b.isEmpty());
		for(int i = 0; i < 90; ++i) b.push(new A);
		A* ka = nullptr;
		for(int c = 0; c < 10; ++c) {
			ZASSERT(b.pop(ka));
			ka->m_data = 1;
			delete ka;
		}
		for(int i = 0; i < 90; ++i) b.push(new A);
		for(A* k = nullptr; b.pop(k); delete k) k->m_data = 1;
		ZASSERT(b.isEmpty());
		return true;
	}
	bool LockfreeCircularBufferTest2()
	{
		class A
		{
		public:
			void clean() { m_data=90; }
			int m_data;
		};
		typedef ScopedSingleton<LockfreeCircularBuffer<A*>> Tester;
		class TestThreadPush: public juce::Thread
		{
		public:
			TestThreadPush() :Thread("LockfreeCircularBufferTest2 thread"){}
			~TestThreadPush(){}
			virtual void run()
			{
				for (;!Thread::threadShouldExit();) {
					Tester::getInstance().push(new A);
				}
			}
		};
		class TestThreadPop: public juce::Thread
		{
		public:
			TestThreadPop() :Thread("LockfreeCircularBufferTest2 thread"){}
			~TestThreadPop(){}
			virtual void run()
			{
				for(;!Thread::threadShouldExit();) {
					for(A* k=nullptr;Tester::getInstance().pop(k);delete k) {
						k->clean();
					}
				}
			}
		};
		TestThreadPush a,c;
		TestThreadPop b,d;
		b.startThread();
		a.startThread();
		d.startThread();
		c.startThread();
		Thread::sleep(60*1000*5);
		a.stopThread(-1);
		b.stopThread(-1);
		c.stopThread(-1);
		d.stopThread(-1);
		return true;
	}
	bool ZthreadPoolTest()
	{
		class TesterJob : public ZthreadPoolJob
		{
		public:
			TesterJob():m_state(0){}
			~TesterJob(){}
			ZthreadPoolJob::RunResult runJob() override
			{
				if (m_state.get()==1) return ZthreadPoolJob::jobHasFinished;
				AlignedHeapBlock<int> a;
				a.malloc(1000);
				zeromem(m_data,100*sizeof(int));
				zeromem(a,1000*sizeof(int));
				return ZthreadPoolJob::jobNeedsRunningAgain;
			}
		public:
			Zatomic<int> m_state;
			int m_data[100];
			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TesterJob);
		};
		TesterJob* a=new TesterJob[900000];
		{
			ZthreadPool pool;
			for (int i=0;i<900000;++i) pool.addJob(a+i);
			pool.StartAllThreads();
			pool.SetAllThreadPriorities(10);
			Thread::sleep(60*1000*10);
			for (int i=0;i<900000;++i) a[i].m_state.store(1);
		}
		delete[] a;
		return true;
	}
	bool ZthreadPoolTest2()
	{
		typedef ScopedSingleton<Zatomic<int64>> Counter;
		Counter::getInstance().store(0ll);
		class TesterJob: public ZthreadPoolJob
		{
		public:
			TesterJob(){}
			~TesterJob(){}
			ZthreadPoolJob::RunResult runJob() override
			{
				Counter::getInstance().add_fetch(1LL);
				return ZthreadPoolJob::jobHasFinished;
			}
		public:
			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TesterJob);
		};
		TesterJob* a = new TesterJob[9000000];
		ZthreadPool pool;
		pool.StartAllThreads();
		pool.SetAllThreadPriorities(10);
		for(TesterJob* p=a;p<(a+9000000);pool.addJob(p++)) {
			//Thread::sleep(1);
		}
		pool.RunAllJobInThisThreadAndStopAllThread();
		delete[] a;
		ZASSERT(Counter::getInstance().get()==9000000);
		return true;
	}
	bool ObjPoolTest()
	{
		class A
		{
		public:
			A():m_data(-101){}
			int m_data;
			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(A);
		};
		typedef CountedSingleton<ObjPool<A>>::User TesterUser;
		class TestThread: public juce::Thread
		{
		public:
			TestThread() :Thread("ObjPoolTest thread"){ this->setCurrentThreadPriority(10); }
			~TestThread(){}
			virtual void run()
			{
				TesterUser user;
				user.getInstance().CreateSome();
				for(; !Thread::threadShouldExit();) {
					A* k=user.getInstance().Pop();
					ZASSERT(k!=nullptr);
					ZASSERT(k->m_data==-101);
					k->m_data=10;
					k->m_data=-101;
					user.getInstance().Push(k);
				}
			}
		};
		TestThread a,b,c,d,e,f;
		a.startThread(10);
		b.startThread(10);
		c.startThread(10);
		d.startThread(10);
		e.startThread(10);
		f.startThread(10);
		Thread::sleep(60*1000);
		a.stopThread(-1);
		b.stopThread(-1);
		c.stopThread(-1);
		d.stopThread(-1);
		e.stopThread(-1);
		f.stopThread(-1);
		return true;
	}
};

static CoreTest coreTest;

ZTD_NAMESPACE_END;
