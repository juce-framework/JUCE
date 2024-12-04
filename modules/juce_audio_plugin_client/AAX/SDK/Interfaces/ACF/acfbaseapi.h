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

	Copyright 2004-2013 Avid Technology, Inc.

************************************************************************/




#ifndef acfbaseapi_h
#define acfbaseapi_h

/*!
    \file acfbaseapi.h
    \brief Defines the common public interfaces that must be implemented 
     or used by all ACF plugins.
    \remarks
     The plugin must export their ACFRegisterPlugin, ACFRegisterComponent, 
     ACFGetClassFactory and ACFCanUnloadNow functions; the typedefs are located 
     here to ensure all are changed at the same time.
 */


#include "acfbasetypes.h"
#include "acfunknown.h"
//#include "acfresult.h"


class IACFClassFactory; 		// implemented by plug-ins

class IACFDefinition;			// implemented by host
class IACFPluginDefinition;		// implemented by host
class IACFComponentDefinition;	// implemented by host
class IACFEnumDefinitions;      // implemented by host
class IACFComponentFactory;		// implemented by host




/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Required Plug-in callbacks:
//	ACFRegisterPlugin
//	ACFRegisterComponent
//	ACFGetClassFactory
//	ACFCanUnloadNow
//
// Optional Plug-in callbacks:
//  ACFStartup
//  ACFShutdown


/*!
   \b ACFRegisterPlugin
   \brief Required callback function to register a plug-in definition with the host.
   \remarks
    The host will call this function to allow the plug-in to initialize a plug-in definition
    with required and optional attributes for this plug-in. This is the first plug-in
    function called by the host during the registration process. The returned plug-in 
    definition is released after the plug-in's components have been registered.
    \note: This function must be exported from every ACF plug-in.
    The Host uses this callback to define the basic attributes necessary to manage 
    an ACF plug-in.
   \param pUnkHost The unknown interface of the ACF host. Use this interface dynamically 
    access host services. In this case use the IACFComponentFactory interface to create 
    a built-in plug-in definition.
   \param ppPluginDefinition Pointer to an IACFPluginDefinition interface pointer
 */
ACFPLUGINAPI ACFRegisterPlugin (
	IACFUnknown * pUnkHost,
	IACFPluginDefinition **ppPluginDefinition
);


/*!
   \b ACFRegisterComponent
   \brief Required callback function to register a plug-in component definition with the host.
   \remarks
    The host will call this function to allow the plug-in to initialize a component 
    definition with required and optional attributes. This plug-in function is called once 
    for every component indicated by the previous call to ACFRegisterPlugin. The returned 
    component definition is released after all of the plug-in's components have been 
    registered. 
   \note This function must be exported from every ACF plug-in.
    The Host uses this callback to define the basic attributes necessary to manage 
    an ACF plug-in component.
   \param pUnkHost The unknown interface of the ACF host. Use this interface dynamically 
    access host services. In this case use the IACFComponentFactory interface to create 
    a built-in component definition.
   \param index The zero based index of the component to define.
   \param ppComponentDefinition Pointer to an IACFComponentDefinition interface pointer
 */
ACFPLUGINAPI ACFRegisterComponent (
	IACFUnknown * pUnkHost,
	acfUInt32 index,
	IACFComponentDefinition **ppComponentDefinition
);


/*!
   \b ACFGetClassFactory
   \brief Required callback function that returns a component class factory for the corresponding clsid.
   \remarks
   	This function performs exactly the same function as the standard DllGetClassObject except that 
   	ACF uses an IACFClassFactory instead of IClassFactory.
   	
    For further details see the examples or search the web for DllGetClassObject.
   \note This function must be exported from every ACF plug-in.
     The Host uses this callback to indirectly create component objects through an associated
    class factory interface, IACFClassFactory. 
   \param pUnkHost The unknown interface of the ACF host. Use this interface to dynamically 
    access host services.
   \param clsid The unique identifier of a component implemenation class
   \param iid The interface identifier for the class factory interface, usually IID_IACFClassFactory
   \param ppOut
 */
ACFPLUGINAPI ACFGetClassFactory (
    IACFUnknown * pUnkHost,
    const acfCLSID& clsid, 
    const acfIID& iid, 
    void** ppOut
);

/*!
   \b ACFCanUnloadNow
   \brief Required callback function that allows the plug-in to determine when it is safe to be unloaded.
   \remarks
    If it is safe to unload the plug-in module (i.e. no outstanding external references to internal
    objects) then return ACF_OK, otherwise ACF_FALSE (or any error code).
    \note When the plug-in registration process is complete ACFCanUnloadNow() should return ACF_OK.
   \note This function must be exported from every ACF plug-in.
     The Host uses this callback to determine if it is safe to unload the plug-in. 
   \param pUnkHost The unknown interface of the ACF host. Use this interface to dynamically 
    access host services.
 */
ACFPLUGINAPI ACFCanUnloadNow (IACFUnknown * pUnkHost);

/*!
   \b ACFStartup
   \brief Opitional callback to the plug-in to allow plug-in module perform global initialization.
   \remarks
    This opitional callback if it exists will be called once by the ACF Host before the first 
	call to ACFGetClassFactory. Use this routine to initialize any global state or services 
	required by the plug-in and any plug-in components.
	\note The plug-in module may have been unloaded after registration or loaded without registration 
	before ACFStartup is called. ACFStartup is only called after a plug-in any its components
	have been registered.
   \note This function may be exported from any ACF plug-in.
     The Host uses this callback to provide an optional hook for plug-ins to safely
    initialize global state.
   \param pUnkHost The unknown interface of the ACF host. Use this interface dynamically 
    access host services.
 */
ACFPLUGINAPI ACFStartup (IACFUnknown * pUnkHost);

/*!
   \b ACFShutdown
   \brief Opitional callback to the plug-in to allow plug-in module perform global cleanup.
   \remarks
    This opitional callback if it exists will be called by the ACF Host before a plug-in module
	is unloaded. Use this routine to cleanup any global state or cached component services 
	used by the plug-in or any of plug-in components. It must be safe for the host to unload the 
	plug-in module after the calling ACFShutdown (i.e. ACFCanUnloadNow() must return ACF_OK).
	\note The ACFShutdown is never called during the registration process; it is only called 
	by the host after the plug-in module has been loaded to create components.
   \note This function may be exported from any ACF plug-in.
     The Host uses this callback to provide an optional hook for plug-ins to safely
    cleanup global state.
   \param pUnkHost The unknown interface of the ACF host. Use this interface dynamically 
    access host services.
 */
ACFPLUGINAPI ACFShutdown (IACFUnknown * pUnkHost);




#if ACF_MAC && !defined (_MSC_VER)
typedef ACFAPICALLTYPE ACFRESULT (* ACFREGISTERPLUGINTYPE) (IACFUnknown *, IACFPluginDefinition **);
typedef ACFAPICALLTYPE ACFRESULT (* ACFREGISTERCOMPONENTTYPE) (IACFUnknown *, acfUInt32, IACFComponentDefinition **);
typedef ACFAPICALLTYPE ACFRESULT (* ACFGETCLASSFACTORYTYPE) (IACFUnknown *, const acfCLSID&, const acfIID&, void**);
typedef ACFAPICALLTYPE ACFRESULT (* ACFCANUNLOADNOWTYPE) (IACFUnknown *);
typedef ACFAPICALLTYPE ACFRESULT (* ACFSTARTUPTYPE) (IACFUnknown *);
typedef ACFAPICALLTYPE ACFRESULT (* ACFSHUTDOWNTYPE) (IACFUnknown *);
#else


typedef ACFRESULT (ACFAPICALLTYPE * ACFREGISTERPLUGINTYPE) (IACFUnknown *, IACFPluginDefinition **);

typedef ACFRESULT (ACFAPICALLTYPE * ACFREGISTERCOMPONENTTYPE) (IACFUnknown *, acfUInt32, IACFComponentDefinition **);

typedef ACFRESULT (ACFAPICALLTYPE * ACFGETCLASSFACTORYTYPE) (IACFUnknown *, const acfCLSID&, const acfIID&, void**);

typedef ACFRESULT (ACFAPICALLTYPE * ACFCANUNLOADNOWTYPE) (IACFUnknown *);

typedef ACFRESULT (ACFAPICALLTYPE * ACFSTARTUPTYPE) (IACFUnknown *);

typedef ACFRESULT (ACFAPICALLTYPE * ACFSHUTDOWNTYPE) (IACFUnknown *);

#endif




/////////////////////////////////////////////////////////////////////////////////////////////////
//

/*!
	\b IID_IACFClassFactory
	\remarks 
    The interface identifier for IACFClassFactory.  
    \note IID_IACFClassFactory != IID_IClassFactory!	\n <b> type: </b> UID
	\n <b> context: </b> global
	\n <b> ACFNamespace name: </b>
*/
DEFINE_ACFUID(acfIID, IID_IACFClassFactory, 0x80996EEE, 0x7FCF, 0x11D6, 0xAC, 0xA8, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);

/*!
   \interface IACFClassFactory
   \brief Publicly inherits from IACFUnknown.Publicly inherits from IACFUnknown.Provides the abstract interface for component creation.
   \remarks
   	Every component implemenation class must have a corresponding class factory that implements 
   	the IACFClassFactory interface.
   	
   	For general information about class factories see the associated documentation for IClassFactory.
   \note Plug-ins implement this interface to encapsulate component creation [Abstract Factory].
     The host will use instances of this interface returned from the plug-in's ACFGetClassFactory
    callback function to manage the creation of new components.
 */

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif // __clang__

class IACFClassFactory : public IACFUnknown
{
public:

/*!
   \b CreateInstance
   \brief Creates an instance of an uninitialized object of the class associated with this
    class factory.
   \remarks
    This method is generally called only by the host through the methods of the IACFComponentFactory 
    interface: CreateComponent() and CreateInstance(). 
   \param pUnkHost Pointer to the host implementation object's IACFUnknown interface.
   \param pUnkOuter Pointer to object's controlling unknown. If NULL then the object is not 
    being created as part of an aggregate. If non-NULL then pointer is the aggregate's IACFUnknown 
    interface (the controlling IACFUnknown).
   \param iid Identifier for the initial interface to the new object.
   \param ppOut Address of pointer variable that receives the interface pointer corresponding to the 
    given iid.
 */
    virtual ACFRESULT ACFMETHODCALLTYPE CreateInstance (
        IACFUnknown *pUnkHost, 
        IACFUnknown* pUnkOuter, 
        const acfIID& iid, 
        void** ppOut
    ) = 0;
    
};



/////////////////////////////////////////////////////////////////////////////////////////////////
//	
// The rest of basic ACF interfaces are implementated by the host. These interfaces are used by 
// the plug-in to define the plug-in and its components and to create registered components.
//	IACFDefinition
//	IACFPluginDefinition
//	IACFComponentDefinition
//	IACFComponentFactory
//



/*!
	\b IID_IACFDefinition
	\remarks 
	The interface identifier for IACFDefinition.
	\n <b> type: </b> UID
	\n <b> context: </b> global
	\n <b> ACFNamespace name: </b>
*/
DEFINE_ACFUID(acfIID, IID_IACFDefinition, 0xE51741F1, 0x7FCF, 0x11D6, 0xAA, 0xC3, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);

/*!
   \interface IACFDefinition
   \brief Publicly inherits from IACFUnknown.This abstract interface is used to indentify all of the plug-in components in the host.
   \remarks
    This interface is the base class for both plug-in and component definitions. All defined 
    attributes are read only.
    
    \note This interface does not provide any attribute enumeration. You must know the uid of the associated 
    with the attribute that you need to find.
   \note This interface is implemented by the host. 
     The plug-in will use this interface to define optional attributes for both plug-in and
    component implementations classes.
 */

class IACFDefinition : public IACFUnknown
{
public:

/*!
   \b DefineAttribute
   \brief Add a read only attribute to the definition.
   \remarks
    Use the method to define additional global attributes for you component. This
    method will fail if the attribute has already been defined.
    
   \param attributeID Unique identifier for attribute
   \param typeID Indicates the type of the attribute data
   \param attrData Pointer to buffer that contains the attribute data
   \param attrDataSize Size of the attribute buffer
 */
    virtual ACFRESULT ACFMETHODCALLTYPE DefineAttribute (
        const acfUID& attributeID,
        const acfUID& typeID,
        const void *attrData,
        acfUInt32 attrDataSize
    ) = 0;


/*!
   \brief Returns information about the given attribute.
   \remarks
    Use this method to retrieve the type and size of a given attribute.
   \param attributeID Unique identifier for attribute
   \param typeID Indicates the type of the attribute data
   \param attrDataSize Size of the attribute data
 */
    virtual ACFRESULT ACFMETHODCALLTYPE GetAttributeInfo (
        const acfUID& attributeID,
        acfUID * typeID,
        acfUInt32 * attrDataSize
    ) = 0;


/*!
   \b CopyAttribute
   \brief Copy the a given attribute.
   \remarks
    Use this method to access the contents of a given attribute.
   \param attributeID Unique identifier for attribute
   \param typeID Indicates the type of the attribute data
   \param attrData Pointer to buffer to copy the attribute data
   \param attrDataSize Size of the attribute buffer
 */
    virtual ACFRESULT ACFMETHODCALLTYPE CopyAttribute (
        const acfUID& attributeID,
        const acfUID& typeID,
        void *attrData,
        acfUInt32 attrDataSize
    ) = 0;

};



/////////////////////////////////////////////////////////////////////////////////////////////////
//

/*!
	\b IID_IACFPluginDefinition
	\remarks 
	The interface identifier for IACFPluginDefinition.
	\n <b> type: </b> UID
	\n <b> context: </b> global
	\n <b> ACFNamespace name: </b>
*/
DEFINE_ACFUID(acfIID, IID_IACFPluginDefinition, 0x00EEF015, 0x7FD0, 0x11D6, 0x84, 0x85, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);

/*!
   \interface IACFPluginDefinition
   \brief Publicly inherits from IACFDefinition.Provides the abstract interface for defining ACF plug-ins.
   \remarks
    An object that implements this interface must be returned to the host from
    the ACFRegisterPlugin callback. This object is only valid during the 
    registration of the plug-in.
   \note This interface is implemented by the host.
     Plug-in The plug-in will use this interface to register itself
    with the ACF host. \note The components implemented by a plug-in are
    registered separately in the ACFRegisterComponent callback.
 */

class IACFPluginDefinition : public IACFDefinition
{
public:

/*!
   \b InitializePlugin
   \brief Initializes the minimum required attributes for any ACF plug-in.
   \remarks
    This method should be called within the context of the ACFRegisterPlugin callback.
    Use the DefineAttribute method to add optional attributes. 
   \param uid Unique identifier for the plug-in
   \param majorVersion Major version of the plug-in
   \param minorVersion Minor version of the plug-in
   \param name The name of the plug-in (may be used for host console debugging)
   \param vendorID The unique vendor identifier (obtained from Avid with licensed ACF SDK)
   \param vendorName The plug-in's vendor name
   \param componentCount The number of component implementation classes exported from
    this plug-in.
   \param cacheDefinition Indicates whether the plug-in registration information can be cached.
    If kACFTrue then the host can launch faster and only load the plug-in modules if and when
    any contained components are actually used.
 */
    virtual ACFRESULT ACFMETHODCALLTYPE InitializePlugin (
	    const acfUID& uid,
	    acfUInt32 majorVersion, 
	    acfUInt32 minorVersion,
	    const acfWChar* name,
	    const acfUID& vendorID,
	    const acfWChar* vendorName,
	    acfUInt32 componentCount,
	    acfBool cacheDefinition
    ) = 0;

};



/////////////////////////////////////////////////////////////////////////////////////////////////
//

/*!
	\b IID_IACFComponentDefinition
	\remarks 
	The interface identifier for IACFComponentDefinition.
	\n <b> type: </b> UID
	\n <b> context: </b> global
	\n <b> ACFNamespace name: </b>
*/
DEFINE_ACFUID(acfIID, IID_IACFComponentDefinition, 0x1718A226, 0x7FD0, 0x11D6, 0xBD, 0x1D, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);

/*!
   \interface IACFComponentDefinition
   \brief Publicly inherits from IACFDefinition.Provides the abstract interface for defining generic components.
   \remarks 
     This interface is used to define the basic attributes required by a generic
     component. This interface is provided as the basis for plug-ins to define 
     their own sharable components. An object that implements this interface 
     must be returned to the host from the ACFRegisterComponent callback. This 
     object is only valid during the registration of the plug-in.
     
   \note The ACF may use component subclass interface for each component type
     - image effects
     - parameters
     - meta sync
     - etc...
     
   \note This interface is implemented by the host.
     The plug-in will use this interface to define a generic component.
 */

class IACFComponentDefinition : public IACFDefinition
{
public:

/*!
   \b InitializeComponent
   \brief Initializes the minimum required attributes for any ACF component.
   \remarks
    This method should be called within the context of the ACFRegisterComponent callback.
    Use the DefineAttribute method to add optional attributes. 
   \param componentID Unique identifier for the component
   \param componentTypeID Indicates the type of the component (utility) being defined \ref ComponentType
   \param majorVersion Major version of the component
   \param minorVersion Minor version of the component
   \param clsid The unique component implementation class identifier
   \param name Name of the component (may be used for host console debugging)
 */
    virtual ACFRESULT ACFMETHODCALLTYPE InitializeComponent (
	    const acfUID& componentID,
	    const acfUID& componentTypeID,
	    acfUInt32 majorVersion, 
	    acfUInt32 minorVersion,
	    const acfCLSID& clsid,
	    const acfWChar* name
    ) = 0;

};



/////////////////////////////////////////////////////////////////////////////////////////////////
//

/*!
	\b IID_IACFComponentFactory
	\remarks 
	The interface identifier for IACFEnumDefinitions.
	\n <b> type: </b> UID
	\n <b> context: </b> global
	\n <b> ACFNamespace name: </b>
*/
DEFINE_ACFUID(acfIID, IID_IACFEnumDefinitions, 0xC34189E3, 0x8398, 0x11D6, 0x84, 0x7E, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);

/*!
   \interface IACFEnumDefinitions
   \brief Provides the abstract interface for enumerating definitions.
   \remarks 
    \note This "Enumeration" interface is different from "standard" COM for several reasons:
    -# Enumerations of related interfaces are expected to support the same type of definitions 
    interface.
    -# There no "partial success" in ACF. If the caller asks for the Next 10 definitions
    and there are only 9 Next method will FAIL. If all of the requested definitions do 
    not support the given iid then the Next method will FAIL. If the caller asks for 0 definitions
    then the Next method will FAIL. (RESULT codes:TBD)
    -# Unlike the "standard" COM enumeration interface this interface could actually be remoted 
    without an a special "RemoteAs" method in a proxy dll. (Of course we have no plans to ever
    remote ACF interfaces...)
   \note This interface is implemented by the host.
     A plug-in will use this interface to access of related definitions.
   \n \b Example: Enumerating all definitions of a given type
   \code
ACFRESULT EnumerateDefinitions(IACFComponentFactory *pComponentFactory)
{
    if (!pComponentFactory)
        return ACF_E_POINTER;
        
    BEGIN_ACF_METHOD
    
    ACFSmartPtr<IACFEnumDefinitions> pEnumDefinitions;
    acfcheck( pComponentFactory->EnumDefinitions( ACFCompType_ImageEffect, &pEnumDefinitions) );
    
    //
    // Access definitions one at a time:
    //
    ACFSmartPtr<IACFComponentDefinition> pComponentDef;
    while (ACFSUCCEEDED(pEnumDefinitions->Next(1, IID_IACFComponentDefinition, (IACFDefinition **)pComponentDef) )
    {
        // Process the definition...
    }
    
    acfcheck( pEnumDefinitions->Reset() );

    //
    // Slurp all of the definitions up in one call to Next.
    //   
    avxUInt32 count = pEnumDefinitions->Count();
    if (0 < count)
    {
        IACFComponentDefinition ** ppComponentDefinitions = new (IACFComponentDefinition*)[count];
        if (!ppComponentDefinitions)
            return ACF_E_OUTOFMEMORY;
        
        if (ACFSUCCEEDED(pEnumDefinitions->Next(count, IID_IACFComponentDefinition, (IACFDefinition **)ppComponentDefinitions) )
        {
            //
            // Process all of the definitions...
            //
            
            
            // Release all of the returned interfaces...
            for (acfUInt32 i = 0; i < count; ++i)
            {
                ppComponentDefinitions[i]->Release();
                ppComponentDefinitions[i] = 0;
            }    
        }
        
        // cleanup
        delete [] ppComponentDefinitions;
    }
           
    END_ACF_METHOD
}
\endcode
 */

class IACFEnumDefinitions : public IACFUnknown
{
public:

/*!
   \b Count
   \brief Return the number of definitions in the enumeration.
   \remarks
 */
    virtual acfUInt32 ACFMETHODCALLTYPE Count (
        void
    ) = 0;

/*!
   \b Next
   \brief Return the next count definitions from the enumeration.
   \remarks
    Use this method to retreive a given number of definitions all with the 
    initial interface given by iid. The caller is responsible for releasing
    the returned intefaces.
   \param count number of definitions to retrieve into x.
   \param iid Identifier for the initial interface to the definitions.
   \param x Address of array variable that receives the interface pointer(s) corresponding to the 
    given iid.
 */
    virtual ACFRESULT ACFMETHODCALLTYPE Next (
        acfUInt32 count,
        const acfIID& iid, 
        IACFDefinition** x
    ) = 0;

/*!
   \b Reset
   \brief Reset to the beginning of the enumeration.
   \remarks
 */
    virtual ACFRESULT ACFMETHODCALLTYPE Reset (
        void
    ) = 0;

/*!
   \b Clone
   \brief Clone to the current state of the enumeration.
   \remarks
   \param ppEnum Address of IACFEnumDefinitions interface pointer variable that receives the
    cloned enumeration.
 */
    virtual ACFRESULT ACFMETHODCALLTYPE Clone (
        IACFEnumDefinitions **ppEnum
    ) = 0;

};



/////////////////////////////////////////////////////////////////////////////////////////////////
//

/*!
	\b IID_IACFComponentFactory
	\remarks 
	The interface identifier for IACFComponentFactory.
	\n <b> type: </b> UID
	\n <b> context: </b> global
	\n <b> ACFNamespace name: </b>
*/
DEFINE_ACFUID(acfIID, IID_IACFComponentFactory, 0x382B6A96, 0x7FD0, 0x11D6, 0xBC, 0xFE, 0x00, 0x30, 0x65, 0x42, 0xA0, 0x24);

/*!
   \interface IACFComponentFactory
   \brief Publicly inherits from IACFUnknown.Provides the abstract interface for object creation.
   \remarks 
    This is one of the services provided by the host implementation object.  The IACFUnknown 
    interface to the host is passed into every plug-in callback and every component 
    implementation class through the IACFClassFactory::CreateInstance 
    method. Use QueryInterface to acquire a reference to the host's component factory
    interface. 
   \note This interface is implemented by the host.
     The plug-in will use this interface to ask the host to create components 
    or specific component implemenation classes that have been defined with the host.
 */

class IACFComponentFactory : public IACFUnknown
{
public:

/*!
   \b CreateComponent
   \brief Creates an instance of an uninitialized component.
   \remarks
    The CreateComponent method is the general component creation method used by the host 
    and all plug-ins. If more than a single class has been registered for the given component
    this method will automatically create the latest acceptable version.
    
    If you don't want the host to choose the class to create for a given component and you
    know the specific implementation class id then use the CreateInstance method instead.
   \param compid Identifier of the Component
   \param pUnkOuter Pointer to object's controlling unknown. If NULL then the object is not 
    being created as part of an aggregate. If non-NULL then pointer is the aggregate's IACFUnknown 
    interface (the controlling IACFUnknown).
   \param iid Identifier for the initial interface to the new object.
   \param x Address of pointer variable that receives the interface pointer corresponding to the 
    given iid.
 */
    virtual ACFRESULT ACFMETHODCALLTYPE CreateComponent (
        const acfUID& compid,
        IACFUnknown* pUnkOuter, 
        const acfIID& iid, 
        void** x
    ) = 0;


/*!
   \b CreateInstance
   \brief Creates an instance of an uninitialized component.
   \remarks
    The Createinstance method is the general object creation method used by the host 
    and all plug-ins. Only use this method you really need to create a particular component 
    implementation class. For example, you might want to create your new version of an existing 
    component by creating and reusing part of a previous version (\note for the two versions to 
    coexist in the same host they should be in the same plug-in, if they are in different plug-ins 
    the the plug-in ids must be different).
   \param clsid Class identifier of the component implementation object.
   \param pUnkOuter Pointer to object's controlling unknown. If NULL then the object is not 
    being created as part of an aggregate. If non-NULL then pointer is the aggregate's IACFUnknown 
    interface (the controlling IACFUnknown).
   \param iid Identifier for the initial interface to the new object.
   \param x Address of pointer variable that receives the interface pointer corresponding to the 
    given iid.
 */
    virtual ACFRESULT ACFMETHODCALLTYPE CreateInstance (
        const acfCLSID& clsid, 
        IACFUnknown* pUnkOuter, 
        const acfIID& iid, 
        void** x
    ) = 0;


/*!
   \b GetClassFactory
   \brief Returns the class factory for a given component class implementation.
   \remarks
    The GetClassFactory method is the low-level method to return a class factory for 
    a particular component implementation. Use this method if you know the implementation
    class and you need create many instances of the same component. The overhead of the
    looking up the factorty (and loading the dll if necessary) is only performed once. Then
    call the factory's CreateInstance() method to create each component instance.
    
    \note The caller is responsible for initializing each component instance created
    from the class factory.
   \param clsid Class identifier of the component implementation object.
   \param iid Identifier for the initial interface to the new class factory object.
   \param x Address of pointer variable that receives the class factory interface pointer 
    corresponding to the given iid.
 */
    virtual ACFRESULT ACFMETHODCALLTYPE GetClassFactory (
        const acfCLSID& clsid, 
        const acfIID& iid, 
        void** x
    ) = 0;

/*!
   \b FindDefinition
   \brief Allows the plug-in to lookup a particular definition that has been registered with
    the host.
   \remarks
    Use the method to find a existing definition that has been registered with the host. 
   \param uid Unique identifier for the definition
   \param iid The interface identifier for the type of definition interface we want.
   \param ppDefinition
 */
    virtual ACFRESULT ACFMETHODCALLTYPE FindDefinition (
        const acfUID& uid,
        const acfIID& iid,
        IACFDefinition **ppDefinition
    ) = 0;


/*!
   \b EnumDefinitions
   \brief Allows enumeration of all of the registered definitions of a particular type.
   \remarks
    This method provides a mechanism of managing groups of plug-in components. This can be 
    used to create an "effects manager", "codec manager", "file translator manager", etc...
   \param componentTypeID Unique identifier for the definition \ref ComponentType
   \param ppEnum
 */
    virtual ACFRESULT ACFMETHODCALLTYPE EnumDefinitions (
        const acfUID& componentTypeID,
        IACFEnumDefinitions **ppEnum
    ) = 0;

};

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

#endif // acfbaseapi_h
