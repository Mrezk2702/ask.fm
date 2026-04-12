#include "SignupWidget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

static const QString FIELD_SU = R"(
    QLineEdit {
        border: 1px solid #ddd; border-radius: 8px;
        padding: 10px 14px; font-size: 14px; color: #333; background: white;
    }
    QLineEdit:focus { border-color: #1a8fe3; }
)";

SignupWidget::SignupWidget(AuthManager& auth, QWidget* parent)
    : QWidget(parent), m_auth(auth)
{
    setStyleSheet("background-color: #f0f2f5;");
    auto* outer = new QVBoxLayout(this);
    outer->setAlignment(Qt::AlignCenter);

    auto* card = new QFrame;
    card->setFixedWidth(420);
    card->setStyleSheet("QFrame { background: white; border-radius: 14px; border: 1px solid #e0e0e0; }");

    auto* cl = new QVBoxLayout(card);
    cl->setContentsMargins(44, 40, 44, 40);
    cl->setSpacing(12);

    auto* logo = new QLabel("ask.fm");
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet("font-size: 36px; font-weight: bold; color: #1a8fe3; border: none;");
    cl->addWidget(logo);

    auto* sub = new QLabel("Create your account");
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

    m_username = new QLineEdit; m_username->setPlaceholderText("Username"); m_username->setStyleSheet(FIELD_SU); cl->addWidget(m_username);
    m_email    = new QLineEdit; m_email->setPlaceholderText("Email");       m_email->setStyleSheet(FIELD_SU);    cl->addWidget(m_email);
    m_password = new QLineEdit; m_password->setPlaceholderText("Password"); m_password->setEchoMode(QLineEdit::Password); m_password->setStyleSheet(FIELD_SU); cl->addWidget(m_password);
    m_confirm  = new QLineEdit; m_confirm->setPlaceholderText("Confirm Password"); m_confirm->setEchoMode(QLineEdit::Password); m_confirm->setStyleSheet(FIELD_SU); cl->addWidget(m_confirm);

    m_signupBtn = new QPushButton("Create Account");
    m_signupBtn->setStyleSheet(R"(
        QPushButton { background: #27ae60; color: white; border: none; border-radius: 8px;
                      padding: 12px; font-size: 15px; font-weight: bold; }
        QPushButton:hover   { background: #219a52; }
        QPushButton:pressed { background: #1a7a42; }
    )");
    cl->addWidget(m_signupBtn);

    auto* row = new QHBoxLayout;
    auto* lbl = new QLabel("Already have an account?");
    lbl->setStyleSheet("color: #888; font-size: 13px; border: none;");
    auto* link = new QPushButton("Sign In");
    link->setFlat(true);
    link->setCursor(Qt::PointingHandCursor);
    link->setStyleSheet("color: #1a8fe3; font-size: 13px; font-weight: bold; border: none;");
    row->addStretch(); row->addWidget(lbl); row->addWidget(link); row->addStretch();
    cl->addLayout(row);

    outer->addWidget(card);

    connect(m_signupBtn, &QPushButton::clicked,     this, &SignupWidget::onSignup);
    connect(m_confirm,   &QLineEdit::returnPressed, this, &SignupWidget::onSignup);
    connect(link,        &QPushButton::clicked,     this, &SignupWidget::goToLogin);
}

void SignupWidget::onSignup() {
    QString user  = m_username->text().trimmed();
    QString email = m_email->text().trimmed();
    QString pass  = m_password->text();
    QString conf  = m_confirm->text();

    if (user.isEmpty() || email.isEmpty() || pass.isEmpty() || conf.isEmpty()) {
        m_error->setText("Please fill in all fields."); m_error->show(); return;
    }
    if (pass != conf) {
        m_error->setText("Passwords do not match."); m_error->show(); return;
    }
    if (pass.length() < 6) {
        m_error->setText("Password must be at least 6 characters."); m_error->show(); return;
    }
    if (!m_auth.signup(user.toStdString(), pass.toStdString(), email.toStdString())) {
        m_error->setText("Username already taken."); m_error->show(); return;
    }
    m_error->hide();
    m_username->clear(); m_email->clear(); m_password->clear(); m_confirm->clear();
    emit signupDone();
}
