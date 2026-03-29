#include "LoginRequestHandler.h"
#include "JsonResponsePacketSerializer.h"
#include "JsonRequestPacketDeserializer.h"
#include "IRequestHandler.h"


LoginRequestHandler::LoginRequestHandler(RequestHandlerFactory* reqHendleFactory, SSL* clientSslSocket) : m_handlerFactory(reqHendleFactory), m_loginManager(reqHendleFactory->getLoginManager()), m_clientSocket(clientSslSocket)
{
}


LoginRequestHandler::~LoginRequestHandler()
{
    m_handlerFactory = nullptr;
    m_loginManager = nullptr;
}


/*
This function checks if the request is relevent.
input: the requests info.
output: if the request is relevent.
*/
const bool LoginRequestHandler::isRequestRelevant(const RequestInfo inf) const
{
    return inf.id == SIGNUP_CODE || inf.id == LOGIN_CODE;
}


/*
This function handles the request of a user.
input: the requests info.
output: the result.
*/
const RequestResult LoginRequestHandler::handleRequest(const RequestInfo inf)
{
    RequestResult result;

    result.newHandler = nullptr;
    try
    {
        if (!isRequestRelevant(inf))
        {
            ErrorResponse resp;
            resp.message = "Non Valid Request Id.";
            result.response = JsonResponsePacketSerializer::serializeResponse(resp);
        }
        else if (inf.id == LOGIN_CODE)
        {
            result = this->login(inf);
        }
        else if (inf.id == SIGNUP_CODE)
        {
            result = this->signup(inf);
        }
        return result;
    }
    catch (std::exception& e)
    {
        ErrorResponse resp;
        resp.message = e.what();
        result.response = JsonResponsePacketSerializer::serializeResponse(resp);
    }
    return result;
}


/*
This function handles the login.
input: the requests info.
output: the result.
*/
const RequestResult LoginRequestHandler::login(const RequestInfo rqstInfo)
{
    RequestResult result;
    LoginResponse resp;
    LoginRequest req;
    ErrorResponse err;
    
    result.newHandler = nullptr;

    req = JsonRequestPacketDeserializer::deserializeLoginRequest(rqstInfo.buffer);

    LoggedUser user = LoggedUser(req.username, m_clientSocket);

    if (this->m_loginManager->login(req.username, req.password, user))
    {
        resp.status = 1;
        result.response = JsonResponsePacketSerializer::serializeResponse(resp);
        result.newHandler = this->m_handlerFactory->createMenuRequestHandler(user); // setting new handler
    }
    // login failed
    else
    {
        err.message = "Already Logged In Or Not Signed Up.";
        result.response = JsonResponsePacketSerializer::serializeResponse(err);
    }
    return result;
}


/*
This function handles the signup.
input: the requests info.
output: the result.
*/
const RequestResult LoginRequestHandler::signup(const RequestInfo rqstInfo)
{
    RequestResult result;
    SignupResponse resp;
    SignupRequest req;
    ErrorResponse err;

    result.newHandler = nullptr;

    req = JsonRequestPacketDeserializer::deserializeSignupRequest(rqstInfo.buffer);

    LoggedUser user = LoggedUser(req.username, m_clientSocket);

    if (this->m_loginManager->signup(req.username, req.password, req.email, user))
    {
        resp.status = 1;
        result.response = JsonResponsePacketSerializer::serializeResponse(resp);
        result.newHandler = this->m_handlerFactory->createMenuRequestHandler(user); // setting new handler
    }
    // signup failed
    else
    {
        err.message = "Signup failed";
        result.response = JsonResponsePacketSerializer::serializeResponse(err);
    }
    return result;
}



