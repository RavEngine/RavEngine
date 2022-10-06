#include "ui_DemoParser.h"
#include "parser/Parser.h"
#include "parser/ParserListener.h"
#include <QApplication>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <QTextBlock>
#include <QDebug>

class Application : public QApplication, public sfz::Parser::Listener {
public:
    Application(int& argc, char *argv[]) : QApplication(argc, argv) {}
    void init();

private:
    void requestParseCheck();
    void runParseCheck();

private:
    void onParseBegin() override;
    void onParseEnd() override;
    void onParseHeader(const sfz::SourceRange& range, const std::string& header) override;
    void onParseOpcode(const sfz::SourceRange& rangeOpcode, const sfz::SourceRange& rangeValue, const std::string& name, const std::string& value) override;
    void onParseError(const sfz::SourceRange& range, const std::string& message) override;
    void onParseWarning(const sfz::SourceRange& range, const std::string& message) override;

    static QTextCursor selectSourceRange(QTextDocument* doc, const sfz::SourceRange& range);

private:
    sfz::Parser _parser;
    Ui::MainWindow _ui;
    QTimer* _recheckTimer = nullptr;
    bool _blockTextChanged = false;
};

static const char defaultSfzText[] = R"SFZ(
//----------------------------------------------------------------------------//
// This is a SFZ test file with many problems.                                //
//----------------------------------------------------------------------------//

/*
 * This is a block comment. Not all the SFZ players accept it.
 * It can span over multiple lines.
*/

// opcode without header
not_in_header=on    // warning

// invalid headers
<> // empty
<ab@cd> // bad identifier

<region>
sample=*sine key=69
sample=My Directory/My Wave.wav // path with spaces
sample=My Directory/My Wave.wav key=69 // path with spaces, and other opcode following
sample=Foo=Bar.wav // path invalid: it cannot contain the '=' sign

#include "FileWhichDoesNotExist.sfz"

// malformed includes
#include "MyFileWhichDoesNotExist1.sfz
#include MyFileWhichDoesNotExist1.sfz"

// #define with some bad variable names
#define $foo 1234
#define Foo 1234
#define $ 1234

// #define with empty expansion, accepted
#define $foo

// expansion
abc$foo=1
abcdef=$foo

// expansion of undefined variables
abc$toto=1
abcdef=$tata

// opcode name which expands to invalid identifier
$titi=1

volume=10 /*
block comments at the end of line
*/

/* unterminated block comment
)SFZ";

void Application::init()
{
    QMainWindow* window = new QMainWindow;
    _ui.setupUi(window);

    _recheckTimer = new QTimer;
    _recheckTimer->setSingleShot(true);
    _recheckTimer->setInterval(200);
    connect(
        _recheckTimer, &QTimer::timeout,
        this, [this]() { runParseCheck(); });

    _ui.sfzEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    _ui.sfzEdit->setPlainText(QString::fromLatin1(defaultSfzText));
    connect(
        _ui.sfzEdit, &QTextEdit::textChanged,
        this, [this]() { if (!_blockTextChanged) requestParseCheck(); });

    _ui.messageTable->setColumnCount(3);
    _ui.messageTable->setHorizontalHeaderLabels(QStringList() << tr("Type") << tr("Line") << tr("Message"));
    _ui.messageTable->horizontalHeader()->setStretchLastSection(true);

    _ui.splitter->setStretchFactor(0, 3);
    _ui.splitter->setStretchFactor(1, 1);

    window->show();

    _parser.setListener(this);
    requestParseCheck();
}

void Application::requestParseCheck()
{
    _recheckTimer->start();
}

void Application::runParseCheck()
{
    QByteArray code = _ui.sfzEdit->toPlainText().toLatin1();
    _blockTextChanged = true;
    _parser.parseString("/virtual.sfz", absl::string_view(code.data(), code.size()));
    _blockTextChanged = false;
}

void Application::onParseBegin()
{
    QTextDocument* doc = _ui.sfzEdit->document();
    QTextCursor cur(doc);
    cur.movePosition(QTextCursor::Start);
    cur.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QTextCharFormat cfmt;
    cur.setCharFormat(cfmt);

    QTableWidget* t = _ui.messageTable;
    while (t->rowCount() > 0)
        t->removeRow(t->rowCount() - 1);
}

void Application::onParseEnd()
{
}

void Application::onParseHeader(const sfz::SourceRange& range, const std::string& header)
{
    (void)header;

    QTextDocument* doc = _ui.sfzEdit->document();
    QTextCursor cur = selectSourceRange(doc, range);
    QTextCharFormat cfmt;
    cfmt.setForeground(QColor::fromRgb(0x4E9A06));
    cur.mergeCharFormat(cfmt);
}

void Application::onParseOpcode(const sfz::SourceRange& rangeOpcode, const sfz::SourceRange& rangeValue, const std::string& name, const std::string& value)
{
    (void)name;
    (void)value;

    QTextDocument* doc = _ui.sfzEdit->document();
    QTextCursor curOpcode = selectSourceRange(doc, rangeOpcode);
    QTextCursor curValue = selectSourceRange(doc, rangeValue);
    QTextCharFormat cfmtOpcode;
    cfmtOpcode.setForeground(QColor::fromRgb(0x75507B));
    QTextCharFormat cfmtValue;
    cfmtValue.setForeground(QColor::fromRgb(0x3465A4));
    curOpcode.mergeCharFormat(cfmtOpcode);
    curValue.mergeCharFormat(cfmtValue);
}

void Application::onParseError(const sfz::SourceRange& range, const std::string& message)
{
    QTableWidget* t = _ui.messageTable;
    int row = t->rowCount();
    t->insertRow(row);

    // QString path = QString::fromStdString(*range.start.filePath);
    // QString file = QFileInfo(path).fileName();
    t->setItem(row, 0, new QTableWidgetItem(tr("Error")));
    t->setItem(row, 1, new QTableWidgetItem(QString::number(range.start.lineNumber)));
    t->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(message)));

    QTextDocument* doc = _ui.sfzEdit->document();
    QTextCursor cur = selectSourceRange(doc, range);
    QTextCharFormat cfmt;
    cfmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    cfmt.setUnderlineColor(Qt::red);
    cur.mergeCharFormat(cfmt);
}

void Application::onParseWarning(const sfz::SourceRange& range, const std::string& message)
{
    QTableWidget* t = _ui.messageTable;
    int row = t->rowCount();
    t->insertRow(row);

    // QString path = QString::fromStdString(*range.start.filePath);
    // QString file = QFileInfo(path).fileName();
    t->setItem(row, 0, new QTableWidgetItem(tr("Warning")));
    t->setItem(row, 1, new QTableWidgetItem(QString::number(range.start.lineNumber)));
    t->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(message)));

    QTextDocument* doc = _ui.sfzEdit->document();
    QTextCursor cur = selectSourceRange(doc, range);
    QTextCharFormat cfmt;
    cfmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    cfmt.setUnderlineColor(Qt::gray);
    cur.mergeCharFormat(cfmt);
}

QTextCursor Application::selectSourceRange(QTextDocument* doc, const sfz::SourceRange& range)
{
    QTextCursor cur(doc->findBlockByLineNumber(range.start.lineNumber));
    cur.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, range.start.columnNumber);
    cur.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, range.end.lineNumber - range.start.lineNumber);
    cur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, range.end.columnNumber);
    return cur;
}

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    app.init();
    return app.exec();
}
