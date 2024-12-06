function(add_git_submodule SUBMODULE_NAME)
find_package(Git QUIET)
if(GIT_FOUND AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SUBMODULE_NAME}/.git")
	message(STATUS "[${SUBMODULE_NAME}] Submodule init ")
	execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive  -- ./${SUBMODULE_NAME}
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
					RESULT_VARIABLE GIT_SUBMOD_RESULT)
	if(NOT GIT_SUBMOD_RESULT EQUAL "0")
		message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
	endif()
endif()
endfunction()
