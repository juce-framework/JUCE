
#include "ztd_dsp.h"

ZTD_NAMESPACE_START;

class DspTest: public UnitTest
{
public:
	DspTest() : UnitTest("ztd_core test") {
        SIMDmemmove((float*)nullptr,(int24*)nullptr,1);
	}
	void runTest()
	{
		beginTest("SIMDmemmove test");
		expect(SIMDmemmoveTest());
		beginTest("SIMDmemzero test");
		expect(SIMDmemzeroTest());
		beginTest("SIMDrev test");
		expect(SIMDmemrevTest());
		beginTest("FIR filter test");
		expect(FIRfilterTest());
	}
private:
	bool SIMDmemmoveTest()
	{
		//ScopedDenormalFlag e;
		AlignedHeapBlock<float> a(800);
		for (int i=0;i<800;++i) a[i]=(float)(i+1);
		SIMDmemmove(a,a+4,796);
		float* p=a;
		for(int i = 4; i < 800; i++) {
            std::cout<<*(p)<<"..."<<i<<std::endl;
            ZASSERT( *(p++)==(float)(i+1) );
		}
        JUCE_ALIGN(16) float data[4];
        zeromem(data,4*4);
        data[3]=1.f;
        SIMDmemmove(data,(float*)nullptr,0);
        for(int i=0;i<3;++i) ZASSERT(data[i]==0.f);
        ZASSERT(data[3]==1.f);
        JUCE_ALIGN(16) float data2[8];
        zeromem(data2,32);
        data2[7]=1.f;
        SIMDmemmove(data2,data2+4,4);
        ZASSERT(data2[0]==0.f);
        ZASSERT(data2[1]==0.f);
        ZASSERT(data2[2]==0.f);
        ZASSERT(data2[3]==1.f);
        ZASSERT(data2[4]==0.f);
        ZASSERT(data2[5]==0.f);
        ZASSERT(data2[6]==0.f);
        ZASSERT(data2[7]==1.f);
		return true;
	}
	bool SIMDmemzeroTest()
	{
		//ScopedDenormalFlag e;
		JUCE_ALIGN(16) float a[800];
		for(int i = 0; i < 800; ++i) a[i] = 1.f;
		SIMDmemzero(a+4,796-4);
		for(int i = 0; i < 4; i++) ZASSERT(a[i]==1.f);
		for(int i = 4; i < 796; i++)  {
            std::cout<<a[i]<<"..."<<i<<std::endl;
            ZASSERT(a[i] == 0.f);
		}
		for(int i = 796; i < 800; i++) ZASSERT(a[i]==1.f);
		JUCE_ALIGN(16) float data[4];
		for(int i = 0; i < 4; ++i) a[i] = -1.f;
		SIMDmemzero(data,4);
		for(int i = 0; i < 3; ++i) ZASSERT(data[i] == 0.f);
		SIMDmemzero(nullptr,0);
		return true;
	}
	bool SIMDmemrevTest()
	{
		ScopedDenormalFlag e;
		JUCE_ALIGN(16) float a[800];
		for(int i = 0; i < 800; ++i) a[i] = (float)( i + 1 );
		SIMDreverse(a,800);
		for(int i = 0; i < 800; i++) ZASSERT( a[i] == ( 800 - i ) );
		SIMDreverse(nullptr,0);

		JUCE_ALIGN(16) float data[8];
		for(int i = 0; i < 8; ++i) data[i] = (float)(i+1);
		SIMDreverse(data,8);
		for(int i = 0; i < 8; i++) ZASSERT(data[i] == (float)( 8 - i ));
		return true;
	}
	bool FIRfilterTest()
	{
		ScopedDenormalFlag e;
		JUCE_ALIGN(16) float a[800];
		for(int i = 0; i < 800; ++i) a[i] = (float)( i + 1 );
		FIRfilter filter;
		FIRfilterIR ir;
		ir.setOne(80);
		filter.setNewIRAndReset(&ir,1200);
		filter.render(a,200);
		filter.render(a,200);
		filter.render(a,400);
		for(int i = 0; i < 800; ++i) ZASSERT( a[i]==(float)(i+1) );
		ir.set(80,[](float* ptr){ ptr[0]=10.0f; });
		filter.render(a,800);
		for(int i = 0; i < 800; ++i) ZASSERT( a[i]==(float)(i+1)*10.f );
		zeromem(a,800*4);
		a[50]=1.f;
		ir.set(80,[](float* ptr){ ptr[0]=10.0f;ptr[1]=-10.f; });
		filter.setNewIRAndReset(&ir,1200);
		filter.render(a,800);
        ZASSERT( a[50]==10.f );
        ZASSERT( a[51]==-10.f );
		return true;
	}
};

static DspTest coreTest;

ZTD_NAMESPACE_END;
