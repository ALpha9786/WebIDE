#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QTreeWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QDialog>
#include <QListWidget>
#include <QColorDialog>
#include <QFontDialog>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageBox>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QProcess>
#include <QStatusBar>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QProgressDialog>
#include <QCompleter>
#include <QAbstractItemView>
#include <QStringListModel>
#include <QStandardPaths>
#include <QScrollBar>
#include <QClipboard>
#include <QMimeData>

// Syntax Highlighter for HTML/CSS/JS/PHP
class CodeHighlighter : public QSyntaxHighlighter {
public:
    CodeHighlighter(QTextDocument *parent = nullptr) : QSyntaxHighlighter(parent) {
        setupRules();
    }

    void setupRules() {
        HighlightingRule rule;

        // HTML Tags
        tagFormat.setForeground(QColor(86, 156, 214));
        tagFormat.setFontWeight(QFont::Bold);
        rule.pattern = QRegularExpression("<[/?]?\\w+");
        rule.format = tagFormat;
        highlightingRules.append(rule);

        rule.pattern = QRegularExpression("/?>");
        highlightingRules.append(rule);

        // Attributes
        attributeFormat.setForeground(QColor(156, 220, 254));
        rule.pattern = QRegularExpression("\\b[A-Za-z-]+(?=\\=)");
        rule.format = attributeFormat;
        highlightingRules.append(rule);

        // Strings
        stringFormat.setForeground(QColor(206, 145, 120));
        rule.pattern = QRegularExpression("\".*?\"|'.*?'");
        rule.format = stringFormat;
        highlightingRules.append(rule);

        // JavaScript Keywords
        keywordFormat.setForeground(QColor(197, 134, 192));
        keywordFormat.setFontWeight(QFont::Bold);
        QStringList keywordPatterns = {
            "\\bfunction\\b", "\\bvar\\b", "\\blet\\b", "\\bconst\\b",
            "\\breturn\\b", "\\bif\\b", "\\belse\\b", "\\bfor\\b",
            "\\bwhile\\b", "\\bswitch\\b", "\\bcase\\b", "\\bbreak\\b",
            "\\bcontinue\\b", "\\btrue\\b", "\\bfalse\\b", "\\bnull\\b",
            "\\bundefined\\b", "\\bnew\\b", "\\bthis\\b", "\\bclass\\b"
        };
        for (const QString &pattern : keywordPatterns) {
            rule.pattern = QRegularExpression(pattern);
            rule.format = keywordFormat;
            highlightingRules.append(rule);
        }

        // CSS Properties
        cssPropertyFormat.setForeground(QColor(156, 220, 254));
        rule.pattern = QRegularExpression("\\b[a-z-]+(?=\\s*:)");
        rule.format = cssPropertyFormat;
        highlightingRules.append(rule);

        // PHP Tags and Keywords
        phpFormat.setForeground(QColor(197, 134, 192));
        phpFormat.setFontWeight(QFont::Bold);
        rule.pattern = QRegularExpression("<\\?php|\\?>");
        rule.format = phpFormat;
        highlightingRules.append(rule);

        // Numbers
        numberFormat.setForeground(QColor(181, 206, 168));
        rule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b");
        rule.format = numberFormat;
        highlightingRules.append(rule);

        // Comments
        commentFormat.setForeground(QColor(106, 153, 85));
        commentFormat.setFontItalic(true);
        rule.pattern = QRegularExpression("//[^\n]*|/\\*.*?\\*/|<!--.*?-->");
        rule.format = commentFormat;
        highlightingRules.append(rule);
    }

protected:
    void highlightBlock(const QString &text) override {
        for (const HighlightingRule &rule : highlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
    
    QTextCharFormat tagFormat;
    QTextCharFormat attributeFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat keywordFormat;
    QTextCharFormat cssPropertyFormat;
    QTextCharFormat phpFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat commentFormat;
};

// Custom Text Editor
class CodeEditor : public QTextEdit {
    Q_OBJECT
    
public:
    CodeEditor(const QString &fileType = "", QWidget *parent = nullptr) : QTextEdit(parent), currentFileType(fileType) {
        highlighter = new CodeHighlighter(document());
        applyTheme(true); // Default dark theme
        setupAutoComplete();
        
        connect(this, &QTextEdit::textChanged, this, &CodeEditor::onTextChanged);
    }

    void applyTheme(bool isDark) {
        if (isDark) {
            setStyleSheet(R"(
                QTextEdit {
                    background-color: #1e1e1e;
                    color: #d4d4d4;
                    border: none;
                    font-family: 'Consolas', 'Courier New', monospace;
                    font-size: 14px;
                }
            )");
        } else {
            setStyleSheet(R"(
                QTextEdit {
                    background-color: #ffffff;
                    color: #000000;
                    border: none;
                    font-family: 'Consolas', 'Courier New', monospace;
                    font-size: 14px;
                }
            )");
        }
    }

    void setFileType(const QString &type) {
        currentFileType = type;
        setupAutoComplete();
    }

    void showImportPanel() {
        if (currentFileType == "html" || currentFileType == "htm" || 
            currentFileType == "xhtml" || currentFileType == "xhtm" || currentFileType == "htma") {
            emit requestImportPanel();
        }
    }

signals:
    void requestImportPanel();

public slots:
    void onTextChanged() {
        // Show suggestions as you type
        QTextCursor cursor = textCursor();
        cursor.select(QTextCursor::WordUnderCursor);
        QString word = cursor.selectedText();
        
        if (word.length() > 1) {
            if (completer && completer->completionCount() > 0) {
                completer->setCompletionPrefix(word);
                QRect cr = cursorRect();
                cr.setWidth(completer->popup()->sizeHintForColumn(0) 
                    + completer->popup()->verticalScrollBar()->sizeHint().width());
                completer->complete(cr);
            }
        }
    }

private:
    void setupAutoComplete() {
        QStringList suggestions;
        
        if (currentFileType == "html" || currentFileType == "htm" || 
            currentFileType == "xhtml" || currentFileType == "xhtm" || currentFileType == "htma") {
            suggestions << "<!DOCTYPE html>" << "<html>" << "</html>" << "<head>" << "</head>"
                       << "<body>" << "</body>" << "<div>" << "</div>" << "<span>" << "</span>"
                       << "<script>" << "</script>" << "<link>" << "<style>" << "</style>"
                       << "<title>" << "</title>" << "<meta>" << "<form>" << "</form>"
                       << "<input>" << "<button>" << "</button>" << "<a>" << "</a>"
                       << "<img>" << "<ul>" << "</ul>" << "<li>" << "</li>" << "<p>" << "</p>"
                       << "<h1>" << "</h1>" << "<h2>" << "</h2>" << "<h3>" << "</h3>"
                       << "<table>" << "</table>" << "<tr>" << "</tr>" << "<td>" << "</td>"
                       << "class=\"\"" << "id=\"\"" << "href=\"\"" << "src=\"\"";
        } else if (currentFileType == "css") {
            suggestions << "background-color:" << "color:" << "font-size:" << "margin:" << "padding:"
                       << "width:" << "height:" << "display:" << "position:" << "border:"
                       << "flex" << "grid" << "absolute" << "relative" << "fixed"
                       << "background:" << "border-radius:" << "box-shadow:" << "text-align:"
                       << "font-family:" << "font-weight:" << "line-height:" << "opacity:"
                       << "transition:" << "transform:" << "justify-content:" << "align-items:";
        } else if (currentFileType == "js") {
            suggestions << "function" << "const" << "let" << "var" << "return" << "if" << "else"
                       << "for" << "while" << "switch" << "case" << "break" << "continue"
                       << "document.getElementById" << "document.querySelector"
                       << "document.querySelectorAll" << "addEventListener" << "console.log"
                       << "fetch" << "async" << "await" << "Promise" << "setTimeout" << "setInterval"
                       << "class" << "constructor" << "this" << "new" << "export" << "import";
        }
        
        if (!suggestions.isEmpty()) {
            completer = new QCompleter(suggestions, this);
            completer->setWidget(this);
            completer->setCompletionMode(QCompleter::PopupCompletion);
            completer->setCaseSensitivity(Qt::CaseInsensitive);
            
            connect(completer, QOverload<const QString &>::of(&QCompleter::activated),
                    this, &CodeEditor::insertCompletion);
        }
    }

    void insertCompletion(const QString &completion) {
        QTextCursor cursor = textCursor();
        cursor.select(QTextCursor::WordUnderCursor);
        cursor.removeSelectedText();
        cursor.insertText(completion);
        setTextCursor(cursor);
    }

    CodeHighlighter *highlighter;
    QCompleter *completer = nullptr;
    QString currentFileType;
};

// Main IDE Window
class WebIDE : public QMainWindow {
    Q_OBJECT

public:
    WebIDE(QWidget *parent = nullptr) : QMainWindow(parent), serverPort(8080), isDarkTheme(true) {
        setupUI();
        applyTheme(isDarkTheme);
        loadSettings();
        setupServer();
    }

    ~WebIDE() {
        saveSettings();
        if (server && server->isListening()) {
            server->close();
        }
    }

private slots:
    void openFolder() {
        // Use native file dialog with better UI
        QFileDialog dialog(this);
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);
        dialog.setOption(QFileDialog::DontUseNativeDialog, false); // Use native OS dialog
        dialog.setViewMode(QFileDialog::Detail);
        dialog.setWindowTitle("Select Folder to Open");
        
        // Set starting directory to home or desktop
        QString startPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        dialog.setDirectory(startPath);
        
        if (dialog.exec()) {
            QStringList folders = dialog.selectedFiles();
            if (!folders.isEmpty()) {
                currentFolder = folders.first();
                loadFolderStructure(currentFolder);
                statusBar()->showMessage("Opened folder: " + currentFolder, 3000);
            }
        }
    }
    
    void quickOpenFolder() {
        // Show quick access dialog with common locations
        QDialog quickDialog(this);
        quickDialog.setWindowTitle("Quick Open Folder");
        quickDialog.setMinimumSize(600, 400);
        
        QVBoxLayout *layout = new QVBoxLayout(&quickDialog);
        
        QLabel *title = new QLabel("<h2>Select a folder location:</h2>");
        layout->addWidget(title);
        
        QListWidget *locationList = new QListWidget();
        locationList->setIconSize(QSize(32, 32));
        
        // Add quick access locations
        QStandardPaths::StandardLocation locations[] = {
            QStandardPaths::HomeLocation,
            QStandardPaths::DesktopLocation,
            QStandardPaths::DocumentsLocation,
            QStandardPaths::DownloadLocation,
            QStandardPaths::MusicLocation,
            QStandardPaths::PicturesLocation,
            QStandardPaths::MoviesLocation
        };
        
        QString locationNames[] = {
            "Home",
            "Desktop",
            "Documents",
            "Downloads",
            "Music",
            "Pictures",
            "Videos"
        };
        
        QStyle::StandardPixmap icons[] = {
            QStyle::SP_DirHomeIcon,
            QStyle::SP_DesktopIcon,
            QStyle::SP_DirIcon,
            QStyle::SP_DirIcon,
            QStyle::SP_DirIcon,
            QStyle::SP_DirIcon,
            QStyle::SP_DirIcon
        };
        
        for (int i = 0; i < 7; i++) {
            QString path = QStandardPaths::writableLocation(locations[i]);
            if (!path.isEmpty() && QDir(path).exists()) {
                QListWidgetItem *item = new QListWidgetItem(
                    style()->standardIcon(icons[i]),
                    locationNames[i] + " - " + path
                );
                item->setData(Qt::UserRole, path);
                locationList->addItem(item);
            }
        }
        
        // Add recent folders if any
        QSettings settings("WebIDE", "Settings");
        QStringList recentFolders = settings.value("recentFolders").toStringList();
        if (!recentFolders.isEmpty()) {
            locationList->addItem(new QListWidgetItem(""));
            QListWidgetItem *recentHeader = new QListWidgetItem("Recent Folders:");
            QFont boldFont = recentHeader->font();
            boldFont.setBold(true);
            recentHeader->setFont(boldFont);
            recentHeader->setFlags(Qt::NoItemFlags);
            locationList->addItem(recentHeader);
            
            for (const QString &folder : recentFolders) {
                if (QDir(folder).exists()) {
                    QListWidgetItem *item = new QListWidgetItem(
                        style()->standardIcon(QStyle::SP_DirIcon),
                        QFileInfo(folder).fileName() + " - " + folder
                    );
                    item->setData(Qt::UserRole, folder);
                    locationList->addItem(item);
                }
            }
        }
        
        layout->addWidget(locationList);
        
        // Buttons
        QHBoxLayout *btnLayout = new QHBoxLayout();
        QPushButton *browseBtn = new QPushButton("Browse Other...");
        QPushButton *openBtn = new QPushButton("Open Selected");
        QPushButton *cancelBtn = new QPushButton("Cancel");
        
        browseBtn->setMinimumWidth(120);
        openBtn->setMinimumWidth(120);
        cancelBtn->setMinimumWidth(120);
        
        btnLayout->addWidget(browseBtn);
        btnLayout->addStretch();
        btnLayout->addWidget(cancelBtn);
        btnLayout->addWidget(openBtn);
        
        layout->addLayout(btnLayout);
        
        // Connect buttons
        connect(browseBtn, &QPushButton::clicked, [&]() {
            quickDialog.accept();
            openFolder(); // Call regular folder browser
        });
        
        connect(cancelBtn, &QPushButton::clicked, &quickDialog, &QDialog::reject);
        
        connect(openBtn, &QPushButton::clicked, [&]() {
            QListWidgetItem *item = locationList->currentItem();
            if (item && item->data(Qt::UserRole).isValid()) {
                QString path = item->data(Qt::UserRole).toString();
                currentFolder = path;
                loadFolderStructure(currentFolder);
                
                // Save to recent folders
                QSettings settings("WebIDE", "Settings");
                QStringList recent = settings.value("recentFolders").toStringList();
                recent.removeAll(path);
                recent.prepend(path);
                if (recent.size() > 10) recent = recent.mid(0, 10);
                settings.setValue("recentFolders", recent);
                
                statusBar()->showMessage("Opened folder: " + currentFolder, 3000);
                quickDialog.accept();
            } else {
                QMessageBox::warning(&quickDialog, "Warning", "Please select a folder from the list");
            }
        });
        
        connect(locationList, &QListWidget::itemDoubleClicked, [&](QListWidgetItem *item) {
            if (item && item->data(Qt::UserRole).isValid()) {
                QString path = item->data(Qt::UserRole).toString();
                currentFolder = path;
                loadFolderStructure(currentFolder);
                
                // Save to recent folders
                QSettings settings("WebIDE", "Settings");
                QStringList recent = settings.value("recentFolders").toStringList();
                recent.removeAll(path);
                recent.prepend(path);
                if (recent.size() > 10) recent = recent.mid(0, 10);
                settings.setValue("recentFolders", recent);
                
                statusBar()->showMessage("Opened folder: " + currentFolder, 3000);
                quickDialog.accept();
            }
        });
        
        quickDialog.exec();
    }

    void newFile() {
        bool ok;
        QString fileName = QInputDialog::getText(this, "New File", "File name:", QLineEdit::Normal, "", &ok);
        if (ok && !fileName.isEmpty()) {
            if (!currentFolder.isEmpty()) {
                QString filePath = currentFolder + "/" + fileName;
                QFile file(filePath);
                if (file.open(QIODevice::WriteOnly)) {
                    file.close();
                    loadFolderStructure(currentFolder);
                    openFileInEditor(filePath);
                }
            } else {
                QMessageBox::warning(this, "Warning", "Please open a folder first");
            }
        }
    }

    void newFolder() {
        bool ok;
        QString folderName = QInputDialog::getText(this, "New Folder", "Folder name:", QLineEdit::Normal, "", &ok);
        if (ok && !folderName.isEmpty()) {
            if (!currentFolder.isEmpty()) {
                QDir dir(currentFolder);
                if (dir.mkdir(folderName)) {
                    loadFolderStructure(currentFolder);
                }
            } else {
                QMessageBox::warning(this, "Warning", "Please open a folder first");
            }
        }
    }

    void saveFile() {
        if (tabWidget->currentIndex() >= 0) {
            QString filePath = tabWidget->tabToolTip(tabWidget->currentIndex());
            CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->currentWidget());
            if (editor && !filePath.isEmpty()) {
                QFile file(filePath);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&file);
                    out << editor->toPlainText();
                    file.close();
                    statusBar()->showMessage("File saved: " + filePath, 3000);
                }
            }
        }
    }

    void saveAllFiles() {
        for (int i = 0; i < tabWidget->count(); ++i) {
            QString filePath = tabWidget->tabToolTip(i);
            CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->widget(i));
            if (editor && !filePath.isEmpty()) {
                QFile file(filePath);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&file);
                    out << editor->toPlainText();
                    file.close();
                }
            }
        }
        statusBar()->showMessage("All files saved", 3000);
    }

    void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column) {
        Q_UNUSED(column);
        QString filePath = item->data(0, Qt::UserRole).toString();
        if (!filePath.isEmpty() && QFileInfo(filePath).isFile()) {
            openFileInEditor(filePath);
        }
    }

    void closeTab(int index) {
        tabWidget->removeTab(index);
    }

    void startServer() {
        if (server->isListening()) {
            server->close();
            serverBtn->setText("Start Server");
            serverStatusLabel->setText("Server: Stopped");
            statusBar()->showMessage("Server stopped", 3000);
        } else {
            serverPort = portSpinBox->value();
            if (server->listen(QHostAddress::Any, serverPort)) {
                serverBtn->setText("Stop Server");
                
                QString ipAddresses;
                QList<QHostAddress> list = QNetworkInterface::allAddresses();
                for (const QHostAddress &address : list) {
                    if (address.protocol() == QAbstractSocket::IPv4Protocol && 
                        address != QHostAddress::LocalHost) {
                        ipAddresses += address.toString() + " ";
                    }
                }
                
                QString serverInfo = QString("Server: Running on port %1").arg(serverPort);
                if (!ipAddresses.isEmpty()) {
                    serverInfo += QString(" | IP: %1").arg(ipAddresses.trimmed());
                }
                
                serverStatusLabel->setText(serverInfo);
                statusBar()->showMessage("Server started on port " + QString::number(serverPort), 3000);
            } else {
                QMessageBox::warning(this, "Error", "Could not start server on port " + QString::number(serverPort));
            }
        }
    }

    void openInBrowser() {
        if (server->isListening()) {
            QString url = QString("http://localhost:%1").arg(serverPort);
            QDesktopServices::openUrl(QUrl(url));
        } else {
            QMessageBox::warning(this, "Warning", "Please start the server first");
        }
    }

    void openTerminal() {
        if (currentFolder.isEmpty()) {
            QMessageBox::warning(this, "Warning", "Please open a folder first");
            return;
        }
        openTerminalInPath(currentFolder);
    }

    void openTerminalInPath(const QString &path) {
        QString terminal;
        QStringList arguments;
        
#ifdef Q_OS_WIN
        // Windows - Try PowerShell, then cmd
        terminal = "powershell.exe";
        arguments << "-NoExit" << "-Command" << QString("cd '%1'").arg(path);
        
        QProcess *process = new QProcess(this);
        process->setWorkingDirectory(path);
        
        if (!process->startDetached(terminal, arguments)) {
            // Fallback to cmd.exe
            terminal = "cmd.exe";
            arguments.clear();
            arguments << "/K" << QString("cd /d \"%1\"").arg(path);
            process->startDetached(terminal, arguments);
        }
#elif defined(Q_OS_LINUX)
        // Linux - Try various terminals
        QStringList terminals = {"x-terminal-emulator", "gnome-terminal", "konsole", "xterm", "terminator", "xfce4-terminal"};
        
        for (const QString &term : terminals) {
            QProcess *process = new QProcess(this);
            process->setWorkingDirectory(path);
            
            if (term == "gnome-terminal") {
                arguments << "--working-directory=" + path;
            } else if (term == "konsole") {
                arguments << "--workdir" << path;
            } else if (term == "xfce4-terminal") {
                arguments << "--working-directory=" + path;
            } else if (term == "terminator") {
                arguments << "--working-directory=" + path;
            }
            
            if (process->startDetached(term, arguments)) {
                statusBar()->showMessage("Terminal opened in: " + path, 3000);
                return;
            }
            arguments.clear();
        }
        
        QMessageBox::warning(this, "Error", "Could not find a terminal emulator");
#elif defined(Q_OS_MAC)
        // macOS
        terminal = "open";
        arguments << "-a" << "Terminal" << path;
        
        QProcess *process = new QProcess(this);
        process->startDetached(terminal, arguments);
#endif
        
        statusBar()->showMessage("Terminal opened in: " + path, 3000);
    }

    void showSettings() {
        QDialog dialog(this);
        dialog.setWindowTitle("Settings");
        dialog.setMinimumSize(500, 400);
        
        QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
        
        // Theme Settings
        QGroupBox *themeGroup = new QGroupBox("Theme");
        QVBoxLayout *themeLayout = new QVBoxLayout();
        
        QButtonGroup *themeButtons = new QButtonGroup(&dialog);
        QRadioButton *darkRadio = new QRadioButton("Dark Theme");
        QRadioButton *lightRadio = new QRadioButton("Light Theme");
        
        themeButtons->addButton(darkRadio, 0);
        themeButtons->addButton(lightRadio, 1);
        
        if (isDarkTheme) {
            darkRadio->setChecked(true);
        } else {
            lightRadio->setChecked(true);
        }
        
        themeLayout->addWidget(darkRadio);
        themeLayout->addWidget(lightRadio);
        themeGroup->setLayout(themeLayout);
        mainLayout->addWidget(themeGroup);
        
        // Editor Settings
        QGroupBox *editorGroup = new QGroupBox("Editor");
        QVBoxLayout *editorLayout = new QVBoxLayout();
        
        QCheckBox *syntaxHighlight = new QCheckBox("Enable Syntax Highlighting");
        syntaxHighlight->setChecked(true);
        editorLayout->addWidget(syntaxHighlight);
        
        QCheckBox *lineNumbers = new QCheckBox("Show Line Numbers");
        lineNumbers->setChecked(false);
        editorLayout->addWidget(lineNumbers);
        
        editorGroup->setLayout(editorLayout);
        mainLayout->addWidget(editorGroup);
        
        mainLayout->addStretch();
        
        // Buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        
        QPushButton *cancelBtn = new QPushButton("Cancel");
        QPushButton *applyBtn = new QPushButton("Apply");
        
        cancelBtn->setMinimumWidth(80);
        applyBtn->setMinimumWidth(80);
        
        buttonLayout->addWidget(cancelBtn);
        buttonLayout->addWidget(applyBtn);
        mainLayout->addLayout(buttonLayout);
        
        // Connect buttons
        connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
        connect(applyBtn, &QPushButton::clicked, [&]() {
            bool newTheme = darkRadio->isChecked();
            if (newTheme != isDarkTheme) {
                isDarkTheme = newTheme;
                applyTheme(isDarkTheme);
                
                // Update all open editors
                for (int i = 0; i < tabWidget->count(); ++i) {
                    CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->widget(i));
                    if (editor) {
                        editor->applyTheme(isDarkTheme);
                    }
                }
            }
            dialog.accept();
        });
        
        dialog.exec();
    }

    void showExportDialog() {
        QDialog dialog(this);
        dialog.setWindowTitle("Export Files");
        dialog.setMinimumSize(500, 400);
        
        QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
        
        QLabel *infoLabel = new QLabel("Select files to export:");
        mainLayout->addWidget(infoLabel);
        
        // File list
        QListWidget *fileList = new QListWidget();
        if (!currentFolder.isEmpty()) {
            QDir dir(currentFolder);
            QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
            for (const QFileInfo &info : entries) {
                QListWidgetItem *item = new QListWidgetItem(info.fileName());
                item->setCheckState(Qt::Unchecked);
                item->setData(Qt::UserRole, info.absoluteFilePath());
                fileList->addItem(item);
            }
        }
        mainLayout->addWidget(fileList);
        
        // Export location
        QHBoxLayout *locationLayout = new QHBoxLayout();
        QLabel *locationLabel = new QLabel("Export to:");
        QLineEdit *locationEdit = new QLineEdit();
        QPushButton *browseBtn = new QPushButton("Browse");
        
        connect(browseBtn, &QPushButton::clicked, [locationEdit]() {
            QString dir = QFileDialog::getExistingDirectory(nullptr, "Select Export Directory");
            if (!dir.isEmpty()) {
                locationEdit->setText(dir);
            }
        });
        
        locationLayout->addWidget(locationLabel);
        locationLayout->addWidget(locationEdit);
        locationLayout->addWidget(browseBtn);
        mainLayout->addLayout(locationLayout);
        
        // Buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        
        QPushButton *cancelBtn = new QPushButton("Cancel");
        QPushButton *exportBtn = new QPushButton("Export");
        
        cancelBtn->setMinimumWidth(80);
        exportBtn->setMinimumWidth(80);
        
        buttonLayout->addWidget(cancelBtn);
        buttonLayout->addWidget(exportBtn);
        mainLayout->addLayout(buttonLayout);
        
        // Connect buttons
        connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
        connect(exportBtn, &QPushButton::clicked, [&]() {
            QString exportPath = locationEdit->text();
            if (exportPath.isEmpty()) {
                QMessageBox::warning(&dialog, "Warning", "Please select an export directory");
                return;
            }
            
            QDir exportDir(exportPath);
            if (!exportDir.exists()) {
                QMessageBox::warning(&dialog, "Warning", "Export directory does not exist");
                return;
            }
            
            int exportCount = 0;
            QProgressDialog progress("Exporting files...", "Cancel", 0, fileList->count(), &dialog);
            progress.setWindowModality(Qt::WindowModal);
            
            for (int i = 0; i < fileList->count(); ++i) {
                progress.setValue(i);
                if (progress.wasCanceled()) {
                    break;
                }
                
                QListWidgetItem *item = fileList->item(i);
                if (item->checkState() == Qt::Checked) {
                    QString sourcePath = item->data(Qt::UserRole).toString();
                    QString destPath = exportPath + "/" + item->text();
                    
                    if (QFile::copy(sourcePath, destPath)) {
                        exportCount++;
                    }
                }
            }
            
            progress.setValue(fileList->count());
            
            QMessageBox::information(&dialog, "Export Complete", 
                QString("Successfully exported %1 file(s)").arg(exportCount));
            dialog.accept();
        });
        
        dialog.exec();
    }

    void handleNewConnection() {
        QTcpSocket *socket = server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            QString request = socket->readAll();
            
            // Parse requested file
            QString requestedFile = "/index.html";
            QRegularExpression re("GET\\s+([^\\s]+)");
            QRegularExpressionMatch match = re.match(request);
            if (match.hasMatch()) {
                requestedFile = match.captured(1);
                if (requestedFile == "/") requestedFile = "/index.html";
            }

            QString filePath = currentFolder + requestedFile;
            QFile file(filePath);
            
            QString response;
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray content = file.readAll();
                response = "HTTP/1.1 200 OK\r\n";
                response += "Content-Type: text/html\r\n";
                response += "Content-Length: " + QString::number(content.size()) + "\r\n";
                response += "\r\n";
                socket->write(response.toUtf8());
                socket->write(content);
            } else {
                response = "HTTP/1.1 404 Not Found\r\n\r\n<h1>404 Not Found</h1>";
                socket->write(response.toUtf8());
            }
            
            socket->disconnectFromHost();
        });
    }

private:
    void setupUI() {
        setWindowTitle("Web IDE - Full Featured");
        resize(1400, 900);

        // Create menu bar
        createMenuBar();

        // Create toolbar
        createToolBar();

        // Main layout
        QWidget *centralWidget = new QWidget();
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // Import panel at top (for HTML files)
        importPanel = new QWidget();
        importPanel->setMaximumHeight(200);
        importPanel->setMinimumHeight(150);
        importLayout = new QVBoxLayout(importPanel);
        importLayout->setContentsMargins(10, 5, 10, 5);
        
        QLabel *noFile = new QLabel("Open an HTML file to see import options");
        noFile->setAlignment(Qt::AlignCenter);
        importLayout->addWidget(noFile);
        
        mainLayout->addWidget(importPanel);

        // Splitter for file tree and editor
        QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);

        // Left panel - File explorer
        leftPanel = new QTabWidget();
        leftPanel->setMaximumWidth(300);
        
        // Explorer tab
        fileTree = new QTreeWidget();
        fileTree->setHeaderLabel("Explorer");
        fileTree->setContextMenuPolicy(Qt::CustomContextMenu);
        fileTree->setDragEnabled(true);
        fileTree->setAcceptDrops(true);
        fileTree->setDropIndicatorShown(true);
        fileTree->setDragDropMode(QAbstractItemView::InternalMove);
        
        connect(fileTree, &QTreeWidget::itemDoubleClicked, this, &WebIDE::onTreeItemDoubleClicked);
        connect(fileTree, &QTreeWidget::customContextMenuRequested, this, &WebIDE::showContextMenu);
        leftPanel->addTab(fileTree, "Explorer");

        // Export tab
        QWidget *exportWidget = new QWidget();
        QVBoxLayout *exportLayout = new QVBoxLayout(exportWidget);
        exportLayout->addWidget(new QLabel("Export Files"));
        
        QPushButton *exportBtn = new QPushButton("Export Selected Files");
        connect(exportBtn, &QPushButton::clicked, this, &WebIDE::showExportDialog);
        exportLayout->addWidget(exportBtn);
        
        exportLayout->addStretch();
        leftPanel->addTab(exportWidget, "Export");

        // Settings tab
        QWidget *settingsWidget = new QWidget();
        QVBoxLayout *settingsLayout = new QVBoxLayout(settingsWidget);
        settingsLayout->addWidget(new QLabel("IDE Settings"));
        QPushButton *settingsBtn = new QPushButton("Open Settings");
        connect(settingsBtn, &QPushButton::clicked, this, &WebIDE::showSettings);
        settingsLayout->addWidget(settingsBtn);
        settingsLayout->addStretch();
        leftPanel->addTab(settingsWidget, "Settings");

        mainSplitter->addWidget(leftPanel);

        // Right panel - Editor tabs
        tabWidget = new QTabWidget();
        tabWidget->setTabsClosable(true);
        tabWidget->setMovable(true);
        connect(tabWidget, &QTabWidget::tabCloseRequested, this, &WebIDE::closeTab);
        connect(tabWidget, &QTabWidget::currentChanged, this, &WebIDE::onTabChanged);
        mainSplitter->addWidget(tabWidget);

        mainSplitter->setStretchFactor(1, 1);
        mainLayout->addWidget(mainSplitter);

        setCentralWidget(centralWidget);

        // Status bar
        statusBar()->showMessage("Ready");
    }

    void showContextMenu(const QPoint &pos) {
        QTreeWidgetItem *item = fileTree->itemAt(pos);
        QMenu contextMenu;
        
        if (item) {
            QString itemPath = item->data(0, Qt::UserRole).toString();
            QFileInfo info(itemPath);
            
            if (info.isDir()) {
                // Folder context menu
                contextMenu.addAction(style()->standardIcon(QStyle::SP_FileIcon), "New File in Folder", [this, itemPath]() {
                    createFileInFolder(itemPath);
                });
                contextMenu.addAction(style()->standardIcon(QStyle::SP_DirIcon), "New Folder in Folder", [this, itemPath]() {
                    createFolderInFolder(itemPath);
                });
                contextMenu.addSeparator();
                contextMenu.addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), "Open Folder Location", [itemPath]() {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(itemPath));
                });
                contextMenu.addAction(style()->standardIcon(QStyle::SP_CommandLink), "Open Terminal Here", [this, itemPath]() {
                    openTerminalInPath(itemPath);
                });
                contextMenu.addAction("Copy Path", [this, itemPath]() {
                    QClipboard *clipboard = QApplication::clipboard();
                    clipboard->setText(itemPath);
                    statusBar()->showMessage("Path copied: " + itemPath, 3000);
                });
                contextMenu.addSeparator();
                contextMenu.addAction(style()->standardIcon(QStyle::SP_TrashIcon), "Delete Folder", [this, itemPath]() {
                    deleteItem(itemPath);
                });
            } else {
                // File context menu
                contextMenu.addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), "Open", [this, itemPath]() {
                    openFileInEditor(itemPath);
                });
                
                // Open images in external viewer
                if (info.suffix().toLower() == "jpg" || info.suffix().toLower() == "jpeg" || 
                    info.suffix().toLower() == "png" || info.suffix().toLower() == "gif" || 
                    info.suffix().toLower() == "bmp" || info.suffix().toLower() == "svg") {
                    contextMenu.addAction("Open Image", [itemPath]() {
                        QDesktopServices::openUrl(QUrl::fromLocalFile(itemPath));
                    });
                }
                
                contextMenu.addSeparator();
                contextMenu.addAction("Copy Path", [this, itemPath]() {
                    QClipboard *clipboard = QApplication::clipboard();
                    clipboard->setText(itemPath);
                    statusBar()->showMessage("Path copied: " + itemPath, 3000);
                });
                contextMenu.addAction("Copy File Name", [this, info]() {
                    QClipboard *clipboard = QApplication::clipboard();
                    clipboard->setText(info.fileName());
                    statusBar()->showMessage("File name copied: " + info.fileName(), 3000);
                });
                contextMenu.addSeparator();
                contextMenu.addAction(style()->standardIcon(QStyle::SP_TrashIcon), "Delete File", [this, itemPath]() {
                    deleteItem(itemPath);
                });
            }
        } else {
            // Empty space context menu
            contextMenu.addAction(style()->standardIcon(QStyle::SP_FileIcon), "New File", this, &WebIDE::newFile);
            contextMenu.addAction(style()->standardIcon(QStyle::SP_DirIcon), "New Folder", this, &WebIDE::newFolder);
            contextMenu.addSeparator();
            contextMenu.addAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "Refresh", [this]() {
                if (!currentFolder.isEmpty()) {
                    loadFolderStructure(currentFolder);
                }
            });
        }
        
        contextMenu.exec(fileTree->mapToGlobal(pos));
    }

    void createFileInFolder(const QString &folderPath) {
        bool ok;
        QString fileName = QInputDialog::getText(this, "New File", "File name:", QLineEdit::Normal, "", &ok);
        if (ok && !fileName.isEmpty()) {
            QString filePath = folderPath + "/" + fileName;
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.close();
                loadFolderStructure(currentFolder);
                openFileInEditor(filePath);
                statusBar()->showMessage("Created file: " + fileName, 3000);
            } else {
                QMessageBox::warning(this, "Error", "Could not create file");
            }
        }
    }

    void createFolderInFolder(const QString &folderPath) {
        bool ok;
        QString folderName = QInputDialog::getText(this, "New Folder", "Folder name:", QLineEdit::Normal, "", &ok);
        if (ok && !folderName.isEmpty()) {
            QDir dir(folderPath);
            if (dir.mkdir(folderName)) {
                loadFolderStructure(currentFolder);
                statusBar()->showMessage("Created folder: " + folderName, 3000);
            } else {
                QMessageBox::warning(this, "Error", "Could not create folder");
            }
        }
    }

    void deleteItem(const QString &path) {
        QFileInfo info(path);
        QString message = info.isDir() ? 
            "Are you sure you want to delete this folder and all its contents?" : 
            "Are you sure you want to delete this file?";
        
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete", message,
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            if (info.isDir()) {
                QDir dir(path);
                if (dir.removeRecursively()) {
                    loadFolderStructure(currentFolder);
                    statusBar()->showMessage("Deleted folder: " + info.fileName(), 3000);
                } else {
                    QMessageBox::warning(this, "Error", "Could not delete folder");
                }
            } else {
                if (QFile::remove(path)) {
                    loadFolderStructure(currentFolder);
                    statusBar()->showMessage("Deleted file: " + info.fileName(), 3000);
                } else {
                    QMessageBox::warning(this, "Error", "Could not delete file");
                }
            }
        }
    }

    void createMenuBar() {
        QMenuBar *menuBar = new QMenuBar();

        // File menu
        QMenu *fileMenu = menuBar->addMenu("File");
        fileMenu->addAction("Quick Open Folder", this, &WebIDE::quickOpenFolder, QKeySequence("Ctrl+Shift+O"));
        fileMenu->addAction("Browse Folder...", this, &WebIDE::openFolder, QKeySequence::Open);
        fileMenu->addAction("New File", this, &WebIDE::newFile, QKeySequence::New);
        fileMenu->addAction("New Folder", this, &WebIDE::newFolder);
        fileMenu->addSeparator();
        fileMenu->addAction("Save", this, &WebIDE::saveFile, QKeySequence::Save);
        fileMenu->addAction("Save All", this, &WebIDE::saveAllFiles);
        fileMenu->addSeparator();
        fileMenu->addAction("Export Files", this, &WebIDE::showExportDialog);
        fileMenu->addSeparator();
        fileMenu->addAction("Exit", this, &QWidget::close, QKeySequence::Quit);

        // Edit menu
        QMenu *editMenu = menuBar->addMenu("Edit");
        editMenu->addAction("Undo");
        editMenu->addAction("Redo");
        editMenu->addSeparator();
        editMenu->addAction("Cut");
        editMenu->addAction("Copy");
        editMenu->addAction("Paste");

        // Settings menu
        QMenu *settingsMenu = menuBar->addMenu("Settings");
        settingsMenu->addAction("Preferences", this, &WebIDE::showSettings);

        // Server menu
        QMenu *serverMenu = menuBar->addMenu("Server");
        serverMenu->addAction("Start/Stop Server", this, &WebIDE::startServer);
        serverMenu->addAction("Open in Browser", this, &WebIDE::openInBrowser);
        
        // Terminal menu
        QMenu *terminalMenu = menuBar->addMenu("Terminal");
        terminalMenu->addAction(style()->standardIcon(QStyle::SP_CommandLink), "Open Terminal", this, &WebIDE::openTerminal, QKeySequence("Ctrl+`"));
        terminalMenu->addAction("Open PowerShell", this, &WebIDE::openTerminal);

        // Keywords menu
        QMenu *keywordsMenu = menuBar->addMenu("Keywords");
        keywordsMenu->addAction("Syntax Highlighting Enabled");

        setMenuBar(menuBar);
    }

    void createToolBar() {
        QToolBar *toolBar = addToolBar("Main Toolbar");
        toolBar->setMovable(false);
        toolBar->setIconSize(QSize(24, 24));

        // Quick Open with big button
        QPushButton *quickOpenBtn = new QPushButton(style()->standardIcon(QStyle::SP_DirOpenIcon), " Quick Open");
        quickOpenBtn->setStyleSheet("QPushButton { padding: 5px 15px; font-weight: bold; }");
        connect(quickOpenBtn, &QPushButton::clicked, this, &WebIDE::quickOpenFolder);
        toolBar->addWidget(quickOpenBtn);
        
        toolBar->addSeparator();
        toolBar->addAction(style()->standardIcon(QStyle::SP_FileIcon), "New File", this, &WebIDE::newFile);
        toolBar->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Save", this, &WebIDE::saveFile);
        
        toolBar->addSeparator();
        
        // Terminal button
        QPushButton *terminalBtn = new QPushButton(style()->standardIcon(QStyle::SP_CommandLink), " Terminal");
        connect(terminalBtn, &QPushButton::clicked, this, &WebIDE::openTerminal);
        toolBar->addWidget(terminalBtn);
        
        toolBar->addSeparator();

        // Server controls
        QLabel *portLabel = new QLabel("Port:");
        toolBar->addWidget(portLabel);
        
        portSpinBox = new QSpinBox();
        portSpinBox->setRange(1024, 65535);
        portSpinBox->setValue(8080);
        toolBar->addWidget(portSpinBox);

        serverBtn = new QPushButton("Start Server");
        connect(serverBtn, &QPushButton::clicked, this, &WebIDE::startServer);
        toolBar->addWidget(serverBtn);

        QPushButton *browserBtn = new QPushButton("Open Browser");
        connect(browserBtn, &QPushButton::clicked, this, &WebIDE::openInBrowser);
        toolBar->addWidget(browserBtn);

        toolBar->addSeparator();
        serverStatusLabel = new QLabel("Server: Stopped");
        toolBar->addWidget(serverStatusLabel);
    }

    void applyTheme(bool isDark) {
        if (isDark) {
            applyDarkTheme();
        } else {
            applyLightTheme();
        }
    }

    void applyDarkTheme() {
        qApp->setStyle("Fusion");
        
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(30, 30, 30));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        
        qApp->setPalette(darkPalette);
        
        setStyleSheet(R"(
            QTreeWidget {
                background-color: #252526;
                color: #cccccc;
                border: none;
            }
            QTreeWidget::item:selected {
                background-color: #094771;
            }
            QTreeWidget::item:hover {
                background-color: #2a2d2e;
            }
            QTabWidget::pane {
                border: 1px solid #3c3c3c;
            }
            QTabBar::tab {
                background-color: #2d2d30;
                color: #cccccc;
                padding: 8px 16px;
                border: 1px solid #3c3c3c;
            }
            QTabBar::tab:selected {
                background-color: #1e1e1e;
                border-bottom: 2px solid #007acc;
            }
            QMenuBar {
                background-color: #2d2d30;
                color: #cccccc;
            }
            QMenuBar::item:selected {
                background-color: #3e3e42;
            }
            QMenu {
                background-color: #2d2d30;
                color: #cccccc;
            }
            QMenu::item:selected {
                background-color: #094771;
            }
            QToolBar {
                background-color: #2d2d30;
                border: none;
            }
            QStatusBar {
                background-color: #007acc;
                color: white;
            }
        )");
    }

    void applyLightTheme() {
        qApp->setStyle("Fusion");
        
        QPalette lightPalette;
        lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
        lightPalette.setColor(QPalette::WindowText, Qt::black);
        lightPalette.setColor(QPalette::Base, Qt::white);
        lightPalette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
        lightPalette.setColor(QPalette::ToolTipBase, Qt::white);
        lightPalette.setColor(QPalette::ToolTipText, Qt::black);
        lightPalette.setColor(QPalette::Text, Qt::black);
        lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
        lightPalette.setColor(QPalette::ButtonText, Qt::black);
        lightPalette.setColor(QPalette::BrightText, Qt::red);
        lightPalette.setColor(QPalette::Link, QColor(0, 0, 255));
        lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215));
        lightPalette.setColor(QPalette::HighlightedText, Qt::white);
        
        qApp->setPalette(lightPalette);
        
        setStyleSheet(R"(
            QTreeWidget {
                background-color: #ffffff;
                color: #000000;
                border: 1px solid #cccccc;
            }
            QTreeWidget::item:selected {
                background-color: #0078d4;
                color: white;
            }
            QTreeWidget::item:hover {
                background-color: #e5e5e5;
            }
            QTabWidget::pane {
                border: 1px solid #cccccc;
            }
            QTabBar::tab {
                background-color: #f0f0f0;
                color: #000000;
                padding: 8px 16px;
                border: 1px solid #cccccc;
            }
            QTabBar::tab:selected {
                background-color: #ffffff;
                border-bottom: 2px solid #0078d4;
            }
            QMenuBar {
                background-color: #f0f0f0;
                color: #000000;
            }
            QMenuBar::item:selected {
                background-color: #e0e0e0;
            }
            QMenu {
                background-color: #ffffff;
                color: #000000;
                border: 1px solid #cccccc;
            }
            QMenu::item:selected {
                background-color: #0078d4;
                color: white;
            }
            QToolBar {
                background-color: #f0f0f0;
                border: none;
            }
            QStatusBar {
                background-color: #0078d4;
                color: white;
            }
        )");
    }

    void loadFolderStructure(const QString &path) {
        fileTree->clear();
        QDir dir(path);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(fileTree);
        rootItem->setText(0, dir.dirName());
        rootItem->setData(0, Qt::UserRole, path);
        rootItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        
        addTreeItems(rootItem, path);
        fileTree->expandItem(rootItem);
    }

    void addTreeItems(QTreeWidgetItem *parent, const QString &path) {
        QDir dir(path);
        QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst);
        
        for (const QFileInfo &info : entries) {
            QTreeWidgetItem *item = new QTreeWidgetItem(parent);
            item->setText(0, info.fileName());
            item->setData(0, Qt::UserRole, info.absoluteFilePath());
            
            if (info.isDir()) {
                item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
                addTreeItems(item, info.absoluteFilePath());
            } else {
                // Set file type icon
                QString ext = info.suffix().toLower();
                if (ext == "html" || ext == "htm" || ext == "xhtml") {
                    item->setIcon(0, style()->standardIcon(QStyle::SP_FileDialogDetailedView));
                } else if (ext == "css") {
                    item->setIcon(0, style()->standardIcon(QStyle::SP_FileDialogContentsView));
                } else if (ext == "js") {
                    item->setIcon(0, style()->standardIcon(QStyle::SP_CommandLink));
                } else if (ext == "php") {
                    item->setIcon(0, style()->standardIcon(QStyle::SP_ComputerIcon));
                } else if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "gif" || ext == "bmp" || ext == "svg") {
                    item->setIcon(0, style()->standardIcon(QStyle::SP_FileDialogInfoView));
                } else if (ext == "json" || ext == "xml") {
                    item->setIcon(0, style()->standardIcon(QStyle::SP_FileDialogListView));
                } else if (ext == "txt" || ext == "md") {
                    item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
                } else {
                    item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
                }
            }
        }
    }

    void onTabChanged(int index) {
        Q_UNUSED(index);
        updateImportPanel();
    }

    void openFileInEditor(const QString &filePath) {
        // Check if file is already open
        for (int i = 0; i < tabWidget->count(); ++i) {
            if (tabWidget->tabToolTip(i) == filePath) {
                tabWidget->setCurrentIndex(i);
                updateImportPanel();
                return;
            }
        }

        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QFileInfo fileInfo(filePath);
            QString ext = fileInfo.suffix().toLower();
            
            CodeEditor *editor = new CodeEditor(ext);
            editor->setPlainText(QString::fromUtf8(file.readAll()));
            editor->applyTheme(isDarkTheme);
            file.close();

            int index = tabWidget->addTab(editor, fileInfo.fileName());
            tabWidget->setTabToolTip(index, filePath);
            tabWidget->setCurrentIndex(index);
            
            connect(editor, &CodeEditor::requestImportPanel, this, &WebIDE::updateImportPanel);
            updateImportPanel();
        }
    }

    void updateImportPanel() {
        // Clear previous content
        QLayoutItem *item;
        while ((item = importLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        
        if (tabWidget->currentIndex() >= 0) {
            QString filePath = tabWidget->tabToolTip(tabWidget->currentIndex());
            QFileInfo fileInfo(filePath);
            QString ext = fileInfo.suffix().toLower();
            
            if (ext == "html" || ext == "htm" || ext == "xhtml" || ext == "xhtm" || ext == "htma") {
                showHTMLImports();
            } else {
                QLabel *noImport = new QLabel("No imports available for this file type");
                noImport->setAlignment(Qt::AlignCenter);
                importLayout->addWidget(noImport);
            }
        } else {
            QLabel *noFile = new QLabel("Open an HTML file to see import options");
            noFile->setAlignment(Qt::AlignCenter);
            importLayout->addWidget(noFile);
        }
    }

    void showHTMLImports() {
        QLabel *titleLabel = new QLabel("<b>Import Libraries</b>");
        importLayout->addWidget(titleLabel);
        
        // Vue.js
        QPushButton *vueBtn = new QPushButton("Vue.js 3");
        connect(vueBtn, &QPushButton::clicked, [this]() {
            insertImport("<script src=\"https://unpkg.com/vue@3/dist/vue.global.js\"></script>");
        });
        importLayout->addWidget(vueBtn);
        
        // jQuery
        QPushButton *jqueryBtn = new QPushButton("jQuery");
        connect(jqueryBtn, &QPushButton::clicked, [this]() {
            insertImport("<script src=\"https://code.jquery.com/jquery-3.7.1.min.js\"></script>");
        });
        importLayout->addWidget(jqueryBtn);
        
        // Bootstrap
        QLabel *bootstrapLabel = new QLabel("<b>Bootstrap 5</b>");
        importLayout->addWidget(bootstrapLabel);
        
        QPushButton *bootstrapCSSBtn = new QPushButton("Bootstrap CSS");
        connect(bootstrapCSSBtn, &QPushButton::clicked, [this]() {
            insertImport("<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css\" rel=\"stylesheet\">");
        });
        importLayout->addWidget(bootstrapCSSBtn);
        
        QPushButton *bootstrapJSBtn = new QPushButton("Bootstrap JS");
        connect(bootstrapJSBtn, &QPushButton::clicked, [this]() {
            insertImport("<script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js\"></script>");
        });
        importLayout->addWidget(bootstrapJSBtn);
        
        // Tailwind CSS
        QPushButton *tailwindBtn = new QPushButton("Tailwind CSS");
        connect(tailwindBtn, &QPushButton::clicked, [this]() {
            insertImport("<script src=\"https://cdn.tailwindcss.com\"></script>");
        });
        importLayout->addWidget(tailwindBtn);
        
        // PHP note
        QLabel *phpLabel = new QLabel("<b>PHP</b>");
        importLayout->addWidget(phpLabel);
        
        QPushButton *phpBtn = new QPushButton("Insert PHP Tag");
        connect(phpBtn, &QPushButton::clicked, [this]() {
            insertImport("<?php\n\n?>");
        });
        importLayout->addWidget(phpBtn);
        
        importLayout->addStretch();
    }

    void insertImport(const QString &code) {
        if (tabWidget->currentIndex() >= 0) {
            CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->currentWidget());
            if (editor) {
                QTextCursor cursor = editor->textCursor();
                
                // Try to insert in <head> section
                QString content = editor->toPlainText();
                int headPos = content.indexOf("</head>");
                
                if (headPos != -1) {
                    cursor.setPosition(headPos);
                    cursor.insertText("    " + code + "\n");
                } else {
                    // Insert at cursor position
                    cursor.insertText(code + "\n");
                }
                
                editor->setTextCursor(cursor);
                statusBar()->showMessage("Imported: " + code.left(50) + "...", 3000);
            }
        }
    }

    void setupServer() {
        server = new QTcpServer(this);
        connect(server, &QTcpServer::newConnection, this, &WebIDE::handleNewConnection);
    }

    void loadSettings() {
        QSettings settings("WebIDE", "Settings");
        serverPort = settings.value("serverPort", 8080).toInt();
        isDarkTheme = settings.value("isDarkTheme", true).toBool();
        portSpinBox->setValue(serverPort);
    }

    void saveSettings() {
        QSettings settings("WebIDE", "Settings");
        settings.setValue("serverPort", portSpinBox->value());
        settings.setValue("isDarkTheme", isDarkTheme);
    }

    QTabWidget *leftPanel;
    QTreeWidget *fileTree;
    QTabWidget *tabWidget;
    QPushButton *serverBtn;
    QSpinBox *portSpinBox;
    QLabel *serverStatusLabel;
    QTcpServer *server;
    QString currentFolder;
    int serverPort;
    bool isDarkTheme;
    QWidget *importPanel;
    QVBoxLayout *importLayout;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    WebIDE ide;
    ide.show();
    
    return app.exec();
}

#include "main.moc"