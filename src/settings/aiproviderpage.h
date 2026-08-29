#pragma once

#include <QWidget>

class OllamaProvider;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

/**
 * The page "AI provider" of SPEC 13: which backend answers, which two models
 * it uses, and whether it can be reached at all (SPEC 7.1).
 *
 * Everything that is persisted is a `kcfg_` widget, so the dialog's manager
 * loads, saves and greys the Apply button by itself — the page has no
 * save() of its own and must not grow one.
 */
class AiProviderPage : public QWidget
{
    Q_OBJECT

public:
    explicit AiProviderPage(QWidget *parent = nullptr);

private:
    void startTest();
    void showResult(qint64 chatMilliseconds, qint64 embedMilliseconds, const QString &error);

    QComboBox *m_chatModel;
    QLineEdit *m_ollamaUrl;
    QComboBox *m_embeddingModel;
    QPushButton *m_test;
    QLabel *m_result;
    OllamaProvider *m_provider;
};
