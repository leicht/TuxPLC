cmake_minimum_required(VERSION 2.6)
		
#Déclaration du projet
project(modbus)
set(LIBRARY_OUTPUT_PATH lib/${CMAKE_BUILD_TYPE})

#Génération de la liste des fichiers sources
file(
	GLOB_RECURSE
	source_files
	src/*
)

add_library(
	modbus
	STATIC
	${source_files}
)
