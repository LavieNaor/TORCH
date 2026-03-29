using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TorClient
{
    // The num of each request
    public enum requestCodes
    {
        SIGNUP_CODE = 1,
        LOGIN_CODE = 2,
        SIGNOUT_CODE = 3,
        CREATE_NEW_GROUP_CHAT_CODE = 4,
        CREATE_NEW_SINGLE_CHAT_CODE = 5,
        LOAD_CHAT_CODE = 6,
        SEND_MESSAGE_CODE = 7,
        GET_ALL_USERS_CODE = 8,
        ADD_USER_CODE = 9,
        REMOVE_USER_CODE = 10,
        // do not use code 11! - code 111 use for recive msg response
        GET_USERS_CHATS_CODE = 12
    }

    public struct LoginRequest
    {
        public string username;
        public string password;
    }

    public struct SignupRequest
    {
        public string username;
        public string password;
        public string email;
    }

    public struct CreateNewGroupChatRequest
    {
        // list of users (vector)
        public List<UserItem> users;
        public string groupName;
    }
    public struct CreateNewSingleChatRequest
    {
        public string username;
    }

    public struct LoadChatRequest
    {
        public uint chatID;
    }

    public struct SendMessageRequest
    {
        public ChatMessage msg;
    }

    public struct RemoveUserRequest
    {
        public string username;
        public uint chatID;
    }

    public struct AddUserRequest
    {
        public string username;
        public uint chatID;
    }

    public struct GetChatsNameRequest
    {
        public string username;
    }


}
