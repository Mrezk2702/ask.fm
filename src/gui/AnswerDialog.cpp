#include "AnswerDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>

AnswerDialog::AnswerDialog(QuestionManager& qm, const QString& token,
                           const QString& questionId, const QString& questionBody,
                           QWidget* parent)
    : QDialog(parent), m_qm(qm), m_token(token), m_questionId(questionId)
{
    setWindowTitle("Answer Question");
    setMinimumWidth(460);
    setStyleSheet("background: #f0f2f5;");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(12);

    auto* title = new QLabel("Answer this question");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #1a8fe3;");
    layout->addWidget(title);

    auto* qBox = new QLabel(questionBody);
    qBox->setWordWrap(true);
    qBox->setStyleSheet(R"(
        color:#333; font-size:14px; background:white;
        border:1px solid #e8ecef; border-radius:8px; padding:12px;
    )");
    layout->addWidget(qBox);

    m_error = new QLabel;
    m_error->setWordWrap(true);
    m_error->setStyleSheet(
        "color:white; background:#e74c3c; border-radius:6px; padding:8px; font-size:13px;");
    m_error->hide();
    layout->addWidget(m_error);

    layout->addWidget(new QLabel("Your answer:"));

    m_answer = new QTextEdit;
    m_answer->setPlaceholderText("Write your answer here...");
    m_answer->setFixedHeight(110);
    m_answer->setStyleSheet(R"(
        QTextEdit { border:1px solid #ddd; border-radius:8px; padding:8px 12px;
                    font-size:14px; background:white; color:#333; }
        QTextEdit:focus { border-color:#1a8fe3; }
    )");
    layout->addWidget(m_answer);

    auto* btnRow = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setStyleSheet(R"(
        QPushButton { background:#f0f2f5; color:#555; border:1px solid #ddd;
                      border-radius:8px; padding:9px 20px; font-size:14px; }
        QPushButton:hover { background:#e2e6ea; }
    )");
    m_submitBtn = new QPushButton("Post Answer");
    m_submitBtn->setStyleSheet(R"(
        QPushButton { background:#27ae60; color:white; border:none;
                      border-radius:8px; padding:9px 20px; font-size:14px; font-weight:bold; }
        QPushButton:hover   { background:#219a52; }
        QPushButton:pressed { background:#1a7a42; }
    )");
    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(m_submitBtn);
    layout->addLayout(btnRow);

    connect(cancelBtn,   &QPushButton::clicked, this, &QDialog::reject);
    connect(m_submitBtn, &QPushButton::clicked, this, &AnswerDialog::onAnswer);
}

void AnswerDialog::onAnswer() {
    QString ans = m_answer->toPlainText().trimmed();
    if (ans.isEmpty()) { m_error->setText("Answer cannot be empty."); m_error->show(); return; }

    if (!m_qm.answerQuestion(m_token.toStdString(), m_questionId.toStdString(), ans.toStdString())) {
        m_error->setText("Failed to post answer. Please try again."); m_error->show(); return;
    }
    emit answered();
    accept();
}
