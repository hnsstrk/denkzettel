#include <KStandardGuiItem>

#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QTest>
#include <QTimer>
#include <QTemporaryDir>

/**
 * Wo genau das Symbol verlorengeht: am `KStandardGuiItem`, am Symbolthema oder
 * am Suchpfad. Nachprüfung zu Befund 5 des S8-UI-Reviews.
 */
namespace
{
void report(const char *label)
{
    const KGuiItem save = KStandardGuiItem::save();
    const QIcon fromItem = save.icon();
    const QIcon fromTheme = QIcon::fromTheme(QStringLiteral("document-save"));

    qInfo("[%s] Thema „%s“ · Suchpfade %s", label, qUtf8Printable(QIcon::themeName()),
          qUtf8Printable(QIcon::themeSearchPaths().join(QStringLiteral(", "))));
    qInfo("       hasThemeIcon(document-save) %d · fromTheme: null %d, Größen %lld, Pixmap 22 leer %d",
          QIcon::hasThemeIcon(QStringLiteral("document-save")) ? 1 : 0, fromTheme.isNull() ? 1 : 0,
          static_cast<long long>(fromTheme.availableSizes().size()),
          fromTheme.pixmap(22).isNull() ? 1 : 0);
    qInfo("       KStandardGuiItem::save(): hasIcon %d, iconName „%s“ · daraus: null %d, Größen %lld",
          save.hasIcon() ? 1 : 0, qUtf8Printable(save.iconName()), fromItem.isNull() ? 1 : 0,
          static_cast<long long>(fromItem.availableSizes().size()));
}
}

int main(int argc, char **argv)
{
    // Wie im Bildläufer: eigene Einstellungen, damit der Lauf nicht vom
    // gespeicherten Zustand des Kunden abhängt.
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());

    const QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));

    report("wie im Bildlauf");

    // Und mit dem Griff, den der Test des Entwicklers tut.
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral("/usr/share/icons"));
    if (QIcon::themeName().isEmpty()) {
        QIcon::setThemeName(QStringLiteral("breeze"));
    }
    report("mit /usr/share/icons im Suchpfad");

    // Und derselbe Dialog, wie der Produktivcode ihn baut: bleibt das Symbol
    // am Knopf, wenn QMessageBox ihn in seine QDialogButtonBox einhängt?
    QMessageBox dialog;
    dialog.setIcon(QMessageBox::Warning);
    dialog.setText(QStringLiteral("Änderungen speichern?"));
    QPushButton *save = dialog.addButton(QStringLiteral("Speichern"), QMessageBox::AcceptRole);
    QPushButton *discard = dialog.addButton(QStringLiteral("Verwerfen"), QMessageBox::DestructiveRole);
    QPushButton *cancel = dialog.addButton(QStringLiteral("Abbrechen"), QMessageBox::RejectRole);

    save->setIcon(KStandardGuiItem::save().icon());
    discard->setIcon(KStandardGuiItem::discard().icon());
    cancel->setIcon(KStandardGuiItem::cancel().icon());

    qInfo("[Dialog] Stil „%s“ · SH_DialogButtonBox_ButtonsHaveIcons %d",
          qUtf8Printable(QApplication::style()->objectName()),
          QApplication::style()->styleHint(QStyle::SH_DialogButtonBox_ButtonsHaveIcons));
    qInfo("         direkt nach setIcon: Speichern null %d, Verwerfen null %d, Abbrechen null %d",
          save->icon().isNull() ? 1 : 0, discard->icon().isNull() ? 1 : 0,
          cancel->icon().isNull() ? 1 : 0);

    dialog.setDefaultButton(save);
    dialog.setEscapeButton(cancel);

    // Und jetzt der Messweg des Reviews: exec(), und aus einem Timer heraus den
    // modalen Dialog greifen — genau wie der Bildläufer es tut.
    QTimer::singleShot(0, qApp, [&dialog, save] {
        QMessageBox *found = nullptr;
        for (int attempt = 0; attempt < 200 && !found; ++attempt) {
            found = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (!found) {
                QTest::qWait(10);
            }
        }
        if (!found) {
            qFatal("kein modaler Dialog");
        }
        QTest::qWaitForWindowExposed(found);
        QTest::qWait(150);

        const QList<QAbstractButton *> buttons = found->buttons();
        for (QAbstractButton *button : buttons) {
            qInfo("         über activeModalWidget: „%s“ Symbol null %d (Name „%s“)",
                  qUtf8Printable(button->text()), button->icon().isNull() ? 1 : 0,
                  qUtf8Printable(button->icon().name()));
        }
        qInfo("         über activeModalWidget: Vorgabeknopf „%s“ · derselbe Dialog wie gebaut %d · "
              "Knopfzahl %lld · „Speichern“ ist derselbe Zeiger %d",
              found->defaultButton() ? qUtf8Printable(found->defaultButton()->text()) : "keiner",
              found == &dialog ? 1 : 0, static_cast<long long>(buttons.size()),
              buttons.contains(static_cast<QAbstractButton *>(save)) ? 1 : 0);
        found->close();
    });

    dialog.exec();
    qInfo("         nach exec(), an den Zeigern von vorhin: Speichern null %d (Name „%s“)",
          save->icon().isNull() ? 1 : 0, qUtf8Printable(save->icon().name()));

    return 0;
}
