cmake_minimum_required(VERSION 2.6)
		
#Déclaration du projet
project(tuxsql)
set(EXECUTABLE_OUTPUT_PATH ../bin/${CMAKE_BUILD_TYPE})

#Déclaration de l'exécutable
add_executable(
	tuxdf1
	src/TuxDf1.c
	src/df1.c
)

TARGET_LINK_LIBRARIES(tuxdf1 sutil mysqlclient)
