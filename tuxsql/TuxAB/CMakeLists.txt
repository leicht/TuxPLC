cmake_minimum_required(VERSION 2.6)
		
#Déclaration du projet
project(tuxsql)
set(EXECUTABLE_OUTPUT_PATH ../bin/${CMAKE_BUILD_TYPE})

#Déclaration de l'exécutable
add_executable(
	tuxab
	src/TuxAB.c
)


TARGET_LINK_LIBRARIES(tuxab tuxeip sutil mysqlclient)
