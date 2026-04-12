#pragma once
#include <QWidget>
#include <questionManager/questionManager.hpp>

class QVBoxLayout;
class QScrollArea;
class QLabel;

class InboxWidget : public QWidget {
    Q_OBJECT
public:
    explicit InboxWidget(QuestionManager& qm, QWidget* parent = nullptr);

    void setSession(const QString& token, const QString& username);
    void refresh();

private:
    QuestionManager& m_qm;
    QString      m_token;
    QString      m_username;
    QVBoxLayout* m_cardsLayout;
    QWidget*     m_cardsContainer;
    QLabel*      m_emptyLabel;
};
