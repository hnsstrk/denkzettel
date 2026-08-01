// Nachbau der Layout-Verschachtelung aus src/ui/librarywindow.cpp
// (Konstruktor Zeilen 157-162, buildHeader() Zeilen 207-230) — ohne KF6,
// nur die Qt-Widgets-Struktur. Zweck: statisch/offscreen nachweisen, wo die
// Fensterhoehe hinlaeuft. Kein Produktivcode, laeuft nur im Scratchpad.

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QStringListModel>
#include <QTextBrowser>
#include <QVBoxLayout>

static QWidget *placeholderPage(const QString &title, const QString &hint)
{
    auto *page = new QWidget();
    auto *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(6);
    auto *t = new QLabel(title, page);
    t->setAlignment(Qt::AlignCenter);
    layout->addWidget(t);
    auto *h = new QLabel(hint, page);
    h->setAlignment(Qt::AlignCenter);
    layout->addWidget(h);
    return page;
}

static QWidget *buildHeader(QWidget *parent)
{
    auto *header = new QWidget(parent);
    auto *search = new QLineEdit(header);
    search->setPlaceholderText(QStringLiteral("Volltextsuche …"));
    search->setEnabled(false);
    auto *wrapper = new QWidget(header);
    auto *wrapperLayout = new QVBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    wrapperLayout->addWidget(search);
    auto *layout = new QVBoxLayout(header);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(wrapper);
    header->setObjectName(QStringLiteral("header"));
    search->setObjectName(QStringLiteral("search"));
    return header;
}

static QWidget *build(bool withStretch)
{
    auto *w = new QWidget(nullptr, Qt::Window);

    auto *list = new QListView(w);
    auto *model = new QStringListModel(QStringList{QStringLiteral("Heute 14:32 — restic-Backup: prune-Policy prüfen"),
                                                   QStringLiteral("Heute 11:05 — Idee für Denkzettel: Bündel-Export"),
                                                   QStringLiteral("Gestern 21:48 — journalctl -u whisperd --since today")},
                                      list);
    list->setModel(model);
    list->setFrameShape(QFrame::NoFrame);

    auto *listPages = new QStackedWidget(w);
    listPages->addWidget(list);
    listPages->addWidget(placeholderPage(QStringLiteral("Noch keine Notizen"), QStringLiteral("Mit Meta+N einen Gedanken festhalten.")));
    listPages->setMinimumWidth(220);

    auto *detailPages = new QStackedWidget(w);
    auto *detail = new QWidget();
    auto *head = new QHBoxLayout();
    head->addWidget(new QLabel(QStringLiteral("Heute 11:05"), detail));
    head->addStretch();
    head->addWidget(new QPushButton(QStringLiteral("Löschen"), detail));
    auto *detailText = new QTextBrowser(detail);
    detailText->setFrameShape(QFrame::NoFrame);
    auto *dl = new QVBoxLayout(detail);
    dl->setContentsMargins(12, 10, 12, 12);
    dl->setSpacing(10);
    dl->addLayout(head);
    dl->addWidget(detailText);
    detailPages->addWidget(detail);
    detailPages->addWidget(placeholderPage(QStringLiteral("Keine Notiz ausgewählt"), QStringLiteral("Zum Lesen links eine Notiz auswählen.")));
    detailPages->setCurrentIndex(1);

    auto *splitter = new QSplitter(Qt::Horizontal, w);
    splitter->addWidget(listPages);
    splitter->addWidget(detailPages);
    splitter->setStretchFactor(1, 1);
    splitter->setChildrenCollapsible(false);
    splitter->setSizes({300, 600});
    splitter->setObjectName(QStringLiteral("splitter"));

    auto *message = new QLabel(QStringLiteral("Notiz gelöscht — noch 4 s"), w);
    message->hide();

    auto *layout = new QVBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(buildHeader(w));
    layout->addWidget(message);
    if (withStretch) {
        layout->addWidget(splitter, 1); // die eine Zeile Unterschied
    } else {
        layout->addWidget(splitter); // Stand von librarywindow.cpp:162
    }

    w->resize(900, 600);
    return w;
}

static void report(const char *label, QWidget *w)
{
    auto *header = w->findChild<QWidget *>(QStringLiteral("header"));
    auto *search = w->findChild<QLineEdit *>(QStringLiteral("search"));
    auto *splitter = w->findChild<QSplitter *>(QStringLiteral("splitter"));

    const QPoint searchTopLeft = search->mapTo(w, QPoint(0, 0));

    const QString out =
        QStringLiteral("== %1 ==\n").arg(QString::fromLatin1(label))
        + QStringLiteral("  Fenster            : %1x%2\n").arg(w->width()).arg(w->height())
        + QStringLiteral("  Kopfzeile (header) : y=%1 h=%2\n").arg(header->y()).arg(header->height())
        + QStringLiteral("  Suchfeld           : y=%1 h=%2  (Leerflaeche darueber: %3 px)\n")
              .arg(searchTopLeft.y()).arg(search->height()).arg(searchTopLeft.y())
        + QStringLiteral("  Splitter           : y=%1 h=%2\n").arg(splitter->y()).arg(splitter->height())
        + QStringLiteral("  Splitter-SizePolicy: horizontal=%1 vertikal=%2  (0=Fixed 1=Minimum 4=Maximum 5=Preferred 7=Expanding)\n")
              .arg(int(splitter->sizePolicy().horizontalPolicy()))
              .arg(int(splitter->sizePolicy().verticalPolicy()));
    printf("%s", qPrintable(out));
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWidget *broken = build(false);
    QWidget *fixed = build(true);
    broken->show();
    fixed->show();
    app.processEvents();

    report("IST  — layout->addWidget(m_splitter)   [librarywindow.cpp:162]", broken);
    report("SOLL — layout->addWidget(m_splitter, 1)", fixed);

    broken->grab().save(QStringLiteral("%1/ist.png").arg(QString::fromLatin1(argv[1])));
    fixed->grab().save(QStringLiteral("%1/soll.png").arg(QString::fromLatin1(argv[1])));
    return 0;
}
