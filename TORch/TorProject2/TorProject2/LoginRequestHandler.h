#pragma once

#include "IRequestHandler.h"
#include "RequestHandlerFactory.h"

class RequestHandlerFactory;
class LoginManager;

class LoginRequestHandler : public IRequestHandler
{
private:

    // fields
    RequestHandlerFactory* m_handlerFactory;
    LoginManager* m_loginManager;
    SSL* m_clientSocket;

    //method to handle login and signup
    const RequestResult login(const RequestInfo rqstInfo); // this function handles the login
    const RequestResult signup(const RequestInfo rqstInfo); // This function handles the signup.

public:
    
    // c'tor & d'tor
    LoginRequestHandler(RequestHandlerFactory* reqHendleFactory, SSL* clientSslSocket);
    ~LoginRequestHandler();

    // methods
    virtual const bool isRequestRelevant(const RequestInfo inf) const override; // This function checks if the request is relevent.
    virtual const RequestResult handleRequest(const RequestInfo inf) override; // This function handles the request of a user.
};