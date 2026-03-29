using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TorClient
{
    // The num of each response
    public enum responseCodes
    {
        ERROR_RESPONSE_CODE = 99,
        SIGNUP_RESPONSE_CODE = 101,
        LOGIN_RESPONSE_CODE = 102,
        SIGNOUT_RESPONSE_CODE = 103,
        CREATE_NEW_GROUP_CHAT_RESPONSE_CODE = 104,
        CREATE_NEW_SINGLE_CHAT_RESPONSE_CODE = 105,
        LOAD_CHAT_RESPONSE_CODE = 106,
        SEND_MESSAGE_RESPONSE_CODE = 107,
        GET_ALL_USERS_RESPONSE_CODE = 108,
        ADD_USER_RESPONSE_CODE = 109,
        REMOVE_USER_RESPONSE_CODE = 110,
        RECIVE_MESSAGE_RESPONSE_CODE = 111,
        GET_USERS_CHATS_RESPONSE_CODE = 112
    }
}
