# CMakeList.txt : CMake project for TorProject2, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (TorProject2

    # entry point
    Source.cpp

    # DB
    sqlite3.c
    sqlite3.h

    IDatabase.h
    SqliteDatabase.cpp
    SqliteDatabase.h

    # core
    Server.cpp
    Server.h
    Communicator.cpp
    Communicator.h
    Helper.cpp
    Helper.h

    # chat
    ChatManager.cpp
    ChatManager.h
    SingleChat.cpp
    SingleChat.h
    GroupChat.cpp
    GroupChat.h

    # login / menu
    IRequestHandler.h
    LoginManager.cpp
    LoginManager.h
    LoginRequestHandler.cpp
    LoginRequestHandler.h
    MenuRequestHandler.cpp
    MenuRequestHandler.h

    # request / response
    RequestHandlerFactory.cpp
    RequestHandlerFactory.h
    JsonRequestPacketDeserializer.cpp
    JsonRequestPacketDeserializer.h
    JsonResponsePacketSerializer.cpp
    JsonResponsePacketSerializer.h

    # users / misc
    LoggedUser.cpp
    LoggedUser.h)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TorProject2 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

target_include_directories(TorProject2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(TorProject2
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/server.crt
    ${CMAKE_CURRENT_BINARY_DIR}/server.crt
    COPYONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/server.key
    ${CMAKE_CURRENT_BINARY_DIR}/server.key
    COPYONLY
)