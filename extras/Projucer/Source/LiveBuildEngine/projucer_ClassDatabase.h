/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

struct ClassDatabase
{
    //==============================================================================
    struct MemberInfo
    {
        enum CodeLocationType
        {
            declaration = 0,
            addedToParent,
            setBoundsParamX,
            setBoundsParamY,
            setBoundsParamW,
            setBoundsParamH,

            // WARNING! When you change any of these, also update the copy that lives in the live editing code

            numCodeLocationTypes
        };

        MemberInfo() {}

        MemberInfo (const MemberInfo& other)
            : name (other.name), type (other.type)
        {
            for (int i = 0; i < numCodeLocationTypes; ++i)
                locations[i] = other.locations[i];
        }

        MemberInfo (const String& nm, const String& ty)
            : name (nm), type (ty)
        {
        }

        MemberInfo (const ValueTree& v)
            : name (v [Ids::name].toString()),
              type (v [Ids::class_].toString())
        {
            for (int i = 0; i < numCodeLocationTypes; ++i)
                locations[i] = v [getIdentifierForCodeLocationType (i)].toString();
        }

        const String& getName() const   { return name; }
        const String& getType() const   { return type; }

        const SourceCodeRange& getLocation (CodeLocationType t) const
        {
            return locations[t];
        }

        void setLocation (CodeLocationType t, const SourceCodeRange& range)
        {
            locations[t] = range;
        }

        void mergeWith (const MemberInfo& other)
        {
            jassert (name == other.name);
            if (other.type.isNotEmpty())
                type = other.type;

            for (int i = 0; i < numCodeLocationTypes; ++i)
                if (other.locations[i].isValid())
                    locations[i] = other.locations[i];
        }

        void nudgeAllCodeRanges (const String& file, const int insertPoint, const int delta)
        {
            for (int i = 0; i < numCodeLocationTypes; ++i)
                locations[i].nudge (file, insertPoint, delta);
        }

        void fileContentChanged (const String& file)
        {
            for (int i = 0; i < numCodeLocationTypes; ++i)
                locations[i].fileContentChanged (file);
        }

        ValueTree toValueTree() const
        {
            ValueTree m (Ids::MEMBER);
            m.setProperty (Ids::name, name, nullptr);
            m.setProperty (Ids::class_, type, nullptr);

            for (int i = 0; i < numCodeLocationTypes; ++i)
                locations[i].writeToValueTree (m, getIdentifierForCodeLocationType (i));

            return m;
        }

    private:
        String name, type;
        SourceCodeRange locations [numCodeLocationTypes];

        static Identifier getIdentifierForCodeLocationType (int typeIndex)
        {
            // (These need to remain in order)
            static_assert (setBoundsParamX + 1 == setBoundsParamY && setBoundsParamY + 1 == setBoundsParamW
                            && setBoundsParamW + 1 == setBoundsParamH, "");

            static const Identifier ids[] =
            {
                "declaration",
                "addedToParent",
                "setBoundsParamX",
                "setBoundsParamY",
                "setBoundsParamW",
                "setBoundsParamH"
            };

            return ids [typeIndex];
        }
    };

    //==============================================================================
    struct MethodInfo
    {
        MethodInfo() {}

        MethodInfo (const MethodInfo& other)
            : name (other.name), returnType (other.returnType),
              declaration (other.declaration), definition (other.definition),
              numArgs (other.numArgs), flags (other.flags)
        {
        }

        String name, returnType;
        SourceCodeRange declaration, definition;
        int numArgs, flags;

        enum
        {
            isConstructor = 1,
            isDefaultConstructor = 2,
            isTemplated = 4,
            isPublic = 8
        };

        MethodInfo (const ValueTree& v)
            : name (v[Ids::name].toString()),
              returnType (v[Ids::returnType].toString()),
              declaration (v[Ids::declaration].toString()),
              definition (v[Ids::definition].toString()),
              numArgs (v[Ids::numArgs]),
              flags (v[Ids::flags])
        {
        }

        ValueTree toValueTree() const
        {
            ValueTree m (Ids::METHOD);
            m.setProperty (Ids::name, name, nullptr);
            m.setProperty (Ids::returnType, returnType, nullptr);
            m.setProperty (Ids::numArgs, numArgs, nullptr);
            m.setProperty (Ids::flags, flags, nullptr);
            declaration.writeToValueTree (m, Ids::declaration);
            definition.writeToValueTree (m, Ids::definition);
            return m;
        }

        void nudgeAllCodeRanges (const String& file, const int insertPoint, const int delta)
        {
            declaration.nudge (file, insertPoint, delta);
            definition.nudge (file, insertPoint, delta);
        }

        void fileContentChanged (const String& file)
        {
            declaration.fileContentChanged (file);
            definition.fileContentChanged (file);
        }
    };

    //==============================================================================
    struct InstantiationFlags
    {
        InstantiationFlags()
            : isAbstract (false),
              inAnonymousNamespace (false),
              noDefaultConstructor (false)
        {}

        InstantiationFlags (const InstantiationFlags& other)
            : isAbstract (other.isAbstract),
              inAnonymousNamespace (other.inAnonymousNamespace),
              noDefaultConstructor (other.noDefaultConstructor)
        {}

        bool canBeInstantiated() const noexcept
        {
            return ! (isAbstract || inAnonymousNamespace || noDefaultConstructor);
        }

        String getReasonForUnavailability() const
        {
            if (isAbstract)             return "This class is abstract";
            if (noDefaultConstructor)   return "This class has no default constructor";
            if (inAnonymousNamespace)   return "This class is declared inside an anonymous namespace";
            return String();
        }

        bool isDisallowed (const InstantiationFlags& disallowedFlags) const
        {
            return ! ((disallowedFlags.isAbstract && isAbstract)
                      || (disallowedFlags.inAnonymousNamespace && inAnonymousNamespace)
                      || (disallowedFlags.noDefaultConstructor && noDefaultConstructor));
        }

        bool isAbstract;
        bool inAnonymousNamespace;
        bool noDefaultConstructor;
    };

    //==============================================================================
    struct Class
    {
        Class() {}
        ~Class() {}

        Class (const Class& other)
            : className (other.className), members (other.members),
              methods (other.methods), classDeclaration (other.classDeclaration),
              instantiationFlags (other.instantiationFlags)
        {
        }

        Class (const String& name, const InstantiationFlags& flags,
               const Array<MemberInfo>& m,
               const Array<MethodInfo>& meth,
               const SourceCodeRange& classDeclarationRange)
            : className (name),
              members (m), methods (meth),
              classDeclaration (classDeclarationRange),
              instantiationFlags (flags)
        {
        }

        Class& operator= (const Class& other)
        {
            className = other.className;
            members = other.members;
            methods = other.methods;
            classDeclaration = other.classDeclaration;
            instantiationFlags = other.instantiationFlags;
            return *this;
        }

        const String& getName() const noexcept          { return className; }

        const InstantiationFlags& getInstantiationFlags() const
        {
            return instantiationFlags;
        }

        void setInstantiationFlags (const InstantiationFlags& newFlags)
        {
            instantiationFlags = newFlags;
        }

        const SourceCodeRange& getClassDeclarationRange() const
        {
            return classDeclaration;
        }

        MemberInfo* findMember (const String& memberName) const
        {
            for (MemberInfo& m : members)
                if (m.getName() == memberName)
                    return &m;

            return nullptr;
        }

        const MethodInfo* getDefaultConstructor() const
        {
            for (const MethodInfo& m : methods)
                if ((m.flags & MethodInfo::isDefaultConstructor) != 0)
                    return &m;

            return nullptr;
        }

        const MethodInfo* getConstructor() const
        {
            if (const MethodInfo* m = getDefaultConstructor())
                return m;

            for (const MethodInfo& m : methods)
                if ((m.flags & MethodInfo::isConstructor) != 0)
                    return &m;

            return nullptr;
        }

        const MethodInfo* getResizedMethod() const
        {
            for (const MethodInfo& m : methods)
                if (m.name == "resized" && m.numArgs == 0)
                    return &m;

            return nullptr;
        }

        File getMainSourceFile() const
        {
            if (const MethodInfo* m = getResizedMethod())
                if (m->definition.isValid())
                    return m->definition.file;

            if (const MethodInfo* m = getConstructor())
                if (m->definition.isValid())
                    return m->definition.file;

            for (MethodInfo& m : methods)
                if (m.definition.isValid() && File (m.definition.file).hasFileExtension ("cpp;mm"))
                    return m.definition.file;

            for (MethodInfo& m : methods)
                if ((m.flags & MethodInfo::isConstructor) != 0 && m.definition.isValid())
                    return m.definition.file;

            for (MethodInfo& m : methods)
                if (m.definition.isValid() && File (m.definition.file).exists())
                    return m.definition.file;

            return File();
        }

        Array<File> getAllSourceFiles() const
        {
            Array<File> files;

            for (const MethodInfo& m : methods)
            {
                files.addIfNotAlreadyThere (m.declaration.file);
                files.addIfNotAlreadyThere (m.definition.file);
            }

            return files;
        }

        bool isDeclaredInFile (const File& file) const
        {
            return file == classDeclaration.file;
        }

        void mergeWith (const Class& other)
        {
            jassert (*this == other);

            if (other.classDeclaration.isValid())
                classDeclaration = other.classDeclaration;

            for (const MemberInfo& m : other.members)
            {
                if (MemberInfo* existing = findMember (m.getName()))
                    existing->mergeWith (m);
                else
                    members.add (m);
            }
        }

        void nudgeAllCodeRanges (const String& file, int index, int delta)
        {
            for (MemberInfo& m : members)   m.nudgeAllCodeRanges (file, index, delta);
            for (MethodInfo& m : methods)   m.nudgeAllCodeRanges (file, index, delta);

            classDeclaration.nudge (file, index, delta);
        }

        void fileContentChanged (const String& file)
        {
            for (MemberInfo& m : members)   m.fileContentChanged (file);
            for (MethodInfo& m : methods)   m.fileContentChanged (file);

            classDeclaration.fileContentChanged (file);
        }

        Class (const ValueTree& v)
        {
            className = v[Ids::name];
            instantiationFlags.isAbstract = v[Ids::abstract];
            instantiationFlags.inAnonymousNamespace = v[Ids::anonymous];
            instantiationFlags.noDefaultConstructor = v[Ids::noDefConstructor];

            classDeclaration = v [Ids::classDecl].toString();

            for (int i = 0; i < v.getNumChildren(); ++i)
                members.add (MemberInfo (v.getChild(i)));
        }

        ValueTree toValueTree() const
        {
            ValueTree v (Ids::CLASS);
            v.setProperty (Ids::name, className, nullptr);
            v.setProperty (Ids::abstract, instantiationFlags.isAbstract, nullptr);
            v.setProperty (Ids::anonymous, instantiationFlags.inAnonymousNamespace, nullptr);
            v.setProperty (Ids::noDefConstructor, instantiationFlags.noDefaultConstructor, nullptr);
            classDeclaration.writeToValueTree (v, Ids::classDecl);

            for (const MemberInfo& m : members)
                v.addChild (m.toValueTree(), -1, nullptr);

            return v;
        }

        bool operator== (const Class& other) const noexcept  { return className == other.className; }
        bool operator!= (const Class& other) const noexcept  { return ! operator== (other); }
        bool operator<  (const Class& other) const noexcept  { return className < other.className; }

        const Array<MemberInfo>& getMembers() const     { return members; }

    private:
        String className;
        Array<MemberInfo> members;
        Array<MethodInfo> methods;
        SourceCodeRange classDeclaration;
        InstantiationFlags instantiationFlags;

        JUCE_LEAK_DETECTOR (Class)
    };

    //==============================================================================
    struct Namespace
    {
        Namespace()  : name ("Global Namespace") {}
        Namespace (const String& n, const String& full)  : name (n), fullName (full) {}

        bool isEmpty() const noexcept
        {
            for (const auto& n : namespaces)
                if (! n.isEmpty())
                    return false;

            return components.size() == 0;
        }

        int getTotalClassesAndNamespaces() const
        {
            int total = components.size();

            for (const auto& n : namespaces)
                total += n.getTotalClassesAndNamespaces();

            return total;
        }

        void add (const Class& c, const String::CharPointerType& localName)
        {
            const String::CharPointerType nextDoubleColon (CharacterFunctions::find (localName, CharPointer_ASCII ("::")));

            if (nextDoubleColon.isEmpty())
                merge (c);
            else
                getOrCreateNamespace (String (localName, nextDoubleColon))->add (c, nextDoubleColon + 2);
        }

        bool containsRecursively (const Class& c) const
        {
            if (components.contains (c))
                return true;

            for (const auto& n : namespaces)
                if (n.containsRecursively (c))
                    return true;

            return false;
        }

        Class* findClass (const String& className) const
        {
            for (Class& c : components)
                if (c.getName() == className)
                    return &c;

            for (const auto& n : namespaces)
                if (Class* c = n.findClass (className))
                    return c;

            return nullptr;
        }

        const MemberInfo* findClassMemberInfo (const String& className, const String& memberName) const
        {
            if (const Class* classInfo = findClass (className))
                return classInfo->findMember (memberName);

            return nullptr;
        }

        void findClassesDeclaredInFile (Array<Class*>& results, const File& file) const
        {
            for (Class& c : components)
                if (c.isDeclaredInFile (file))
                    results.add (&c);

            for (const auto& n : namespaces)
                n.findClassesDeclaredInFile (results, file);
        }

        void merge (const Namespace& other)
        {
            if (components.size() == 0)
            {
                components = other.components;
            }
            else
            {
                for (const auto& c : other.components)
                    merge (c);
            }

            for (const auto& n : other.namespaces)
                getOrCreateNamespace (n.name)->merge (n);
        }

        void merge (const Class& c)
        {
            const int existing = components.indexOf (c);

            if (existing < 0)
                components.add (c);
            else
                components.getReference (existing).mergeWith (c);
        }

        Namespace* findNamespace (const String& targetName) const
        {
            for (auto& n : namespaces)
                if (n.name == targetName)
                    return &n;

            return nullptr;
        }

        Namespace* createNamespace (const String& newName)
        {
            namespaces.add (Namespace (newName, fullName + "::" + newName));
            return findNamespace (newName);
        }

        Namespace* getOrCreateNamespace (const String& newName)
        {
            if (Namespace* existing = findNamespace (newName))
                return existing;

            return createNamespace (newName);
        }

        void addInstantiableClasses (SortedSet<Class>& classes) const
        {
            for (const auto& c : components)
                if (c.getInstantiationFlags().canBeInstantiated())
                    classes.add (c);

            for (const auto& n : namespaces)
                n.addInstantiableClasses (classes);
        }

        void swapWith (Namespace& other) noexcept
        {
            name.swapWith (other.name);
            components.swapWith (other.components);
            namespaces.swapWith (other.namespaces);
        }

        void nudgeAllCodeRanges (const String& file, int index, int delta) const
        {
            for (auto& c : components)  c.nudgeAllCodeRanges (file, index, delta);
            for (auto& n : namespaces)  n.nudgeAllCodeRanges (file, index, delta);
        }

        void fileContentChanged (const String& file) const
        {
            for (auto& c : components)  c.fileContentChanged (file);
            for (auto& n : namespaces)  n.fileContentChanged (file);
        }

        bool matches (const Namespace& other) const
        {
            if (name == other.name
                 && components == other.components
                 && namespaces.size() == other.namespaces.size())
            {
                for (int i = namespaces.size(); --i >= 0;)
                    if (! namespaces.getReference (i).matches (other.namespaces.getReference(i)))
                        return false;

                return true;
            }

            return false;
        }

        void getAllClassNames (StringArray& results, const InstantiationFlags& disallowedFlags) const
        {
            for (const auto& c : components)
                if (c.getInstantiationFlags().isDisallowed (disallowedFlags))
                    results.add (c.getName());

            for (const auto& n : namespaces)
                n.getAllClassNames (results, disallowedFlags);
        }

        ValueTree toValueTree() const
        {
            ValueTree v (Ids::CLASSLIST);

            v.setProperty (Ids::name, name, nullptr);

            for (const auto& c : components)    v.addChild (c.toValueTree(), -1, nullptr);
            for (const auto& n : namespaces)    v.addChild (n.toValueTree(), -1, nullptr);

            return v;
        }

        void loadFromValueTree (const ValueTree& v)
        {
            name = v[Ids::name];

            for (int i = 0; i < v.getNumChildren(); ++i)
            {
                const ValueTree c (v.getChild(i));

                if (c.hasType (Ids::CLASS))
                    components.add (Class (c));
                else if (c.hasType (Ids::CLASSLIST))
                    createNamespace (c[Ids::name])->loadFromValueTree (c);
            }
        }

        bool operator== (const Namespace& other) const noexcept  { return name == other.name; }
        bool operator!= (const Namespace& other) const noexcept  { return ! operator== (other); }
        bool operator<  (const Namespace& other) const noexcept  { return name < other.name; }

        String name, fullName;
        SortedSet<Class> components;
        SortedSet<Namespace> namespaces;

        JUCE_LEAK_DETECTOR (Namespace)
    };

    struct ClassList
    {
        ClassList() {}

        void clear()
        {
            Namespace newNamespace;
            globalNamespace.swapWith (newNamespace);
        }

        void registerComp (const Class& comp)
        {
            globalNamespace.add (comp, comp.getName().getCharPointer());
        }

        void merge (const ClassList& other)
        {
            globalNamespace.merge (other.globalNamespace);
        }

        void swapWith (ClassList& other) noexcept
        {
            globalNamespace.swapWith (other.globalNamespace);
        }

        //==============================================================================
        ValueTree toValueTree() const
        {
            return globalNamespace.toValueTree();
        }

        static ClassList fromValueTree (const ValueTree& v)
        {
            ClassList l;
            l.globalNamespace.loadFromValueTree (v);
            return l;
        }

        Namespace globalNamespace;

        bool operator== (const ClassList& other) const noexcept  { return globalNamespace.matches (other.globalNamespace); }
        bool operator!= (const ClassList& other) const noexcept  { return ! operator== (other); }

    private:
        JUCE_LEAK_DETECTOR (ClassList)
    };
};
