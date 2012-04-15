cmake_minimum_required(VERSION 2.6)
		
#Déclaration du projet
project(tuxsql)
set(EXECUTABLE_OUTPUT_PATH ../../bin/${CMAKE_BUILD_TYPE})

set(CMAKE_EXECUTABLE_SUFFIX .cgi)

#Génération de la liste des fichiers sources
file(
	GLOB_RECURSE
	source_files
	src/*
)

#Déclaration de l'exécutable
add_executable(
	histochart
	${source_files}
)

TARGET_LINK_LIBRARIES(histochart gd cgic sutil mysqlclient)
