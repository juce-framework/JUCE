#filters out the part that needs to compile from the module sources:
function(_juce_clean_sources_list sourceList cleanList moduleName)
    foreach (source IN LISTS sourceList)
        get_filename_component(fileName ${source} NAME)

        if (NOT fileName MATCHES ${moduleName}*)
            list(APPEND localCleanList ${source})
            set_source_files_properties(${source} PROPERTIES HEADER_FILE_ONLY TRUE)
        endif ()
    endforeach ()

    set(${cleanList} "${localCleanList}" PARENT_SCOPE)
endfunction()

#A bit of a hack: Becuase CMake can't 'hold on' to the target properties here
#We're only storing them in a few properties so we can reflect on them later:
function(_juce_add_module_sources moduleName containingPath)
    get_property(ModuleNames GLOBAL PROPERTY juce_module_names)
    list(APPEND ModuleNames ${moduleName})
    set_property(GLOBAL PROPERTY juce_module_names ${ModuleNames})
    set_property(GLOBAL PROPERTY ${moduleName}_containing_path ${containingPath})
    file(GLOB_RECURSE browsable_files "${containingPath}/${moduleName}/*")
    _juce_clean_sources_list("${browsable_files}" clean_list ${moduleName})
    set_property(GLOBAL PROPERTY ${moduleName}_source_files ${clean_list})
    target_sources(${moduleName} INTERFACE ${clean_list})
endfunction()

function(_juce_reflect_module_sources moduleName)
    get_property(sourceFiles GLOBAL PROPERTY ${moduleName}_source_files)
    get_property(path GLOBAL PROPERTY ${moduleName}_containing_path)
    source_group(TREE ${path} PREFIX "JUCE Modules" FILES ${sourceFiles})
    set_source_files_properties(${sourceFiles} PROPERTIES HEADER_FILE_ONLY TRUE)
endfunction()

#Call this when you add a target to update all the sources targets
function(_juce_reflect_sources)
    get_property(ModuleNames GLOBAL PROPERTY juce_module_names)

    foreach(moduleName ${ModuleNames})
        _juce_reflect_module_sources(${moduleName})
    endforeach()
endfunction()