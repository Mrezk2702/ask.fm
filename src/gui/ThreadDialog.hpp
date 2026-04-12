#pragma once
#include <QDialog>
#include <questionManager/questionManager.hpp>

class QVBoxLayout;
class QScrollArea;
class QTextEdit;
class QCheckBox;
class QPushButton;
class QLabel;

class ThreadDialog : public QDialog {
    Q_OBJECT
public:
    explicit ThreadDialog(QuestionManager& qm,
                          const QString& token,
                          const QString& questionId,
                          const QString& currentUser = "",
                          QWidget* parent = nullptr);
private slots:
    void onReply();

private:
    void loadThread();

    QuestionManager& m_qm;
    QString      m_token;
    QString      m_questionId;
    QString      m_currentUser;
    QVBoxLayout* m_cardsLayout;
    QWidget*     m_cardsContainer;
    QTextEdit*   m_replyBody;
    QCheckBox*   m_anonymous;
    QPushButton* m_replyBtn;
    QLabel*      m_error;
    QLabel*      m_replyTitle;
};
