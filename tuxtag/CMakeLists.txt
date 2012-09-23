cmake_minimum_required(VERSION 2.6)
		
#Déclaration du projet
project(tuxtag)
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})
set(CMAKE_C_FLAGS "-g -Wall")

#Génération de la liste des fichiers sources
file(
	GLOB_RECURSE
	source_files
	src/*
)

#Déclaration de l'exécutable
add_executable(
	tuxtag
	${source_files}
)

TARGET_LINK_LIBRARIES(tuxtag tuxeip m)
