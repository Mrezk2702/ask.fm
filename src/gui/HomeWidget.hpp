#pragma once
#include <QWidget>
#include <QTimer>
#include <auth/auth.hpp>
#include <questionManager/questionManager.hpp>

class QTabWidget;
class QLabel;
class QPushButton;
class InboxWidget;
class FeedWidget;

class HomeWidget : public QWidget {
    Q_OBJECT
public:
    explicit HomeWidget(AuthManager& auth, QuestionManager& qm, QWidget* parent = nullptr);

    void setSession(const QString& token, const QString& username);

signals:
    void loggedOut();

private slots:
    void onLogout();
    void onAsk();
    void onRefresh();

private:
    AuthManager&     m_auth;
    QuestionManager& m_qm;
    QString      m_token;
    QString      m_username;
    QTabWidget*  m_tabs;
    InboxWidget* m_inbox;
    FeedWidget*  m_feed;
    QLabel*      m_userLabel;
    QPushButton* m_askBtn;
    QPushButton* m_refreshBtn;
    QPushButton* m_logoutBtn;
    QTimer*      m_autoRefresh;
};
