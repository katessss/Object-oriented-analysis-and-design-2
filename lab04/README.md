Вот подробный отчет по твоей лабораторной работе в формате Markdown (.md). Ты
можешь скопировать этот текст, сохранить его в файл README.md или вставить в
Word.

Отчет по лабораторной работе

Тема: Реализация архитектурного паттерна Identity Map (Карта объектов)

1. Цель работы

Изучение и практическое применение паттерна проектирования Identity Map (Карта
объектов) для обеспечения целостности данных и оптимизации работы с
реляционной базой данных в Java-приложении.

2. Описание предметной области

В качестве предметной области выбрана «Библиотечная система». Система позволяет
управлять книгами (Book), выполнять поиск по идентификатору (ID), названию или
автору, а также сохранять новые записи в базу данных.

3. Используемые технологии

  - Язык программирования: Java 8+
  - СУБД: SQLite (настоящая файловая база данных)
  - Библиотеки: JDBC (драйвер sqlite-jdbc), Swing (графический интерфейс).

4. Архитектура системы

Диаграмма классов (UML)

@startuml
allowmixing
skinparam classAttributeIconSize 0

class Book {
    - Long id
    - String title
    - String author
    + getId(): Long
    + getTitle(): String
    + getAuthor(): String
    + toString(): String
}

class BookIdentityMap {
    - Map<Long, Book> books
    + addBook(book: Book): void
    + getBook(id: Long): Book
    + clear(): void
    + getAll(): Collection<Book>
}

class BookMapper {
    - Connection connection
    - BookIdentityMap identityMap
    + findById(id: Long): Book
    + findByAuthor(author: String): List<Book>
    + findByTitle(title: String): List<Book>
    + insert(title: String, author: String): void
    - mapRow(rs: ResultSet): Book
}

database "SQLite (library.sqlite)" as DB

Main "1" --> "1" BookMapper : использует
Main "1" --> "1" BookIdentityMap : владеет
BookMapper "1" ..> "1" BookIdentityMap : проверяет/обновляет
BookIdentityMap "1" o-- "*" Book : хранит ссылки
BookMapper -- DB : SQL SELECT/INSERT
@enduml

5. Identity Map

В работе были применен паттерны проектирования Identity Map (Коллекция объектов): Реализован в классе BookIdentityMap.
Гарантирует, что для одной строки БД существует только один экземпляр
объекта в памяти. Это проверяется через сравнение хеш-кодов объектов (Ref).

При вызове метода findById(id):

1.  Программа обращается к BookIdentityMap.
2.  Если объект найден в карте (RAM LOAD), он возвращается немедленно, минуя
    SQL-запрос.
3.  Если объект не найден (DB LOAD), выполняется SQL-запрос SELECT.
4.  Полученные данные преобразуются в объект Book.
5.  Созданный объект регистрируется в BookIdentityMap и возвращается
    пользователю.

7. Результаты работы

В ходе выполнения работы было разработано Java-приложение с графическим
интерфейсом. Приложение наглядно демонстрирует работу паттерна:

  - При первом поиске книги лог отображает [DB LOAD].
  - При повторном поиске той же книги отображается [RAM LOAD].
  - Поле Ref (хеш-код памяти) остается неизменным, что доказывает использование
    одного и того же экземпляра объекта.

8. Вывод

Реализованный паттерн Identity Map позволяет избежать конфликтов при обновлении
данных (синхронизация состояний объектов) и существенно снижает нагрузку на
базу данных за счет кэширования объектов в рамках текущей сессии.
