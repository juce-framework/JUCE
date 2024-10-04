/***********************************************************************

	This file is part of the Avid AAX SDK.

	The AAX SDK is subject to commercial or open-source licensing.

	By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
	Agreement and Avid Privacy Policy.

	AAX SDK License: https://developer.avid.com/aax
	Privacy Policy: https://www.avid.com/legal/privacy-policy-statement

	Or: You may also use this code under the terms of the GPL v3 (see
	www.gnu.org/licenses).

	THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
	DISCLAIMED.

	Copyright (c) 2004, 2024 Avid Technology, Inc. All rights reserved.

************************************************************************/


#ifndef acfuids_h
#define acfuids_h

/*!
    \file acfuids.h
    \brief This file is the master definition of all UIDs for ACF and is included in acfunknown.h.
*/
       
    
/*!
   \defgroup ComponentType UIDS: Component Types
   The component types fundamentally determine the nature of the plugin.
   Compnent Type uid's are used to specify the type of interfaces the plugin will define in the  IACFComponentDefinition::InitializeComponent() call.
   Types of ACF Components that can be implemented by a plug-in (and registered with the host)
*/

//@{

/*! 
\remarks Used to specify a codec component 
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n <b> context: </b> private \n
*/
    DEFINE_ACFUID(acfUID, ACFCompType_Codec,			0xD5960204, 0x709B, 0x11D6, 0x91, 0xB2, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

/*! 
 \remarks Used to specify a component defined by  IAVXEffectDefinition and using the  IAVXEffect interface 
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n <b> context: </b> global \n
*/
    DEFINE_ACFUID(acfUID, ACFCompType_ImageEffect,	0x30f97600, 0x68fc, 0x11d5, 0x8c, 0xc5, 0x8a, 0xb4, 0x1b, 0x9e, 0xf8, 0x40);


/*! 
\remarks Used to specify a utility component. 
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n <b> context: </b> private \n
*/
    DEFINE_ACFUID(acfUID, ACFCompType_Utility,		0xa911bb00, 0x76b4, 0x11d5, 0x9b, 0xa2, 0x8e, 0x1a, 0xdd, 0x2d, 0x2a, 0x27);

/*! 
\remarks Used to specify a component defined by  IAVXModalUI_V0 and using the  IAVXModalUI_V0 interface 
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n <b> context: </b> IAVXEffect \n
*/
    DEFINE_ACFUID(acfUID, ACFCompType_ModalUI,		0xBF35E5C6, 0xE6CC, 0x11D6, 0xAE, 0xD1, 0x00, 0x03, 0x93, 0x83, 0x00, 0x8C);

/*! 
\remarks Used to specify a component defined by  IACFModelessUI and using the  IACFModelessUI interface 
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n <b> context: </b> IAVXEffect \n
*/
    DEFINE_ACFUID(acfUID, ACFCompType_ModelessUI,	0x56C23ACA, 0xB582, 0x11D8, 0x9A, 0x6E, 0x00, 0x0A, 0x95, 0xB0, 0x00, 0x3C);

/*! 
\remarks Used to specify a component defined by  IAVXGraphicOverlay and using the  IAVXGraphicOverlay interface 
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n <b> context: </b> IAVXEffect \n
*/
    DEFINE_ACFUID(acfUID, ACFCompType_GraphicOverlay,		0xa1c8a58,  0xcbe3, 0x4690, 0x9c, 0x46, 0x2e, 0xa1, 0xb0, 0x79, 0x5b, 0xde);

/*! 
\remarks Used to specify a component defined by  IACFDefinition and using the  IACFDefinition interface 
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n <b> context: </b> global \n
*/
    DEFINE_ACFUID(acfUID, ACFCompType_TypeDefinition, 0xd7df1180, 0x68fe, 0x11d5, 0x8c, 0xc5, 0x8a, 0x8e, 0xf7, 0x9e, 0x1c, 0x30);
   	
/*! 
\remarks Used to specify a default component
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n <b> context: </b> global \n
*/
    DEFINE_ACFUID(acfUID, ACFCompType_Default,		0xec66ec80, 0x6986, 0x11d5, 0x9b, 0xa2, 0x8d, 0xb4, 0xb4, 0x2d, 0xfd, 0x37);

/*! 
\remarks Used to specify the an IAVXConformAVX1 component. 
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n <b> context: </b> global \n
7E14445E-506C-4cb4-ACD9-81D20720D032
*/

DEFINE_ACFUID(acfUID, ACFCompType_AVX1Conform, 0x7e14445e, 0x506c, 0x4cb4, 0xac, 0xd9, 0x81, 0xd2, 0x7, 0x20, 0xd0, 0x32);


/*! 
\remarks Used to specify the an IAVXEffectConformDefinition component. 
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n <b> context: </b> global \n
6127BDDD-4679-4cf9-B865-53D22E167763
*/

DEFINE_ACFUID(acfUID, ACFCompType_AdvancedConform, 0x6127bddd, 0x4679, 0x4cf9, 0xb8, 0x65, 0x53, 0xd2, 0x2e, 0x16, 0x77, 0x63);

/*! 
\remarks Used to specify the an AVXEffectConformDefinition component. 
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n <b> context: </b> global \n
	F0BB6D92-D63C-4e73-96F5-F75461EC29CE


DEFINE_ACFUID(acfUID, ACFCompType_AVXEffectConformDefinition, 0xf0bb6d92, 0xd63c, 0x4e73, 0x96, 0xf5, 0xf7, 0x54, 0x61, 0xec, 0x29, 0xce);
*/			

/*!
\remarks Used to specify a component defined by  IACFComponentDefinition and implements the 
 IAVXUpdateParams interface.
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n \b context: global \n
 {A32E8C65-971F-48b4-828F-0A1C2EFDFE62}
*/
DEFINE_ACFUID(acfUID, ACFCompType_UpdateParams, 0xa32e8c65, 0x971f, 0x48b4, 0x82, 0x8f, 0xa, 0x1c, 0x2e, 0xfd, 0xfe, 0x62);
 
/*!
\remarks Used to specify a property controller associated to a property container
\n <b> type: </b> UIDS \n \b function: \em ACFRegisterComponent \n \n context: global \n
 {AE2A8ED4-6239-4757-9F6D-82E45489F437}
*/
DEFINE_ACFUID(acfIID, ACFCompType_PropertyContainerController, 0xae2a8ed4, 0x6239, 0x4757, 0x9f , 0x6d , 0x82 , 0xe4 , 0x54 , 0x89 , 0xf4 , 0x37 ); 

//@} End ComponentType
    


/*!  
   \defgroup GuidStdComponentID UIDS: Standard Component Types
   The component ids for built-in host provided components.
*/

//@{

/*! 
\remarks Used to specify standard component.
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> IACFComponentFactory \n
*/
	DEFINE_ACFUID(acfUID, ACFCompID_PluginDefinition, 0x6F8508AE, 0x7295, 0x11D6, 0x89, 0x52, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

/*! 
\remarks Used to create the basic component definition in a class factory using  IACFComponentFactory interface 
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> global \n
*/
    DEFINE_ACFUID(acfUID, ACFCompID_ComponentDefinition, 0x6C422BFA, 0x70E5, 0x11D6, 0x8C, 0xDD, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);
/*! 
\remarks Used to create the PropertyContainer component definition in a class factory using  IACFComponentFactory interface 
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> global \n
*/

    DEFINE_ACFUID(acfIID, ACFCompID_PropertyContainerDefinition, 0xb6b7a135, 0x555e, 0x4b07, 0x82 , 0xbe , 0x58 , 0xd1 , 0xd6 , 0xdf , 0x53 , 0xab ); 

/*! 
\remarks Used to create the basic effect component in a class factory using  IACFComponentFactory interface 
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> global \n
*/
    DEFINE_ACFUID(acfUID, ACFCompID_EffectDefinition, 0xfa6ce63d, 0x35c3, 0x469c, 0x8d, 0x80, 0xf9, 0xcf, 0x8f, 0xce, 0x7a, 0x1);
/*! 
\remarks Used to create the basic effect component in a class factory using  IACFComponentFactory interface 
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> global \n
*/
    DEFINE_ACFUID(acfUID, ACFCompID_CompositeEffectDefinition, 0x5046f68f, 0x512c, 0x4c44, 0xae , 0x50 , 0x09 , 0x1e , 0x24 , 0xbd , 0x19 , 0x31 ); 

/*! 
\remarks Used to simplify parameter and layout creation  IACFComponentFactory interface 
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> effects \n
*/
	DEFINE_ACFUID(acfUID, ACFCompID_ParamDefinitionGenerator,0x53e3772e, 0x8930, 0x4fe9, 0x85, 0xc9, 0x4f, 0xc6, 0x50, 0xf2, 0x38, 0x15);

/*! 
\remarks Used to simplify parameter and layout creation  IACFComponentFactory interface 
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> effects \n
*/
DEFINE_ACFUID(acfUID, ACFCompID_ParamDefinitionGeneratorFromString, 0x9b1386d0, 0x381d, 0x4c6e, 0x82, 0x4a, 0xe1, 0x26, 0x7c, 0x2, 0xb8, 0xcf);


/*! 
\remarks Used to create the basic parameter component in a class factory using  IACFComponentFactory interface 
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> effects \n
*/
	DEFINE_ACFUID(acfUID, ACFCompID_ParameterDefinition, 0x2ca1d73c, 0x7de5, 0x4f8c, 0x81, 0xe0, 0x40, 0xa0, 0x65, 0x5f, 0x9a, 0x4a);

/*! 
\remarks Used to create the basic layout component in a class factory using  IACFComponentFactory interface 
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> effects \n
*/	
	DEFINE_ACFUID(acfUID, ACFCompID_LayoutDefinition, 0xb710c2bc, 0xdd00, 0x4f3c, 0xa8, 0xcc, 0xed, 0x54, 0x51, 0xf2, 0xb, 0xb4 );

/*! 
\remarks Used to create the basic array component in a class factory using  IACFComponentFactory interface 
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> global \n
\n <b> ACFNamespace name: </b> com.avid.component.arraytypedefinition
\n <b> UID String: </b> 045d982d-51a2-3393-a0fb-e86ebdec0027
*/	
	DEFINE_ACFUID(acfUID, ACFCompID_ArrayTypeDefinition, 0x045d982d, 0x51a2, 0x00a0, 0xa0, 0xfb, 0xe8, 0x6e, 0xbd, 0xec, 0x00, 0x27);


/*! 
\remarks Used to contain a list of definitions IAVX2ConformAssociation
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> global \n
\n <b> ACFNamespace name: </b>
\n <b> UID String: </b> 4C95268F-71F6-4c52-B498-A640ABC75A12
*/	

	DEFINE_ACFUID(acfUID, ACFCompID_ConformAssociation, 0x4c95268f, 0x71f6, 0x4c52, 0xb4, 0x98, 0xa6, 0x40, 0xab, 0xc7, 0x5a, 0x12);
	

/*! 
\remarks Used to create the basic array component in a class factory using IAVXConformDefinition component. 
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> global \n
\n <b> ACFNamespace name: </b>
\n <b> UID String: </b> DBC683A8-40CD-4abd-9ABC-8528C2509F8F
*/	

	DEFINE_ACFUID(acfUID, ACFCompID_EffectConformDefinition, 0xdbc683a8, 0x40cd, 0x4abd, 0x9a, 0xbc, 0x85, 0x28, 0xc2, 0x50, 0x9f, 0x8f);


//@} End StdComponentID


/*! 
\remarks AVXComponentID_HostMemeoryAllocator
This is the component id used for the host memory allocator. All memory
allocated with this component will be from the host's memory address space.
Use this object to allocate dynamic buffers for plug-ins. The plug-in could also
override the global C++ new function in the plug-in so that all C++ objects can use
the same host memory allocator (this will require implementing the ACFStartup
and ACFShutdown callbacks).

\note The host memory allocator is implemented by the host it is accessed through
the IACFMalloc interface with IID equal to IID_IACFMalloc (see acfmemory.h).
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> global \n
\n <b> ACFNamespace name: </b> 
\n <b> UID String: </b> 
*/
	DEFINE_ACFUID(acfUID, ACFCompID_HostMemoryAllocator, 0xc97930cf, 0xfbb8, 0x4789, 0xaf, 0x19, 0x51, 0xd4, 0xc2, 0x25, 0x59, 0xc0);




/*!  
   \defgroup GuidHostProvidedParams UIDS: Pre-Defined Parameters 
   The component ids for built-in host provided parameters.
*/
//@{
/*! 
\remarks Used to specify that an effect uses a Modal dialog  IACFComponentFactory interface 
\n <b> type: </b> UIDS;  \n <b> method:  </b>CreateComponent; \n <b> context: </b> global \n
\n <b> ACFNamespace name: </b> com.avid.component.compositetypedefinition
\n <b> UID String: </b>
*/		
	DEFINE_ACFUID( acfUID,  ABOUT_BOX_GUID, 0x3c24cecc, 0xc6b, 0x4aec, 0xbd, 0x75, 0x1a, 0x56, 0xaf, 0x25, 0x16, 0xbe);
//@} End HostProvidedParams





 
 /*!
  \defgroup GuidBaseDatatTypes  UIDS: Base Data Types
  Definition ids for built-in types 
  */ 
    //
//@{   
 
 /*!
  \defgroup GuidFundementalTypes  UIDS: Fundamental Types
  Definition ids for built-in type 
  */ 
    //
	//@{
	/*!
	\remarks Used to define a parameter of type acfUInt8
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_UInt8, 0x7C62A3A4, 0x7103, 0x11D6, 0xA6, 0x63, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

	/*!
	\remarks Used to define a parameter of type  acfSInt8
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_SInt8, 0x10AF7E67, 0x7103, 0x11D6, 0xA6, 0x63, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

	/*!
	 \remarks Used to define a parameter of type  acfUInt16
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_UInt16, 0x2C74B500, 0x7103, 0x11D6, 0xA6, 0x63, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

	/*!
	\remarks Used to define a parameter of type  acfSInt16
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_SInt16, 0x30E1D692, 0x7103, 0x11D6, 0xA6, 0x63, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

	/*!
	\remarks Used to define a parameter of type  acfUInt32
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_UInt32, 0x4EB1A950, 0x7103, 0x11D6, 0xA6, 0x63, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

	/*!
	\remarks Used to define a parameter of type  acfSInt32
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_SInt32, 0x52B99EB7, 0x7103, 0x11D6, 0xA6, 0x63, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

	/*!
	\warning This type is not yet supported
	\remarks Used to define a parameter of type  acfSInt64
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	\warning This type is not yet supported
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_UInt64, 0x64E362FC, 0x7103, 0x11D6, 0xA6, 0x63, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

	/*!
	\warning This type is not yet supported
	\remarks Used to define a parameter of type  acfSInt64
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>	
	\warning This type is not yet supported
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_SInt64, 0x68553414, 0x7103, 0x11D6, 0xA6, 0x63, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

	/*!
	\warning This type is not yet supported
	\remarks Used to define a parameter of type  avx2Float
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	\warning This type is not yet supported
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_Float, 0x2C74B501, 0x7233, 0x11D6, 0xAC, 0xF3, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

	/*!
	\remarks Used to define a parameter of type  acfDouble
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_Double, 0xB6F8CE00, 0x7233, 0x11D6, 0xAC, 0xF3, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);
	
	/*!
	\remarks Used to define a enumerator of type  acfBool.An acfBool can have the value kACFFalse or kACFTrue.
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_Boolean, 0x27A179D8, 0x7103, 0x11D6, 0x94, 0xF7, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);
	
	/*!
	\remarks Used to define a parameter of type  acfChar
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b> com.avid.component.type.char
	\n <b> UID String: </b> f3f962a3-bb90-3e7d-afe4-8242bc3e4711
	*/
	DEFINE_ACFUID(acfUID, ACFTypeID_Char, 0xf3f962a3, 0xbb90, 0x00af, 0xaf, 0xe4, 0x82, 0x42, 0xbc, 0x3e, 0x47, 0x11);

	/*!
	\remarks Used to define a parameter of type  acfUniChar
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_UniChar, 0x4E0677D6, 0x7293, 0x11D6, 0xB4, 0xE4, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);


	/*!
	\remarks Used to define a parameter of type  acfUID
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_UID, 0x905DBB84, 0x7104, 0x11D6, 0xAE, 0xD3, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);
	//@} End FundementalTypes

    /*! 
		\defgroup GuidStringTypes UIDS: String Types 
	\n \note String types are null terminated
	*/
	//@{
	
	/*!
	\remarks Used to define a unicode string parameters and definition attributes.
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IACFDefinition and subclasses; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_UnicodeString, 0x40C6B9BA, 0x7103, 0x11D6, 0xBB, 0x30, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

	/*!
	\remarks Used to define a parameter that acts as an ASCII string
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_ASCIIString, 0xbac7f4c7, 0x7eeb, 0x4898, 0xbe, 0x2c, 0x8, 0x5f, 0x7, 0x11, 0x3c, 0xc5);

	/*!
	\remarks Used to define a UTF-8 encoded string parameter.
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
	DEFINE_ACFUID(acfUID, ACFTypeID_UTF8String, 0x219a99cc, 0x2c8b, 0x4224, 0x86, 0xfe, 0xc0, 0x57, 0x94, 0x5, 0x5e, 0x1d);

	/*!
	\remarks Used to define a UTF-16 encoded string parameter.
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
	DEFINE_ACFUID(acfUID, ACFTypeID_UTF16String, 0xdd6a731e, 0x8ed1, 0x4d19, 0xa0, 0x96, 0x2b, 0xa, 0x69, 0x67, 0xc1, 0x3f);
	//@} End StringTypes 

	/*! 
	\defgroup PersistentGuidStringTypes UIDS: Private String Types 
	\n \note String types are null terminated
	*/
	//@{

	/*!
	\remarks Internal type that is used only by some host applications to persist ACFTypeID_UnicodeString parameters and definition attributes.
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IACFDefinition and subclasses; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\warning A component may encounter this data type on older hosts that did not have any support for persisting ACFTypeID_UnicodeString
	data. This type is reserved for host applications and should never be used in any definition by a plug-in. 
	Newer plug-ins should be written to either fail gracefully or convert the returned UTF8 as necessary.
	*/
	DEFINE_ACFUID(acfUID, ACFTypeID_WideStringPersistedAsUTF8, 0x72223182, 0xaa0a, 0x4c76, 0xba, 0x3d, 0xf8, 0x8c, 0x91, 0x8b, 0x67, 0xb5);

	/*!
	\remarks Internal type that is used only by some host applications to persist ACFTypeID_UTF16String parameters and definition attributes.
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IACFDefinition and subclasses; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\warning A component may encounter this data type on older hosts that did not have any support for persisting ACFTypeID_UTF16String
	data. This type is reserved for host applications and should never be used in any definition by a plug-in. 
	Newer plug-ins should be written to either fail gracefully or convert the returned UTF8 as necessary.
	*/
	DEFINE_ACFUID(acfUID, ACFTypeID_UTF16StringPersistedAsUTF8, 0x3319f04a, 0xac69, 0x4525, 0xb9, 0xe8, 0x22, 0x6, 0x36, 0x2f, 0xd2, 0x33);
	//@} End StringTypes 

    /*! 
		\defgroup GuidArrayTypes UIDS: Array types
	*/
	//@{

	/*!
	\remarks Used to define a parameter that is an array of Uid's
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_UIDArray, 0xD2E8D2E4, 0x7963, 0x11D6, 0xB8, 0x7B, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);

	/*!
	\remarks Used to define a a user defined parameter. Byte swapping is the responsibility of plugin.
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_ByteArray, 0xC02F2CAA, 0x7293, 0x11D6, 0xB4, 0xE4, 0x00, 0x30, 0x65, 0x8A, 0x65, 0x04);

	/*!
	\remarks Used to define a parameter that is an array of  acfUInt16's
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_Int16Array, 0xF33C004C, 0x7963, 0x11D6, 0x88, 0x28, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);

	/*!
	\remarks Used to define a parameter that is an array of  acfUInt32 's
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_Int32Array, 0x06576FE2, 0x7964, 0x11D6, 0xB7, 0x42, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);

	/*!
	\remarks Used to define a parameter that is an array of  acfUInt64
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_Int64Array, 0x2F64D88A, 0x7964, 0x11D6, 0x84, 0x4F, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);

	/*!
	\remarks Used to define a parameter that is an array of  acfFloat
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b> 5FFF159C-C1F0-4093-B701-02AA4ACE2FD5
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_FloatArray, 0x5fff159c, 0xc1f0, 0x4093, 0xb7, 0x1, 0x2, 0xaa, 0x4a, 0xce, 0x2f, 0xd5);

	/*!
	\remarks Used to define a parameter that is an array of  acfFloat
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b> E1C94795-A4EB-46a0-8132-9085C235079A
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_DoubleArray, 0xe1c94795, 0xa4eb, 0x46a0, 0x81, 0x32, 0x90, 0x85, 0xc2, 0x35, 0x7, 0x9a);

	/*!
	\remarks Used to define a parameter that is an array of  acfBool
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b> C6B68935-3015-4ecd-B7BA-1750295DE9A8
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_BooleanArray, 0xc6b68935, 0x3015, 0x4ecd, 0xb7, 0xba, 0x17, 0x50, 0x29, 0x5d, 0xe9, 0xa8);

	//@} end ArrayTypes

    
    /*!
		\defgroup GuidSpecialTypes UIDS: Specialized Types
    */
	//@{

	/*!
	 \remarks Used to define a value of type acfPoint
	 \n <b> type: </b> acfPoint  \n <b> method:  </b>IACFValue; \n <b> context: </b> global \n
	 \n <b> UID String: </b> ddf6e71d-f596-3b6a-b47b-1de3e6fc1710
	 */
	DEFINE_ACFUID(acfUID, ACFTypeID_Point, 0xddf6e71d, 0xf596, 0x3b6a, 0xb4, 0x7b, 0x1d, 0xe3, 0xe6, 0xfc, 0x17, 0x10);

	/*!
	 \remarks Used to define a value of type acfRect
	 \n <b> type: </b> acfRect  \n <b> method:  </b>IACFValue; \n <b> context: </b> global \n
	 \n <b> UID String: </b> 6c64c82c-3941-36c4-a8bb-c6c1caa15313
	 */
	DEFINE_ACFUID(acfUID, ACFTypeID_Rect, 0x6c64c82c, 0x3941, 0x36c4, 0xa8, 0xbb, 0xc6, 0xc1, 0xca, 0xa1, 0x53, 0x13);

	/*!
	 \remarks Used to define a value of type acfSize
	 \n <b> type: </b> acfSize  \n <b> method:  </b>IACFValue; \n <b> context: </b> global \n
	 \n <b> UID String: </b> abc15b28-60fa-371e-a9ea-95edbe1f9ebd
	 */
	DEFINE_ACFUID(acfUID, ACFTypeID_Size, 0xabc15b28, 0x60fa, 0x371e, 0xa9, 0xea, 0x95, 0xed, 0xbe, 0x1f, 0x9e, 0xbd);

	/*!
	 \remarks Used to define a value of type acfRational32
	 \n <b> type: </b> acfRational32  \n <b> method:  </b>IACFValue; \n <b> context: </b> global \n
	 \n <b> UID String: </b> a73f3bf4-1d20-3d0e-a328-024e8d5ad043
	 */
	DEFINE_ACFUID(acfUID, ACFTypeID_Rational32, 0xa73f3bf4, 0x1d20, 0x3d0e, 0xa3, 0x28, 0x02, 0x4e, 0x8d, 0x5a, 0xd0, 0x43);

	/*!
	 \remarks Used to define a value of type acfRational64
	 \n <b> type: </b> acfRational64  \n <b> method:  </b>IACFValue; \n <b> context: </b> global \n
	 \n <b> UID String: </b> 25eff40a-ec3d-319e-8c53-a21debd06ea9
	 */
	DEFINE_ACFUID(acfUID, ACFTypeID_Rational64, 0x25eff40a, 0xec3d, 0x319e, 0x8c, 0x53, 0xa2, 0x1d, 0xeb, 0xd0, 0x6e, 0xa9);

	/*!
	\remarks Used to define a parameter of type  avx2Point
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_Point2D, 0x79B43006, 0x7971, 0x11D6, 0xB1, 0x42, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);
	
	/*!
	\remarks Used to define a parameter of type  avx2Rect
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_Rect2D, 0x849C0848, 0x7971, 0x11D6, 0xB5, 0x20, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);
	
	/*!
	\remarks Used to define a color parameter of type  avx2RGB
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_ColorRGB, 0xAA3FDAF4, 0x7971, 0x11D6, 0x87, 0x93, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);
	
	/*!
	\remarks Used to define a color parameter of type  avx2HSV
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_ColorHSV, 0xBB2476BF, 0x7971, 0x11D6, 0xAF, 0xA7, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);
	
	/*!
	\remarks Used to define a parameter that is a Unicode list of strings.
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	\warning Not supported.
	\todo support for UnicodeList Type
	*/
    DEFINE_ACFUID(acfUID, ACFTypeID_UnicodeList, 0x10BCD509, 0x7972, 0x11D6, 0xA3, 0xA2, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);
	
	/*!
		\remarks Used to define a parameter that acts as a trigger to a dialog. This parameter will always
		appear as a button. The button launches a modal UI, such as an about box.
		\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
		\n <b> ACFNamespace name: </b>
		\n <b> UID String: </b>

	*/
	// This needs a string format - it has not been generated from a string 
	DEFINE_ACFUID(acfUID, ACFTypeID_DialogTrigger, 0x3f420350, 0x38b, 0x48e7, 0xbe, 0x92, 0x73, 0x5e, 0xe2, 0x60, 0xe8, 0xde);

	/*!
	\remarks Used to initialize a acfUID to NULL 
	\n <b> type: </b> UIDS;  \n <b> method:  </b>any; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b> none
	*/
	DEFINE_ACFUID(acfUID, acfUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	//@} end SpecialTypes
 
//@} end BaseDatatTypes


	/*
		\defgroup GuidStandardLayerGUIDs UIDS: Layer Attributes
	*/
//@{	

	/*!
	\remarks Used to define a parameter of type  acfUID
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
	DEFINE_ACFUID(acfUID, ACFLayerID_Input, 0x1867872a, 0xda5c, 0x4192, 0xab, 0xea, 0xac, 0x39, 0xa4, 0x5f, 0x2b, 0x8a);
	
	/*!
	\remarks Used to define a parameter of type  acfUID
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
	DEFINE_ACFUID(acfUID, ACFLayerID_Output, 0x1867872b, 0xda5c, 0x4192, 0xab, 0xea, 0xac, 0x39, 0xa4, 0x5f, 0x2b, 0x8a);
	
	/*!
	\remarks Used to define a parameter of type  acfUID
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
	DEFINE_ACFUID(acfUID, ACFLayerID_Foreground, 0x1867872c, 0xda5c, 0x4192, 0xab, 0xea, 0xac, 0x39, 0xa4, 0x5f, 0x2b, 0x8a);
	
	/*!
	\remarks Used to define a parameter of type  acfUID
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
	DEFINE_ACFUID(acfUID, ACFLayerID_Background, 0x1867872d, 0xda5c, 0x4192, 0xab, 0xea, 0xac, 0x39, 0xa4, 0x5f, 0x2b, 0x8a);
	
	/*!
	\remarks Used to define a parameter of type  acfUID
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
	DEFINE_ACFUID(acfUID, ACFLayerID_Mask, 0x1867872e, 0xda5c, 0x4192, 0xab, 0xea, 0xac, 0x39, 0xa4, 0x5f, 0x2b, 0x8a);
	
	/*!
	\remarks Used to define a parameter of type  acfUID
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> global \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
	DEFINE_ACFUID(acfUID, ACFLayerID_Source, 0x1867872f, 0xda5c, 0x4192, 0xab, 0xea, 0xac, 0x39, 0xa4, 0x5f, 0x2b, 0x8a);
	
 //@} end StandardLayerGUIDs



	/*
		\defgroup StandardInputOutput UIDS: GUIDs Input and Output data types
	*/
 //@{	
	
	/*!
	\remarks This identifier Used to access image data from an IAVXEffectContext  acfUID
	\n <b> type: </b> IAVXImageInput;  \n <b> method:  </b>IAVXEffectContext::GetInput; \n <b> context: </b> IAVXRenderContext::Render \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b> 6B8C6DB2-4B6F-497f-AB9B-AF2635CD7538
	*/
	DEFINE_ACFUID(acfUID, AVXInputTypeID_Image, 0x4abd6f8b, 0xf4d6, 0x47ff, 0xa2, 0x13, 0x70, 0x6e, 0x37, 0xa5, 0xe5, 0xec);
	
	/*!
	\remarks This identifier Used to Timecode data 
	\n <b> type: </b> IAVXTimeCodeInformateion;  \n <b> method:  </b>IAVXInput::GetInpute; \n <b> context: </b> IAVXRenderContext::Render \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b> 04D8E7A8-EE65-4525-BD6D-EF8E9BA28C2A
	*/
	DEFINE_ACFUID(acfUID, AVXInputTypeID_TimeCode, 0x4d8e7a8, 0xee65, 0x4525, 0xbd, 0x6d, 0xef, 0x8e, 0x9b, 0xa2, 0x8c, 0x2a  );

	/*!
	\remarks This identifier Used to BinInformation data 
	\n <b> type: </b> IAVXBinInformateion;  \n <b> method:  </b>IAVXInput::GetInput; \n <b> context: </b> IAVXRenderContext::Render \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b> // 1C1B7D33-793B-4260-891A-A904F13C2735
	*/
	DEFINE_ACFUID(acfUID, AVXInputTypeID_BinInformation, 0x1c1b7d33, 0x793b, 0x4260, 0x89, 0x1a, 0xa9, 0x4, 0xf1, 0x3c, 0x27, 0x35);

	/*!
	\remarks This identifier Used to access image data from an IAVXOutput  acfUID
	\n <b> type: </b> IAVXImage;  \n <b> method:  </b>IAVXOutput::GetOutput; \n <b> context: </b> IAVXRenderContext::Render \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b> A3F7AE7E-4DA7-4c20-851E-4736E86E705E
	*/
	DEFINE_ACFUID(acfUID, AVXOutputTypeID_Image, 0xa3f7ae7e, 0x4da7, 0x4c20, 0x85, 0x1e, 0x47, 0x36, 0xe8, 0x6e, 0x70, 0x5e);


 //@} end StandardInputOutputGUIDs



/*!
	\defgroup ParameterAttributes UIDS: Parameter Attributes
 */
//@{
	/*!
	\remarks 		 This attribute defines the behavior of the keyframe system in use by the parameter.
		 It Used to specify whether the parameter to be created is animateable or static (non-animatable)
		 Static will be a non interpolated parameter.default is kAVXKeyFrameBehavior_Animateable.
	\n <b> type: </b> avx2KeyFrameBehavior;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b> {6E10012D-2164-438b-8755-E62EE434040E}
	 */
	DEFINE_ACFUID(acfUID, AVXTypeID_KeyframeBehavior, 0x6e10012d, 0x2164, 0x438b, 0x87, 0x55, 0xe6, 0x2e, 0xe4, 0x34, 0x4, 0xe);


	/*!
	\remarks iCoordinate behavior Used to define the translation in 2+D space that is required.
	\n <b> type: </b> acfUInt32;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>		 		          
	 */
	DEFINE_ACFUID(acfUID, AVXTypeID_CoordinateBehavior,0x6961da11, 0x873a, 0x47f2, 0xb9, 0x8a, 0x8a, 0x9c, 0x5e, 0xab, 0x52, 0xb8);
		

	/*!
	\remarks The guid that explicitly defines the parameters type.
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	\internal \todo rename  assure it goes to the date type
	*/     
	DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_DataTypeID, 0xa9eec91, 0xab42, 0x45c0, 0x81, 0x24, 0xf0, 0xa6, 0xe6, 0x73, 0x82, 0xb1 );


	/*!
	\remarks The keyframe behavior.
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	 */
	DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_KeyframeBehavior, 0xb444d868, 0x706e, 0x47e4, 0xbe, 0xe5, 0xaa, 0xff, 0x77, 0xf4, 0xc1, 0x2d  );


	DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_KeyframeInterpolation, 0xaa341317, 0x3697, 0x4ce1, 0xa6, 0xd6, 0x66, 0xce, 0xdd, 0x41, 0xd3, 0x4e);
	// {AA341317-3697-4ce1-A6D6-66CEDD41D34E}	
	DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_KeyframeExtrapolation, 0x4c34a0a6, 0xb29c, 0x49b6, 0xa1, 0xe5, 0xd5, 0x62, 0x84, 0xf, 0xbf, 0xf4);
	// {4C34A0A6-B29C-49b6-A1E5-D562840FBFF4}
		

	/*!
	\remarks 	 The coordinate behavior.
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
	\n <b> ACFNamespace name: </b>
	\n <b> UID String: </b>
	*/
	DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_CoordinateBehavior,0x767eba2d, 0x375d, 0x47c7, 0xba, 0xbd, 0x66, 0x96, 0xa3, 0x3e, 0x6f, 0xb );


	/*!
		\remarks Used to specify the default value of the parameter
		\n <b> type: </b> as defined by ACFATTR_ParamDefinition_DataTypeID \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
		\n <b> AVX Namespace name: </b> com.avid.fx.param.attr.default
		\n <b> UID String: </b>
	 */
	DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_Default, 0xbd3bfeeb, 0x042d, 0x3aae, 0xbf, 0x21, 0x70, 0xeb, 0xc7, 0x46, 0x46, 0xa8 );

/*!
	\remarks specifies the minimum value of the parameter.
	\n <b> type: </b> as defined by ACFATTR_ParamDefinition_DataTypeID  \n <b> method:  </b>IAVXParameterDefinition, IAVXLayoutDefinition. \n <b> context: </b> parameter definition \n
	\n <b> AVX Namespace name: </b> com.avid.fx.param.attr.minimum
	\n <b> UID String: </b>
	\internal \todo The minimum value for a layout and a parameter are independent. The range can vary.
 */
DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_Minimum, 0x81864ee2, 0xa3f1, 0x3235, 0x90, 0xc6, 0x1b, 0x16, 0x51, 0xf0, 0x3b, 0xf5 );

/*!
	\remarks specifies the maximum value of the parameter.
	\n <b> type: </b> as defined by ACFATTR_ParamDefinition_DataTypeID  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
	\n <b> ACFNamespace name: </b> com.avid.fx.param.attr.maximum
	\n <b> UID String: </b>
	\internal \todo The minimum value for a layout and a parameter are independent. The range can vary.
 */
DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_Maximum, 0xf41f6e7c, 0x34d5, 0x3f4f, 0xb3, 0xff, 0xab, 0x02, 0xaf, 0x32, 0xae, 0x63 );

/*!
	\remarks Used to define the step value of a UI widget
	\n <b> type: </b>  as defined by ACFATTR_ParamDefinition_DataTypeID  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
	\n <b> ACFNamespace name: </b> com.avid.fx.param.attr.increment
	\n <b> UID String: </b>
	The default is 1.0. It is used to define the stepping value of a slider. You can use it to set the stepping value of a slider or UI widget. 
	For example a value of 0.1 on a slider with a range from 0-100 will Have slider drag the slider with values 20.1, 20.2 ... as opposed to a default of 1.0 where the values would go 20.0, 21.0, 22.0 .... A value of 2.0 would go 20, 22, 24 etc.
 */
DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_Increment, 0xf1faf361, 0x73c1, 0x3129, 0xb3, 0x2b, 0x34, 0x75, 0x7a, 0x23, 0x65, 0xd5 );

/*!
	\remarks Used to define a multiplier that remaps a UI value. For example a parameter may represent a value that ranges from 0-1. The UI may represent the value different by using this multiplier
	\n <b> type: </b> acfDouble  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
	\n <b> AVX Namespace name: </b> com.avid.fx.param.attr.uiremap
	\n <b> UID String: </b>	1a47fb2d-ef70-338e-b7bf-e824cee15284
 */
DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_UIRemap, 0x1a47fb2d, 0xef70, 0x338e, 0xb7, 0xbf, 0xe8, 0x24, 0xce, 0xe1, 0x52, 0x84 );

/*!
	\remarks Used to define the precision to display the UI value. This optional attribute
	Used to override the display precision for double parameters. This is valid to 0 to display double parameters as integers.
	A special value of -1 (the default) indicates that the host should use its standard precision for displaying doubles.
	\n <b> type: </b> acfSInt32  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
	\n <b> AVX Namespace name: </b> com.avid.fx.param.attr.uiprecision
	\n <b> UID String: </b>	83bcd82e-133d-3b08-82b6-cd45ca5f7424
 */
DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_UIPrecision, 0x83bcd82e, 0x133d, 0x0082, 0x82, 0xb6, 0xcd, 0x45, 0xca, 0x5f, 0x74, 0x24 );

/*!	
	\remarks Used to define the mapping of the structure members to the default layout order for built-in Point2D, Rect2D, ColorHSV and ColorRGB.
	\note To be compatible with the layouts for AVX 1.x parameters the values for the built-in types are 
	- Point2D: {0, 1}, 
	- Rect2D : {1, 0, 3, 2},
	- ColorHSV : {0, 1, 2}, and 
	- ColorRGB : {0, 1, 2}.
	\n <b> type: </b> Int32Array  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
	\n <b> AVX Namespace name: </b> com.avid.fx.param.attr.defaultcomponentorder
	\n <b> UID String: </b>	27995d37-50aa-36ec-b458-089602dcb96d
 */
DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_DefaultComponentOrder, 0x27995d37, 0x50aa, 0x00b4, 0xb4, 0x58, 0x08, 0x96, 0x02, 0xdc, 0xb9, 0x6d );

/*!
	\remarks Used to define a silent parameter. Changing such a parameter value will not invalidate an existing render.
	\n <b> type: </b> Boolean  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
	\n <b> AVX Namespace name: </b> 
	\n <b> UID String: </b>	C38E4D31-42A1-4d8f-9580-CA8326597EEA
 */
DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_Silent, 0xc38e4d31, 0x42a1, 0x4d8f, 0x95, 0x80, 0xca, 0x83, 0x26, 0x59, 0x7e, 0xea );

/*!
\remarks Used to define a transient parameter. Transient parameters are non-persistent and silent.
\warning Do not store any pointers in transient parameters.
\n <b> type: </b> Boolean  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
\n <b> AVX Namespace name: </b> 
\n <b> UID String: </b>	72CFF9FA-4EC9-435c-B82A-BEC4B6452E59
*/
DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_Transient, 0x72cff9fa, 0x4ec9, 0x435c, 0xb8, 0x2a, 0xbe, 0xc4, 0xb6, 0x45, 0x2e, 0x59 );

/*!
	\remarks A hidden parameter that indicates whether or not an effect internally uses an accumulation buffer so that the results of one frame depend on the results of previous frames.
	A True value signifies that the effect must always be fully rendered in one atomic operation, while a False value (the default) means the effect is allowed to be partially rendered.
	\note FX should not create this parameter themselves, but only get access to it via a "find" or "get" mechanism instead.
	\n <b> type: </b> Boolean  \n <b> method:  </b>IAVXParameterDefinition; \n <b> context: </b> parameter definition \n
	\n <b> AVX Namespace name: </b>
	\n <b> UID String: </b>	B5D3B187-84A9-46EC-B5D3-D77CE2632873
*/
DEFINE_ACFUID(acfUID, ACFATTR_ParamDefinition_NeedAccumulationBuffer, 0xb5d3b187, 0x84a9, 0x46ec, 0xb5, 0xd3, 0xd7, 0x7c, 0xe2, 0x63, 0x28, 0x73 );

/*!
	\remarks An Attribute, specified by a parameter to use  an enabler button with provided by the host.
	\n <b> type: </b> acfBoolen or type specificed by ACFATTR_LayoutDefinition_EnablerDefault;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout definition \n
	\n <b> AVX Namespace name: </b> com.avid.fx.layout.attr.enabler
	\n <b> UID String: </b> D3306718-7B0A-4af3-85D6-CAE0D4F100F3
 */
DEFINE_ACFUID(acfUID, ACFATTR_LayoutDefinition_Enabler, 0xd9597d01, 0x641f, 0x00b7, 0xb7, 0xcc, 0xf4, 0xe1, 0x4f, 0x5a, 0x82, 0xff );

/*!
	\remarks An Attribute, specified for a group layout to be be initially open.
	\n <b> type: </b> acfBool;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout definition \n
	\n <b> AVX Namespace name: </b> com.avid.fx.layout.attr.autoopen
	\n <b> UID String: </b> 3ce128c3-4efd-32d8-a282-d678b150f419
 */
DEFINE_ACFUID(acfUID, ACFATTR_LayoutDefinition_AutoOpen, 0x3ce128c3, 0x4efd, 0x00a2, 0xa2, 0x82, 0xd6, 0x78, 0xb1, 0x50, 0xf4, 0x19 );

/*!
	\remarks An Attribute, specified for a group layout to be be initially open.
	\n <b> type: </b> acfBool;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout definition \n
	\n <b> AVX Namespace name: </b> com.avid.fx.layout.attr.textbelow
	\n <b> UID String: </b> fc03f25f-39c5-3044-82e7-cbb20245e54e
 */
DEFINE_ACFUID(acfUID, ACFATTR_LayoutDefinition_TextBelow, 0xfc03f25f, 0x39c5, 0x0082, 0x82, 0xe7, 0xcb, 0xb2, 0x02, 0x45, 0xe5, 0x4e );

/*!
\remarks An Attribute, specified for a layout to be be initially hidden.
\n <b> type: </b> acfBool;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout definition \n
\n <b> UID String: </b> 1C0D5318-6035-4b4c-831B-DF71E3BD5152
*/
DEFINE_ACFUID(acfUID, ACFATTR_LayoutDefinition_Hidden, 0x1c0d5318, 0x6035, 0x4b4c, 0x83, 0x1b, 0xdf, 0x71, 0xe3, 0xbd, 0x51, 0x52 );

/*
\remarks Defining this attribute specifies the default for the enabler of this parameter.
\n If a default is not specified for an enabler, it is assumed to false.
\n <b> type: </b> acfBoolean;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> parameter definition \n
\n <b> UID String: </b> 8FFF1ACF-187B-45cb-85D0-CEDC576CC77B
*/
DEFINE_ACFUID(acfUID, ACFATTR_LayoutDefinition_EnablerDefault, 0x8fff1acf, 0x187b, 0x45cb, 0x85, 0xd0, 0xce, 0xdc, 0x57, 0x6c, 0xc7, 0x7b);

//@} end ParameterAttributes



/*!
	\defgroup Widgets  UIDS: Widgets and User Interface
	\brief view ID's for defining the widget used in defining the layout
*/

//@{ 

/*
	\remarks Used to define a layout that uses the described widget 
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
	\n <b> AVX Namespace name: </b> com.avid.fx.layout.widget.std.slider
	\n <b> UID String: </b>
*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_SLIDER, 0xb456be51, 0x430d, 0x00a9, 0xa9, 0x0e, 0x02, 0x31, 0xcf, 0x96, 0x42, 0x91);

/*
	\remarks Used to define a layout that uses the described widget 
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
	\n <b> AVX Namespace name: </b> com.avid.fx.layout.widget.std.angle
	\n <b> UID String: </b>
*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_ANGLE, 0xb637ee40, 0x7a41, 0x009f, 0x9f, 0x8d, 0x01, 0x7d, 0x92, 0x62, 0xe8, 0x05);

/*
	\remarks Used to define a layout that uses the described widget 
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
	\n <b> AVX Namespace name: </b> com.avid.fx.layout.widget.std.treadmill
	\n <b> UID String: </b>
*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_TREADMILL, 0x0939e237, 0xd8b8, 0x0083, 0x83, 0x57, 0xab, 0x47, 0x49, 0x40, 0xba, 0x18);

/*
	\remarks Used to define a layout that uses the described widget 
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
	\n <b> ACFNamespace name: </b> com.avid.fx.layout.widget.std.checkbox
	\n <b> UID String: </b>
*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_CHECKBOX, 0xdfdcd5ca, 0x1e9a, 0x00b0, 0xb0, 0x01, 0xdf, 0xed, 0x26, 0xbe, 0xde, 0x85);

/*
	\remarks Used to define a layout that uses the described widget 
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
	\n <b> AVX Namespace name: </b> com.avid.fx.layout.widget.std.button
	\n <b> UID String: </b>
*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_BUTTON, 0x940d8d3a, 0x730c, 0x00a6, 0xa6, 0x84, 0xb9, 0x14, 0xc5, 0x20, 0xa2, 0xfb);


/*!
	\remarks Used to define a layout that uses the described widget 
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
	\n <b> ACFNamespace name: </b> com.avid.fx.layout.widget.std.listbox
	\n <b> UID String: </b> f22bc77f-0e0a-34a7-b0a5-3efae9efa7e1

*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_LISTBOX, 0xf22bc77f, 0x0e0a, 0x00b0, 0xb0, 0xa5, 0x3e, 0xfa, 0xe9, 0xef, 0xa7, 0xe1);

/*!
	\remarks Used to define a layout that uses the described widget 
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
	\n <b> ACFNamespace name: </b> com.avid.fx.layout.widget.std.colorpicker
	\n <b> UID String: </b>
*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_COLORPICKER, 0xb953fb71, 0xc56c, 0x0080, 0x80, 0x0a, 0xb5, 0xa8, 0x2f, 0xc0, 0xcf, 0x53);

/*!
	\remarks Used to define a layout that uses the described widget 
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
	\n <b> ACFNamespace name: </b> com.avid.fx.layout.widget.std.rgbpicker
	\n <b> UID String: </b> 9d2fdae5-cbc5-3ee8-b384-1bbbcefe5bd8
*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_RGBPICKER, 0x9d2fdae5, 0xcbc5, 0x00b3, 0xb3, 0x84, 0x1b, 0xbb, 0xce, 0xfe, 0x5b, 0xd8);

/*!
	\remarks Used to define a layout that uses the described widget 
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
	\n <b> ACFNamespace name: </b> com.avid.fx.layout.widget.std.listitem
	\n <b> UID String: </b> 1e2edea8-3314-3a90-b03b-f3c9c211d57a
*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_LISTITEM, 0x1e2edea8, 0x3314, 0x00b0, 0xb0, 0x3b, 0xf3, 0xc9, 0xc2, 0x11, 0xd5, 0x7a);

/*!
	\remarks Used to define a layout that uses the described widget 
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
	\n <b> ACFNamespace name: </b> com.avid.fx.layout.widget.std.hsvpicker
	\n <b> UID String: </b> dff4738b-272c-3f49-88a1-4490cc657297
*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_HSVPICKER, 0xdff4738b, 0x272c, 0x0088, 0x88, 0xa1, 0x44, 0x90, 0xcc, 0x65, 0x72, 0x97);

/*!
	\remarks Used to define a layout that uses the described widget 
	\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
	\n <b> ACFNamespace name: </b> com.avid.fx.layout.widget.std.lumaslider
	\n <b> UID String: </b> d238ff99-2cd7-370c-9da6-4fda5fd4b3b7
*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_LUMASLIDER, 0xd238ff99, 0x2cd7, 0x009d, 0x9d, 0xa6, 0x4f, 0xda, 0x5f, 0xd4, 0xb3, 0xb7);


/*!
\remarks Used to define a layout that uses the described widget 
\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
\n <b> ACFNamespace name: </b> com.avid.fx.layout.widget.std.editbox
\n <b> UID String: </b> 625675dc-eba5-345e-8460-77b262998442
*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_EDITBOX, 0x625675dc, 0xeba5, 0x0084, 0x84, 0x60, 0x77, 0xb2, 0x62, 0x99, 0x84, 0x42);

/*!
\remarks Used to define a layout that uses the  keyframe graph for animateable custom data
\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
\n <b> ACFNamespace name: </b> com.avid.fx.layout.widget.std.customdatagraph
\n <b> UID Byte Array: </b> 
*/
DEFINE_ACFUID(acfUID, ACF_CUSTOM_DATAGRAPH, 0x46bc527e, 0x02ab, 0x341a, 0x82, 0xac, 0xf9, 0xf5, 0x78, 0x6c, 0xd2, 0xad);

/*!
\remarks Used to define a layout that uses the described widget
\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXLayoutDefinition; \n <b> context: </b> layout \n
\n <b> ACFNamespace name: </b> com.avid.fx.layout.widget.std.numericaleditbox
\n <b> UID String: 1DFFE390-B178-455A-8B3F-08BEEFFF430F</b> 
*/
DEFINE_ACFUID(acfUID, ACF_WIDGET_NUMERICALEDITBOX, 0x1dffe390, 0xb178, 0x455a, 0x8b, 0x3f, 0x8, 0xbe, 0xef, 0xff, 0x43, 0xf);
//@}

/*!
\defgroup Keyframe_Interpolator_UIDs   UIDS: Keyframe Interpolation and Extrapolation UIDs 
\brief view ID's for defining the keyframe interpolation and extrapolation behavior
*/

//@{ 

/*!
\remarks Used when no keyframe interpolation has been defined. 
\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXAnimateableParameter; \n <b> context: </b> IAVXAnimateableParameter \n
\n <b> ACFNamespace name: </b> Because these id's match the AAF specification there is no predefined ACFNamespace
\note The UIDS is equivalent to AAF's \e aafUID_tNoInterpolator
*/
DEFINE_ACFUID( acfUID, AVXInterpID_None,  0x5B6C85A3, 0x0EDE, 0x11d3, 0x80, 0xA9, 0x00, 0x60, 0x08, 0x14, 0x3E, 0x6F );

/*!
 \remarks used when the parameter value between keyframes is help constant based on the closest key in time.
\ note AVXInterpID_Constant provides the same behavior as a static parameter. 
\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXAnimateableParameter; \n <b> context: </b> IAVXAnimateableParameter \n
\n <b> ACFNamespace name: </b> Because these id's match the AAF specification there is no predefined ACFNamespace
\note The UIDS is equivalent to AAF's \e aafUID_tConstantInterpolator
*/
DEFINE_ACFUID( acfUID, AVXInterpID_Constant,  0x5B6C85A5, 0x0EDE, 0x11d3, 0x80, 0xA9, 0x00, 0x60, 0x08, 0x14, 0x3E, 0x6F );

/*!
\remarks defines values between keys to be calculated by the equation for a line: y=mx+b 
\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXAnimateableParameter; \n <b> context: </b> IAVXAnimateableParameter \n
\n <b> ACFNamespace name: </b> Because these id's match the AAF specification there is no predefined ACFNamespace
\note The UIDS is equivalent to AAF's \e aafUID_tLineraInterpolator
*/
DEFINE_ACFUID( acfUID,AVXInterpID_Linear,  0x5B6C85A4, 0x0EDE, 0x11d3, 0x80, 0xA9, 0x00, 0x60, 0x08, 0x14, 0x3e, 0x6f );

/*!
\remarks defines values between two keyframes to be calculated base on a the equation for a cubic spline.  
\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXAnimateableParameter; \n <b> context: </b> IAVXAnimateableParameter \n
\n <b> ACFNamespace name: </b> Because these id's match the AAF specification there is no predefined ACFNamespace
\note The UIDS is equivalent to AAF's \e aafUID_tAvidCubicInterpolator
*/
DEFINE_ACFUID(acfUID,AVXInterpID_Cubic, 0xa04a5439, 0x8a0e, 0x4cb7, 0x97, 0x5f, 0xa5, 0xb2, 0x55, 0x86, 0x68, 0x83 );

/*!
\remarks defines values between two keyframes to be calculated base on a the equation for a bezier curve.  
\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXAnimateableParameter; \n <b> context: </b> IAVXAnimateableParameter \n
\n <b> ACFNamespace name: </b> Because these id's match the AAF specification there is no predefined ACFNamespace
\note The UIDS is equivalent to AAF's \e aafUID_tAvidBezierInterpolator
*/
DEFINE_ACFUID( acfUID,AVXInterpID_Bezier, 0xdf394eda, 0x6ac6, 0x4566, 0x8d, 0xbe, 0xf2, 0x8b, 0x0b, 0xdd, 0x78, 0x1a );

/*!
\remarks Used when no keyframe extrapolation has been defined. 
\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXAnimateableParameter; \n <b> context: </b> IAVXAnimateableParameter \n
\n <b> ACFNamespace name: </b> Because these id's match the AAF specification there is no predefined ACFNamespace
\note The UIDS is equivalent to AAF's \e aafUID_tAvidNullExtrapolator
*/
DEFINE_ACFUID( acfUID,AVXExtrapID_Null,  0x5d08d9a9, 0x75df, 0x4a34, 0x92, 0x03, 0x1b, 0x8d, 0xd1, 0x2c, 0x43, 0xc3 );

/*!
\remarks the value of the youngest keyframe Used and remains constant to -/+ infinity. 
\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXAnimateableParameter; \n <b> context: </b> IAVXAnimateableParameter \n
\n <b> ACFNamespace name: </b> Because these id's match the AAF specification there is no predefined ACFNamespace
\note The UIDS is equivalent to AAF's \e aafUID_tHoldExtrapolator
*/
DEFINE_ACFUID( acfUID,AVXExtrapID_Hold,  0x0e24dd54, 0x66cd, 0x4f1a, 0xb0, 0xa0, 0x67, 0x0a, 0xc3, 0xa7, 0xa0, 0xb3 );

/*!
\remarks extrapolation of keyframes will be based on the closest, youngest keyframe, who's slope will determine values to +/- infinity
\n <b> type: </b> UIDS;  \n <b> method:  </b>IAVXAnimateableParameter; \n <b> context: </b> IAVXAnimateableParameter \n
\n <b> ACFNamespace name: </b> Because these id's match the AAF specification there is no predefined ACFNamespace
\note The UIDS is equivalent to AAF's \e aafUID_tLinearExtrapolator
*/
DEFINE_ACFUID( acfUID,AVXExtrapID_Linear, 0x35c777be, 0x2cd1, 0x4360, 0x96, 0x3d, 0xb5, 0x15, 0x69, 0x39, 0xba, 0x5f );

//@}



#endif // acfuids_h
