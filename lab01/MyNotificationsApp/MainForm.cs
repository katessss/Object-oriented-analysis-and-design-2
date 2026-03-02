using System;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;
using NotificationsApp.Domain;

namespace NotificationsApp.UI
{
    public sealed class MainForm : Form
    {
        private readonly ComboBox cmbChannel = new() { DropDownStyle = ComboBoxStyle.DropDownList, Font = new Font("Segoe UI", 10) };
        private readonly TextBox txtRecipient = new() { PlaceholderText = "Кому (email / phone / chatId)", Font = new Font("Segoe UI", 10) };
        private readonly TextBox txtMessage = new() { Multiline = true, ScrollBars = ScrollBars.Vertical, PlaceholderText = "Введите текст сообщения...", Font = new Font("Segoe UI", 10) };
        
        private readonly GroupBox gbOptions = new() { Text = "Настройки канала", Font = new Font("Segoe UI", 9, FontStyle.Bold) };
        
        private readonly Panel pnlEmail = new() { Dock = DockStyle.Fill, Visible = false, Padding = new Padding(10) };
        private readonly Panel pnlSms = new() { Dock = DockStyle.Fill, Visible = false, Padding = new Padding(10) };
        private readonly Panel pnlTg = new() { Dock = DockStyle.Fill, Visible = false, Padding = new Padding(10) };

        private readonly ListBox lstAttachments = new() { Dock = DockStyle.Fill };
        private readonly Button btnAddAttachment = new() { Text = "Добавить файл", Dock = DockStyle.Left, Width = 150 };
        private readonly Button btnRemoveAttachment = new() { Text = "Удалить'", Dock = DockStyle.Right, Width = 150 };

        private readonly CheckBox chkFlash = new() { Text = "Редим Flash-SMS", Dock = DockStyle.Top, AutoSize = true };
        private readonly Button btnCheckBalance = new() { Text = "Проверить баланс", Height = 40, Dock = DockStyle.Top, BackColor = Color.Gainsboro };
        private readonly NumericUpDown numUserId = new() { Minimum = 0, Maximum = 1000000, Width = 80, Text = "ID" };


        private readonly TextBox txtWebhook = new() { PlaceholderText = "Webhook URL...", Dock = DockStyle.Top };
        private readonly Button btnUpdateWebhook = new() { Text = "Обновить' Webhook", Height = 35, Dock = DockStyle.Top, BackColor = Color.Gainsboro };
        private readonly TextBox txtButtons = new() { PlaceholderText = "Knopki cherez zapyatuyu...", Dock = DockStyle.Bottom };

        private readonly Button btnSend = new() { 
            Text = "ОТПРАВИТЬ", 
            BackColor = Color.Green, 
            ForeColor = Color.White, 
            Font = new Font("Segoe UI", 12, FontStyle.Bold),
            FlatStyle = FlatStyle.Flat,
            Height = 70,
            Dock = DockStyle.Fill
        };

        private readonly RichTextBox rtbLog = new() { ReadOnly = true, BackColor = Color.Black, ForeColor = Color.White, Font = new Font("Consolas", 10), Dock = DockStyle.Fill };

        private readonly EmailSender emailSender = new EmailSender("Admin");
        private readonly SmsSender smsSender = new SmsSender("KEY");
        private readonly TelegramSender tgSender = new TelegramSender("TOKEN");

        public MainForm()
        {
            Text = "Notification Manager";
            Size = new Size(650, 850);
            StartPosition = FormStartPosition.CenterScreen;

            // --- КРИТИЧЕСКИ ВАЖНАЯ СТРОКА: Привязываем Console к UI ---
            Console.SetOut(new ConsoleToTextBoxWriter(this));

            BuildLayout();
            SetupPanels();
            SetupEvents();

            cmbChannel.Items.AddRange(new object[] { "Email", "SMS", "Telegram" });
            cmbChannel.SelectedIndex = 0;
        }

        // Публичный метод для логирования (теперь его вызывает и консоль)
        public void Log(string message, Color color)
        {
            if (rtbLog.IsDisposed) return;
            if (rtbLog.InvokeRequired)
            {
                rtbLog.BeginInvoke(new Action(() => Log(message, color)));
                return;
            }
            rtbLog.SelectionStart = rtbLog.TextLength;
            rtbLog.SelectionLength = 0;
            rtbLog.SelectionColor = color;
            rtbLog.AppendText($"[{DateTime.Now:HH:mm:ss}] {message}{Environment.NewLine}");
            rtbLog.SelectionColor = rtbLog.ForeColor;
            rtbLog.ScrollToCaret();
        }

        private void BuildLayout()
        {
            TableLayoutPanel root = new() { Dock = DockStyle.Fill, Padding = new Padding(10), RowCount = 5, ColumnCount = 1 };
            root.RowStyles.Add(new RowStyle(SizeType.Absolute, 50));
            root.RowStyles.Add(new RowStyle(SizeType.Absolute, 150));
            root.RowStyles.Add(new RowStyle(SizeType.Absolute, 250));
            root.RowStyles.Add(new RowStyle(SizeType.Absolute, 80));
            root.RowStyles.Add(new RowStyle(SizeType.Percent, 100));

            Panel pnlTop = new() { Dock = DockStyle.Fill };
            cmbChannel.Width = 120;
            txtRecipient.Location = new Point(130, 0);
            txtRecipient.Width = 470;
            pnlTop.Controls.Add(cmbChannel);
            pnlTop.Controls.Add(txtRecipient);
            root.Controls.Add(pnlTop, 0, 0);

            txtMessage.Dock = DockStyle.Fill;
            root.Controls.Add(txtMessage, 0, 1);

            gbOptions.Dock = DockStyle.Fill;
            gbOptions.Controls.Add(pnlEmail);
            gbOptions.Controls.Add(pnlSms);
            gbOptions.Controls.Add(pnlTg);
            root.Controls.Add(gbOptions, 0, 2);

            root.Controls.Add(btnSend, 0, 3);
            root.Controls.Add(rtbLog, 0, 4);

            Controls.Add(root);
        }

        private void SetupPanels()
        {
            Panel pnlEmailButtons = new() { Dock = DockStyle.Bottom, Height = 40 };
            pnlEmailButtons.Controls.Add(btnAddAttachment);
            pnlEmailButtons.Controls.Add(btnRemoveAttachment);
            pnlEmail.Controls.Add(lstAttachments);
            pnlEmail.Controls.Add(new Label { Text = "Вложения:", Dock = DockStyle.Top, Height = 20 });
            pnlEmail.Controls.Add(pnlEmailButtons);

            pnlSms.Controls.Add(btnCheckBalance);
            pnlSms.Controls.Add(new Label { Height = 20, Dock = DockStyle.Top });
            pnlSms.Controls.Add(chkFlash);

            // Настройка SMS
            pnlSms.Padding = new Padding(20);
            pnlSms.Controls.Add(btnCheckBalance);
            // Создаем маленькую панель для ID, чтобы текст и поле были в одну строку
            Panel pnlIdRow = new() { Dock = DockStyle.Top, Height = 30 };
            Label lblId = new() { Text = "ID pol'zovatelya:", Dock = DockStyle.Left, Width = 120 };
            numUserId.Dock = DockStyle.Left;
            pnlIdRow.Controls.Add(numUserId);
            pnlIdRow.Controls.Add(lblId);
            pnlSms.Controls.Add(pnlIdRow); // Добавляем строку с ID
            pnlSms.Controls.Add(new Label { Height = 10, Dock = DockStyle.Top }); // Отступ
            pnlSms.Controls.Add(chkFlash);

            pnlTg.Controls.Add(txtButtons);
            pnlTg.Controls.Add(new Label { Text = "Кнопки (через запятуб):", Dock = DockStyle.Bottom, Height = 20 });
            pnlTg.Controls.Add(new Label { Height = 40, Dock = DockStyle.Top });
            pnlTg.Controls.Add(btnUpdateWebhook);
            pnlTg.Controls.Add(new Label { Height = 10, Dock = DockStyle.Top });
            pnlTg.Controls.Add(txtWebhook);
            pnlTg.Controls.Add(new Label { Text = "Webhook URL:", Dock = DockStyle.Top, Height = 20 });
        }

        private void SetupEvents()
        {
            cmbChannel.SelectedIndexChanged += (_, _) => {
                pnlEmail.Visible = cmbChannel.Text == "Email";
                pnlSms.Visible = cmbChannel.Text == "SMS";
                pnlTg.Visible = cmbChannel.Text == "Telegram";
                gbOptions.Text = "Настройки: " + cmbChannel.Text;
            };

            btnCheckBalance.Click += (_, _) => {
                 int userId = (int)numUserId.Value; 
                double bal = smsSender.check_balance(userId);
                Log("Balans dlya ID " + userId + ": " + bal + " rub.", Color.Cyan);
            };

            btnUpdateWebhook.Click += (_, _) => {
                string url = string.IsNullOrWhiteSpace(txtWebhook.Text) ? "https://api.telegram.org" : txtWebhook.Text;
                tgSender.update_webhook(url);
                Log("Webhook obnovlen v nastrojkax.", Color.Yellow);
            };

            btnAddAttachment.Click += (_, _) => {
                using OpenFileDialog ofd = new();
                if (ofd.ShowDialog() == DialogResult.OK) lstAttachments.Items.Add(ofd.FileName);
            };

            btnRemoveAttachment.Click += (_, _) => {
                if (lstAttachments.SelectedIndex != -1) lstAttachments.Items.RemoveAt(lstAttachments.SelectedIndex);
            };

            btnSend.Click += (_, _) => OnSend();
        }

        private void OnSend()
        {
            string recipient = txtRecipient.Text ?? "";
            string message = txtMessage.Text ?? "";

            if (string.IsNullOrWhiteSpace(recipient)) {
                MessageBox.Show("Введите получателя!"); return;
            }
            if (string.IsNullOrWhiteSpace(message)) {
                MessageBox.Show("Введите текст!"); return;
            }

            try {
                if (cmbChannel.Text == "Email") {
                    foreach (var f in lstAttachments.Items) 
                        if (f != null) emailSender.AddAttachment(f.ToString() ?? "");
                    emailSender.send_notice(recipient, message);
                    lstAttachments.Items.Clear();
                } else if (cmbChannel.Text == "SMS") {
                    if (chkFlash.Checked) smsSender.EnableFlashMode();
                    smsSender.send_notice(recipient, message);
                } else if (cmbChannel.Text == "Telegram") {
                    string[] btns = txtButtons.Text.Split(',').Select(b => b.Trim()).Where(b => b != "").ToArray();
                    if (btns.Length > 0) tgSender.AddInlineKeyboard(btns);
                    tgSender.send_notice(recipient, message);
                }
                
                txtMessage.Clear();
                Log("Uspeshno zaversheno.", Color.Lime);
            } catch (Exception ex) { 
                Log("OSHIBKA: " + ex.Message, Color.Red); 
            }
        }

        // КЛАСС-ПЕРЕХОДНИК ДЛЯ CONSOLE.WRITELINE
        private class ConsoleToTextBoxWriter : System.IO.TextWriter {
            private readonly MainForm _form;
            public ConsoleToTextBoxWriter(MainForm form) => _form = form;
            public override System.Text.Encoding Encoding => System.Text.Encoding.UTF8;
            public override void WriteLine(string? value) {
                // Входящие сообщения от Domain (Console.WriteLine) красим в белый
                _form.Log(value ?? "", Color.White);
            }
        }
    }
}