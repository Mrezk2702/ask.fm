#include "AskDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

AskDialog::AskDialog(QuestionManager& qm, const QString& token,
                     const QString& targetUser, QWidget* parent)
    : QDialog(parent), m_qm(qm), m_token(token)
{
    setWindowTitle("Ask a Question");
    setMinimumWidth(460);
    setStyleSheet("background: #f0f2f5;");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(12);

    auto* title = new QLabel("Ask a Question");
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #1a8fe3;");
    layout->addWidget(title);

    m_error = new QLabel;
    m_error->setWordWrap(true);
    m_error->setStyleSheet(
        "color: white; background: #e74c3c; border-radius: 6px; padding: 8px; font-size: 13px;");
    m_error->hide();
    layout->addWidget(m_error);

    layout->addWidget(new QLabel("Ask to (username):"));

    m_targetUser = new QLineEdit;
    m_targetUser->setPlaceholderText("Enter username");
    m_targetUser->setText(targetUser);
    m_targetUser->setStyleSheet(R"(
        QLineEdit { border:1px solid #ddd; border-radius:8px; padding:9px 13px;
                    font-size:14px; background:white; color:#333; }
        QLineEdit:focus { border-color:#1a8fe3; }
    )");
    layout->addWidget(m_targetUser);

    layout->addWidget(new QLabel("Your question:"));

    m_body = new QTextEdit;
    m_body->setPlaceholderText("What do you want to ask?");
    m_body->setFixedHeight(100);
    m_body->setStyleSheet(R"(
        QTextEdit { border:1px solid #ddd; border-radius:8px; padding:8px 12px;
                    font-size:14px; background:white; color:#333; }
        QTextEdit:focus { border-color:#1a8fe3; }
    )");
    layout->addWidget(m_body);

    m_anonymous = new QCheckBox("Send anonymously");
    m_anonymous->setStyleSheet("color: #555; font-size: 13px;");
    layout->addWidget(m_anonymous);

    auto* btnRow = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setStyleSheet(R"(
        QPushButton { background:#f0f2f5; color:#555; border:1px solid #ddd;
                      border-radius:8px; padding:9px 20px; font-size:14px; }
        QPushButton:hover { background:#e2e6ea; }
    )");
    m_askBtn = new QPushButton("Ask");
    m_askBtn->setStyleSheet(R"(
        QPushButton { background:#1a8fe3; color:white; border:none;
                      border-radius:8px; padding:9px 24px; font-size:14px; font-weight:bold; }
        QPushButton:hover   { background:#1578c2; }
        QPushButton:pressed { background:#1265a8; }
    )");
    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(m_askBtn);
    layout->addLayout(btnRow);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_askBtn,  &QPushButton::clicked, this, &AskDialog::onAsk);
}

void AskDialog::onAsk() {
    QString to   = m_targetUser->text().trimmed();
    QString body = m_body->toPlainText().trimmed();
    if (to.isEmpty())   { m_error->setText("Please enter a username."); m_error->show(); return; }
    if (body.isEmpty()) { m_error->setText("Question cannot be empty."); m_error->show(); return; }

    std::string id = m_qm.askQuestion(
        m_token.toStdString(), to.toStdString(), body.toStdString(), m_anonymous->isChecked());

    if (id.empty()) { m_error->setText("Failed — check the username is correct."); m_error->show(); return; }
    emit questionAsked();
    accept();
}
