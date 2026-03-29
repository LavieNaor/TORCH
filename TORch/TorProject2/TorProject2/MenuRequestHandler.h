#pragma once

#include "IRequestHandler.h"
#include "RequestHandlerFactory.h"

class RequestHandlerFactory;
class ChatManager;

class MenuRequestHandler : public IRequestHandler
{
private:

    // fields
    RequestHandlerFactory* m_handlerFactory;
    ChatManager* m_chatManager;
    LoggedUser m_user;

    //method to handle login and signup
    const RequestResult signout(const RequestInfo rqstInfo); // this function handles the login
    const RequestResult createNewGroupChat(const RequestInfo rqstInfo); // This function handles the signup.
    const RequestResult createNewSingleChat(const RequestInfo rqstInfo); // This function handles the signup.
    const RequestResult loadChat(const RequestInfo rqstInfo); // This function handles the signup.

    const RequestResult sendMessage(const RequestInfo rqstInfo); // This function handles the signup.
    const RequestResult getAllUsers(const RequestInfo rqstInfo); // This function handles the signup.

    const RequestResult addUser(const RequestInfo rqstInfo); // This function handles the signup.
    const RequestResult removeUser(const RequestInfo rqstInfo); // This function handles the signup.

    const RequestResult getUserChats(const RequestInfo rqstInfo); // This function handles the signup.
    
public:

    // c'tor & d'tor
    MenuRequestHandler(RequestHandlerFactory* reqHendleFactory, const LoggedUser user);
    ~MenuRequestHandler();

    // methods
    virtual const bool isRequestRelevant(const RequestInfo inf) const override; // This function checks if the request is relevent.
    virtual const RequestResult handleRequest(const RequestInfo inf) override; // This function handles the request of a user.
};