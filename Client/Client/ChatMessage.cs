using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;

namespace TorClient
{
    public class ChatMessage
    {
        public string content { get; set; } // the content of the msg
        public string sender { get; set; } // sender username
        public bool isTxtMsg { get; set; } // if the msg is txt msg or file 
        public uint chatId { get; set; } // the id of the chat that the msg sent to
        public string time {  get; set; } // the time the msg was sent

        // c'tor
        public ChatMessage(string content, string sender, uint chatId, bool isSent, bool isTxtMsg, string time)
        {
            this.content = content;
            this.sender = sender;
            this.chatId = chatId;
            this.isTxtMsg = isTxtMsg;

            DateTime fromIso8601UTCtime;

            // check if the time is in 24' clock format
            bool success24 = DateTime.TryParseExact(time, "MM/dd/yyyy HH:mm:ss",
                System.Globalization.CultureInfo.InvariantCulture,
                System.Globalization.DateTimeStyles.None,
                out fromIso8601UTCtime);

            // check if the time is in 12' clock format
            if (!success24)
            {
                fromIso8601UTCtime = DateTime.Parse(time);
            }

            // set the time in 12' clock format
            this.time = fromIso8601UTCtime.ToString("hh:mm tt");
        }
    }
}