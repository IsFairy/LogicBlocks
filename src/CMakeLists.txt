# add logging library target
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/plog" "extern/plog")

add_library(LogicTerm_lib
            ${CMAKE_CURRENT_SOURCE_DIR}/LogicTerm/TermImpl.cpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicTerm/TermImpl.hpp

            ${CMAKE_CURRENT_SOURCE_DIR}/LogicTerm/LogicTerm.cpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicTerm/LogicTerm.hpp

            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicTerm/Logic.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicBlock/Model.hpp

            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicUtil/util_logic.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicUtil/util_logicterm.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/utils/logging.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/utils/csv_util.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/utils/json_util.hpp)

# set include directories
target_include_directories(LogicTerm_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# add logging library
target_link_libraries(LogicTerm_lib PUBLIC plog)

# set required C++ standard and disable compiler specific extensions
target_compile_features(LogicTerm_lib PUBLIC cxx_std_17)
set_target_properties(LogicTerm_lib PROPERTIES CMAKE_CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

# enable interprocedural optimization if it is supported
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
if (ipo_supported)
	set_target_properties(LogicTerm_lib PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

# set compiler flags depending on compiler
# since these are defined `PUBLIC` they are propagated to all linking targets
if (MSVC)
	target_compile_options(LogicTerm_lib PUBLIC /utf-8)
else ()
	target_compile_options(LogicTerm_lib PUBLIC -Wall -Wextra $<$<CONFIG:DEBUG>:-Og>)
	if (BINDINGS)
		# adjust visibility settings for building Python bindings
		target_compile_options(LogicTerm_lib PUBLIC -fvisibility=hidden)
	endif ()
	if (NOT DEPLOY)
		# only include machine-specific optimizations when building for the host machine
		target_compile_options(LogicTerm_lib PUBLIC -mtune=native)
		include(CheckCXXCompilerFlag)
		check_cxx_compiler_flag(-march=native HAS_MARCH_NATIVE)
		if (HAS_MARCH_NATIVE)
			target_compile_options(LogicTerm_lib PUBLIC -march=native)
		endif ()
	endif ()
endif ()

# add coverage compiler and linker flag to the library and all targets that link against it, if COVERAGE is set
if (COVERAGE)
	target_compile_options(LogicTerm_lib PUBLIC --coverage)
	target_link_libraries(LogicTerm_lib PUBLIC --coverage)
endif ()


add_library(SMTLibLogic
            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicBlock/LogicBlock.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicBlock/SMTLibLogicBlock.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/LogicBlock/SMTLibLogicBlock.cpp)
target_link_libraries(SMTLibLogic PUBLIC LogicTerm_lib)
add_library(logicblocks::SMTLibLogic ALIAS SMTLibLogic)

add_library(CNFLogic
            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicBlock/LogicBlock.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicBlock/CNFLogicBlock.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/LogicBlock/CNFLogicBlock.cpp)
target_link_libraries(CNFLogic PUBLIC LogicTerm_lib)
add_library(logicblocks::CNFLogic ALIAS CNFLogic)

if (Z3_FOUND)
	add_library(Z3Logic
	            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicBlock/LogicBlock.hpp
	            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicBlock/Z3Logic.hpp
	            ${CMAKE_CURRENT_SOURCE_DIR}/LogicBlock/Z3Logic.cpp
	            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicBlock/Z3Model.hpp
	            ${CMAKE_CURRENT_SOURCE_DIR}/LogicBlock/Z3Model.cpp)
	target_link_libraries(Z3Logic PUBLIC LogicTerm_lib z3::z3lib)
	add_library(logicblocks::Z3Logic ALIAS Z3Logic)
endif ()

add_library(Encodings
            ${${PROJECT_NAME}_SOURCE_DIR}/include/LogicBlock/LogicBlock.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/Encodings/Encodings.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/Encodings/Encodings.cpp)
target_link_libraries(Encodings PUBLIC LogicTerm_lib)
add_library(logicblocks::Encodings ALIAS Encodings)

add_library(Logic INTERFACE)
target_link_libraries(Logic INTERFACE SMTLibLogic CNFLogic)
if (Z3_FOUND)
	target_link_libraries(Logic INTERFACE Z3Logic)
endif ()
add_library(logicblocks::Logic ALIAS Logic)

if (GENERATE_POSITION_INDEPENDENT_CODE OR BINDINGS)
	include(CheckPIESupported)
	check_pie_supported()
	set_target_properties(LogicTerm_lib PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
	set_target_properties(SMTLibLogic PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
	set_target_properties(CNFLogic PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
	set_target_properties(Encodings PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
	if (Z3_FOUND)
		set_target_properties(Z3Logic PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
	endif ()
endif ()
