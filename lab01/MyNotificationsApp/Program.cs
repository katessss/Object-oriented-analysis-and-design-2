using System;
using System.Windows.Forms;
using NotificationsApp.UI; 

namespace MyNotificationsApp
{
    static class Program
    {
        [STAThread]
        static void Main()
        {
            ApplicationConfiguration.Initialize();
            Application.Run(new MainForm());
        }
    }
}