cmake_minimum_required(VERSION 2.6)
		
#Déclaration du projet
project(tuxreader)
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

#Déclaration de l'exécutable
add_executable(
	tuxreader
	src/tuxreader.c
)

TARGET_LINK_LIBRARIES(tuxreader tuxeip)
