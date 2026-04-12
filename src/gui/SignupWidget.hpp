#pragma once
#include <QWidget>
#include <auth/auth.hpp>

class QLineEdit;
class QPushButton;
class QLabel;

class SignupWidget : public QWidget {
    Q_OBJECT
public:
    explicit SignupWidget(AuthManager& auth, QWidget* parent = nullptr);

signals:
    void signupDone();
    void goToLogin();

private slots:
    void onSignup();

private:
    AuthManager& m_auth;
    QLineEdit*   m_username;
    QLineEdit*   m_email;
    QLineEdit*   m_password;
    QLineEdit*   m_confirm;
    QPushButton* m_signupBtn;
    QLabel*      m_error;
};
