using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace TorClient
{
    public class ConversationItem
    {
        public string Name { get; }
        public uint Id { get; }
        public string Initial { get; }
        public string TimeLastMsg { get; }
        public string LastMsg { get; }
        public int Unread { get; }
        public Brush AvatarBrush { get; }
        public Visibility UnreadVisibility => Unread > 0 ? Visibility.Visible : Visibility.Collapsed;

        public ConversationItem(string name, uint id, string initial, string brushKey, string timeLastMsg, string lastMsg, int unread)
        {
            Name = name;
            Id = id;
            Initial = initial;
            TimeLastMsg = timeLastMsg;
            LastMsg = lastMsg;
            Unread = unread;
            AvatarBrush = MenuWindow.BrushMap.ContainsKey(brushKey) ? MenuWindow.BrushMap[brushKey] : MenuWindow.BrushMap["AccentGradient"];
        }
    }
}
