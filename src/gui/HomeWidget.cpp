#include "HomeWidget.hpp"
#include "InboxWidget.hpp"
#include "FeedWidget.hpp"
#include "AskDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

HomeWidget::HomeWidget(AuthManager& auth, QuestionManager& qm, QWidget* parent)
    : QWidget(parent), m_auth(auth), m_qm(qm)
{
    setStyleSheet("background: #f0f2f5;");
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Nav bar
    auto* navbar = new QWidget;
    navbar->setFixedHeight(56);
    navbar->setStyleSheet("background: #1a8fe3; border-bottom: 1px solid #1578c2;");
    auto* navLayout = new QHBoxLayout(navbar);
    navLayout->setContentsMargins(20, 0, 20, 0);
    navLayout->setSpacing(12);

    auto* brand = new QLabel("ask.fm");
    brand->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");

    m_userLabel = new QLabel;
    m_userLabel->setStyleSheet("color: rgba(255,255,255,0.85); font-size: 13px;");

    m_askBtn = new QPushButton("+ Ask Someone");
    m_askBtn->setCursor(Qt::PointingHandCursor);
    m_askBtn->setStyleSheet(R"(
        QPushButton { background:rgba(255,255,255,0.2); color:white;
                      border:1px solid rgba(255,255,255,0.4); border-radius:7px;
                      padding:7px 16px; font-size:13px; font-weight:bold; }
        QPushButton:hover   { background:rgba(255,255,255,0.35); }
        QPushButton:pressed { background:rgba(255,255,255,0.15); }
    )");

    m_refreshBtn = new QPushButton("↻ Refresh");
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setStyleSheet(R"(
        QPushButton { background:rgba(255,255,255,0.15); color:rgba(255,255,255,0.85);
                      border:1px solid rgba(255,255,255,0.3); border-radius:7px;
                      padding:7px 14px; font-size:13px; }
        QPushButton:hover   { background:rgba(255,255,255,0.3); color:white; }
        QPushButton:pressed { background:rgba(255,255,255,0.1); }
    )");

    m_logoutBtn = new QPushButton("Logout");
    m_logoutBtn->setCursor(Qt::PointingHandCursor);
    m_logoutBtn->setStyleSheet(R"(
        QPushButton { background:transparent; color:rgba(255,255,255,0.7);
                      border:none; padding:7px 10px; font-size:13px; }
        QPushButton:hover { color:white; }
    )");

    navLayout->addWidget(brand);
    navLayout->addStretch();
    navLayout->addWidget(m_userLabel);
    navLayout->addWidget(m_askBtn);
    navLayout->addWidget(m_refreshBtn);
    navLayout->addWidget(m_logoutBtn);
    root->addWidget(navbar);

    // Tabs
    m_tabs = new QTabWidget;
    m_tabs->setStyleSheet(R"(
        QTabWidget::pane { border:none; background:#f0f2f5; }
        QTabBar::tab { background:transparent; color:#888; padding:10px 28px;
                       font-size:14px; border:none; border-bottom:3px solid transparent; }
        QTabBar::tab:selected { color:#1a8fe3; border-bottom:3px solid #1a8fe3; font-weight:bold; }
        QTabBar::tab:hover:!selected { color:#555; }
        QTabWidget > QTabBar { background:white; border-bottom:1px solid #e8ecef; }
    )");

    m_inbox = new InboxWidget(m_qm);
    m_feed  = new FeedWidget(m_qm);
    m_tabs->addTab(m_inbox, "Inbox");
    m_tabs->addTab(m_feed,  "My Feed");
    root->addWidget(m_tabs, 1);

    connect(m_askBtn,     &QPushButton::clicked, this, &HomeWidget::onAsk);
    connect(m_refreshBtn, &QPushButton::clicked, this, &HomeWidget::onRefresh);
    connect(m_logoutBtn,  &QPushButton::clicked, this, &HomeWidget::onLogout);

    // Refresh the active tab whenever the user switches
    connect(m_tabs, &QTabWidget::currentChanged, this, [this](int idx) {
        if (idx == 0) m_inbox->refresh();
        if (idx == 1) m_feed->refresh();
    });

    // Auto-refresh inbox every 5 seconds to pick up new questions
    m_autoRefresh = new QTimer(this);
    m_autoRefresh->setInterval(5000);
    connect(m_autoRefresh, &QTimer::timeout, this, &HomeWidget::onRefresh);
}

void HomeWidget::setSession(const QString& token, const QString& username) {
    m_token = token; m_username = username;
    m_userLabel->setText("Hello, " + username);
    m_inbox->setSession(token, username);
    m_feed->setSession(token, username);
    m_tabs->setCurrentIndex(0);
    m_autoRefresh->start();
}

void HomeWidget::onRefresh() {
    int idx = m_tabs->currentIndex();
    if (idx == 0) m_inbox->refresh();
    else           m_feed->refresh();
}

void HomeWidget::onAsk() {
    auto* dlg = new AskDialog(m_qm, m_token, "", this);
    dlg->exec(); dlg->deleteLater();
}

void HomeWidget::onLogout() {
    m_autoRefresh->stop();
    if (!m_token.isEmpty()) m_auth.logout(m_token.toStdString());
    m_token.clear(); m_username.clear();
    emit loggedOut();
}
