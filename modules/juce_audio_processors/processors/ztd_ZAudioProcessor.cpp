

ZAudioProcessor::Editor::Editor(ZAudioProcessor& processor) 
	:AudioProcessorEditor(&processor)
	,m_processor(processor)
	,m_numParam(processor.getAllNumParameters())
	,m_paraUIsnap(m_numParam)
	//,m_signals(m_numParam)
	,m_signals()
{
	m_signals.ensureStorageAllocated(m_numParam);
	jassert(&processor != nullptr);
	//for(int i = 0; i < m_numParam; ++i) m_signals[i] = nullptr;
}


void ZAudioProcessor::Editor::ConnectParaObj(ParameterChangeObject* obj,int index)
{
	jassert(index >= 0 && index < m_numParam);
	obj->m_processor = &m_processor;
	obj->m_index = index;
	//m_signals[index] = obj;
	m_signals.add(obj);
}

void ZAudioProcessor::Editor::HandleParametersChange()
{
	for(int i = 0; i < m_signals.size(); ++i) {
		float const temp = m_processor.getParameter(m_signals[i]->m_index);
		if(m_paraUIsnap[i] != temp) {
			m_signals[i]->HandleValueChange(temp);
		}
		m_paraUIsnap[i] = temp;
	}
}

/*
void ZAudioProcessor::Editor::ConnectParaObj(ParameterChangeObject* obj,int index)
{
	jassert(index >= 0 && index < m_numParam);
	obj->m_processor = &m_processor;
	obj->m_index = index;
	m_signals[index] = obj;
}

void ZAudioProcessor::Editor::HandleParametersChange()
{
	for(int i = 0; i < m_numParam; ++i) {
		float const temp = getZAudioProcessor().getParameter(i);
		if(m_paraUIsnap[i] != temp) {
			if(m_signals[i] != nullptr) m_signals[i]->HandleValueChange(temp);
		}
		m_paraUIsnap[i] = temp;
	}
}*/

