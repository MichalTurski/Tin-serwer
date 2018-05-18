cmake_minimum_required(VERSION 3.10)
project(Serwer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -Wall)

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
        client.cpp client.h
        Server.cpp Server.h
        utils.h ClientMock.h ConHandler.cpp ConHandler.h)
