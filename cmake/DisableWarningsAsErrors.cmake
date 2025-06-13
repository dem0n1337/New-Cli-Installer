# Disable warnings as errors for all targets
function(disable_warnings_as_errors_for_all_targets)
    get_property(all_targets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target ${all_targets})
        get_target_property(target_type ${target} TYPE)
        if(NOT target_type STREQUAL "INTERFACE_LIBRARY")
            get_target_property(compile_options ${target} COMPILE_OPTIONS)
            if(compile_options)
                list(REMOVE_ITEM compile_options "-Werror")
                set_target_properties(${target} PROPERTIES COMPILE_OPTIONS "${compile_options}")
            endif()
            
            # Also remove from compile flags
            get_target_property(compile_flags ${target} COMPILE_FLAGS)
            if(compile_flags)
                string(REPLACE "-Werror" "" compile_flags "${compile_flags}")
                set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${compile_flags}")
            endif()
        endif()
    endforeach()
endfunction()

# Function to recursively disable warnings as errors in subdirectories
function(disable_warnings_as_errors_recursive dir)
    get_property(subdirs DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirs})
        disable_warnings_as_errors_recursive(${subdir})
    endforeach()
    
    # Disable for current directory
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error" PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error" PARENT_SCOPE)
endfunction()
