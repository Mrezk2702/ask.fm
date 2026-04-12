#pragma once
#include <QWidget>
#include <auth/auth.hpp>

class QLineEdit;
class QPushButton;
class QLabel;

class LoginWidget : public QWidget {
    Q_OBJECT
public:
    explicit LoginWidget(AuthManager& auth, QWidget* parent = nullptr);

signals:
    void loginSuccess(const QString& token, const QString& username);
    void goToSignup();

private slots:
    void onLogin();

private:
    AuthManager& m_auth;
    QLineEdit*   m_username;
    QLineEdit*   m_password;
    QPushButton* m_loginBtn;
    QLabel*      m_error;
};
