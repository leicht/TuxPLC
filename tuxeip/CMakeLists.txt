cmake_minimum_required(VERSION 2.6)
		
#Déclaration du projet
project(TuxEIP)
set(LIBRARY_OUTPUT_PATH ./lib)
SET( CMAKE_C_FLAGS "-fPIC")

ADD_LIBRARY(tuxeip STATIC 
            src/AB.c
            src/CIP_IOI.c
            src/CM.c
            src/ErrCodes.c
            src/Ethernet_IP.c
            src/LGX.c
            src/MR.c
            src/PLC.c
            src/SendData.c)
            
