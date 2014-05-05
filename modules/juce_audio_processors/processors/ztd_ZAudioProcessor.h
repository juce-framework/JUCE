#ifndef ztd_ZAfffffffffffffffffffffffffffffffffffffgfdgfdudioProcessor_h__
#define ztd_ZAfffffffffffffffffffffffffffffffffffffgfdgfdudioProcessor_h__

class ParameterChangeObject;
class ZAudioProcessorEditor;

class ZAudioProcessor :public helper::ZAudioProcessorBase
{
public:



	friend class ZAudioProcessorEditor;

public:
	class ParamIterator
	{
	public:
		friend class ZAudioProcessor;
		inline float getNextEvent(int index) noexcept { return m_processor.getParameter(index); }
		~ParamIterator(){};
	private:
		ZAudioProcessor& m_processor;
		ParamIterator(ZAudioProcessor& processor)
			:m_processor(processor)
		{}
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamIterator);
	};
public:
	ZAudioProcessor(const char* processorName,initializer_list<helper::Param> autoParams)
		:ZAudioProcessorBase(autoParams)
		,m_processorName(processorName)
		,m_isBypassing(true)
	{}
	ZAudioProcessor(const char* processorName,initializer_list<helper::Param> autoParams,initializer_list<helper::OptParam> optParams)
		:ZAudioProcessorBase(autoParams,optParams)
		,m_processorName(processorName)
		,m_isBypassing(true)
	{}
	~ZAudioProcessor(){}
protected:
	bool isBypassing() const { return m_isBypassing; }
	virtual void processBlock(AudioSampleBuffer& buffer,MidiBuffer& midiMessages,ParamIterator& paramMessages)=0;
public:
	virtual void processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages) final 
	{
		m_isBypassing=false;
		ParamIterator i(*this);
		processBlock(buffer,midiMessages,i);
	}
	virtual void processBlockBypassed(AudioSampleBuffer& buffer, MidiBuffer& midiMessages) final
	{
		m_isBypassing=true;
		processBlock(buffer,midiMessages);
	}
	virtual const String getName() const final { return m_processorName; }
private:
	bool m_isBypassing;
	const char*const m_processorName;
	NONCOPYABLE_CLASS(ZAudioProcessor);
	JUCE_LEAK_DETECTOR(ZAudioProcessor);
};


#endif // ztd_ZAudioProcessor_h__
