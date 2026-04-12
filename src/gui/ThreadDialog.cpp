#include "ThreadDialog.hpp"
#include "QuestionCard.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

ThreadDialog::ThreadDialog(QuestionManager& qm, const QString& token,
                           const QString& questionId, const QString& currentUser,
                           QWidget* parent)
    : QDialog(parent), m_qm(qm), m_token(token),
      m_questionId(questionId), m_currentUser(currentUser)
{
    setWindowTitle("Thread View");
    setMinimumSize(560, 560);
    setStyleSheet("background: #f0f2f5;");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Header
    auto* header = new QWidget;
    header->setStyleSheet("background: #1a8fe3;");
    auto* hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(20, 14, 20, 14);
    auto* hTitle = new QLabel("Thread");
    hTitle->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    auto* closeBtn = new QPushButton("Close");
    closeBtn->setStyleSheet(R"(
        QPushButton { background:rgba(255,255,255,0.2); color:white; border:none;
                      border-radius:6px; padding:5px 14px; font-size:13px; }
        QPushButton:hover { background:rgba(255,255,255,0.35); }
    )");
    hLayout->addWidget(hTitle);
    hLayout->addStretch();
    hLayout->addWidget(closeBtn);
    root->addWidget(header);

    // Scrollable cards
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background: #f0f2f5;");

    m_cardsContainer = new QWidget;
    m_cardsContainer->setStyleSheet("background: #f0f2f5;");
    m_cardsLayout = new QVBoxLayout(m_cardsContainer);
    m_cardsLayout->setContentsMargins(16, 16, 16, 16);
    m_cardsLayout->setSpacing(10);
    m_cardsLayout->addStretch();
    scroll->setWidget(m_cardsContainer);
    root->addWidget(scroll, 1);

    // Reply panel
    auto* replyPanel = new QWidget;
    replyPanel->setStyleSheet("background:white; border-top:1px solid #e8ecef;");
    auto* replyLayout = new QVBoxLayout(replyPanel);
    replyLayout->setContentsMargins(16, 12, 16, 12);
    replyLayout->setSpacing(8);

    m_replyTitle = new QLabel("Reply to this thread:");
    m_replyTitle->setStyleSheet("color: #555; font-size: 13px;");
    replyLayout->addWidget(m_replyTitle);

    m_error = new QLabel;
    m_error->setWordWrap(true);
    m_error->setStyleSheet(
        "color:white; background:#e74c3c; border-radius:6px; padding:6px; font-size:12px;");
    m_error->hide();
    replyLayout->addWidget(m_error);

    m_replyBody = new QTextEdit;
    m_replyBody->setPlaceholderText("Write a reply...");
    m_replyBody->setFixedHeight(72);
    m_replyBody->setStyleSheet(R"(
        QTextEdit { border:1px solid #ddd; border-radius:8px; padding:7px 11px;
                    font-size:13px; background:#f9f9f9; color:#333; }
        QTextEdit:focus { border-color:#1a8fe3; background:white; }
    )");
    replyLayout->addWidget(m_replyBody);

    auto* replyBtnRow = new QHBoxLayout;
    m_anonymous = new QCheckBox("Anonymous");
    m_anonymous->setStyleSheet("color: #777; font-size: 12px;");
    m_replyBtn = new QPushButton("Post Reply");
    m_replyBtn->setStyleSheet(R"(
        QPushButton { background:#1a8fe3; color:white; border:none;
                      border-radius:7px; padding:7px 18px; font-size:13px; font-weight:bold; }
        QPushButton:hover   { background:#1578c2; }
        QPushButton:pressed { background:#1265a8; }
    )");
    replyBtnRow->addWidget(m_anonymous);
    replyBtnRow->addStretch();
    replyBtnRow->addWidget(m_replyBtn);
    replyLayout->addLayout(replyBtnRow);
    root->addWidget(replyPanel);

    if (m_token.isEmpty()) replyPanel->hide();

    loadThread();

    connect(closeBtn,   &QPushButton::clicked, this, &QDialog::accept);
    connect(m_replyBtn, &QPushButton::clicked, this, &ThreadDialog::onReply);
}

void ThreadDialog::loadThread() {
    while (m_cardsLayout->count() > 1) {
        auto* item = m_cardsLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }

    auto threadOpt = m_qm.getThread(m_questionId.toStdString());
    if (!threadOpt.has_value()) {
        auto* lbl = new QLabel("Thread not found.");
        lbl->setStyleSheet("color:#999; font-size:13px; padding:20px;");
        lbl->setAlignment(Qt::AlignCenter);
        m_cardsLayout->insertWidget(0, lbl);
        return;
    }
    const auto& thread = *threadOpt;

    auto* parentCard = new QuestionCard(thread.parent, m_currentUser, QuestionCard::Mode::Feed);
    m_cardsLayout->insertWidget(0, parentCard);

    if (!thread.replies.empty()) {
        auto* divLabel = new QLabel(QString("  %1 repl%2")
            .arg(thread.replies.size())
            .arg(thread.replies.size() == 1 ? "y" : "ies"));
        divLabel->setStyleSheet("color:#aaa; font-size:12px; padding:4px 0;");
        m_cardsLayout->insertWidget(1, divLabel);

        int idx = 2;
        for (const auto& reply : thread.replies) {
            if (reply.is_deleted) continue;
            auto* card = new QuestionCard(reply, m_currentUser, QuestionCard::Mode::Thread);
            m_cardsLayout->insertWidget(idx++, card);
        }
    }
}

void ThreadDialog::onReply() {
    QString body = m_replyBody->toPlainText().trimmed();
    if (body.isEmpty()) { m_error->setText("Reply cannot be empty."); m_error->show(); return; }

    std::string id = m_qm.askThreadQuestion(
        m_token.toStdString(), m_questionId.toStdString(),
        body.toStdString(), m_anonymous->isChecked());

    if (id.empty()) { m_error->setText("Failed to post reply."); m_error->show(); return; }
    m_error->hide();
    m_replyBody->clear();
    loadThread();
}
