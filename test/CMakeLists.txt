FUNCTION(build_test FILE)
	GET_FILENAME_COMPONENT(BASE "${FILE}" NAME_WE)
	GET_FILENAME_COMPONENT(EXT "${FILE}" EXT)


	ADD_EXECUTABLE("${BASE}" ${FILE} main.cpp)

	TARGET_LINK_LIBRARIES("${BASE}" litlua)
	
	SET_TARGET_PROPERTIES("${BASE}" PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")

	ADD_TEST(NAME "${BASE}"
		COMMAND "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/${BASE}"
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/"
	)
ENDFUNCTION()

if(BUILD_TESTING)
	build_test("class.cpp")
	build_test("static_function.cpp")
	build_test("table.cpp")
	build_test("lua_return.cpp")
endif()