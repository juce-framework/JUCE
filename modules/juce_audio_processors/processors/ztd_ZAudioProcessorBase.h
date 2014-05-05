#ifndef ztd_ZAudiojkchgfdgdfhgProcessor_h__
#define ztd_ZAudiojkchgfdgdfhgProcessor_h__


HELPER_NAMESPACE_START;

class Param
{
public:

	NONCOPYABLE_CLASS(Param);
	STACK_CLASS(Param);

	forcedinline Param(const char* name,float defValue,const initializer_list<const char*>& label,bool isRealtime=true,bool shouldBeSave=true,ParamDisplayTransFunc paramTransFunc=linearParamTrans)
		:m_name(name)
		,m_defValue(defValue)
		,m_isRealtime(isRealtime)
		,m_shouldBeSave(shouldBeSave)
		,m_label(label)
		,m_paramTransFunc(paramTransFunc)
	{};
	forcedinline Param(const char* name,int defValue,const initializer_list<const char*>& label,bool isRealtime=true,bool shouldBeSave=true,ParamDisplayTransFunc paramTransFunc=linearParamTrans)
		:m_name(name)
		,m_defValue((float)defValue / (float)( label.size() - 1 ))
		,m_isRealtime(isRealtime)
		,m_shouldBeSave(shouldBeSave)
		,m_label(label)
		,m_paramTransFunc(paramTransFunc)
	{};
	forcedinline Param(const char* name,bool defValue,const initializer_list<const char*>& label,bool isRealtime=true,bool shouldBeSave=true,ParamDisplayTransFunc paramTransFunc=linearParamTrans)
		:m_name(name)
		,m_defValue((float)defValue)
		,m_isRealtime(isRealtime)
		,m_shouldBeSave(shouldBeSave)
		,m_label(label)
		,m_paramTransFunc(paramTransFunc)
	{}
	~Param() = default;
public:
	const char*const m_name;
	float const      m_defValue;
	bool m_isRealtime;
	bool m_shouldBeSave;
	ParamDisplayTransFunc m_paramTransFunc;
	const initializer_list<const char*>& m_label;
};

class OptParam
{
public:

	NONCOPYABLE_CLASS(OptParam);
	STACK_CLASS(OptParam);

	forcedinline OptParam(const char* name,float defValue,bool isRealtime=false)
		:m_name(name)
		,m_defValue(defValue)
		,m_isRealtime(isRealtime)
	{};
	~OptParam() = default;
public:
	const char* m_name;
	float m_defValue;
	bool m_isRealtime;
};


class ParameterInfoContainer
{
public:
	forcedinline ParameterInfoContainer(const initializer_list<Param>& list);
	forcedinline ParameterInfoContainer(const initializer_list<Param>& list,const initializer_list<OptParam>& optParam);
	~ParameterInfoContainer()=default;
	forcedinline const char* getLabel(int index,int numLabel=0) const { return m_label[ m_stepStart[index] + numLabel ]; }
	forcedinline const char* getName(int index) const { return m_name[index]; }
	forcedinline float       getDefValue(int index) const { return m_defValue[index]; }
	forcedinline int         getParaStep(int index) const { return m_numStep[index]; }
	forcedinline bool        getIsRealtime(int index) const { return m_isRealtime[index]; }
public:
	forcedinline int getNumParamHostKnown()const{ return m_numParams; }
	forcedinline int getNumParamHostUnknown()const { return m_numOptParams; }
	forcedinline int getNumParam()const{ return m_numOptParams+m_numParams; }
	forcedinline int getMaxLabelSize() const { return m_label.size(); }
public:
	void PopVSTXML(const String& filename) const;
public:
	int m_numParams;
	int m_numOptParams;
	Array<const char*> m_name;
	Array<float>       m_defValue;
	Array<bool>        m_isRealtime;
	Array<int>         m_numStep;
	Array<int>         m_stepStart;
	Array<const char*> m_label;
	Array<int>         m_messageQueueIndex;
	Array<ParamDisplayTransFunc> m_paramDisTransFunc;

	XmlElement* _CreateRangeXml(String name,int index) const;
	ParameterInfoContainer(const initializer_list<Param>& list,int addonSize);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterInfoContainer);
};


class ZAudioProcessorBase :public AudioProcessor
{
public:
	ZAudioProcessorBase(const initializer_list<Param>& autoParams);
	ZAudioProcessorBase(const initializer_list<Param>& autoParams,const initializer_list<OptParam>& optParam);
	~ZAudioProcessorBase()=default;
public:
	virtual int getNumParameters() final  { return m_paramInfos.getNumParamHostKnown(); }
	int         getAllNumParameters() { return m_paramInfos.getNumParam(); }
	int         getNumParametersHostUnknown() { return m_paramInfos.getNumParamHostUnknown(); }
	virtual bool isParameterAutomatable(int) const final { return true; }
	virtual bool isMetaParameter(int) const final { return false; }
public:
	virtual const String getParameterName(int parameterIndex) final { return m_paramInfos.getName(parameterIndex); }
	virtual String getParameterLabel(int index) const final;
	virtual const String getParameterText(int parameterIndex) final override;
	virtual int getParameterNumSteps(int parameterIndex) final;
	virtual float getParameterDefaultValue(int parameterIndex) final;
	
	virtual float getParameter(int parameterIndex) final;
	virtual void setParameter(int parameterIndex, float newValue) final;
	void resetParameter(int parameterIndex);
	void resetAllParameter();
public:
	XmlElement* SaveParamStateToXml();
	bool LoadParamStateFromXml(XmlElement*const k);
public:
	forcedinline void PopVSTXML(const String& filename) const { m_paramInfos.PopVSTXML(filename); }
private:
	ParameterInfoContainer m_paramInfos;
	AlignedHeapBlock<ParameterQueue> m_paramQueue;
	HeapBlock<Atomic<float>> m_paramSnap;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZAudioProcessorBase);
};


HELPER_NAMESPACE_END;


#endif // ztd_ZAudioProcessor_h__
