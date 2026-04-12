#include "QuestionCard.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>
#include <QFrame>

static QString formatRelativeTime(time_t t) {
    if (t == 0) return "";
    time_t now = time(nullptr);
    int64_t diff = now - t;
    if (diff < 60)    return "just now";
    if (diff < 3600)  return QString("%1m ago").arg(diff / 60);
    if (diff < 86400) return QString("%1h ago").arg(diff / 3600);
    return QDateTime::fromSecsSinceEpoch(t).toString("MMM d, yyyy");
}

static const QString BTN_PRIMARY = R"(
    QPushButton {
        background: #1a8fe3; color: white; border: none;
        border-radius: 6px; padding: 6px 14px; font-size: 12px; font-weight: bold;
    }
    QPushButton:hover   { background: #1578c2; }
    QPushButton:pressed { background: #1265a8; }
)";

static const QString BTN_DANGER = R"(
    QPushButton {
        background: #e74c3c; color: white; border: none;
        border-radius: 6px; padding: 6px 14px; font-size: 12px; font-weight: bold;
    }
    QPushButton:hover   { background: #c0392b; }
    QPushButton:pressed { background: #a93226; }
)";

static const QString BTN_SECONDARY = R"(
    QPushButton {
        background: #f0f2f5; color: #555; border: 1px solid #ddd;
        border-radius: 6px; padding: 6px 14px; font-size: 12px;
    }
    QPushButton:hover   { background: #e2e6ea; }
    QPushButton:pressed { background: #d5d9dd; }
)";

QuestionCard::QuestionCard(const Question_t& q,
                           const QString& currentUser,
                           Mode mode,
                           QWidget* parent)
    : QFrame(parent)
{
    setObjectName("questionCard");
    setStyleSheet(R"(
        QFrame#questionCard {
            background: white;
            border: 1px solid #e8ecef;
            border-radius: 12px;
        }
    )");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 14, 18, 14);
    root->setSpacing(8);

    // ── Header row ────────────────────────────────────────────────────────
    auto* headerRow = new QHBoxLayout;
    QString sender = (q.is_anonymous || q.from_user.empty())
        ? "Anonymous" : QString::fromStdString(q.from_user);

    auto* fromLabel = new QLabel(sender);
    fromLabel->setStyleSheet(q.is_anonymous
        ? "color: #999; font-size: 12px; font-style: italic;"
        : "color: #1a8fe3; font-size: 12px; font-weight: bold;");

    auto* arrowLabel = new QLabel("→");
    arrowLabel->setStyleSheet("color: #aaa; font-size: 12px;");

    auto* toLabel = new QLabel(QString::fromStdString(q.to_user));
    toLabel->setStyleSheet("color: #555; font-size: 12px;");

    auto* timeLabel = new QLabel(formatRelativeTime(q.created_at));
    timeLabel->setStyleSheet("color: #bbb; font-size: 11px;");

    headerRow->addWidget(fromLabel);
    headerRow->addWidget(arrowLabel);
    headerRow->addWidget(toLabel);
    headerRow->addStretch();
    headerRow->addWidget(timeLabel);
    root->addLayout(headerRow);

    auto* div = new QFrame;
    div->setFrameShape(QFrame::HLine);
    div->setStyleSheet("border: none; border-top: 1px solid #f0f0f0;");
    root->addWidget(div);

    // ── Question body ─────────────────────────────────────────────────────
    auto* bodyLabel = new QLabel(QString::fromStdString(q.body));
    bodyLabel->setWordWrap(true);
    bodyLabel->setStyleSheet("color: #222; font-size: 14px;");
    root->addWidget(bodyLabel);

    // ── Answer (Feed / Thread mode) ───────────────────────────────────────
    if ((mode == Mode::Feed || mode == Mode::Thread) && !q.answer.empty()) {
        auto* ansDiv = new QFrame;
        ansDiv->setFrameShape(QFrame::HLine);
        ansDiv->setStyleSheet("border: none; border-top: 1px solid #f0f0f0; margin-top:4px;");
        root->addWidget(ansDiv);

        auto* ansHeader = new QHBoxLayout;
        auto* ansTag = new QLabel("Answer");
        ansTag->setStyleSheet(
            "color: white; background: #27ae60; border-radius: 4px;"
            "padding: 2px 8px; font-size: 11px; font-weight: bold;");
        auto* ansTime = new QLabel(formatRelativeTime(q.answered_at));
        ansTime->setStyleSheet("color: #bbb; font-size: 11px;");
        ansHeader->addWidget(ansTag);
        ansHeader->addStretch();
        ansHeader->addWidget(ansTime);
        root->addLayout(ansHeader);

        auto* ansLabel = new QLabel(QString::fromStdString(q.answer));
        ansLabel->setWordWrap(true);
        ansLabel->setStyleSheet("color: #333; font-size: 13px; padding-left: 4px;");
        root->addWidget(ansLabel);
    }

    if (!q.parent_id.empty()) {
        auto* badge = new QLabel("↪ reply to #" + QString::fromStdString(q.parent_id));
        badge->setStyleSheet("color: #aaa; font-size: 11px;");
        root->addWidget(badge);
    }

    // ── Buttons ───────────────────────────────────────────────────────────
    auto* btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);
    btnRow->addStretch();

    QString qid  = QString::fromStdString(q.id);
    QString body = QString::fromStdString(q.body);

    if (mode == Mode::Inbox) {
        auto* answerBtn = new QPushButton("Answer");
        answerBtn->setStyleSheet(BTN_PRIMARY);
        connect(answerBtn, &QPushButton::clicked, this, [this, qid, body]() {
            emit answerRequested(qid, body);
        });
        btnRow->addWidget(answerBtn);
    }

    if (mode == Mode::Inbox || mode == Mode::Feed) {
        bool canDelete = (currentUser == QString::fromStdString(q.to_user) ||
                          currentUser == QString::fromStdString(q.from_user));
        if (canDelete) {
            auto* delBtn = new QPushButton("Delete");
            delBtn->setStyleSheet(BTN_DANGER);
            connect(delBtn, &QPushButton::clicked, this, [this, qid]() {
                emit deleteRequested(qid);
            });
            btnRow->addWidget(delBtn);
        }
    }

    if ((mode == Mode::Inbox || mode == Mode::Feed) && q.parent_id.empty()) {
        auto* threadBtn = new QPushButton("Thread");
        threadBtn->setStyleSheet(BTN_SECONDARY);
        connect(threadBtn, &QPushButton::clicked, this, [this, qid]() {
            emit viewThreadRequested(qid);
        });
        btnRow->addWidget(threadBtn);
    }

    root->addLayout(btnRow);
}
