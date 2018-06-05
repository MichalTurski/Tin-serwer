cmake_minimum_required(VERSION 3.10)
project(Serwer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -Wall)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_TERMINATION")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_CLIENT_MOCK")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_THREAD_POOL")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_MQ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_TIMEOUT")

include_directories(/usr/lib/)
link_libraries(ssl)
link_libraries(crypto)
link_libraries(pthread)
link_libraries(rt)
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
        log.cpp log.h
#        AddQueue.cpp AddQueue.h
#        ReadQueue.cpp ReadQueue.h
        common.h CTPL/ctpl_stl.h ClientMock.h
        Receiver.cpp Receiver.h)
