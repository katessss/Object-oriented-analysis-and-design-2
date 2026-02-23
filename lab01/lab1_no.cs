using System;
using System.Collections.Generic;

public class EmailService
{
    private string _senderName;
    private string Recipient;
    private List<string> files = new List<string>(); 
    public int MaxLength=> 5000;

    public EmailService(string senderName, string recipient)
    {
        _senderName = senderName;
        Recipient = recipient;
    }

    public void set_sender_identity(string newName)
    {
        _senderName = newName;
        Console.WriteLine($"Имя отправителя Email изменено на: {_senderName}");
    }

    private string format_to_html(string text) => 
        $"<html><body><h1>Новое сообщение</h1><p>{text}</p></body></html>";

    // приклепление файлов
    public void add_attachment(string file_name) { 
        files.Add(file_name);
        Console.WriteLine($"Файл '{file_name}' прикреплен к письму.");
    }

    public void Send(string message)
    {
        string htmlMessage = format_to_html(message);
        if (files.Count > 0) {
            Console.WriteLine($"Прикрепленные файлы: {string.Join(", ", files)}");
        }
        
        Console.WriteLine($"Отправка Email на {Recipient}: {htmlMessage}");
    }
}



public class SmsService
{
    private string _apiKey;
    private string Recipient;
    private bool _isFlash = false;
    public int MaxLength => 160;

    public SmsService(string apiKey, string recipient)
    {
        _apiKey = apiKey;
        Recipient = recipient;
    }

    public double check_balance(int user_id)
    {
        // Имитация запроса к API
        double api_answer = user_id * 10.5; 
        Console.WriteLine($"API ответ: {api_answer} руб.");
        return api_answer;
    }

    public void enable_flash_mode()
    {
        _isFlash = true;
        Console.WriteLine("Режим Flash-SMS активирован.");
    }

    public void Send(string message)
    {
        Console.WriteLine($"Отправка SMS на {Recipient}: {message} (Flash: {_isFlash})");
    }
}



public class TGService
{
    private string _botToken;
    private string Recipient;
    public int MaxLength => 4096;

    public TGService(string botToken, string recipient)
    {
        _botToken = botToken;
        Recipient = recipient;
    }

    public void update_webhook(string url)
    {
        Console.WriteLine($"Webhook для бота обновлен: {url}");
    }

    private string applay_markdown(string text) => $"*{text}*";

    // Уникальный метод продукта
    public void add_inline_keyboard(string[] buttons) { 
        this.buttons = buttons;
        Console.WriteLine($"Добавлены кнопки: [{string.Join(" | ", buttons)}]");
    }

    public void Send(string message)
    {
        string formatted = applay_markdown(message);
        if (buttons != null && buttons.Length > 0)
        {
            Console.WriteLine($"[Кнопки]: {string.Join(" | ", buttons)}");
        }
        Console.WriteLine($"Отправка в TG чат {Recipient}: {formatted}");
    }
}


