#include "ChatManager.h"
#include "GroupChat.h"

ChatManager* ChatManager::_instance = nullptr;
int ChatManager::_instanceCounter = 0;
int ChatManager::_lastChatId = 0;

ChatManager* ChatManager::getInstance(IDatabase* DB)
{
    ChatManager::_instanceCounter++; //adding one to instance counter
    if (ChatManager::_instance == nullptr) //checking if we shoud init login manager
    {
        ChatManager::_instance = new ChatManager(DB);

        for (int i = 1; i <= _lastChatId; i++)
        {
            _instance->m_chats.insert(std::make_pair(i, _instance->m_database->loadChat(i)));
        }
    }

    return ChatManager::_instance;
}


void ChatManager::deleteInstance(ChatManager* instance)
{
    if (ChatManager::_instance != nullptr && ChatManager::_instanceCounter == 1)//checking if we should deleted the instance
    {
        delete ChatManager::_instance;
        ChatManager::_instance = nullptr;
        ChatManager::_instanceCounter--;//removing one from counter
    }
    else if (ChatManager::_instance != nullptr)
    {
        ChatManager::_instanceCounter--;//removing one from counter
    }

    instance = nullptr;
}

ChatManager::ChatManager() : m_database(nullptr)
{
}


ChatManager::ChatManager(IDatabase* DB) : m_database(DB)
{
    _lastChatId = m_database->getLastChatId();

    for (int i = 1; i <= _lastChatId; i++)
    {
        delete this->m_chats[i];
        this->m_chats.erase(i);
    }
}


ChatManager::~ChatManager()
{
    m_database->close();
    delete m_database;
}

const int ChatManager::createSingleChat(std::vector<User> users, ChatData chatData)
{
    SingleChat* newChat =  nullptr;
    std::vector<Message> messages;
    UserInChat userInChat;
    std::map<User, UserInChat> usersInChat;

    if (users.size() != 2)
    {
        throw std::runtime_error("Not enough users");
    }
    else if (!m_database->doesUserExist(users[0].username))
    {
        throw std::runtime_error("User doesn't exist");
    }
    else if (m_database->doesSingleChatExist(users[0], users[1]) != 0)
    {
        throw std::runtime_error("Chat already exist");
    }

    // TODO: Add mutex, get the id from the DB!!!
    chatData.id = ++_lastChatId;
    m_database->addChat(chatData);

    time(&userInChat.joinedAt);
    userInChat.joinedAt += 2 * 60 * 60;
    userInChat.role = "MEMBER";

    for (int i = 0; i < users.size(); i++)
    {
        userInChat.user = users[i];
        usersInChat[users[i]] = userInChat;
        m_database->addMemberToChat(userInChat, chatData.id);
    }

    newChat = new SingleChat(chatData, usersInChat, messages);

    // insert the chat to the map chats
    this->m_chats.insert(std::make_pair(chatData.id, newChat));

    return chatData.id;
}

const int ChatManager::createGroupChat(std::vector<User> users, ChatData chatData, GroupChatData groupChatData)
{
    SingleChat* newChat = nullptr;
    std::vector<Message> messages;
    UserInChat userInChat;
    std::map<User, UserInChat> usersInChat;

    if (users.size() < 2)
    {
        throw std::runtime_error("Not enough users");
    }

    for (int i = 0; i < users.size() - 1; i++)
    {
        if (!m_database->doesUserExist(users[i].username))
        {
            throw std::runtime_error("User doesn't exist");
        }
    }

    if (groupChatData.groupName.compare("") == 0)
    {
        throw std::runtime_error("Group name is empty!");
    }

    // TODO: Add mutex, get the id from the DB!!!
    chatData.id = ++_lastChatId;
    m_database->addGroup(chatData, groupChatData);

    time(&userInChat.joinedAt);
    userInChat.joinedAt += 2 * 60 * 60;
    userInChat.role = "MEMBER";

    for (int i = 0; i < users.size() - 1; i++)
    {
        userInChat.user = users[i];
        usersInChat[users[i]] = userInChat;
        m_database->addMemberToChat(userInChat, chatData.id);
    }

    userInChat.user = users[users.size() - 1];
    userInChat.role = "ADMIN";
    usersInChat[users[users.size() - 1]] = userInChat;

    m_database->addMemberToChat(userInChat, chatData.id);

    newChat = new GroupChat(chatData, groupChatData, usersInChat, messages);

    // insert the chat to the map chats
    this->m_chats.insert(std::make_pair(chatData.id, newChat));

    return chatData.id;
}

const std::vector<Message> ChatManager::loadMessagesChat(ChatData data)
{
    return m_database->loadMessagesChat(data.id);
}


const std::map<unsigned int, std::string> ChatManager::loadChatsOfUser(std::string username)
{
    // get the chats id of the username
    std::vector<unsigned int> chatsId = m_database->getChatsIdsByUser(username);

    std::map<unsigned int, std::string> chats; // {"chatId": "chatName"}

    std::map<User, UserInChat> usersInChat;

    // get the name of each chat
    for (auto it = chatsId.begin(); it != chatsId.end(); it++)
    {
        SingleChat* chat = m_chats[*it];

        // group chat
        if (chat->getType() == "GROUP")
        {
            chats[*it] = ((GroupChat*)m_chats[*it])->getGroupName();
        }

        // private chat
        else if (chat->getType() == "PRIVATE")
        {
            usersInChat = chat->getUsers();

            // search the username who is not the sender
            for (auto itName = usersInChat.begin(); itName != usersInChat.end(); itName++)
            {
                if (itName->first.username != username)
                {
                    chats[*it] = itName->first.username;
                }
            }
        }
    }

    // return the map
    return chats;
}


const std::vector<ChatData> ChatManager::getChats()
{
    std::vector<ChatData> chats;

    // get all the chats data
    for (auto it = this->m_chats.begin(); it != m_chats.end(); it++)
    {
        chats.push_back(it->second->getChatData());
    }
    return chats;
}


SingleChat* ChatManager::getChat(unsigned int id)
{
    // search the chat by the ID
    for (auto it = this->m_chats.begin(); it != m_chats.end(); it++)
    {
        if (it->first == id)
        {
            return it->second;
        }
    }
    return nullptr;
}

std::vector<User> ChatManager::getAllOtherUsers(User user)
{
    return m_database->getAllOtherUsersExist(user);
}

std::map<User, UserInChat> ChatManager::getAllUsersInChat(unsigned int id)
{
    return m_database->getAllUsersInChat(id);
}

const std::vector<User> ChatManager::sendMessage(Message msgToSend)
{
    SingleChat* chatToSend = getChat(msgToSend.chatId);

    if (m_chats.find(msgToSend.chatId) == m_chats.end())
    {
        throw std::runtime_error("Chat id wrong");
    }

    m_database->addMessage(msgToSend);

    return chatToSend->sendMessage(msgToSend, msgToSend.sender);
}


const bool ChatManager::addUser(User UserThatAdd, ChatData data, std::vector<User> usersToAdd)
{
    GroupChat* chatToAdd = nullptr;
    std::vector<UserInChat> usersInChat;

    if (m_chats[data.id]->getUsers()[UserThatAdd].role.compare("ADMIN") != 0)
    {
        throw std::runtime_error("You are not admin");
    }

    // transform to groupChat poniter
    chatToAdd = (GroupChat*)getChat(data.id);

    // run on all the users
    for (auto user = usersToAdd.begin(); user != usersToAdd.end(); user++)
    {

        // create userInChat var
        UserInChat userInChat;
        userInChat.user = *user;
        time(&userInChat.joinedAt);
        userInChat.joinedAt += 2 * 60 * 60;
        userInChat.role = "MEMBER";

        // add the user to the DB
        if (!(chatToAdd->addUser(userInChat) && m_database->addMemberToChat(userInChat, data.id)))
        {
            return false;
        }
    }

    return true;
}


const bool ChatManager::removeUser(User UserThatRemove, ChatData data, User userToRemove)
{
    GroupChat* chatToRemove = nullptr;

    if (m_chats[data.id]->getUsers()[UserThatRemove].role.compare("ADMIN") != 0)
    {
        throw std::runtime_error("You are not admin");
    }

    // transform to groupChat poniter
    chatToRemove = (GroupChat*)getChat(data.id);

    // add the user
    return chatToRemove->removeUser(userToRemove) && m_database->removeMemberFromGroup(userToRemove, data.id);
}


