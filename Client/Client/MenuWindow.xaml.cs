using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Net.Security;
using System.Text.Json;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Windows.Threading;


namespace TorClient
{
    public partial class MenuWindow : Window, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler? PropertyChanged;

        public ConversationItem _activeConversation;
        private readonly Random _random = new();

        SslStream _clientStream;
        Client _client;

        string _username;

        Thread _t;

        // Fields for user picker
        private string _userPickerMode = "";
        private List<UserItem> _selectedUsers = new();
        private bool _prevHadConversation = false;

        private List<UserItem> _allUsers = new();

        // Fields for conversatiom list
        public ObservableCollection<ConversationItem> Conversations { get; } = new();

        public ObservableCollection<string> EmojiList { get; } = new();

        public Visibility PlaceholderVisibility =>
            string.IsNullOrEmpty(SearchBox?.Text) ? Visibility.Visible : Visibility.Collapsed;

        public Visibility MessagePlaceholderVisibility =>
            string.IsNullOrEmpty(MessageInput?.Text) ? Visibility.Visible : Visibility.Collapsed;

        private static readonly string[] EmojiChars = {
            "\U0001F600","\U0001F602","\U0001F60D","\U0001F970","\U0001F60E","\U0001F929","\U0001F60A","\U0001F973",
            "\U0001F607","\U0001F917","\U0001F60F","\U0001F60B","\U0001F914","\U0001F634","\U0001F631","\U0001F525",
            "\u2764\uFE0F","\U0001F49C","\U0001F499","\u2728","\U0001F389","\U0001F44D","\U0001F44B","\U0001F64C",
            "\U0001F4AA","\U0001F38A","\U0001F31F","\U0001F4AB","\U0001F308","\U0001F98B","\U0001F338","\U0001F355",
            "\u2615","\U0001F3B5","\U0001F3AE","\U0001F4F8"
        };

        private static Brush CreateFrozenGradient(Color c1, Color c2)
        {
            var brush = new LinearGradientBrush(c1, c2, 45);
            brush.Freeze();
            return brush;
        }

        // colors
        public static readonly Dictionary<string, Brush> BrushMap = new()
        {
            ["AccentGradient"] = CreateFrozenGradient(Color.FromRgb(102, 126, 234), Color.FromRgb(118, 75, 162)),
            ["PinkGradient"] = CreateFrozenGradient(Color.FromRgb(240, 147, 251), Color.FromRgb(245, 87, 108)),
            ["CyanGradient"] = CreateFrozenGradient(Color.FromRgb(79, 172, 254), Color.FromRgb(0, 242, 254)),
            ["GreenGradient"] = CreateFrozenGradient(Color.FromRgb(67, 233, 123), Color.FromRgb(56, 249, 215)),
            ["OrangeGradient"] = CreateFrozenGradient(Color.FromRgb(250, 112, 154), Color.FromRgb(254, 225, 64)),
        };

        private List<ConversationItem> _allConversations = new();

        private List<ChatMessage> _messages = new();

        public MenuWindow(Client client, SslStream clientStream, string username, Thread t)
        {
            InitializeComponent();

            _client = client;

            _clientStream = clientStream;
            _t = t;

            DataContext = this;

            _username = username;
            UsernameTextBlock.Text = "You (" + _username + ")";

            foreach (string emoji in EmojiChars) EmojiList.Add(emoji);

            initiateAllConversations();

            foreach (ConversationItem conv in _allConversations) Conversations.Add(conv);

            ConversationsList.ItemsSource = Conversations;
            EmojiGrid.ItemsSource = EmojiList;
        }

        private string getRandomBrushMapColor()
        {
            List<string> keys = BrushMap.Keys.ToList();

            int randomIndex = _random.Next(0, keys.Count);

            string randomKey = keys[randomIndex];

            return randomKey;
        }

        private void initiateAllConversations()
        {
            GetChatsNameRequest req;
            Dictionary<uint, string> chats;
            JObject answerJson;
            string color = "";
            string chatsStr;

            // get the parmeters from text box
            req.username = _username;

            answerJson = Helper.sendMsgAndReceiveAnswer(req, _client, (int)requestCodes.GET_USERS_CHATS_CODE);

            if(answerJson["status"] != null && (uint)answerJson["status"] == 1)
            {
                // get chats and check if it is null or empty
                chatsStr = answerJson["chats"]?.ToString();

                chats = string.IsNullOrWhiteSpace(chatsStr)
                    ? new Dictionary<uint, string>()
                    : Newtonsoft.Json.JsonConvert.DeserializeObject<Dictionary<uint, string>>(chatsStr)
                        ?? new Dictionary<uint, string>();

                // adding to _allConversations the chats
                foreach (uint key in chats.Keys)
                {
                    color = getRandomBrushMapColor();

                    _allConversations.Add(new ConversationItem(chats[key], key, chats[key][0].ToString(), color, "", "", 0));
                }
            }
        }


        private void initiateMessages(uint chatId)
        {
            LoadChatRequest req;
            List<ChatMessage> chatsMessages;
            JObject answerJson;

            // get the parmeters from text box
            req.chatID = chatId;

            answerJson = Helper.sendMsgAndReceiveAnswer(req, _client, (int)requestCodes.LOAD_CHAT_CODE);

            if (answerJson["status"] != null && (uint)answerJson["status"] == 1)
            {

                _messages = answerJson["messages"].ToObject<List<ChatMessage>>();
            }
        }

        private void initiateUsers()
        {
            string req = "";
            JObject answerJson;

            answerJson = Helper.sendMsgAndReceiveAnswer(req, _client, (int)requestCodes.GET_ALL_USERS_CODE);

            if (answerJson["status"] != null && (uint)answerJson["status"] == 1 && answerJson["users"] != null)
            {
                _allUsers = answerJson["users"].ToObject<List<UserItem>>();
            }
        }

        private void ChatWindow_Loaded(object sender, RoutedEventArgs e)
        {
            try
            {
                LoadMessages();
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error loading messages: {ex.Message}", "Error");
            }
        }

        private void LoadMessages()
        {
            MessagesPanel.Children.Clear();

            var dateDivider = new Border
            {
                HorizontalAlignment = HorizontalAlignment.Center,
                Margin = new Thickness(0, 10, 0, 16),
                Padding = new Thickness(16, 4, 16, 4),
                CornerRadius = new CornerRadius(20),
                Background = new SolidColorBrush(Color.FromArgb(15, 255, 255, 255)),
            };
            dateDivider.Child = new TextBlock
            {
                Text = "Today",
                Foreground = (Brush)FindResource("TextMuted"),
                FontSize = 12,
                FontWeight = FontWeights.Medium,
            };
            MessagesPanel.Children.Add(dateDivider);

            initiateMessages(_activeConversation.Id);

            foreach (var msg in _messages)
                AddMessageBubble(msg);

            ScrollToBottom();
        }

        private void AddMessageBubble(ChatMessage msg)
        {
            var activeConv = _allConversations.FirstOrDefault(c => c.Name == _activeConversation.Name);
            var initial = activeConv?.Initial ?? "?";
            bool isSent = msg.sender.Equals(_username);

            var bubble = new Border
            {
                CornerRadius = new CornerRadius(18, 18, isSent ? 6 : 18, isSent ? 18 : 6),
                Padding = new Thickness(16, 12, 16, 12),
                MaxWidth = 500,
                Margin = new Thickness(0, 3, 0, 3),
                HorizontalAlignment = isSent ? HorizontalAlignment.Right : HorizontalAlignment.Left,
            };

            if (isSent)
            {
                bubble.Background = (Brush)FindResource("AccentGradient");
            }
            else
            {
                bubble.Background = (Brush)FindResource("ReceivedBubble");
                bubble.BorderBrush = (Brush)FindResource("BorderBrush");
                bubble.BorderThickness = new Thickness(1);
            }

            var msgText = new TextBlock
            {
                Text = msg.content,
                Foreground = isSent ? Brushes.White : (Brush)FindResource("TextPrimary"),
                FontSize = 14,
                TextWrapping = TextWrapping.Wrap,
            };
            bubble.Child = msgText;

            var timeText = new TextBlock
            {
                Text = msg.time,
                FontSize = 11,
                Foreground = (Brush)FindResource("TextMuted"),
                HorizontalAlignment = isSent ? HorizontalAlignment.Right : HorizontalAlignment.Left,
                Margin = new Thickness(4, 2, 4, 0),
            };

            if (!isSent)
            {
                var avatarBorder = new Border
                {
                    Width = 30,
                    Height = 30,
                    CornerRadius = new CornerRadius(10),
                    Background = activeConv?.AvatarBrush ?? BrushMap["AccentGradient"],
                    VerticalAlignment = VerticalAlignment.Bottom,
                };
                var avatarText = new TextBlock
                {
                    Text = initial,
                    Foreground = Brushes.White,
                    FontSize = 12,
                    FontWeight = FontWeights.SemiBold,
                    HorizontalAlignment = HorizontalAlignment.Center,
                    VerticalAlignment = VerticalAlignment.Center,
                };
                avatarBorder.Child = avatarText;

                var row = new StackPanel { Orientation = Orientation.Horizontal };
                row.Children.Add(avatarBorder);
                row.Children.Add(new Border { Width = 8 });
                var col = new StackPanel();
                col.Children.Add(bubble);
                col.Children.Add(timeText);
                row.Children.Add(col);

                MessagesPanel.Children.Add(row);
            }
            else
            {
                var col = new StackPanel { HorizontalAlignment = HorizontalAlignment.Right };
                col.Children.Add(bubble);
                col.Children.Add(timeText);
                MessagesPanel.Children.Add(col);
            }
        }

        private void AddTypingIndicator()
        {
            var activeConv = _allConversations.FirstOrDefault(c => c.Name == _activeConversation.Name);
            var panel = new StackPanel { Orientation = Orientation.Horizontal, Tag = "typing" };

            var avatarBorder = new Border
            {
                Width = 30,
                Height = 30,
                CornerRadius = new CornerRadius(10),
                Background = activeConv?.AvatarBrush ?? BrushMap["AccentGradient"],
                VerticalAlignment = VerticalAlignment.Bottom,
            };
            avatarBorder.Child = new TextBlock
            {
                Text = activeConv?.Initial ?? "?",
                Foreground = Brushes.White,
                FontSize = 12,
                FontWeight = FontWeights.SemiBold,
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Center,
            };
            panel.Children.Add(avatarBorder);
            panel.Children.Add(new Border { Width = 8 });

            var dotsBorder = new Border
            {
                CornerRadius = new CornerRadius(18),
                Background = (Brush)FindResource("ReceivedBubble"),
                BorderBrush = (Brush)FindResource("BorderBrush"),
                BorderThickness = new Thickness(1),
                Padding = new Thickness(14, 10, 14, 10),
            };
            var dotsPanel = new StackPanel { Orientation = Orientation.Horizontal };
            for (int i = 0; i < 3; i++)
            {
                dotsPanel.Children.Add(new Ellipse
                {
                    Width = 7,
                    Height = 7,
                    Fill = (Brush)FindResource("TextMuted"),
                    Margin = new Thickness(i > 0 ? 4 : 0, 0, 0, 0),
                });
            }
            dotsBorder.Child = dotsPanel;
            panel.Children.Add(dotsBorder);
            MessagesPanel.Children.Add(panel);
            ScrollToBottom();
        }

        private void RemoveTypingIndicator()
        {
            for (int i = MessagesPanel.Children.Count - 1; i >= 0; i--)
            {
                if (MessagesPanel.Children[i] is StackPanel sp && sp.Tag?.ToString() == "typing")
                {
                    MessagesPanel.Children.RemoveAt(i);
                    break;
                }
            }
        }

        private void SendButton_Click(object sender, MouseButtonEventArgs e) => SendMessage();

        private void MessageInput_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                SendMessage();
                e.Handled = true;
            }
            Dispatcher.BeginInvoke(DispatcherPriority.Background, () =>
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(MessagePlaceholderVisibility)));
            });
        }

        private void SendMessage()
        {
            var text = MessageInput.Text?.Trim();
            if (string.IsNullOrEmpty(text)) return;

            JObject answerJson;

            ChatMessage msg = new ChatMessage(text, _username, _activeConversation.Id, true, true, DateTime.Now.ToString("h:mm tt"));

            answerJson = Helper.sendMsgAndReceiveAnswer(msg, _client, (int)requestCodes.SEND_MESSAGE_CODE);

            if(answerJson["status"] != null && (uint)answerJson["status"] == 1)
            {
                AddMessageBubble(msg);
                MessageInput.Text = "";
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(MessagePlaceholderVisibility)));
                ScrollToBottom();
            }
        }

        private void Conversation_Click(object sender, MouseButtonEventArgs e)
        {
            if (sender is Border border && border.DataContext is ConversationItem conv)
            {
                _activeConversation = conv;
                HeaderName.Text = conv.Name;
                HeaderInitial.Text = conv.Initial;
                HeaderAvatar.Background = conv.AvatarBrush;

                UserPickerPanel.Visibility = Visibility.Collapsed;
                ChatHeader.Visibility = Visibility.Visible;
                MessageInputArea.Visibility = Visibility.Visible;
                MessagesScroll.Visibility = Visibility.Visible;
                EmptyState.Visibility = Visibility.Collapsed;

                LoadMessages();
            }
        }
        private void SearchBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            var search = SearchBox.Text;
            Conversations.Clear();
            var filtered = string.IsNullOrEmpty(search)
                ? _allConversations
                : _allConversations.Where(c => c.Name.Contains(search, StringComparison.OrdinalIgnoreCase));
            foreach (var c in filtered) Conversations.Add(c);
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(PlaceholderVisibility)));
        }

        private void NewChatButton_Click(object sender, RoutedEventArgs e)
        {
            NewChatPopup.IsOpen = !NewChatPopup.IsOpen;
        }

        private void NewPrivateChat_Click(object sender, RoutedEventArgs e)
        {
            NewChatPopup.IsOpen = false;
            ShowUserPicker("chat");
        }

        private void NewGroup_Click(object sender, RoutedEventArgs e)
        {
            NewChatPopup.IsOpen = false;
            ShowUserPicker("group");
        }

        private void ShowUserPicker(string mode)
        {
            _userPickerMode = mode;
            _selectedUsers.Clear();
            _prevHadConversation = ChatHeader.Visibility == Visibility.Visible;
            UserPickerTitle.Text = mode == "chat" ? "Select User" : "Select Users";
            UserPickerDoneBtn.Visibility = Visibility.Collapsed;

            GroupNamePanel.Visibility = mode == "group" ? Visibility.Visible : Visibility.Collapsed;
            GroupNameInput.Text = "";

            EmptyState.Visibility = Visibility.Collapsed;
            ChatHeader.Visibility = Visibility.Collapsed;
            MessageInputArea.Visibility = Visibility.Collapsed;
            MessagesScroll.Visibility = Visibility.Collapsed;
            UserPickerPanel.Visibility = Visibility.Visible;

            initiateUsers();

            PopulateUserList();
        }

        private void PopulateUserList()
        {
            string randBrashKey = "";

            UserListPanel.Children.Clear();
            foreach (UserItem user in _allUsers)
            {
                var isSelected = _selectedUsers.Contains(user);
                var item = new Border
                {
                    Padding = new Thickness(12, 10, 16, 10),
                    CornerRadius = new CornerRadius(12),
                    Cursor = Cursors.Hand,
                    Background = isSelected
                        ? new SolidColorBrush(Color.FromArgb(38, 102, 126, 234))
                        : Brushes.Transparent,
                    Tag = user.username,
                    Margin = new Thickness(0, 2, 0, 2),
                };

                var row = new DockPanel();

                randBrashKey = getRandomBrushMapColor();

                var avatar = new Border
                {
                    Width = 42,
                    Height = 42,
                    CornerRadius = new CornerRadius(14),
                    Background = BrushMap[randBrashKey],
                };
                avatar.Child = new TextBlock
                {
                    Text = user.username[0].ToString(),
                    Foreground = Brushes.White,
                    FontWeight = FontWeights.SemiBold,
                    FontSize = 16,
                    HorizontalAlignment = HorizontalAlignment.Center,
                    VerticalAlignment = VerticalAlignment.Center,
                };
                row.Children.Add(avatar);

                var nameText = new TextBlock
                {
                    Text = user.username,
                    Foreground = (Brush)FindResource("TextPrimary"),
                    FontSize = 15,
                    FontWeight = FontWeights.Medium,
                    VerticalAlignment = VerticalAlignment.Center,
                    Margin = new Thickness(14, 0, 0, 0),
                };
                row.Children.Add(nameText);

                if (_userPickerMode == "group")
                {
                    var checkBorder = new Border
                    {
                        Width = 22,
                        Height = 22,
                        CornerRadius = new CornerRadius(11),
                        HorizontalAlignment = HorizontalAlignment.Right,
                    };
                    DockPanel.SetDock(checkBorder, Dock.Right);
                    if (isSelected)
                    {
                        checkBorder.Background = BrushMap["AccentGradient"];
                        checkBorder.Child = new TextBlock
                        {
                            Text = "\u2713",
                            Foreground = Brushes.White,
                            FontSize = 13,
                            HorizontalAlignment = HorizontalAlignment.Center,
                            VerticalAlignment = VerticalAlignment.Center,
                        };
                    }
                    else
                    {
                        checkBorder.BorderBrush = new SolidColorBrush(Color.FromArgb(51, 255, 255, 255));
                        checkBorder.BorderThickness = new Thickness(2);
                        checkBorder.Background = Brushes.Transparent;
                    }
                    row.Children.Insert(0, checkBorder);
                }

                item.Child = row;
                item.MouseLeftButtonDown += UserItem_Click;

                item.Style = CreateUserItemHoverStyle(isSelected);

                UserListPanel.Children.Add(item);
            }
        }

        private Style CreateUserItemHoverStyle(bool isSelected)
        {
            var style = new Style(typeof(Border));
            var defaultBg = isSelected
                ? new SolidColorBrush(Color.FromArgb(38, 102, 126, 234))
                : Brushes.Transparent;
            style.Setters.Add(new Setter(Border.BackgroundProperty, defaultBg));
            var trigger = new Trigger { Property = Border.IsMouseOverProperty, Value = true };
            trigger.Setters.Add(new Setter(Border.BackgroundProperty,
                isSelected
                    ? new SolidColorBrush(Color.FromArgb(50, 102, 126, 234))
                    : new SolidColorBrush(Color.FromArgb(13, 255, 255, 255))));
            style.Triggers.Add(trigger);
            return style;
        }

        private void UserItem_Click(object sender, MouseButtonEventArgs e)
        {
            if (sender is Border border && border.Tag is string name)
            {
                UserItem user = _allUsers.FirstOrDefault(u => u.username == name);
                if (user == null) return;

                if (_userPickerMode == "chat")
                {
                    _selectedUsers.Clear();
                    _selectedUsers.Add(user);
                    ConfirmUserSelection();
                }
                else
                {
                    if (_selectedUsers.Contains(user))
                        _selectedUsers.Remove(user);
                    else
                        _selectedUsers.Add(user);

                    UserPickerDoneBtn.Visibility = _selectedUsers.Count > 0
                        ? Visibility.Visible : Visibility.Collapsed;
                    UserPickerDoneText.Text = $"Done ({_selectedUsers.Count})";
                    PopulateUserList();
                }
            }
        }

        private void UserPickerBack_Click(object sender, RoutedEventArgs e)
        {
            UserPickerPanel.Visibility = Visibility.Collapsed;
            _selectedUsers.Clear();
            if (_prevHadConversation)
            {
                ChatHeader.Visibility = Visibility.Visible;
                MessageInputArea.Visibility = Visibility.Visible;
                MessagesScroll.Visibility = Visibility.Visible;
                EmptyState.Visibility = Visibility.Collapsed;
            }
            else
            {
                EmptyState.Visibility = Visibility.Visible;
            }
        }

        private void UserPickerDone_Click(object sender, RoutedEventArgs e)
        {
            ConfirmUserSelection();
        }

        private void ConfirmUserSelection()
        {
            if (_selectedUsers.Count == 0) return;

            if (_userPickerMode == "chat")
            {
                CreateNewSingleChatRequest req;
                req.username = _selectedUsers.FirstOrDefault().username;

                JObject answerJson = Helper.sendMsgAndReceiveAnswer(req, _client, (int)requestCodes.CREATE_NEW_SINGLE_CHAT_CODE);

                // check if the create single chat succeed
                if (answerJson["status"] != null && (uint)answerJson["status"] == 1)
                {
                    string userName = _selectedUsers[0].username;
                    UserItem user = _allUsers.FirstOrDefault(u => u.username == userName);
                    HeaderName.Text = userName;
                    HeaderInitial.Text = user?.username[0].ToString() ?? userName[..1];
                    string brushKey = getRandomBrushMapColor();
                    HeaderAvatar.Background = BrushMap.ContainsKey(brushKey) ? BrushMap[brushKey] : BrushMap["AccentGradient"];

                    _activeConversation = new ConversationItem(userName, (uint)answerJson["chatId"], user.username[0].ToString(), brushKey, "Now", "Chat started", 0);

                    var existing = _allConversations.FirstOrDefault(c => c.Name == userName);
                    if (existing != null)
                    {
                        _allConversations.Remove(existing);
                        _activeConversation = existing;
                    }
                    _allConversations.Insert(0, _activeConversation);
                    Conversations.Clear();
                    foreach (var c in _allConversations) Conversations.Add(c);
                }
                else
                {
                    return;
                }
            }
            else
            {
                string groupName = GroupNameInput.Text.Trim();

                if(groupName.Length == 0) 
                {
                    MessageBox.Show("Group name is empty!!!");
                    return; 
                }

                CreateNewGroupChatRequest req;
                req.users = _selectedUsers;
                req.groupName = groupName;

                JObject answerJson = Helper.sendMsgAndReceiveAnswer(req, _client, (int)requestCodes.CREATE_NEW_GROUP_CHAT_CODE);

                // check if the create group succeed
                if (answerJson["status"] != null && (uint)answerJson["status"] == 1)
                {
                    HeaderName.Text = groupName;
                    HeaderInitial.Text = "G";
                    HeaderAvatar.Background = BrushMap["CyanGradient"];

                    _activeConversation = new ConversationItem(groupName, (uint)answerJson["chatId"], groupName[0].ToString(), "CyanGradient", "Now", "Group created", 0);

                    _allConversations.Insert(0, _activeConversation);
                    Conversations.Clear();
                    foreach (var c in _allConversations) Conversations.Add(c);
                }
                else
                {
                    return;
                }
            }

            UserPickerPanel.Visibility = Visibility.Collapsed;
            ChatHeader.Visibility = Visibility.Visible;
            MessageInputArea.Visibility = Visibility.Visible;
            MessagesScroll.Visibility = Visibility.Visible;
            EmptyState.Visibility = Visibility.Collapsed;
            _selectedUsers.Clear();
            MessagesPanel.Children.Clear();
            ScrollToBottom();
        }

        private void EmojiButton_Click(object sender, RoutedEventArgs e)
        {
            EmojiPopup.IsOpen = !EmojiPopup.IsOpen;
        }

        private void Emoji_Click(object sender, MouseButtonEventArgs e)
        {
            if (sender is Border border && border.DataContext is string emoji)
            {
                MessageInput.Text += emoji;
                MessageInput.CaretIndex = MessageInput.Text.Length;
                EmojiPopup.IsOpen = false;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(MessagePlaceholderVisibility)));
            }
        }

        public void receiveMessage(JObject jsonReceiveMessage)
        {
            var chatMessage = jsonReceiveMessage["message"]?.ToObject<ChatMessage>();

            if (_activeConversation.Id == chatMessage?.chatId)
            {
                Dispatcher.Invoke(() =>
                {
                    AddMessageBubble(chatMessage);
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(MessagePlaceholderVisibility)));
                    ScrollToBottom();
                });
            }
        }

        public void addNewConversation(JObject jsonReceiveMessage)
        {
            string color = getRandomBrushMapColor();
            string chatName = jsonReceiveMessage["chatName"].ToString();

            uint chatId = uint.Parse(jsonReceiveMessage["chatId"].ToString());

            ConversationItem newConversation = new ConversationItem(chatName, chatId, chatName[0].ToString(), color, "", "", 0);

            Dispatcher.Invoke(() =>
            {
                _allConversations.Insert(0, newConversation);
                Conversations.Insert(0, newConversation);
            });
        }

        private void ScrollToBottom()
        {
            Dispatcher.BeginInvoke(DispatcherPriority.Loaded, () =>
            {
                MessagesScroll.ScrollToEnd();
            });
        }

        private void exitApp(object sender, CancelEventArgs e)
        {
            Application.Current.Shutdown();
            _clientStream.Close();

            _t.Join();
        }
    }
}
