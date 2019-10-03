function(tako_setup target)
    if (DEFINED EMSCRIPTEN)
        set(CMAKE_EXECUTABLE_SUFFIX ".html" PARENT_SCOPE)
        set_target_properties(${target} PROPERTIES LINK_FLAGS "-s USE_GLFW=3 --emrun")
    endif()
endfunction()

function(tako_assets_dir dir)
    #configure_file(${file} ${file} COPYONLY)
    message(${dir})
    if (DEFINED EMSCRIPTEN)
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "--embed-file ${dir}" PARENT_SCOPE)
    endif()

endfunction()