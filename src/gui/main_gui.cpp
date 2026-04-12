#include <QApplication>
#include <QFont>
#include "MainWindow.hpp"
#include <storage/FileStore.hpp>
#include <auth/auth.hpp>
#include <questionManager/questionManager.hpp>
#include <filesystem>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("ask.fm");
    app.setOrganizationName("askfm");

    app.setStyleSheet(R"(
        QScrollBar:vertical {
            background: #f0f2f5; width: 8px; border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #ccc; border-radius: 4px; min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background: #aaa; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar::add-page:vertical,  QScrollBar::sub-page:vertical { background: none; }
    )");

    namespace fs = std::filesystem;
    FileStore       store(fs::current_path() / "data");
    AuthManager     auth(store);
    QuestionManager qm(store, auth);

    MainWindow window(store, auth, qm);
    window.show();
    return app.exec();
}
