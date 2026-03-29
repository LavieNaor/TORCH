#include "LoginManager.h"

LoginManager* LoginManager::_instance = nullptr;
int LoginManager::_instanceCounter = 0;

LoginManager* LoginManager::getInstance(IDatabase* DB)
{
    LoginManager::_instanceCounter++; //adding one to instance counter
    if (LoginManager::_instance == nullptr) //checking if we shoud init login manager
    {
        LoginManager::_instance = new LoginManager(DB);
    }

    return LoginManager::_instance;
}

void LoginManager::deleteInstance(LoginManager* instance)
{
    if (LoginManager::_instance != nullptr && LoginManager::_instanceCounter == 1)//checking if we should deleted the instance
    {
        delete LoginManager::_instance;
        LoginManager::_instance = nullptr;
        LoginManager::_instanceCounter--;//removing one from counter
    }
    else if (LoginManager::_instance != nullptr)
    {
        LoginManager::_instanceCounter--;//removing one from counter
    }

    instance = nullptr;
}



LoginManager::LoginManager()
{
}

LoginManager::LoginManager(IDatabase* DB) : m_database(DB)
{
}

LoginManager::~LoginManager()
{
    m_database->close();
    delete m_database;
}


const bool LoginManager::signup(const std::string username, const std::string password, const std::string email, const LoggedUser newUserLoggedIn)
{
    std::string::size_type shtrudleIndex = email.find("@");
    std::string::size_type pointIndex = email.find(".");
    bool hasUppercaseLetter = false, hasLowercaseLetter = false, hasDigit = false, hasSpecialCharacter = false;


    if (m_database->doesUserExist(username))
    {
        throw std::runtime_error("Username Already Exists.");
    }
    else if (shtrudleIndex == std::string::npos || shtrudleIndex == 0 || pointIndex == std::string::npos || !(pointIndex > shtrudleIndex))
    {
        throw std::runtime_error("email syntax invalid");
    }
    else if (password.length() < 8)
    {
        throw std::runtime_error("password length must be 8");
    }

    for (int i = 0; i < shtrudleIndex; i++)
    {
        if (!std::isalnum(email[i]))
        {
            throw std::runtime_error("email syntax before @ is invalid Exists.");
        }
    }

    for (int i = 0; i < password.length(); i++)
    {
        if (std::islower(password[i]))
        {
            hasLowercaseLetter = true;
        }
        else if (std::isupper(password[i]))
        {
            hasUppercaseLetter = true;
        }
        else if (std::isdigit(password[i]))
        {
            hasDigit = true;
        }
        else if (password[i] == '*' || password[i] == '&' || password[i] == '^' || password[i] == '%' || password[i] == '$' || password[i] == '#' || password[i] == '@' || password[i] == '!')
        {
            hasSpecialCharacter = true;
        }
    }
    
    if (!hasUppercaseLetter)
    {
        throw std::runtime_error("password must contain an uppercase letter");
    }
    else if (!hasLowercaseLetter)
    {
        throw std::runtime_error("password must contain a lowercase letter");
    }
    else if (!hasDigit)
    {
        throw std::runtime_error("password must contain a digit");
    }
    else if (!hasSpecialCharacter)
    {
        throw std::runtime_error("password must contain a special character");
    }

    std::unique_lock<std::mutex> lockLog(this->_lockLog);//locking the logged users vector
    m_loggedUsers.push_back(newUserLoggedIn);
    lockLog.unlock(); // unlock the logged users vector

    m_database->addNewUser(username, password, email);
    return true;
}

const bool LoginManager::login(const std::string username, const std::string password, const LoggedUser newUserLoggedIn)
{
    if (m_database->doesPasswordMatch(username, password))
    {
        std::unique_lock<std::mutex> lockLog(this->_lockLog);//locking the logged users vector

        for (auto curr = this->m_loggedUsers.begin(); curr != m_loggedUsers.end(); ++curr)
        {
            if (curr->getUser().username.compare(username) == 0)
            {
                lockLog.unlock(); // unlock the logged users vector
                return false; //user connected return false
            }
        }

        m_loggedUsers.push_back(newUserLoggedIn);
        lockLog.unlock(); // unlock the logged users vector

        return true; //adding user to logged users list
    }
    return false;
}

void LoginManager::logout(const std::string username)
{
    std::unique_lock<std::mutex> lockLog(this->_lockLog); //locking the logged users vector

    for (auto i = m_loggedUsers.begin(); i != m_loggedUsers.end(); i++)
    {
        if (i->getUser().username.compare(username) == 0)
        {
            m_loggedUsers.erase(i);
            lockLog.unlock(); // unlock the logged users vector
            return;
        }
    }

    lockLog.unlock(); // unlock the logged users vector
}

std::vector<LoggedUser> LoginManager::getLoggedUsersToSend(const std::vector<User> usersToSend)
{
    std::vector<LoggedUser> loggedUsersToSend;
    for (auto i = usersToSend.begin(); i != usersToSend.end(); i++)
    {
        for (auto j = m_loggedUsers.begin(); j != m_loggedUsers.end(); j++)
        {
            if (i->username.compare(j->getUser().username) == 0)
            {
                loggedUsersToSend.push_back(*j);
            }
        }
    }

    return loggedUsersToSend;
}
