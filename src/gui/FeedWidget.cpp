#include "FeedWidget.hpp"
#include "QuestionCard.hpp"
#include "ThreadDialog.hpp"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>

FeedWidget::FeedWidget(QuestionManager& qm, QWidget* parent)
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

    m_emptyLabel = new QLabel("No answered questions yet.");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: #aaa; font-size: 15px; padding: 60px;");
    m_emptyLabel->hide();
    m_cardsLayout->addWidget(m_emptyLabel);
    m_cardsLayout->addStretch();

    scroll->setWidget(m_cardsContainer);
    root->addWidget(scroll);
}

void FeedWidget::setSession(const QString& token, const QString& username) {
    m_token = token; m_username = username; refresh();
}

void FeedWidget::refresh() {
    while (m_cardsLayout->count() > 2) {
        auto* item = m_cardsLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }
    if (m_username.isEmpty()) return;

    auto questions = m_qm.getFeed(m_username.toStdString());
    if (questions.empty()) { m_emptyLabel->show(); return; }
    m_emptyLabel->hide();

    int idx = 0;
    for (const auto& q : questions) {
        auto* card = new QuestionCard(q, m_username, QuestionCard::Mode::Feed, m_cardsContainer);
        m_cardsLayout->insertWidget(idx++, card);

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
