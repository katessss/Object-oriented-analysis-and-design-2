using System;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;
using NotificationsApp.Domain;

namespace NotificationsApp.UI
{
    public sealed class MainForm : Form
    {
        private readonly ComboBox cmbChannel = new() { DropDownStyle = ComboBoxStyle.DropDownList };
        private readonly TextBox txtRecipient = new() { PlaceholderText = "Получатель (email / phone / chatId)" };
        private readonly TextBox txtMessage = new() { Multiline = true, ScrollBars = ScrollBars.Vertical };

        private readonly GroupBox gbEmail = new() { Text = "Email" };
        private readonly ListBox lstAttachments = new();
        private readonly Button btnAddAttachment = new() { Text = "Добавить файл…" };
        private readonly Button btnRemoveAttachment = new() { Text = "Удалить" };

        private readonly GroupBox gbSms = new() { Text = "SMS" };
        private readonly CheckBox chkFlash = new() { Text = "Flash SMS" };

        private readonly GroupBox gbTg = new() { Text = "Telegram" };
        private readonly TextBox txtButtons = new() { PlaceholderText = "Кнопки через запятую: Да, Нет, ..." };

        private readonly Button btnSend = new() { Text = "Отправить" };
        private readonly TextBox txtLog = new() { Multiline = true, ReadOnly = true, ScrollBars = ScrollBars.Vertical };

        // Sender-ы из твоего доменного файла
        private readonly EmailSender emailSender = new EmailSender("UI Sender");
        private readonly SmsSender smsSender = new SmsSender("API_KEY");
        private readonly TelegramSender tgSender = new TelegramSender("BOT_TOKEN");

        public MainForm()
        {
            Text = "Lab1 — Notifications";
            StartPosition = FormStartPosition.CenterScreen;
            MinimumSize = new Size(900, 650);

            // Перенаправляем Console.WriteLine в окно лога
            Console.SetOut(new TextBoxWriter(txtLog));

            cmbChannel.Items.AddRange(new object[] { "Email", "SMS", "Telegram" });
            cmbChannel.SelectedIndex = 0;
            cmbChannel.SelectedIndexChanged += (_, __) => UpdateOptionsVisibility();

            txtMessage.Height = 120;

            btnSend.Click += (_, __) => OnSend();

            BuildLayout();
            BuildEmailGroup();
            BuildSmsGroup();
            BuildTelegramGroup();

            UpdateOptionsVisibility();
        }

        private void BuildLayout()
        {
            var root = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                Padding = new Padding(10),
                RowCount = 5,
                ColumnCount = 2
            };

            root.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 180));
            root.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));

            root.RowStyles.Add(new RowStyle(SizeType.Absolute, 40));
            root.RowStyles.Add(new RowStyle(SizeType.Absolute, 150));
            root.RowStyles.Add(new RowStyle(SizeType.Absolute, 170));
            root.RowStyles.Add(new RowStyle(SizeType.Absolute, 45));
            root.RowStyles.Add(new RowStyle(SizeType.Percent, 100));

            // row 0
            root.Controls.Add(cmbChannel, 0, 0);
            root.Controls.Add(txtRecipient, 1, 0);

            // row 1 message
            var msgPanel = new Panel { Dock = DockStyle.Fill };
            msgPanel.Controls.Add(txtMessage);
            txtMessage.Dock = DockStyle.Fill;
            msgPanel.Controls.Add(new Label { Text = "Сообщение:", Dock = DockStyle.Top, Height = 18 });
            root.Controls.Add(msgPanel, 0, 1);
            root.SetColumnSpan(msgPanel, 2);

            // row 2 options stack
            var optionsPanel = new Panel { Dock = DockStyle.Fill };
            gbEmail.Dock = DockStyle.Fill;
            gbSms.Dock = DockStyle.Fill;
            gbTg.Dock = DockStyle.Fill;
            optionsPanel.Controls.Add(gbEmail);
            optionsPanel.Controls.Add(gbSms);
            optionsPanel.Controls.Add(gbTg);

            root.Controls.Add(optionsPanel, 0, 2);
            root.SetColumnSpan(optionsPanel, 2);

            // row 3 send
            var sendPanel = new Panel { Dock = DockStyle.Fill };
            btnSend.Dock = DockStyle.Right;
            btnSend.Width = 170;
            sendPanel.Controls.Add(btnSend);
            root.Controls.Add(sendPanel, 0, 3);
            root.SetColumnSpan(sendPanel, 2);

            // row 4 log
            var logPanel = new Panel { Dock = DockStyle.Fill };
            logPanel.Controls.Add(txtLog);
            txtLog.Dock = DockStyle.Fill;
            logPanel.Controls.Add(new Label { Text = "Лог:", Dock = DockStyle.Top, Height = 18 });
            root.Controls.Add(logPanel, 0, 4);
            root.SetColumnSpan(logPanel, 2);

            Controls.Add(root);
        }

        private void BuildEmailGroup()
        {
            var layout = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                Padding = new Padding(8),
                RowCount = 2,
                ColumnCount = 2
            };
            layout.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
            layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 35));
            layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));
            layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));

            lstAttachments.Dock = DockStyle.Fill;
            layout.Controls.Add(lstAttachments, 0, 0);
            layout.SetColumnSpan(lstAttachments, 2);

            btnAddAttachment.Dock = DockStyle.Fill;
            btnRemoveAttachment.Dock = DockStyle.Fill;
            layout.Controls.Add(btnAddAttachment, 0, 1);
            layout.Controls.Add(btnRemoveAttachment, 1, 1);

            btnAddAttachment.Click += (_, __) =>
            {
                using var ofd = new OpenFileDialog { Title = "Выберите файл-вложение" };
                if (ofd.ShowDialog(this) == DialogResult.OK)
                    lstAttachments.Items.Add(ofd.FileName);
            };

            btnRemoveAttachment.Click += (_, __) =>
            {
                if (lstAttachments.SelectedItem != null)
                    lstAttachments.Items.Remove(lstAttachments.SelectedItem);
            };

            gbEmail.Controls.Add(layout);
        }

        private void BuildSmsGroup()
        {
            var layout = new FlowLayoutPanel { Dock = DockStyle.Fill, Padding = new Padding(8) };
            layout.Controls.Add(chkFlash);
            gbSms.Controls.Add(layout);
        }

        private void BuildTelegramGroup()
        {
            var layout = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                Padding = new Padding(8),
                RowCount = 2,
                ColumnCount = 1
            };
            layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 20));
            layout.RowStyles.Add(new RowStyle(SizeType.Percent, 100));

            layout.Controls.Add(new Label { Text = "Inline кнопки:", Dock = DockStyle.Fill }, 0, 0);
            txtButtons.Dock = DockStyle.Top;
            layout.Controls.Add(txtButtons, 0, 1);

            gbTg.Controls.Add(layout);
        }

        private void UpdateOptionsVisibility()
        {
            var ch = (cmbChannel.SelectedItem?.ToString() ?? "Email");

            gbEmail.Visible = ch == "Email";
            gbSms.Visible = ch == "SMS";
            gbTg.Visible = ch == "Telegram";

            gbEmail.BringToFront();
            gbSms.BringToFront();
            gbTg.BringToFront();
        }

        private void OnSend()
        {
            var ch = (cmbChannel.SelectedItem?.ToString() ?? "Email");
            var recipient = (txtRecipient.Text ?? "").Trim();
            var message = txtMessage.Text ?? "";

            if (string.IsNullOrWhiteSpace(recipient))
            {
                MessageBox.Show(this, "Введите получателя.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            if (string.IsNullOrWhiteSpace(message))
            {
                MessageBox.Show(this, "Введите сообщение.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            try
            {
                if (ch == "Email")
                {
                    foreach (var item in lstAttachments.Items.Cast<object>())
                        emailSender.AddAttachment(item.ToString() ?? "");

                    emailSender.send_notice(recipient, message);
                    lstAttachments.Items.Clear();
                }
                else if (ch == "SMS")
                {
                    if (chkFlash.Checked)
                        smsSender.EnableFlashMode();

                    smsSender.send_notice(recipient, message);
                    chkFlash.Checked = false;
                }
                else // Telegram
                {
                    var btns = ParseButtons(txtButtons.Text);
                    if (btns.Length > 0)
                        tgSender.AddInlineKeyboard(btns);

                    tgSender.send_notice(recipient, message);
                    txtButtons.Text = "";
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ERROR] {ex}");
                MessageBox.Show(this, ex.Message, "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private static string[] ParseButtons(string raw)
        {
            if (string.IsNullOrWhiteSpace(raw)) return Array.Empty<string>();
            return raw.Split(',')
                .Select(x => x.Trim())
                .Where(x => x.Length > 0)
                .Take(10)
                .ToArray();
        }

        private sealed class TextBoxWriter : System.IO.TextWriter
        {
            private readonly TextBox _box;
            public TextBoxWriter(TextBox box) => _box = box;
            public override System.Text.Encoding Encoding => System.Text.Encoding.UTF8;

            public override void Write(string? value)
            {
                if (value == null || _box.IsDisposed) return;
                _box.BeginInvoke(new Action(() => _box.AppendText(value)));
            }

            public override void WriteLine(string? value)
                => Write((value ?? "") + Environment.NewLine);
        }
    }
}