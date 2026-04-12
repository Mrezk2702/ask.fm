#pragma once
#include <QFrame>
#include <models/questions.hpp>

class QLabel;
class QPushButton;

class QuestionCard : public QFrame {
    Q_OBJECT
public:
    enum class Mode { Inbox, Feed, Thread };

    QuestionCard(const Question_t& q,
                 const QString& currentUser,
                 Mode mode,
                 QWidget* parent = nullptr);
signals:
    void answerRequested(const QString& questionId, const QString& questionBody);
    void deleteRequested(const QString& questionId);
    void viewThreadRequested(const QString& questionId);
};
