#pragma once
#include <QDialog>
#include <questionManager/questionManager.hpp>

class QTextEdit;
class QPushButton;
class QLabel;

class AnswerDialog : public QDialog {
    Q_OBJECT
public:
    explicit AnswerDialog(QuestionManager& qm,
                          const QString& token,
                          const QString& questionId,
                          const QString& questionBody,
                          QWidget* parent = nullptr);
signals:
    void answered();

private slots:
    void onAnswer();

private:
    QuestionManager& m_qm;
    QString      m_token;
    QString      m_questionId;
    QTextEdit*   m_answer;
    QPushButton* m_submitBtn;
    QLabel*      m_error;
};
