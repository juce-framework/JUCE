#ifndef ztd_ZAudioProcessorEditor_h__
#define ztd_ZAudioProcessorEditor_h__

ZTD_NAMESPACE_START;

class ZAudioProcessorEditor:public AudioProcessorEditor
{
public:
	class ParameterChangeObject
	{
	public:
		friend class ZAudioProcessor::Editor;
		ParameterChangeObject()
			:m_index(-1)
			,m_processor(nullptr)
		{};
		virtual ~ParameterChangeObject() = default;
		forcedinline void sendParaChange(float value) const
		{
			if(m_index < 0 || m_processor == nullptr) return;
			const bool k = m_processor->isParameterAutomatable(m_index);
			if(k) m_processor->beginParameterChangeGesture(m_index);
			m_processor->setParameter(m_index,value);
			if(k) m_processor->endParameterChangeGesture(m_index);
		};
		virtual void HandleValueChange(float value) = 0;
		virtual float RequestDefValue()
		{
			float const temp = m_processor->getParameterDefaultValue(m_index);
			sendParaChange(temp);
			return temp;
		}
	private:
		int m_index;
		AudioProcessor* m_processor;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterChangeObject);
	};
public:
	ZAudioProcessorEditor(ZAudioProcessor& processor);
	~ZAudioProcessorEditor(){};
	void ConnectParaObj(ParameterChangeObject* obj,int index);
	ZAudioProcessor& getZAudioProcessor() { return m_processor; };
	void HandleParametersChange();
private:
	using AudioProcessorEditor::getAudioProcessor;
	ZAudioProcessor& m_processor;
	const int m_numParam;
	HeapBlock<float> m_paraUIsnap;
	//HeapBlock<ParameterChangeObject*> m_signals;
	Array<ParameterChangeObject*> m_signals;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor);
};

ZTD_NAMESPACE_END;

#endif // ztd_ZAudioProcessorEditor_h__
