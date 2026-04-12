#include "InboxWidget.hpp"
#include "QuestionCard.hpp"
#include "AnswerDialog.hpp"
#include "ThreadDialog.hpp"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>

InboxWidget::InboxWidget(QuestionManager& qm, QWidget* parent)
    : QWidget(parent), m_qm(qm)
{
    setStyleSheet("background: #f0f2f5;");
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background: #f0f2f5;");

    m_cardsContainer = new QWidget;
    m_cardsContainer->setStyleSheet("background: #f0f2f5;");
    m_cardsLayout = new QVBoxLayout(m_cardsContainer);
    m_cardsLayout->setContentsMargins(24, 20, 24, 20);
    m_cardsLayout->setSpacing(10);

    m_emptyLabel = new QLabel("Your inbox is empty. No new questions yet!");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: #aaa; font-size: 15px; padding: 60px;");
    m_emptyLabel->hide();
    m_cardsLayout->addWidget(m_emptyLabel);
    m_cardsLayout->addStretch();

    scroll->setWidget(m_cardsContainer);
    root->addWidget(scroll);
}

void InboxWidget::setSession(const QString& token, const QString& username) {
    m_token = token; m_username = username; refresh();
}

void InboxWidget::refresh() {
    while (m_cardsLayout->count() > 2) {
        auto* item = m_cardsLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }
    if (m_token.isEmpty()) return;

    auto questions = m_qm.getInbox(m_token.toStdString());
    if (questions.empty()) { m_emptyLabel->show(); return; }
    m_emptyLabel->hide();

    int idx = 0;
    for (const auto& q : questions) {
        auto* card = new QuestionCard(q, m_username, QuestionCard::Mode::Inbox, m_cardsContainer);
        m_cardsLayout->insertWidget(idx++, card);

        connect(card, &QuestionCard::answerRequested, this,
                [this](const QString& qid, const QString& body) {
            auto* dlg = new AnswerDialog(m_qm, m_token, qid, body, this);
            connect(dlg, &AnswerDialog::answered, this, &InboxWidget::refresh);
            dlg->exec(); dlg->deleteLater();
        });

        connect(card, &QuestionCard::deleteRequested, this, [this](const QString& qid) {
            if (QMessageBox::question(this, "Delete", "Delete this question?",
                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                m_qm.deleteQuestion(m_token.toStdString(), qid.toStdString());
                refresh();
            }
        });

        connect(card, &QuestionCard::viewThreadRequested, this, [this](const QString& qid) {
            auto* dlg = new ThreadDialog(m_qm, m_token, qid, m_username, this);
            dlg->exec(); dlg->deleteLater(); refresh();
        });
    }
}
