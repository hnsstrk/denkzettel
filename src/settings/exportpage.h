#pragma once

#include <QString>
#include <QWidget>

class QFormLayout;
class QLabel;
class QLineEdit;
class QSpinBox;

/**
 * The page "Export" of SPEC 13: where the bundles of SPEC 8.1 are written to,
 * and the three thresholds that decide when the service asks — the two of the
 * overflow guard (SPEC 11, count and age) and the one of the topic clustering
 * (SPEC 7.3).
 *
 * Everything persisted is a `kcfg_` widget, so the dialog's manager loads,
 * saves and greys the Apply button by itself — the page has no save() of its
 * own and must not grow one (as on the AI provider page).
 *
 * **The folder field starts empty and has no default** (issue #75): the path
 * is a folder outside this project that only the user knows, and this
 * repository is public.
 */
class ExportPage : public QWidget
{
    Q_OBJECT

public:
    explicit ExportPage(QWidget *parent = nullptr);
    ~ExportPage() override;

protected:
    /** Where the folder in the configuration is checked, see the definition. */
    void showEvent(QShowEvent *event) override;
    /** Remembers the folder in the field when it is entered, see the definition. */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    /**
     * One threshold row: label, spin box, unit — and the box back to size it.
     * The key is a plain literal and not a QString so that no two neighbouring
     * parameters share a type; two adjacent QStrings are a clang-tidy finding
     * (bugprone-easily-swappable-parameters) and the CI fails on every one.
     */
    QSpinBox *threshold(QFormLayout *form, const QString &label, const char *key, int maximum, const QString &unit);
    void chooseFolder();
    void setFolder(const QString &candidate);

    QLineEdit *m_vaultPath;
    QLabel *m_pathResult;
    /** What a refused folder goes back to: the last one that passed, and the
     *  one that stood in the field when the user entered it. */
    QString m_accepted;
};
