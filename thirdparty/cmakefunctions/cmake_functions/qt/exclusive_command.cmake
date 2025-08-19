cmake_minimum_required(VERSION 3.17) #CMAKE_CURRENT_FUNCTION_LIST_DIR has been added in 3.17

# Define the function to add the PowerShell script as a post-build command
function(add_exclusive_command_as_postbuild target)
    # Get the directory where the CMakeLists.txt is located (same as PowerShell script)
    get_filename_component(SCRIPT_DIR "${CMAKE_CURRENT_FUNCTION_LIST_DIR}" ABSOLUTE)

    # Define the PowerShell script path
    set(PS1_SCRIPT "${SCRIPT_DIR}/exclusive_command.ps1")

    # Add the post-build command for the target
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${PS1_SCRIPT}" ${ARGN}
        COMMENT "Running PowerShell script after building ${target}..."
    )
endfunction()