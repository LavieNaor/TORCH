#include "MenuRequestHandler.h"
#include "JsonResponsePacketSerializer.h"
#include "JsonRequestPacketDeserializer.h"
#include "Communicator.h"

MenuRequestHandler::MenuRequestHandler(RequestHandlerFactory* reqHendleFactory, const LoggedUser user) : m_handlerFactory(reqHendleFactory), m_chatManager(m_handlerFactory->getChatManager()), m_user(user)
{
}

MenuRequestHandler::~MenuRequestHandler()
{
	m_chatManager = nullptr;
	m_handlerFactory = nullptr;
}

const bool MenuRequestHandler::isRequestRelevant(const RequestInfo inf) const
{
	return inf.id >= SIGNOUT_CODE && inf.id <= GET_USERS_CHATS_CODE;
}

const RequestResult MenuRequestHandler::handleRequest(const RequestInfo inf)
{
    RequestResult result;

    result.newHandler = nullptr;
    try
    {
        // if the request is not relevant
        if (!isRequestRelevant(inf))
        {
            ErrorResponse resp;
            resp.message = "Non Valid Request Id.";
            result.response = JsonResponsePacketSerializer::serializeResponse(resp);
        }
        else if (inf.id == SIGNOUT_CODE)
        {
            result = this->signout(inf);
        }
        else if (inf.id == CREATE_NEW_GROUP_CHAT_CODE)
        {
            result = this->createNewGroupChat(inf);
        }
        else if (inf.id == CREATE_NEW_SINGLE_CHAT_CODE)
        {
            result = this->createNewSingleChat(inf);
        }
        else if (inf.id == LOAD_CHAT_CODE)
        {
            result = this->loadChat(inf);
        }
        else if (inf.id == SEND_MESSAGE_CODE)
        {
            result = this->sendMessage(inf);
        }
        else if (inf.id == GET_ALL_USERS_CODE)
        {
            result = this->getAllUsers(inf);
        }
        else if (inf.id == ADD_USER_CODE)
        {
            result = this->addUser(inf);
        }
        else if (inf.id == REMOVE_USER_CODE)
        {
            result = this->removeUser(inf);
        }
        else if(inf.id == GET_USERS_CHATS_CODE)
        {
            result = this->getUserChats(inf);
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

const RequestResult MenuRequestHandler::signout(const RequestInfo rqstInfo)
{
    RequestResult result;
    LogoutResponse resp;

    // build the response
    resp.status = 1;
    result.newHandler = m_handlerFactory->createLoginRequestHandler(m_user.getSslSocket());

    // logout from the manager
    m_handlerFactory->getLoginManager()->logout(m_user.getUser().username);

    // serialize the response
    result.response = JsonResponsePacketSerializer::serializeResponse(resp);
    result.newHandler = this->m_handlerFactory->createLoginRequestHandler(m_user.getSslSocket()); // go back to login handler

    return result;

}

const RequestResult MenuRequestHandler::createNewGroupChat(const RequestInfo rqstInfo)
{
    // for the sender & recivers
    RequestResult result, resultForUsersToCreateChatWith;
    CreateNewGroupChatResponse resp;
    UpdateCreateNewChatResponse usersToCreateChatWithResponse;
    CreateNewGroupChatRequest req;
    ErrorResponse err;
    ChatData chatData;
    GroupChatData groupChatData;

    std::vector<LoggedUser> loggedUsersToSend;

    // build the response for sender, new hendler nullptr for not change the current hendler (menu hendler)
    result.newHandler = nullptr;

    // deserialize the req
    req = JsonRequestPacketDeserializer::deserializeCreateNewGroupChatRequest(rqstInfo.buffer);

    // add the username of the admin to the members of the chat
    req.users.push_back(m_user.getUser());

    // set the chat type to group
    chatData.chatType = "GROUP";

    // set the group settings
    groupChatData.createdBy = m_user.getUser();
    groupChatData.groupName = req.groupName;
    groupChatData.description = "";
    time(&(chatData.createdAt));
    chatData.createdAt += 2 * 60 * 60;

    // add the group chat to the DB
    chatData.id = m_chatManager->createGroupChat(req.users, chatData, groupChatData);


    // new chat create succeed
    if (chatData.id > 0)
    {
        req.users.erase(req.users.end() - 1); // remove the user who create the chat (last user in the vector) to leave the users who need to know at the vector to check if he logged in

        // get the logged user to send them the create group request
        loggedUsersToSend = m_handlerFactory->getLoginManager()->getLoggedUsersToSend(req.users);

        // TODO: add the msg to DB of the users are not logged in

        usersToCreateChatWithResponse.chatId = chatData.id;
        usersToCreateChatWithResponse.chatName = groupChatData.groupName;
        usersToCreateChatWithResponse.status = 1;

        resultForUsersToCreateChatWith.response = JsonResponsePacketSerializer::serializeResponse(usersToCreateChatWithResponse);
        resultForUsersToCreateChatWith.newHandler = nullptr;

        // run on all group members
        for (auto userToSend = loggedUsersToSend.begin(); userToSend != loggedUsersToSend.end(); userToSend++)
        {
            try
            {
                // send the request to the logged user
                Communicator::send(resultForUsersToCreateChatWith, userToSend->getSslSocket());
            }
            catch (std::exception e) // ------------------------ need to change it to specific exception
            {
                std::cout << "socket had closed" << std::endl;
            }
        }

        // build the response
        resp.status = 1;
        resp.chatId = chatData.id;

        // serialize the response
        result.response = JsonResponsePacketSerializer::serializeResponse(resp);
    }

    // login failed
    else
    {
        err.message = "Chat Id Error.";
        result.response = JsonResponsePacketSerializer::serializeResponse(err);
    }

    return result;
}

const RequestResult MenuRequestHandler::createNewSingleChat(const RequestInfo rqstInfo)
{
    // for the sender & reciver
    RequestResult result, resultForUserToCreateChatWith;
    CreateNewSingleChatResponse resp;
    UpdateCreateNewChatResponse userToCreateChatWithResponse;
    CreateNewSingleChatRequest req;
    ErrorResponse err;
    ChatData chatData;

    std::vector<LoggedUser> loggedUsersToCreateChatWith;

    // build the response for sender, new hendler nullptr for not change the current hendler (menu hendler)
    result.newHandler = nullptr;

    // deserialize the request
    req = JsonRequestPacketDeserializer::deserializeCreateNewSingleChatRequest(rqstInfo.buffer);
    req.users.push_back(m_user.getUser());

    // set the chat type to private
    chatData.chatType = "PRIVATE";

    // set the chat settings
    time(&(chatData.createdAt));
    chatData.createdAt += 2 * 60 * 60;

    chatData.id = m_chatManager->createSingleChat(req.users, chatData);

    // new chat create succeed
    if (chatData.id > 0)
    {
        // build the response
        resp.status = 1;
        resp.chatId = chatData.id;

        req.users.erase(req.users.begin() + 1); // remove the user who create the chat to leave the user who need to know at the vector to check if he logged in

        // get the logged user to send them the create group request
        loggedUsersToCreateChatWith = m_handlerFactory->getLoginManager()->getLoggedUsersToSend(req.users);

        // TODO: add the msg to DB of the users are not logged in

        // check if the user is not the m_user
        if (loggedUsersToCreateChatWith.size() == 1)
        {
            userToCreateChatWithResponse.status = 1;
            userToCreateChatWithResponse.chatId = chatData.id;
            userToCreateChatWithResponse.chatName = m_user.getUser().username; // the username of the user who open the chat

            // serialize the response for other users
            resultForUserToCreateChatWith.response = JsonResponsePacketSerializer::serializeResponse(userToCreateChatWithResponse);
            resultForUserToCreateChatWith.newHandler = nullptr;

            try
            {
                // send the request to the logged user
                Communicator::send(resultForUserToCreateChatWith, loggedUsersToCreateChatWith[0].getSslSocket());
            }
            catch (std::exception e) // ------------------------ need to change it to specific exception
            {
                std::cout << "socket had closed" << std::endl;
            }
        }

        result.response = JsonResponsePacketSerializer::serializeResponse(resp);
    }

    // login failed
    else
    {
        err.message = "Chat Id Error.";
        result.response = JsonResponsePacketSerializer::serializeResponse(err);
    }

    return result;
}

const RequestResult MenuRequestHandler::loadChat(const RequestInfo rqstInfo)
{
    RequestResult result;
    LoadChatResponse resp;
    LoadChatRequest req;
    ErrorResponse err;
    ChatData cd;

    // build the response, new hendler nullptr for not change the current hendler (menu hendler)
    result.newHandler = nullptr;

    // deserizlize the request
    req = JsonRequestPacketDeserializer::deserializeLoadChatRequest(rqstInfo.buffer);
    cd.id = req.chatID;

    // if the id is valid
    if (cd.id > 0)
    {
        // get the messages from the manager
        resp.messages = this->m_chatManager->loadMessagesChat(cd);

        // build the response
        resp.status = 1;
        result.response = JsonResponsePacketSerializer::serializeResponse(resp);
    }
    // login failed
    else
    {
        err.message = "Chat Id Error.";
        result.response = JsonResponsePacketSerializer::serializeResponse(err);
    }

    return result;
}

const RequestResult MenuRequestHandler::sendMessage(const RequestInfo rqstInfo)
{
    // for the sender
    RequestResult result, resultForUsersForReceive;
    SendMessageResponse resp;
    SendMessageRequest req;

    // for the recivers
    ReciveMessageResponse respForUsersForReceive;

    // to get the logged in recivers users
    std::vector<User> usersInChat;
    std::vector<LoggedUser> loggedUsersToSend;

    ErrorResponse err;
    ChatData chatData;
    Message msgToSend;

    // build the response for sender, new hendler nullptr for not change the current hendler (menu hendler)
    result.newHandler = nullptr;

    // deserialize the response
    req.msg.sender = m_user.getUser();
    req = JsonRequestPacketDeserializer::deserializeSendMessageRequest(rqstInfo.buffer);
    chatData.id = req.msg.chatId;


    // if the chat id is valid
    if (chatData.id > 0)
    {
        // set the fields of the message
        msgToSend = req.msg;
        msgToSend.chatId = chatData.id;
        msgToSend.sender = m_user.getUser();
        time(&(msgToSend.time));
        msgToSend.time += 2 * 60 * 60;

        // get the users of the chat
        usersInChat = this->m_chatManager->sendMessage(msgToSend);

        // get the logged user of the chat
        loggedUsersToSend = m_handlerFactory->getLoginManager()->getLoggedUsersToSend(usersInChat);


        // TODO: add the msg to DB of the users are not logged in


        // build the response for recivers
        respForUsersForReceive.msg = msgToSend;
        respForUsersForReceive.status = 1;

        // serialize the response for other users
        resultForUsersForReceive.response = JsonResponsePacketSerializer::serializeResponse(respForUsersForReceive);
        resultForUsersForReceive.newHandler = nullptr;

        // run on all the recivers
        for (auto i = loggedUsersToSend.begin(); i != loggedUsersToSend.end(); i++)
        {
            // send the message to reciver
            Communicator::send(resultForUsersForReceive, i->getSslSocket());
        }

        // build the response for sender
        resp.status = 1;
        result.response = JsonResponsePacketSerializer::serializeResponse(resp);
    }

    // send message failed failed
    else
    {
        err.message = "Chat Id Error.";
        result.response = JsonResponsePacketSerializer::serializeResponse(err);
    }

    return result;
}

const RequestResult MenuRequestHandler::getAllUsers(const RequestInfo rqstInfo)
{
    RequestResult result;
    GetAllUsersResponse resp;
    ErrorResponse err;

    result.newHandler = nullptr;

    // get all the users from the manager
    resp.users = m_chatManager->getAllOtherUsers(m_user.getUser());

    // build the response
    resp.status = 1;
    result.response = JsonResponsePacketSerializer::serializeResponse(resp);

    return result;
}

const RequestResult MenuRequestHandler::addUser(const RequestInfo rqstInfo)
{
    // fot the sender
    RequestResult result;
    AddUserResponse resp;
    AddUserRequest req;

    // for the reciveres
    RequestResult userToAddResult;
    UpdateCreateNewChatResponse userToAddResp;

    // to get the logged in recivers users
    std::vector<LoggedUser> loggedUsersToSend;
    

    ErrorResponse err;
    ChatData cd;

    // build the response, new hendler nullptr for not change the current hendler (menu hendler)
    result.newHandler = nullptr;

    // deserialize the request
    req = JsonRequestPacketDeserializer::deserializeAddUserRequest(rqstInfo.buffer);
    cd.id = req.chatID;
    std::vector<User> usersToAdd = req.usernames;


    // if the chat id is valid
    if (cd.id > 0 && m_chatManager->addUser(m_user.getUser(), cd, usersToAdd))
    {
        // build the response for the user to add
        userToAddResp.status = 1;
        userToAddResp.chatId = cd.id; // chat id
        userToAddResp.chatName = ((GroupChat*)m_chatManager->getChat(cd.id))->getGroupName(); // get the group name by the group id

        // serialize the user to add response
        userToAddResult.response = JsonResponsePacketSerializer::serializeResponse(resp);
        
        // check if the users are logged in  
        loggedUsersToSend = m_handlerFactory->getLoginManager()->getLoggedUsersToSend(usersToAdd);

        // users logged in
        if (loggedUsersToSend.size() >= 1)
        {
            // run on all the logged user that the msg should be sent
            for (auto userToSend = loggedUsersToSend.begin(); userToSend != loggedUsersToSend.end(); userToSend++)
            {
                Communicator::send(userToAddResult, userToSend->getSslSocket());
            }
        }
        
        // users loged out
        else
        {
            // TODO: add the msg to DB of the users are not logged in
        }

        // build the response for the sender
        resp.status = 1;
        // serialize the sender response
        result.response = JsonResponsePacketSerializer::serializeResponse(resp);
    }

    // add user failed
    else
    {
        err.message = "Chat Id Error OR Add User Error.";
        result.response = JsonResponsePacketSerializer::serializeResponse(err);
    }

    return result;
}

// TODO: realize the ability of the function to add num of users instead of add only 1 user every request
const RequestResult MenuRequestHandler::removeUser(const RequestInfo rqstInfo)
{

    // for the sender
    RequestResult result;
    RemoveUserResponse resp;
    RemoveUserRequest req;
    ErrorResponse err;

    // to get the logged in recivers users
    std::vector<User> userVectorToSend;
    std::vector<LoggedUser> loggedUsersToSend;

    ChatData cd;
    User userToRemove;

    // build the response, new hendler nullptr for not change the current hendler (menu hendler)
    result.newHandler = nullptr;

    // deserialize the request
    req = JsonRequestPacketDeserializer::deserializeRemoveUserRequest(rqstInfo.buffer);
    cd.id = req.chatID;
    userToRemove.username = req.username;

    // if the chat id is valid
    if (cd.id > 0 && m_chatManager->removeUser(m_user.getUser(), cd, userToRemove))
    {
        // build the response for the sender & recivers
        resp.status = 1;
        resp.chatId = cd.id;
        // serialize the sender response & recivers
        result.response = JsonResponsePacketSerializer::serializeResponse(resp);


        // check if the users are logged in
        userVectorToSend.push_back(userToRemove);
        loggedUsersToSend = m_handlerFactory->getLoginManager()->getLoggedUsersToSend(userVectorToSend);

        // users logged in
        if (loggedUsersToSend.size() >= 1)
        {
            // run on all the logged user that the msg should be sent
            for (auto userToSend = loggedUsersToSend.begin(); userToSend != loggedUsersToSend.end(); userToSend++)
            {
                Communicator::send(result, userToSend->getSslSocket()); // send to "userToSend" that he has been removed from the group chat
            }
        }

        // users loged out
        else
        {
            // TODO: add the msg to DB of the users are not logged in
        }
    }

    // remove user failed failed
    else
    {
        err.message = "Chat Id Error OR Remove User Error.";
        result.response = JsonResponsePacketSerializer::serializeResponse(err);
    }

    return result;
}

const RequestResult MenuRequestHandler::getUserChats(const RequestInfo rqstInfo)
{
    RequestResult result;
    GetUserChatsResponse resp;
    ErrorResponse err;
    GetChatsNameRequest req;

    // build the response, new hendler nullptr for not change the current hendler (menu hendler)
    result.newHandler = nullptr;

    // deserialize the req to get the parameters
    req = JsonRequestPacketDeserializer::deserializeGetChatsNameRequest(rqstInfo.buffer);

    // get the chats names from the manager
    resp.chats = m_chatManager->loadChatsOfUser(req.username);

    // build the response
    resp.status = 1;
    result.response = JsonResponsePacketSerializer::serializeResponse(resp);

    return result;
}
