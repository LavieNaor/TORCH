using MaterialDesignThemes.Wpf;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Security;
using System.Net.Sockets;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace TorClient
{
    /// <summary>
    /// Interaction logic for SignupWindow.xaml
    /// </summary>
    public partial class SignupWindow : Window
    {

        // fields
        private Client _client;
        private SslStream _clientStream;

        Thread _t;

        /// <summary>
        /// c'tor with paramaters
        /// </summary>
        /// <param name="client"></param>
        /// <param name="clientStream"></param>
        public SignupWindow(Client client, SslStream clientStream, Thread t)
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

        }

        /// <summary>
        /// This function signup to the server
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void signupBtn_Click(object sender, RoutedEventArgs e)
        {
            // create requset struct
            SignupRequest req;

            // create answer json
            JObject answerJson;


            // get the parmeters from text box
            req.username = txtUsername.Text;
            req.password = txtPassword.Password;
            req.email = txtEmail.Text;

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
            else if (req.email.CompareTo("") == 0)
            {
                System.Windows.Application.Current.Dispatcher.BeginInvoke(() => MessageBox.Show("Email is empty!"));
                return;
            }

            // send encrypted msg to entry node and get an answer
            answerJson = Helper.sendMsgAndReceiveAnswer(req, _client, (int)requestCodes.SIGNUP_CODE);

            // check if the signup succeed
            if (answerJson["status"] != null && (uint)answerJson["status"] == 1)
            {
                // open the menu window
                MenuWindow wnd = new MenuWindow(_client, _clientStream, req.username, _t);
                App.MenuWindow = wnd;
                wnd.Show();
                this.Close();
            }
        }

        private void loginBtn_Click(object sender, RoutedEventArgs e)
        {
            // open the login window
            MainWindow wnd = new MainWindow(_client, _clientStream, _t);
            wnd.Show();
            this.Close();
        }
    }
}
