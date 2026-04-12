#pragma once
#include <QMainWindow>
#include <storage/FileStore.hpp>
#include <auth/auth.hpp>
#include <questionManager/questionManager.hpp>

class QStackedWidget;
class LoginWidget;
class SignupWidget;
class HomeWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(FileStore& store,
                        AuthManager& auth,
                        QuestionManager& qm,
                        QWidget* parent = nullptr);
public slots:
    void showLogin();
    void showSignup();
    void showHome(const QString& token, const QString& username);

private:
    QStackedWidget*  m_stack;
    LoginWidget*     m_login;
    SignupWidget*    m_signup;
    HomeWidget*      m_home;
    FileStore&       m_store;
    AuthManager&     m_auth;
    QuestionManager& m_qm;
};
