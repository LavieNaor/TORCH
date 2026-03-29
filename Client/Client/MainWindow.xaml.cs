using MaterialDesignThemes.Wpf;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Net;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Cryptography.X509Certificates;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;


namespace TorClient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>

    public partial class MainWindow : Window
    {
        // fields
        private Client _client;
        private SslStream _clientStream;

        Thread _t;

        // basic c'tor
        public MainWindow()
        {
            InitializeComponent();

            _client = new Client(Helper.randNodes());

            _client.entryNodeSock = Helper.createConnection(_client);
            _clientStream = _client.entryNodeSock;

            Helper.handshake(_client);

            _t = new Thread(() => Helper.receiveAnsThread(_client));
            _t.Start();
        }

        /// <summary>
        /// c'tor with paramaters
        /// </summary>
        /// <param name="client"></param>
        /// <param name="clientStream"></param>
        public MainWindow(Client client, SslStream clientStream, Thread t)
        {
            InitializeComponent();
            _client = client;
            _clientStream = clientStream;
            _t = t;
        }

        /// <summary>
        /// This function close the app
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void exitApp(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
            _clientStream.Close();

            _t.Join();

            this.Close();
        }

        /// <summary>
        /// This function makes the application moveable
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
        {
            base.OnMouseLeftButtonDown(e);
            DragMove();
        }


        public bool IsDarkTheme { get; set; }
        private readonly PaletteHelper paletteHelper = new PaletteHelper();

        /// <summary>
        /// The function change the theme to dark mode
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void toggleTheme(object sender, RoutedEventArgs e)
        {
            ITheme theme = paletteHelper.GetTheme();

            if (IsDarkTheme = theme.GetBaseTheme() == BaseTheme.Dark)
            {
                IsDarkTheme = false;
                theme.SetBaseTheme(Theme.Light);
            }
            else
            {
                IsDarkTheme = true;
                theme.SetBaseTheme(Theme.Dark);
            }
            paletteHelper.SetTheme(theme);
        }

        /// <summary>
        /// This function opens the signup window
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void signupBtn_Click(object sender, RoutedEventArgs e)
        {
            SignupWindow wnd = new SignupWindow(_client, _clientStream, _t);
            wnd.Show();
            this.Close();
        }


        /// <summary>
        /// This function login to the server
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void loginBtn_Click(object sender, RoutedEventArgs e)
        {
            // create requset struct
            LoginRequest req;

            // create answer json
            JObject answerJson;

            // get the parmeters from text box
            req.username = txtUsername.Text;
            req.password = txtPassword.Password;

            // check empty fields
            if (req.username.CompareTo("") == 0)
            {
                System.Windows.Application.Current.Dispatcher.BeginInvoke(() => MessageBox.Show("Username is empty!"));
                return;
            }
            else if (req.password.CompareTo("") == 0)
            {
                System.Windows.Application.Current.Dispatcher.BeginInvoke(() => MessageBox.Show("Password is empty!"));
                return;
            }

            // send encrypted msg to entry node and get an answer
            answerJson = Helper.sendMsgAndReceiveAnswer(req, _client, (int)requestCodes.LOGIN_CODE);

            // check if the login succeed
            if (answerJson["status"] != null && (uint)answerJson["status"] == 1)
            {
                // open the menu window
                MenuWindow wnd = new MenuWindow(_client, _clientStream, req.username, _t);
                App.MenuWindow = wnd;
                wnd.Show();
                this.Close();
            }
        }
    }
}