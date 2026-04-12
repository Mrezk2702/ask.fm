#pragma once
#include <QDialog>
#include <questionManager/questionManager.hpp>

class QLineEdit;
class QTextEdit;
class QCheckBox;
class QPushButton;
class QLabel;

class AskDialog : public QDialog {
    Q_OBJECT
public:
    explicit AskDialog(QuestionManager& qm,
                       const QString& token,
                       const QString& targetUser = "",
                       QWidget* parent = nullptr);
signals:
    void questionAsked();

private slots:
    void onAsk();

private:
    QuestionManager& m_qm;
    QString      m_token;
    QLineEdit*   m_targetUser;
    QTextEdit*   m_body;
    QCheckBox*   m_anonymous;
    QPushButton* m_askBtn;
    QLabel*      m_error;
};
