#include "LoginWidget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

static const QString FIELD = R"(
    QLineEdit {
        border: 1px solid #ddd; border-radius: 8px;
        padding: 10px 14px; font-size: 14px; color: #333; background: white;
    }
    QLineEdit:focus { border-color: #1a8fe3; }
)";

LoginWidget::LoginWidget(AuthManager& auth, QWidget* parent)
    : QWidget(parent), m_auth(auth)
{
    setStyleSheet("background-color: #f0f2f5;");
    auto* outer = new QVBoxLayout(this);
    outer->setAlignment(Qt::AlignCenter);

    auto* card = new QFrame;
    card->setFixedWidth(420);
    card->setStyleSheet("QFrame { background: white; border-radius: 14px; border: 1px solid #e0e0e0; }");

    auto* cl = new QVBoxLayout(card);
    cl->setContentsMargins(44, 44, 44, 44);
    cl->setSpacing(14);

    auto* logo = new QLabel("ask.fm");
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet("font-size: 36px; font-weight: bold; color: #1a8fe3; border: none;");
    cl->addWidget(logo);

    auto* sub = new QLabel("Sign in to your account");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color: #888; font-size: 13px; border: none; margin-bottom: 6px;");
    cl->addWidget(sub);

    m_error = new QLabel;
    m_error->setAlignment(Qt::AlignCenter);
    m_error->setWordWrap(true);
    m_error->setStyleSheet(
        "color: white; background: #e74c3c; border-radius: 6px; padding: 8px; font-size: 13px; border: none;");
    m_error->hide();
    cl->addWidget(m_error);

    m_username = new QLineEdit;
    m_username->setPlaceholderText("Username");
    m_username->setStyleSheet(FIELD);
    cl->addWidget(m_username);

    m_password = new QLineEdit;
    m_password->setPlaceholderText("Password");
    m_password->setEchoMode(QLineEdit::Password);
    m_password->setStyleSheet(FIELD);
    cl->addWidget(m_password);

    m_loginBtn = new QPushButton("Sign In");
    m_loginBtn->setStyleSheet(R"(
        QPushButton { background: #1a8fe3; color: white; border: none; border-radius: 8px;
                      padding: 12px; font-size: 15px; font-weight: bold; }
        QPushButton:hover   { background: #1578c2; }
        QPushButton:pressed { background: #1265a8; }
    )");
    cl->addWidget(m_loginBtn);

    auto* row = new QHBoxLayout;
    auto* lbl = new QLabel("Don't have an account?");
    lbl->setStyleSheet("color: #888; font-size: 13px; border: none;");
    auto* link = new QPushButton("Sign Up");
    link->setFlat(true);
    link->setCursor(Qt::PointingHandCursor);
    link->setStyleSheet("color: #1a8fe3; font-size: 13px; font-weight: bold; border: none;");
    row->addStretch();
    row->addWidget(lbl);
    row->addWidget(link);
    row->addStretch();
    cl->addLayout(row);

    outer->addWidget(card);

    connect(m_loginBtn, &QPushButton::clicked,     this, &LoginWidget::onLogin);
    connect(m_password, &QLineEdit::returnPressed, this, &LoginWidget::onLogin);
    connect(link,       &QPushButton::clicked,     this, &LoginWidget::goToSignup);
}

void LoginWidget::onLogin() {
    QString user = m_username->text().trimmed();
    QString pass = m_password->text();
    if (user.isEmpty() || pass.isEmpty()) {
        m_error->setText("Please fill in all fields.");
        m_error->show(); return;
    }
    auto token = m_auth.login(user.toStdString(), pass.toStdString());
    if (!token) {
        m_error->setText("Invalid username or password.");
        m_error->show();
        m_password->clear(); return;
    }
    m_error->hide();
    m_password->clear();
    emit loginSuccess(QString::fromStdString(*token), user);
}
