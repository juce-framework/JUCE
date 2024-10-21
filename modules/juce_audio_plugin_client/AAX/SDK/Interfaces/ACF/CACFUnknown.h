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

	Copyright (c) 2004, 2018, 2024 Avid Technology, Inc. All rights reserved.

************************************************************************/



#ifndef CACFUnknown_h
#define CACFUnknown_h

/*!
	\file CACFUnknown.h
	\brief Class declaration for ACF objects that need to inherit an implementation of
	IACFUnknown super-class that will support aggregation.

  Modified 1998-06-30 by TRR for use by AAF.
  Modified 2001-04-14 by TRR for use by AVX2.
  Renamed 2004-02-27 by TRR for use by ACF.
*/

#include "acfbaseapi.h"
#include "acfresult.h"
#include <new>


/*!
  \brief 32-bit Atomic increment
 */
ACFEXTERN_C acfUInt32 ACFInterlockedIncrement (acfUInt32 & value);

/*!
  \brief 32-bit Atomic decrement
 */
ACFEXTERN_C acfUInt32 ACFInterlockedDecrement (acfUInt32 & value);


//=--------------------------------------------------------------------------=
// ACF_DECLARE_STANDARD_UNKNOWN
//=--------------------------------------------------------------------------=
// Macro to insert standard implementation if delegating IACFUnknown interface.
//
// All objects that are going to inherit from CACFUnknown for their IACFUnknown
// implementation should put this in their class declaration instead of the
// three IACFUnknown methods. This macro is usually embedded within the ACF_DECLARE_CONCRETE
// macro. (See the example for ACF_DECLARE_FACTORY)
//
#define ACF_DECLARE_STANDARD_UNKNOWN() \
  ACFMETHOD(QueryInterface)(const acfIID & riid, void **ppvObjOut) ACF_OVERRIDE \
  { \
    return ExternalQueryInterface(riid, ppvObjOut); \
  } \
  ACFMETHOD_(acfUInt32, AddRef)(void) ACF_OVERRIDE \
  { \
    return ExternalAddRef(); \
  } \
  ACFMETHOD_(acfUInt32, Release)(void) ACF_OVERRIDE \
  { \
    return ExternalRelease(); \
  } 


//=--------------------------------------------------------------------------=
// ACF_DECLARE_SINGLETON_UNKNOWN
//=--------------------------------------------------------------------------=
//  Macro to insert a singleton, non-aggregating implementation IACFUnknown
//  interface.
//
// All objects that are going to inherit from CACFUnknown for their IACFUnknown
// implementation should put this in their class declaration instead of the
// three IACFUnknown methods. This macro is usually embedded within the ACF_DECLARE_SINGLETON
// macro. (See the example for ACF_DECLARE_FACTORY)
//
//#error change this to use regular reference counting
#define ACF_DECLARE_SINGLETON_UNKNOWN() ACF_DECLARE_STANDARD_UNKNOWN()
/*\
  ACFMETHOD(QueryInterface)(const acfIID & riid, void **ppvObjOut) \
  { \
    return InternalQueryInterface(riid, ppvObjOut); \
  } \
  ACFMETHOD_(acfUInt32, InternalAddRef)(void) \
  { \
    return 1; \
  } \
  ACFMETHOD_(acfUInt32, InternalRelease)(void) \
  { \
    return 1; \
  } \
  ACFMETHOD_(acfUInt32, AddRef)(void) \
  { \
    return 1; \
  } \
  ACFMETHOD_(acfUInt32, Release)(void) \
  { \
    return 1; \
  }*/


//=--------------------------------------------------------------------------=
// ACF_DECLARE_COM_QUERYINTERFACE
//=--------------------------------------------------------------------------=
// Macro to insert when COM interface needs to be implemented by an ACF object
// deriving from CACFUnknown class.
//
// This macro is used as a bridging technology between ACF and COM while
// porting COM interfaces to ACF.
// Once completely ported, this macro can be simply removed from the class.
//
#define ACF_DECLARE_COM_QUERYINTERFACE() \
  ACFMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut) \
  { \
    return InternalQueryInterface( reinterpret_cast<const acfIID &>(riid), ppvObjOut); \
  } \


//=--------------------------------------------------------------------------=
// ACF_DECLARE_FACTORY
//=--------------------------------------------------------------------------=
//  Macro to declare the interface for object class factory.
//
//  All COM objects will need to use this macro within their class declaration
// so that DllGetClassObject can correctly instantiate an appropriate 
// ACFClassFactory that will use the defined factory method to implement
// the IClassFactory interface. This macro is usually embedded within the 
// ACF_DECLARE_CONCRETE macro.
//
// The Following example will declare the factory method of Foo
// module CFoo.h
// class CFoo : 
//   public IFoo,
//   public CACFUnknown
// {
// public:
//   CFoo(IACFUnknown *pUnkOuter);
//   virtual ~CFoo();
// 
//   // Declare the factory for this class.
//   ACF_DECLARE_FACTORY();
//   
//   // Declare the standard delegating unknown methods
//   ACF_DECLARE_STANDARD_UNKNOWN();
//   
//   // IFoo methods
//   //...
// };
//
// 
// // will generate the following method declarations 
// class CFoo : 
//   public IFoo,
//   public CACFUnknown
// {
// public:
//   CFoo(IACFUnknown *pUnkOuter);
//   virtual ~CFoo();
// 
//   // Declare the factory for this class.
//   static ACFRESULT ACFCreate(IACFUnknown *pUnkOuter, void **ppvObjOut);
//
//   // Declare the standard delegating unknown methods
//   ACFMETHOD(QueryInterface)(const acfIID & riid, void **ppvObjOut)
//   {
//     return ExternalQueryInterface(riid, ppvObjOut);
//   }
//   ACFMETHOD_(acfUInt32, AddRef)(void)
//   {
//     return ExternalAddRef();
//   }
//   ACFMETHOD_(acfUInt32, Release)(void)
//   {
//     return ExternalRelease();
//   } 
//   
//   // IFoo methods
//   //...
// };
// 
#define XACF_DECLARE_FACTORY(xclass) \
    static ACFRESULT ACFCreate(IACFUnknown *pUnkHost, IACFUnknown *pUnkOuter, xclass **ppvObjOut)

#define ACF_DECLARE_FACTORY() \
    static ACFRESULT ACFCreate(IACFUnknown *pUnkHost, IACFUnknown *pUnkOuter, const acfIID & iid, void **ppvObjOut)


//=--------------------------------------------------------------------------=
// ACF_DEFINE_FACTORY
//=--------------------------------------------------------------------------=
//  Macro to define the implementation for object class factory.
//
// Base name of the ACF class implemenation. This should be the expected
// COM implementation calss name. 
//
// All ACF objects can need to use this macro within their class definition
// so that ACFGetClassFactory can correctly instantiate an appropriate 
// ACFClassFactory that will use the defined factory method to implement
// the IACFClassFactory interface. This macro is usually embedded within the 
// ACF_DECLARE_CONCRETE macro.
//
// The Following example will define the factory method of Foo |
// module Foo.cpp
// ACF_DEFINE_FACTORY(CFoo)
// 
// will generate the following method definition
// ACFRESULT CFoo::ACFCreate(IACFUnknown *pUnkHost, IACFUnknown *pUnkOuter, void **ppvObjOut)
// {
//   ACFRESULT result = ACF_OK;
//   *ppvObjOut = NULL;
//   CFoo *pNewObject new CFoo(pUnkOuter);
//   if (NULL == pNewObject)
//     return ACF_E_OUTOFMEMORY;
//   result = pNewObject->InitializeInstance(pUnkHost);
//   if (ACFFAILED(result))
//   {
//     delete pNewObject;
//     return result;
//   }
//   *ppvObjOut = pNewObject;
//   ((IACFUnknown *)(*ppvObjOut))->AddRef();
//   return S_OK;
// }
//
#define XACF_DEFINE_FACTORY(xclass) \
  ACFRESULT xclass::ACFCreate(IACFUnknown * pUnkHost, IACFUnknown *pUnkOuter, xclass **ppvObjOut) \
  { \
    ACFRESULT result = ACF_OK; \
    *ppvObjOut = 0; \
    xclass *pNewObject = new (std::nothrow) xclass(pUnkOuter); \
    if (!pNewObject) \
      return ACF_E_OUTOFMEMORY; \
    result = pNewObject->InitializeInstance(pUnkHost); \
    if (ACFFAILED(result)) \
    { \
      delete pNewObject; \
      return result; \
    } \
    *ppvObjOut = pNewObject; \
    pNewObject->InternalAddRef(); \
    return result; \
  }

#define ACF_DEFINE_FACTORY(xclass) \
ACFRESULT xclass::ACFCreate(IACFUnknown * pUnkHost, IACFUnknown *pUnkOuter, const acfIID & iid, void **ppvObjOut) \
{ \
  ACFRESULT result = ACF_OK; \
  *ppvObjOut = 0; \
  xclass *pNewObject = new (std::nothrow) xclass(pUnkOuter); \
  if (!pNewObject) \
    return ACF_E_OUTOFMEMORY; \
  pNewObject->InternalAddRef(); \
  result = pNewObject->InitializeInstance(pUnkHost); \
  if (ACFFAILED(result)) \
  { \
    delete pNewObject; \
    return result; \
  } \
  result = pNewObject->InternalQueryInterface(iid, ppvObjOut); \
  pNewObject->InternalRelease(); \
  return result; \
}

 
//=--------------------------------------------------------------------------=
// ACF_DECLARE_CONCRETE
//=--------------------------------------------------------------------------=
//  Macro to declare the interface for object class factory.
//
//  All COM objects will need to use this macro within their class declaration
// so that DllGetClassObject can correctly instantiate an appropriate 
// ACFClassFactory that will use the defined factory method to implement
// the IClassFactory interface. Note: this is just a short cut for calling
// ACF_DECLARE_STANDARD_UNKNOWN(); followed by ACF_DECLARE_FACTORY();
//
#define ACF_DECLARE_CONCRETE() \
  ACF_DECLARE_STANDARD_UNKNOWN() \
  ACF_DECLARE_FACTORY() 

// JEB added this
#define XACF_DECLARE_CONCRETE(xclass) \
  ACF_DECLARE_STANDARD_UNKNOWN() \
  XACF_DECLARE_FACTORY(xclass) 

//=--------------------------------------------------------------------------=
// ACF_DEFINE_CONCRETE
//=--------------------------------------------------------------------------=
//  Macro to define the implementation for object class factory.
//
//  All COM objects will need to use this macro within their class definition
// so that DllGetClassObject can correctly instantiate an appropriate 
// ACFClassFactory that will use the defined factory method to implement
// the IACFClassFactory interface.Note: this is just a alias for calling
// ACF_DEFINE_FACTORY(xclass); .
//
#define ACF_DEFINE_CONCRETE(xclass) \
  ACF_DEFINE_FACTORY(xclass) 

#define ACF_DEFINE_XCONCRETE(xclass) \
  ACF_DEFINE_XFACTORY(xclass) 

// JEB added this
#define XACF_DEFINE_CONCRETE(xclass) \
  XACF_DEFINE_FACTORY(xclass) 

 
//=--------------------------------------------------------------------------=
// ACF_DECLARE_SINGLETON
//=--------------------------------------------------------------------------=
//  Macro to declare the interface for object class factory.
//
//  All COM objects will need to use this macro within their class declaration
// so that DllGetClassObject can correctly instantiate an appropriate 
// ACFClassFactory that will use the defined factory method to implement
// the IClassFactory interface. Note: this is just a short cut for calling
// ACF_DECLARE_STANDARD_UNKNOWN(); followed by ACF_DECLARE_FACTORY();
//
#define ACF_DECLARE_SINGLETON() \
  ACF_DECLARE_SINGLETON_UNKNOWN() \
  ACF_DECLARE_FACTORY() 

//=--------------------------------------------------------------------------=
// ACF_DEFINE_SINGLETON
//=--------------------------------------------------------------------------=
//  Macro to define the implementation for object class factory.
//
//  All COM objects will need to use this macro within their class definition
// so that DllGetClassObject can correctly instantiate an appropriate 
// ACFClassFactory that will use the defined factory method to implement
// the IACFClassFactory interface.Note: this is just a alias for calling
// ACF_DEFINE_FACTORY(xclass); .
//
#define ACF_DEFINE_SINGLETON(xclass) \
  ACF_DEFINE_FACTORY(xclass) 


//=--------------------------------------------------------------------------=
// Virtual base class that provides a default implementation if IACFUnknown
// which also allows subclasses to be aggregated.
//
// This class doesn't inherit from IACFUnknown since people inheriting from it
// are going to do so, and just delegate their IACFUnknown calls to the External*
// member functions on this object. The internal private unknown object does
// need to inherit from IACFUnknown, since it will be used directly as an IACFUnknown
// object.
//
class CACFUnknown;

class CACFUnknown
{
public:
  // Default Constructor
  CACFUnknown();

  // Constructor, create with controlling unknown.
  CACFUnknown(IACFUnknown *pUnkOuter);
  
  // Destructor
  virtual ~CACFUnknown();

  // Manage total number of outstanding "live" objects. Used by ACFCanUnloadNow.
  static acfUInt32 GetActiveObjectCount(void);

protected:
  // Override this method initialize contained or aggregated interfaces.
  ACFMETHOD(InitializeInstance)(IACFUnknown *)
  {
    return ACF_OK;
  }
  
  // Override this method release any contained or aggregated interfaces.
  ACFMETHOD_(void, FinalRelease)(void) {}
  
  // Override this method to implement reusable pools of components.
  // The default implementation will just call delete this.
  ACFMETHOD_(void, ReclaimMemory)(void);
  
  // Return the current "controlling" unknown pointer (not reference counted.
  IACFUnknown * GetControllingUnknown(void) const
  { 
    return m_pUnkOuter;
  }
  
  // Delagates IACFUnknown.QueryInterface to controlling unknown.
  ACFRESULT ExternalQueryInterface(const acfIID & riid, void **ppvObjOut)
  {
    return m_pUnkOuter->QueryInterface(riid, ppvObjOut);
  }

  // Delagates IACFUnknown.AddRef to controlling unknown.
  acfUInt32 ExternalAddRef(void)
  {
    return m_pUnkOuter->AddRef();
  }

  // Delagates IACFUnknown.Release to controlling unknown.
  acfUInt32 ExternalRelease(void)
  {
    return m_pUnkOuter->Release();
  }


  // People should use this during creation to return their private
  // unknown
  //
  inline IACFUnknown *GetPrivateUnknown (void)
  {
    return &m_UnkPrivate;
  }

  
  // Called by CACFUnknown::QueryInterface.
  // This method is overridden by all derived classes to check for 
  // specific interface implementations.
  ACFMETHOD(InternalQueryInterface)(const acfIID & riid, void **ppvObjOut);
  
  // Allows access to the non-aggregating IACFUnknown implementation
  // for this class.
  ACFMETHOD_(acfUInt32, InternalAddRef)(void);

  // Allows access to the non-aggregating IACFUnknown implementation
  // for this class.
  ACFMETHOD_(acfUInt32, InternalRelease)(void);

  // Manage total number of outstanding "live" objects. Used by ACFCanUnloadNow.
  static acfUInt32 IncrementActiveObjects(void);
  static acfUInt32 DecrementActiveObjects(void);

private:
  // Copy constructor
  CACFUnknown(const CACFUnknown& );

  // Assignment
  CACFUnknown& operator= (const CACFUnknown&);

  // PrivateUnknown
  // The inner, private unknown implementation is for the aggregator
  // to control the lifetime of this object, and for those cases where
  // this object isn't aggregated.
  //
  class PrivateUnknown : public IACFUnknown
  {
  public:
    virtual ~PrivateUnknown() {}
	  
    inline void SetParentUnknown(CACFUnknown *parent)
    {
      m_This = parent;
    }
    
    // Implements non-delegating IACFUnknown.QueryInterface.
    ACFMETHOD(QueryInterface)(const acfIID & riid, void **ppvObjOut)
    {
      return This()->InternalQueryInterface(riid, ppvObjOut);
    }

    // Implements non-delegating IACFUnknown.AddRef.
    ACFMETHOD_(acfUInt32, AddRef)(void)
    {
      return This()->InternalAddRef();
    }

    // Implements non-delegating IACFUnknown.Release.
    ACFMETHOD_(acfUInt32, Release)(void)
    {
      return This()->InternalRelease();
    }


    private:
      // Return pointer to outer object's this pointer.
      CACFUnknown *This() { return m_This; }
      CACFUnknown *m_This;
  };

  // so they can reference themselves in CACFUnknown from pMainUnknown()
  //
  friend class PrivateUnknown;

  // so the class factory's CreateInstance method can call the private
  // non delegating methods.
  //
  friend class ACFClassFactory;


  // number of live objects.
  static acfUInt32 m_ActiveObjects;

  // Member data:

  // Current reference count for this object.
  acfUInt32 m_cRef;

  // Outer controlling Unknown
  IACFUnknown *m_pUnkOuter;

  // Nested class instance that implements the non-delegating IACFUnknown interface.
  PrivateUnknown m_UnkPrivate;
};

#endif // CACFUnknown_h
