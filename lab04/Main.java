import java.sql.*;
import java.util.*;
import javax.swing.*;
import java.awt.*;

// 1. Entity class
class Book {
    private Long id;
    private String title;
    private String author;

    public Book(Long id, String title, String author) {
        this.id = id;
        this.title = title;
        this.author = author;
    }
    public Long getId() { return id; }
    public String getTitle() { return title; }
    public String getAuthor() { return author; }

    @Override
    public String toString() {
        return "ID:" + id + " | " + title + " (" + author + ") [Ref:" + Integer.toHexString(System.identityHashCode(this)) + "]";
    }
}

// 2. Identity Map Pattern
class BookIdentityMap {
    private Map<Long, Book> books = new HashMap<Long, Book>();

    public void addBook(Book book) {
        books.put(book.getId(), book);
    }

    public Book getBook(Long id) {
        return books.get(id);
    }

    public void clear() {
        books.clear();
    }

    public Collection<Book> getAll() {
        return books.values();
    }
}

// 3. Data Mapper
class BookMapper {
    private Connection connection;
    private BookIdentityMap identityMap;

    public BookMapper(Connection connection, BookIdentityMap identityMap) {
        this.connection = connection;
        this.identityMap = identityMap;
    }

    public Book findById(Long id) {
        Book book = identityMap.getBook(id);
        if (book != null) return book;
        return loadSingle("SELECT * FROM books WHERE id = ?", id);
    }

    public java.util.List<Book> findByAuthor(String author) {
        return loadMultiple("SELECT * FROM books WHERE author LIKE ?", "%" + author + "%");
    }

    public java.util.List<Book> findByTitle(String title) {
        return loadMultiple("SELECT * FROM books WHERE title LIKE ?", "%" + title + "%");
    }

    private Book loadSingle(String sql, Long id) {
        try (PreparedStatement pstmt = connection.prepareStatement(sql)) {
            pstmt.setLong(1, id);
            ResultSet rs = pstmt.executeQuery();
            if (rs.next()) {
                return mapRow(rs);
            }
        } catch (SQLException e) { e.printStackTrace(); }
        return null;
    }

    private java.util.List<Book> loadMultiple(String sql, String param) {
        java.util.List<Book> result = new ArrayList<Book>();
        try (PreparedStatement pstmt = connection.prepareStatement(sql)) {
            pstmt.setString(1, param);
            ResultSet rs = pstmt.executeQuery();
            while (rs.next()) {
                result.add(mapRow(rs));
            }
        } catch (SQLException e) { e.printStackTrace(); }
        return result;
    }

    private Book mapRow(ResultSet rs) throws SQLException {
        Long id = rs.getLong("id");
        Book cachedBook = identityMap.getBook(id);
        if (cachedBook != null) {
            return cachedBook;
        }
        Book newBook = new Book(id, rs.getString("title"), rs.getString("author"));
        identityMap.addBook(newBook);
        return newBook;
    }

    public void insert(String title, String author) {
        String sql = "INSERT INTO books (title, author) VALUES (?, ?)";
        try (PreparedStatement pstmt = connection.prepareStatement(sql)) {
            pstmt.setString(1, title);
            pstmt.setString(2, author);
            pstmt.executeUpdate();
        } catch (SQLException e) { e.printStackTrace(); }
    }
}

// 4. Main UI Class
public class Main extends JFrame {
    private BookMapper mapper;
    private BookIdentityMap identityMap;
    private JTextField idIn = new JTextField(3), tIn = new JTextField(10), aIn = new JTextField(10);
    private JTextArea logArea = new JTextArea(15, 60);
    private DefaultListModel<String> ramModel = new DefaultListModel<String>();

    public Main() {
        initDatabase();
        setupUI();
    }

    private void initDatabase() {
        try {
            Class.forName("org.sqlite.JDBC");
            Connection conn = DriverManager.getConnection("jdbc:sqlite:library.sqlite");
            Statement st = conn.createStatement();
            st.execute("CREATE TABLE IF NOT EXISTS books (id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT, author TEXT)");
            
            identityMap = new BookIdentityMap();
            mapper = new BookMapper(conn, identityMap);
        } catch (Exception e) { e.printStackTrace(); }
    }

    private void setupUI() {
        setTitle("Identity Map Pattern - SQL vs RAM");
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setLayout(new BorderLayout(10, 10));

        JPanel inputs = new JPanel();
        inputs.add(new JLabel("ID:")); inputs.add(idIn);
        inputs.add(new JLabel("Title:")); inputs.add(tIn);
        inputs.add(new JLabel("Author:")); inputs.add(aIn);

        JPanel buttons = new JPanel();
        JButton bId = new JButton("Find ID");
        JButton bTitle = new JButton("Search Title");
        JButton bAuthor = new JButton("Search Author");
        JButton bSave = new JButton("Save New");
        JButton bClear = new JButton("Clear RAM");
        buttons.add(bId); buttons.add(bTitle); buttons.add(bAuthor); buttons.add(bSave); buttons.add(bClear);

        JPanel top = new JPanel(new GridLayout(2, 1));
        top.add(inputs); top.add(buttons);

        logArea.setEditable(false);
        logArea.setFont(new Font("Consolas", Font.PLAIN, 12));
        JList<String> list = new JList<String>(ramModel);
        JScrollPane scrollList = new JScrollPane(list);
        scrollList.setBorder(BorderFactory.createTitledBorder("RAM Status"));
        scrollList.setPreferredSize(new Dimension(200, 0));

        add(top, BorderLayout.NORTH);
        add(new JScrollPane(logArea), BorderLayout.CENTER);
        add(scrollList, BorderLayout.EAST);

        // --- Action Listeners with Status Tags ---

        bId.addActionListener(e -> {
            try {
                Long id = Long.parseLong(idIn.getText());
                // Detection logic: is it in RAM before calling mapper?
                boolean inRAM = (identityMap.getBook(id) != null);
                Book b = mapper.findById(id);
                
                String prefix = inRAM ? "[RAM LOAD] " : "[DB LOAD]  ";
                logArea.append(b != null ? prefix + b + "\n" : "[NOT FOUND] ID " + id + "\n");
                refreshRam();
            } catch (Exception ex) { logArea.append("[ERROR] Invalid ID input\n"); }
        });

        bTitle.addActionListener(e -> {
            String title = tIn.getText();
            java.util.List<Book> res = mapper.findByTitle(title);
            logArea.append("--- Search Title: '" + title + "' ---\n");
            for(Book b : res) {
                // Here we can't easily know if it was in RAM before for each item without checking manually
                // But for the sake of demo, we'll let the Ref show the truth
                logArea.append("  -> " + b + "\n");
            }
            refreshRam();
        });

        bAuthor.addActionListener(e -> {
            String author = aIn.getText();
            java.util.List<Book> res = mapper.findByAuthor(author);
            logArea.append("--- Search Author: '" + author + "' ---\n");
            for(Book b : res) {
                logArea.append("  -> " + b + "\n");
            }
            refreshRam();
        });

        bSave.addActionListener(e -> {
            mapper.insert(tIn.getText(), aIn.getText());
            logArea.append("[SYSTEM] New record saved to SQL Database.\n");
        });

        bClear.addActionListener(e -> {
            identityMap.clear();
            refreshRam();
            logArea.append("[SYSTEM] Identity Map (RAM) has been cleared.\n");
        });

        pack(); setLocationRelativeTo(null);
    }

    private void refreshRam() {
        ramModel.clear();
        for (Book b : identityMap.getAll()) ramModel.addElement("ID:" + b.getId());
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new Main().setVisible(true));
    }
}