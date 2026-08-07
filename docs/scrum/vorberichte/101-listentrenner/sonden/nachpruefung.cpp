/*
 * Nachprüfung der Befunde aus Messung A, die Messung B nicht selbst gemessen
 * hatte (Vorprüfung #101, 07.08.2026, Bearbeiter B).
 *
 * Prüffrage 1: Aus welcher Gruppe liest `KColorScheme::frameContrast()`?
 *   Messung A misst `[KDE]`, Messung B hatte `[General]` behauptet.
 * Prüffrage 2: Gilt die Sekunden-Trägheit von KConfig auch auf diesem Weg?
 *   AK 4 verlangt zwei Schemata in einem Lauf.
 * Prüffrage 3: Wie breit ist das Sichtfeld einer QListView mit Rollbalken
 *   unter dem Plattformthema des Kunden? Messung A misst 279, Messung B 286.
 * Prüffrage 4: Stoßen die Zeilenrechtecke lückenlos aneinander (Messung A, F5)?
 * Prüffrage 5: Tragen die Höhen 27 / 35 / 72 aus Messung A? Nachgerechnet aus
 *   den Schriftmaßen und den Konstanten von `notelistdelegate.cpp:12–28`.
 */

#include <KColorScheme>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QListView>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QStyledItemDelegate>
#include <QTest>
#include <QTextStream>
#include <QThread>

namespace
{
QString g_globals;

void writeAppConfig(const QString &body)
{
    QFile file(g_globals);
    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QTextStream stream(&file);
    stream << body;
    stream.flush();
    file.close();
}

class RectDelegate : public QStyledItemDelegate
{
public:
    mutable QList<QRect> rects;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        while (rects.size() <= index.row()) {
            rects.append(QRect());
        }
        rects[index.row()] = option.rect;
        QStyledItemDelegate::paint(painter, option, index);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(index)
        return QSize(option.rect.width(), 40);
    }
};
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("denkzettel"));
    QTextStream out(stdout);

    QStandardPaths::setTestModeEnabled(true);
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QDir().mkpath(configDir);
    g_globals = configDir + QStringLiteral("/kdeglobals");
    QFile::remove(g_globals);
    KSharedConfig::openConfig()->reparseConfiguration();

    out << "## Prüffrage 1 — aus welcher Gruppe liest KColorScheme::frameContrast()?\n";
    out << QStringLiteral("ohne Konfiguration (leerer Sandkasten): %1\n")
               .arg(KColorScheme::frameContrast(), 0, 'f', 4);

    const QString probe = configDir + QStringLiteral("/probe.colors");
    {
        QFile file(probe);
        file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        QTextStream stream(&file);
        stream << "[General]\nframeContrast=0.45\n\n[KDE]\nframeContrast=0.55\n";
        stream.flush();
    }
    KSharedConfigPtr both = KSharedConfig::openConfig(probe, KConfig::SimpleConfig);
    out << QStringLiteral("Datei mit [General]=0.45 und [KDE]=0.55 → %1\n")
               .arg(KColorScheme::frameContrast(both), 0, 'f', 4);

    {
        QFile file(probe);
        file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        QTextStream stream(&file);
        stream << "[General]\nframeContrast=0.45\n";
        stream.flush();
    }
    KSharedConfigPtr generalOnly = KSharedConfig::openConfig(probe + QStringLiteral("2"), KConfig::SimpleConfig);
    QFile::copy(probe, probe + QStringLiteral("2"));
    generalOnly->reparseConfiguration();
    out << QStringLiteral("Datei nur mit [General]=0.45 → %1\n\n")
               .arg(KColorScheme::frameContrast(generalOnly), 0, 'f', 4);

    out << "## Prüffrage 2 — zwei Werte im selben Lauf über die Anwendungskonfiguration\n";
    for (const QString &value : {QStringLiteral("0.85"), QStringLiteral("0.05"), QStringLiteral("0.5")}) {
        writeAppConfig(QStringLiteral("[KDE]\nframeContrast=%1\n").arg(value));
        KSharedConfig::openConfig()->reparseConfiguration();
        out << QStringLiteral("ohne Pause: geschrieben %1 → gelesen %2\n")
                   .arg(value)
                   .arg(KColorScheme::frameContrast(), 0, 'f', 4);
    }
    for (const QString &value : {QStringLiteral("0.85"), QStringLiteral("0.05"), QStringLiteral("0.5")}) {
        writeAppConfig(QStringLiteral("[KDE]\nframeContrast=%1\n").arg(value));
        QThread::msleep(1100);
        KSharedConfig::openConfig()->reparseConfiguration();
        out << QStringLiteral("mit 1100 ms: geschrieben %1 → gelesen %2\n")
                   .arg(value)
                   .arg(KColorScheme::frameContrast(), 0, 'f', 4);
    }
    QFile::remove(g_globals);

    out << "\n## Prüffrage 3 — Breite des Zeilenrechtecks mit und ohne Rollbalken\n";
    QStandardItemModel model;
    for (int row = 0; row < 12; ++row) {
        model.appendRow(new QStandardItem(QStringLiteral("Zeile %1").arg(row)));
    }
    QListView list;
    RectDelegate delegate;
    list.setModel(&model);
    list.setItemDelegate(&delegate);
    list.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list.setFrameShape(QFrame::NoFrame);
    list.resize(300, 600);
    list.show();
    QTest::qWait(200);
    out << QStringLiteral("Stil: %1, Plattformthema-Umgebung: %2\n")
               .arg(QApplication::style()->objectName())
               .arg(QString::fromLocal8Bit(qgetenv("QT_QPA_PLATFORMTHEME")));
    out << QStringLiteral("ohne Rollbalken: Ansicht %1 · Sichtfeld %2 · Zeile %3\n")
               .arg(list.width())
               .arg(list.viewport()->width())
               .arg(delegate.rects.value(0).width());

    list.resize(300, 200);
    QTest::qWait(200);
    out << QStringLiteral("mit Rollbalken:  Ansicht %1 · Sichtfeld %2 · Zeile %3 · Rollbalken %4 px\n")
               .arg(list.width())
               .arg(list.viewport()->width())
               .arg(delegate.rects.value(0).width())
               .arg(list.verticalScrollBar()->width());

    out << "\n## Prüffrage 4 — stoßen die Zeilenrechtecke aneinander?\n";
    list.resize(300, 600);
    QTest::qWait(200);
    out << QStringLiteral("spacing(): %1\n").arg(list.spacing());
    for (int row = 0; row + 1 < 4; ++row) {
        const QRect upper = delegate.rects.value(row);
        const QRect lower = delegate.rects.value(row + 1);
        out << QStringLiteral("Zeile %1 endet bei y=%2, Zeile %3 beginnt bei y=%4 → Lücke %5\n")
                   .arg(row)
                   .arg(upper.bottom())
                   .arg(row + 1)
                   .arg(lower.y())
                   .arg(lower.y() - upper.bottom() - 1);
    }

    out << "\n## Prüffrage 5 — die Höhen 27 / 35 / 72 nachgerechnet\n";
    const QFont small = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
    QFont head = small;
    head.setWeight(QFont::DemiBold);
    const QFont body = QApplication::font();
    const int hSmall = QFontMetrics(head).height();
    const int hBody = QFontMetrics(body).height();
    out << QStringLiteral("Kopfschrift %1 %2 pt → %3 px · Fließtext %4 %5 pt → %6 px\n")
               .arg(head.family())
               .arg(head.pointSizeF())
               .arg(hSmall)
               .arg(body.family())
               .arg(body.pointSizeF())
               .arg(hBody);
    out << QStringLiteral("erster Kopf   6 + %1 + 6 = %2\n").arg(hSmall).arg(6 + hSmall + 6);
    out << QStringLiteral("weiterer Kopf 14 + %1 + 6 = %2\n").arg(hSmall).arg(14 + hSmall + 6);
    out << QStringLiteral("Notiz      2×9 + %1 + 3 + 2×%2 = %3\n")
               .arg(QFontMetrics(small).height())
               .arg(hBody)
               .arg(18 + QFontMetrics(small).height() + 3 + 2 * hBody);

    out.flush();
    return 0;
}
