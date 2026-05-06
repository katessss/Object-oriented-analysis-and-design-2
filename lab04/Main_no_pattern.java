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
        // Without Identity Map, Ref will be DIFFERENT every time you load the same ID
        return "ID:" + id + " | " + title + " (" + author + ") [Ref:" + Integer.toHexString(System.identityHashCode(this)) + "]";
    }
}

// 2. Data Mapper (NO Identity Map here)
class BookMapper {
    private Connection connection;

    public BookMapper(Connection connection) {
        this.connection = connection;
    }

    public Book findById(Long id) {
        // ALWAYS loading from DB
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
        // WITHOUT IDENTITY MAP: Always create a NEW object instance
        return new Book(rs.getLong("id"), rs.getString("title"), rs.getString("author"));
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

// 3. Main UI Class
public class Main extends JFrame {
    private BookMapper mapper;
    private JTextField idIn = new JTextField(3), tIn = new JTextField(10), aIn = new JTextField(10);
    private JTextArea logArea = new JTextArea(15, 60);

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
            
            mapper = new BookMapper(conn);
        } catch (Exception e) { e.printStackTrace(); }
    }

    private void setupUI() {
        setTitle("NO Identity Map - Every load creates NEW object");
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
        buttons.add(bId); buttons.add(bTitle); buttons.add(bAuthor); buttons.add(bSave);

        JPanel top = new JPanel(new GridLayout(2, 1));
        top.add(inputs); top.add(buttons);

        logArea.setEditable(false);
        logArea.setFont(new Font("Consolas", Font.PLAIN, 12));
        logArea.setBackground(new Color(240, 230, 230)); // Reddish tint to indicate "bad" practice

        add(top, BorderLayout.NORTH);
        add(new JScrollPane(logArea), BorderLayout.CENTER);

        // --- Action Listeners ---

        bId.addActionListener(e -> {
            try {
                Long id = Long.parseLong(idIn.getText());
                // Always [DB LOAD] because there is no RAM cache
                Book b = mapper.findById(id);
                logArea.append("[DB LOAD ONLY] " + (b != null ? b : "Not found") + "\n");
            } catch (Exception ex) { logArea.append("[ERROR] Invalid ID\n"); }
        });

        bTitle.addActionListener(e -> {
            String title = tIn.getText();
            java.util.List<Book> res = mapper.findByTitle(title);
            logArea.append("--- Search Title: '" + title + "' (All NEW objects) ---\n");
            for(Book b : res) {
                logArea.append("  -> " + b + "\n");
            }
        });

        bAuthor.addActionListener(e -> {
            String author = aIn.getText();
            java.util.List<Book> res = mapper.findByAuthor(author);
            logArea.append("--- Search Author: '" + author + "' (All NEW objects) ---\n");
            for(Book b : res) {
                logArea.append("  -> " + b + "\n");
            }
        });

        bSave.addActionListener(e -> {
            mapper.insert(tIn.getText(), aIn.getText());
            logArea.append("[SYSTEM] Saved to SQL.\n");
        });

        pack(); setLocationRelativeTo(null);
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new Main().setVisible(true));
    }
}