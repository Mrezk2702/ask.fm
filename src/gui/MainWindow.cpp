#include "MainWindow.hpp"
#include "LoginWidget.hpp"
#include "SignupWidget.hpp"
#include "HomeWidget.hpp"
#include <QStackedWidget>

MainWindow::MainWindow(FileStore& store, AuthManager& auth,
                       QuestionManager& qm, QWidget* parent)
    : QMainWindow(parent), m_store(store), m_auth(auth), m_qm(qm)
{
    setWindowTitle("ask.fm");
    setMinimumSize(820, 600);
    resize(960, 700);

    m_stack  = new QStackedWidget(this);
    m_login  = new LoginWidget(m_auth);
    m_signup = new SignupWidget(m_auth);
    m_home   = new HomeWidget(m_auth, m_qm);

    m_stack->addWidget(m_login);   // 0
    m_stack->addWidget(m_signup);  // 1
    m_stack->addWidget(m_home);    // 2
    setCentralWidget(m_stack);

    connect(m_login,  &LoginWidget::loginSuccess, this, &MainWindow::showHome);
    connect(m_login,  &LoginWidget::goToSignup,   this, &MainWindow::showSignup);
    connect(m_signup, &SignupWidget::signupDone,   this, &MainWindow::showLogin);
    connect(m_signup, &SignupWidget::goToLogin,    this, &MainWindow::showLogin);
    connect(m_home,   &HomeWidget::loggedOut,      this, &MainWindow::showLogin);

    showLogin();
}

void MainWindow::showLogin()  { m_stack->setCurrentIndex(0); }
void MainWindow::showSignup() { m_stack->setCurrentIndex(1); }
void MainWindow::showHome(const QString& token, const QString& username) {
    m_home->setSession(token, username);
    m_stack->setCurrentIndex(2);
}
