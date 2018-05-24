cmake_minimum_required(VERSION 3.10)
project(Serwer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -Wall)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_TERMINATION")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_CLIENT_MOCK")

include_directories(/usr/lib/)
link_libraries(ssl)
link_libraries(crypto)
link_libraries(pthread)
add_executable(Serwer main.cpp
        packet.cpp packet.h
        privkey.cpp privkey.h
        pubkey.cpp pubkey.h
        RNG.cpp RNG.h
        sesskey.cpp sesskey.h
        ConHandler.cpp ConHandler.h
        client.cpp client.h
        Server.cpp Server.h
        Service.cpp Service.h
        ServiceTable.cpp ServiceTable.h
        utils.h ClientMock.h
        CTPL/ctpl_stl.h)
