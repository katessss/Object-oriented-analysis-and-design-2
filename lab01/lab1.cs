using System;
using System.Collections.Generic;
namespace NotificationsApp.Domain
{

    public abstract class Notification
    {
        protected string Recipient;
        public abstract int MaxLength { get; }

        protected Notification(string recipient)
        {
            Recipient = recipient;
        }

        public abstract void Send(string message);
    }

    public class EmailNotification : Notification
    {
        private List<string> files = new List<string>(); 
        public override int MaxLength => 5000;

        public EmailNotification(string email): base(email) 
        { }

        // Уникальное форматирование для Email
        private string format_to_html(string text) => 
            $"<html><body><h1>Новое сообщение</h1><p>{text}</p></body></html>";

        // приклепление файлов
        public void add_attachment(string file_name) { 
            files.Add(file_name);
            Console.WriteLine($"Файл '{file_name}' прикреплен к письму.");
        }

        public override void Send(string message)
        {
            string htmlMessage = format_to_html(message);
            if (files.Count > 0) {
                Console.WriteLine($"Прикрепленные файлы: {string.Join(", ", files)}");
            }
            
            Console.WriteLine($"Отправка Email на {Recipient}: {htmlMessage}");
        }
    }


    public class SmsNotification : Notification
    {
        private bool _isFlash = false;
        public override int MaxLength => 160;

        public SmsNotification(string phone): base(phone) { }

        // flash-сообщение
        public void enable_flash_mode()
        {
            _isFlash = true;
            Console.WriteLine("Режим Flash-SMS активирован.");
        }

        public override void Send(string message)
        {
            string prefix = _isFlash ? "[FLASH] " : "";
            Console.WriteLine($"Отправка SMS на {Recipient}: {prefix}{message}");
        }
    }

    public class TelegramNotification : Notification
    {
        private string[] buttons; 
        public override int MaxLength => 4096;

        public TelegramNotification(string chatId): base(chatId) { }

        // Уникальное форматирование
        private string applay_markdown(string text) => $"*{text}*";

        // Уникальный метод продукта
        public void add_inline_keyboard(string[] buttons) { 
            this.buttons = buttons;
            Console.WriteLine($"Добавлены кнопки: [{string.Join(" | ", buttons)}]");
        }

        public override void Send(string message)
        {
            string formatted = applay_markdown(message);
            if (buttons != null && buttons.Length > 0)
            {
                Console.WriteLine($"[Кнопки]: {string.Join(" | ", buttons)}");
            }
            Console.WriteLine($"Отправка в TG чат {Recipient}: {formatted}");
        }
    }

    // СОЗДАТЕЛИ (ФАБРИКИ)

    public abstract class NotificationSender
    {
        private string validate_message(string message, int maxLength)
        {
            if (message.Length > maxLength)
            {
                Console.WriteLine("Предупреждение: Сообщение слишком длинное и будет обрезано.");
                return message.Substring(0, maxLength);
            }
            return message;
        }

        protected abstract Notification create_notification(string recipient);

        // Хук: по умолчанию ничего не делает
        protected virtual void Configure(Notification notification) { }


        public void send_notice(string recipient, string message)
        {
            Notification notification = create_notification(recipient);
            Configure(notification);

            string preparedMessage = $"[{DateTime.Now:HH:mm:ss}] {message}";
            string validatedMessage = validate_message(preparedMessage, notification.MaxLength);
            
            notification.Send(validatedMessage);
        }
    }

    public class EmailSender : NotificationSender
    {  
        private string _senderName;
        private readonly List<string> _attachments = new();

        public EmailSender(string senderName)
        {
            _senderName = senderName;
        }

        protected override Notification create_notification(string recipient) => new EmailNotification(recipient);

        public void AddAttachment(string fileName) => _attachments.Add(fileName);

        protected override void Configure(Notification notification)
        {
            if (notification is EmailNotification email && _attachments.Count > 0)
            {
                foreach (var f in _attachments)
                {
                    email.add_attachment(f);
                }
                _attachments.Clear();
            }
        }

        public void set_sender_identity(string newName)
        {
            _senderName = newName;
            Console.WriteLine($"Имя отправителя Email изменено на: {_senderName}");
        }
    }



    public class SmsSender : NotificationSender
    {
        private string _apiKey;
        private bool _useFlash;

        public SmsSender(string apiKey)
        {
            _apiKey = apiKey;
        }

        protected override Notification create_notification(string recipient)
        {
            return new SmsNotification(recipient);
        }

        public void EnableFlashMode()
        {
            _useFlash = true;
        }

        protected override void Configure(Notification notification)
        {
            if (notification is SmsNotification sms && _useFlash)
            {
                sms.enable_flash_mode();
                _useFlash = false; // Сброс после использовани
            }
        }

        public double check_balance(int user_id)
        {
            // Имитация запроса к API
            double api_answer = user_id * 10.5; 
            Console.WriteLine($"API ответ: {api_answer} руб.");
            return api_answer;
        }
    }


    public class TelegramSender : NotificationSender
    {
        private string _botToken;
        private string[] _buttons;

        public TelegramSender(string token)
        {
            _botToken = token;
        }

        protected override Notification create_notification(string recipient)
        {
            return new TelegramNotification(recipient);
        }

        public void AddInlineKeyboard(string[] buttons)
        {
            _buttons = buttons;
        }

        protected override void Configure(Notification notification)
        {
            if (notification is TelegramNotification tg && _buttons != null && _buttons.Length > 0)
            {
                tg.add_inline_keyboard(_buttons);
                _buttons = null;
            }

        }
        public void update_webhook(string url)
        {
            Console.WriteLine($"Webhook для бота обновлен: {url}");
        }
        
    }
}