# Find glslc compiler
find_program(GLSLC glslc)

if(NOT GLSLC)
    message(FATAL_ERROR "glslc not found. Please make sure Vulkan SDK is installed and in your PATH.")
endif()

message(STATUS "Found glslc: ${GLSLC}")

# Function to precompile shaders and generate headers
function(precompile_shaders)
    # Get all shader files
    file(GLOB SHADER_FILES
        "${CMAKE_SOURCE_DIR}/shaders/*.vert"
        "${CMAKE_SOURCE_DIR}/shaders/*.frag"
        "${CMAKE_SOURCE_DIR}/shaders/*.comp"
        "${CMAKE_SOURCE_DIR}/shaders/*.geom"
        "${CMAKE_SOURCE_DIR}/shaders/*.tesc"
        "${CMAKE_SOURCE_DIR}/shaders/*.tese"
    )

    message(STATUS "Found shader files: ${SHADER_FILES}")

    # Create output directories
    set(SHADER_BINARY_DIR "${CMAKE_BINARY_DIR}/shaders")
    set(SHADER_INCLUDE_DIR "${CMAKE_BINARY_DIR}/include/shaders")
    file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})
    file(MAKE_DIRECTORY ${SHADER_INCLUDE_DIR})

    target_include_directories(game PRIVATE "${CMAKE_BINARY_DIR}/include")

    message(STATUS "Shader output directory: ${SHADER_BINARY_DIR}")
    message(STATUS "Shader include directory: ${SHADER_INCLUDE_DIR}")

    # Iterate through all shader files
    foreach(SHADER ${SHADER_FILES})
        get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
        get_filename_component(SHADER_EXT ${SHADER} EXT)
        string(SUBSTRING ${SHADER_EXT} 1 -1 SHADER_EXT)
        
        set(SPIRV "${SHADER_BINARY_DIR}/${SHADER_NAME}_${SHADER_EXT}.spv")
        set(HEADER "${SHADER_INCLUDE_DIR}/${SHADER_NAME}_${SHADER_EXT}.hpp")
        
        message(STATUS "Processing shader: ${SHADER}")
        message(STATUS "Output will be: ${SPIRV}")
        message(STATUS "Header will be: ${HEADER}")

        # Compile shader
        execute_process(
            COMMAND ${GLSLC} -o ${SPIRV} ${SHADER}
            RESULT_VARIABLE GLSLC_RESULT
        )
        if(NOT GLSLC_RESULT EQUAL 0)
            message(WARNING "Failed to compile shader: ${SHADER}")
        else()
            file(READ ${SPIRV} SPIRV_HEX HEX)
            
            # Convert hex data to C++ array initialization
            string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," SPIRV_FORMAT_HEX ${SPIRV_HEX})
            string(REGEX REPLACE ",$" "" SPIRV_FORMAT_HEX ${SPIRV_FORMAT_HEX})

            # Calculate size of the shader
            string(LENGTH ${SPIRV_FORMAT_HEX} SPIRV_LENGTH)
            math(EXPR SPIRV_SIZE "${SPIRV_LENGTH} / 5 + 1") # 5 characters per byte (0x??,)

            # Create header file content
            set(HEADER_CONTENT "#pragma once\n\n")
            string(APPEND HEADER_CONTENT "#include <array>\n\n")
            string(APPEND HEADER_CONTENT "namespace shaders {\n\n")
            string(APPEND HEADER_CONTENT "    constexpr std::array<unsigned char, ${SPIRV_SIZE}> ${SHADER_NAME}_${SHADER_EXT} = {\n")
            
            # Split the hex data into lines of 20 elements each
            string(REGEX REPLACE "([^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,)" "\\1\n        " SPIRV_FORMAT_HEX ${SPIRV_FORMAT_HEX})
            
            string(APPEND HEADER_CONTENT "        ${SPIRV_FORMAT_HEX}\n")
            string(APPEND HEADER_CONTENT "    }\;\n\n")
            string(APPEND HEADER_CONTENT "} // namespace shaders\n")

            # Write header file
            file(WRITE ${HEADER} ${HEADER_CONTENT})
        endif()

        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
        list(APPEND SPIRV_HEADER_FILES ${HEADER})
    endforeach()

    # Create a custom target that depends on all compiled shaders
    add_custom_target(
        shaders 
        ALL  # This makes the target build by default
        DEPENDS ${SPIRV_BINARY_FILES} ${SPIRV_HEADER_FILES}
    )

    # Add the include directory to the project
    include_directories(${SHADER_INCLUDE_DIR})

    message(STATUS "Created 'shaders' target depending on: ${SPIRV_BINARY_FILES} ${SPIRV_HEADER_FILES}")
endfunction()

# Call the function to precompile shaders
precompile_shaders()

message(STATUS "Shader precompilation script has finished running")

add_dependencies(game shaders)
