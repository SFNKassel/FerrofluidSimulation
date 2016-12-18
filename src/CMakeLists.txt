FILE (GLOB_RECURSE project_SRCS *.cpp *.cxx *.cc *.C *.c *.h *.hpp)
SET (project_LIBS )
SET (project_BIN ${PROJECT_NAME})

ADD_EXECUTABLE(${project_BIN} ${project_SRCS})
TARGET_LINK_LIBRARIES(${project_BIN} ${project_LIBS})

SET_TARGET_PROPERTIES(${project_BIN} PROPERTIES VERSION "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}" OUTPUT_NAME ${project_BIN} CLEAN_DIRECT_OUTPUT 1)
if (CMAKE_COMPILER_IS_GNUCXX)
  	message(status "Setting GCC flags")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -pedantic")
elseif(WIN32)
	message(status "Setting ICC")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} /O3 /Ob2 /Oi /Ot /GT /GA /Qopt-matmul /Qparallel /Quse-intel-optimized-headers /Gy /arch:AVX2 /fp:fast=2 /Qstd=c++11")
endif()

set_property(TARGET ${project_BIN} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${project_BIN} PROPERTY CXX_STANDARD_REQUIRED ON)

INSTALL(TARGETS ${project_BIN} DESTINATION bin)
