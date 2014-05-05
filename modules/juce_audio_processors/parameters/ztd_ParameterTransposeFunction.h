#ifndef ztd_ParameterTranspose_h__
#define ztd_ParameterTranspose_h__

typedef String (*ParamDisplayTransFunc)(float);

inline String linearParamTrans(float value) { return String(value,2); }

inline String dBparamTrans(float value) 
{
	const float decibels=Decibels::gainToDecibels(value);
	return value<=0.f?"-INF":String(decibels,1);
}


inline String freqParamTrans(float value) { return String((value),2); }

#endif // ztd_ParameterTranspose_h__
