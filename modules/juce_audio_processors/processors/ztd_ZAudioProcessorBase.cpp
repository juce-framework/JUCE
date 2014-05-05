

HELPER_NAMESPACE_START;


ParameterInfoContainer::ParameterInfoContainer(const initializer_list<Param>& list,int addonSize)
	:m_numParams(list.size())
	,m_numOptParams(addonSize)
	,m_name()
	,m_defValue()
	,m_isRealtime()
	,m_numStep()
	,m_stepStart()
	,m_label()
	,m_paramDisTransFunc()
{
	const int size = list.size();
	m_name.ensureStorageAllocated(size+addonSize);
	m_defValue.ensureStorageAllocated(size+addonSize);
	m_numStep.ensureStorageAllocated(size);
	m_stepStart.ensureStorageAllocated(size);
	m_label.ensureStorageAllocated(size);
	m_isRealtime.ensureStorageAllocated(size+addonSize);
	m_paramDisTransFunc.ensureStorageAllocated(size);
	auto k = list.begin();
	int x = 0;
	for(int i = 0; i < size; ++i) {
		m_name.add(k[i].m_name);
		m_defValue.add(k[i].m_defValue);
		m_stepStart.add(x);
		m_isRealtime.add(k[i].m_isRealtime);
		m_paramDisTransFunc.add(k[i].m_paramTransFunc);
		const initializer_list<const char*>& temp = k[i].m_label;
		m_numStep.add(temp.size());
		m_label.ensureStorageAllocated(x + temp.size());
		for(auto e : temp) {
			m_label.add(e);
			++x;
		}
	}
}

ParameterInfoContainer::ParameterInfoContainer(const initializer_list<Param>& list) 
	:ParameterInfoContainer(list,0)
{

}


ParameterInfoContainer::ParameterInfoContainer(const initializer_list<Param>& list,const initializer_list<OptParam>& optlist) 
	:ParameterInfoContainer(list,optlist.size())
{
	for(size_t i = 0; i < optlist.size(); ++i) {
		m_name.add(optlist.begin()[i].m_name);
		m_isRealtime.add(optlist.begin()[i].m_isRealtime);
		m_defValue.add(optlist.begin()[i].m_defValue);
	}
}

void ParameterInfoContainer::PopVSTXML(const String& filename) const
{
	XmlElement K("VSTPluginProperties");
	XmlElement*const k = new XmlElement("VSTParametersStructure");
	K.addChildElement(k);

	for(int i = 0; i < m_numParams; ++i) {
		XmlElement* e = new XmlElement("Param");
		e->setAttribute("name",m_name[i]);
		e->setAttribute("id",i);

		if(m_numStep[i] != 1) {
			const String name = String("LabelTypeInIndex") + String(i);
			k->addChildElement(_CreateRangeXml(name,i));
			e->setAttribute("type",name);
			e->setAttribute("label","");
		} else {
			e->setAttribute("label",getLabel(i));
		}

		k->addChildElement(e);
	}
	File file(filename);
	file.create();
	String text = K.createDocument("",false,false);
	Logger::writeToLog(text);
	file.replaceWithText(text);
}

XmlElement* ParameterInfoContainer::_CreateRangeXml(String name,int index) const
{
	XmlElement* a = new XmlElement("ValueType");
	a->setAttribute("name",name);
	int const size = getParaStep(index);
	jassert(size > 1);
	if(size == 2) {
		XmlElement* x1 = new XmlElement("Entry");
		x1->setAttribute("name",getLabel(index,0));
		x1->setAttribute("value",String("[0,0.5["));
		a->addChildElement(x1);
		x1 = new XmlElement("Entry");
		x1->setAttribute("name",getLabel(index,1));
		x1->setAttribute("value",String("[0.5,1]"));
		a->addChildElement(x1);
	} else {
		for(int i = 0; i < size; ++i) {
			XmlElement* x1 = new XmlElement("Entry");
			x1->setAttribute("name",getLabel(index,i));
			const String c = ( i == size - 1 ) ? "]" : "[";
			x1->setAttribute("value",String("[") + String(1.f / (float)size*(float)i) + "," + String(1.f / (float)size*( (float)i + 1 )) + c);
			a->addChildElement(x1);
		}
	}
	return a;
}


//-------------------------------------------------------------------------------------

ZAudioProcessorBase::ZAudioProcessorBase(const initializer_list<Param>& autoParams) 
	:AudioProcessor()
	,m_paramInfos(autoParams)
	,m_paramSnap(m_paramInfos.getNumParam(),true)
{
	resetAllParameter();
}

ZAudioProcessorBase::ZAudioProcessorBase(const initializer_list<Param>& autoParams,const initializer_list<OptParam>& optParam)
	: AudioProcessor()
	,m_paramInfos(autoParams,optParam)
	,m_paramSnap(m_paramInfos.getNumParam(),true)
{
	resetAllParameter();
}

XmlElement* ZAudioProcessorBase::SaveParamStateToXml()
{
	XmlElement* a = new XmlElement("parameterState");
	for(int i = 0; i < getAllNumParameters(); ++i) {
		a->setAttribute(m_paramInfos.getName(i),String(m_paramSnap[i].get()));
	};
	return a;
}

bool ZAudioProcessorBase::LoadParamStateFromXml(XmlElement*const k)
{
	try {
		if(k == nullptr) throw -3;
		if(k->getTagName() != "parameterState") throw -3;
		for(int i = 0; i < getAllNumParameters(); ++i) {
			if(k->getAttributeName(i) != m_paramInfos.getName(i)) throw -3;
			m_paramSnap[i] = k->getAttributeValue(i).getFloatValue();
		};
	} catch(...) { return false; }
	return true;
}

const String ZAudioProcessorBase::getParameterText(int parameterIndex)
{
	if(m_paramInfos.m_numStep[parameterIndex] == 1) {
		return m_paramInfos.m_paramDisTransFunc[parameterIndex](m_paramSnap[parameterIndex].get()) + " " + m_paramInfos.getLabel(parameterIndex,0);
	} else {
		const float value = m_paramSnap[parameterIndex].get();
		const int numStep = m_paramInfos.m_numStep[parameterIndex];
		float const i = (float)( numStep - 1 );
		int const k = jmax(0,(int)( value*i ));
		int const temp = jmin(numStep - 1,k);
		return m_paramInfos.getLabel(parameterIndex,temp);
	}
}

String ZAudioProcessorBase::getParameterLabel(int index) const
{
	return m_paramInfos.m_numStep[index] == 1 ? m_paramInfos.getLabel(index,0) : "";
}

int ZAudioProcessorBase::getParameterNumSteps(int parameterIndex)
{
	int const k = m_paramInfos.m_numStep[parameterIndex]; return k == 1 ? 0x7fffffff : k;
}

float ZAudioProcessorBase::getParameterDefaultValue(int parameterIndex)
{
	return m_paramInfos.m_defValue[parameterIndex];
}

float ZAudioProcessorBase::getParameter(int parameterIndex)
{
	return m_paramSnap[parameterIndex].get();
}

void ZAudioProcessorBase::setParameter(int parameterIndex,float newValue)
{
	m_paramSnap[parameterIndex] = newValue;
}

void ZAudioProcessorBase::resetParameter(int parameterIndex)
{
	m_paramSnap[parameterIndex] = m_paramInfos.getDefValue(parameterIndex);
}

void ZAudioProcessorBase::resetAllParameter()
{
	for (int i=0;i<m_paramInfos.getNumParam();++i) {
		m_paramSnap[i]=m_paramInfos.getDefValue(i);
	}
}

HELPER_NAMESPACE_END;