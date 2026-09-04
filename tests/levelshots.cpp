#include "capture/audiorecorder.h"
#include "capture/recordingwindow.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QApplication>
#include <QAudioBuffer>
#include <QAudioFormat>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QSet>
#include <QSignalSpy>
#include <QStringList>
#include <QStyle>
#include <QTemporaryDir>
#include <QTest>

#include <algorithm>
#include <cmath>
#include <numbers>

/**
 * The pictures and the numbers of the level meter's scale (SPEC 4).
 *
 * Not a test and out of `add_test()`, for the reason `readmeshots` and
 * `originshots` are out of it: a broken picture writer must not turn the suite
 * red. It is built with the suite all the same, or it ages unnoticed and then
 * writes plausible pictures of an **old** state (CLAUDE.md, rule 4).
 *
 * Committed for the reason `originshots` is: the floor of the decibel scale was
 * decided on the numbers this run prints, and a runner nobody can build again
 * leaves them unrepeatable. It answers one question — how far does the row come
 * up for an amplitude the run itself sets — and it answers it the way the user
 * reads it, by **counting the lit bars in the picture** rather than by asking
 * the widget. The count needs no bar width and no gap: it walks the middle row
 * of the meter from left to right and counts the runs of the lit colour, so it
 * keeps working when those two numbers change (finding 48: a check that shares
 * a constant with its subject cannot contradict it).
 *
 * Both colours are read off the picture too, at the two ends where the answer
 * is not in doubt: a silent buffer leaves every bar unlit, a full-scale one
 * lights every one of them. That is the value set from outside which finding 10
 * asks for — without it "brighter" would be this run's own guess.
 *
 * **Never the microphone** (the user's instruction of 28.08.2026): every
 * amplitude here comes from a tone the run generates.
 *
 * Usage — the environment is not optional, see rule 2 and finding 28:
 *
 *   cmake --build build --target levelshots
 *   env -u LANGUAGE LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 \
 *       QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *       QT_FORCE_STDERR_LOGGING=1 \
 *       build/bin/levelshots <target directory>
 *
 * For a German picture the catalogue has to be findable at runtime, and the
 * **build** is what compiles it (finding 57), so the order is build, install
 * into a throwaway root, run:
 *
 *   cmake --build build
 *   dest=$(mktemp -d); DESTDIR="$dest" cmake --install build
 *   env LANGUAGE=de LANG=de_DE.UTF-8 LC_ALL=de_DE.UTF-8 \
 *       XDG_DATA_DIRS="$dest/usr/share:/usr/share" \
 *       QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *       QT_FORCE_STDERR_LOGGING=1 \
 *       build/bin/levelshots <target directory>
 */
namespace
{
/** One buffer of 100 ms of a 440 Hz tone whose peak is `amplitude`. */
void feed(AudioRecorder &recorder, qreal amplitude)
{
    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelConfig(QAudioFormat::ChannelConfigMono);
    format.setSampleFormat(QAudioFormat::Int16);

    constexpr int Frames = 4800;
    QByteArray samples(qsizetype{Frames} * 2, Qt::Uninitialized);
    auto *value = reinterpret_cast<qint16 *>(samples.data());
    for (int i = 0; i < Frames; ++i) {
        const double t = static_cast<double>(i) / 48000.0;
        value[i] = static_cast<qint16>(amplitude * 32767.0
                                       * std::sin(2.0 * std::numbers::pi * 440.0 * t));
    }
    recorder.encode(QAudioBuffer(samples, format));
    // The row is repainted through the event loop, and a grab in the same turn
    // would draw the state before the buffer (finding 19's family).
    QTest::qWait(150);
}

/**
 * The colours of the meter's bars along its middle row, left to right, in the
 * order they are drawn.
 */
QList<QColor> barColours(const QWidget &window, const QWidget &meter, const QImage &picture)
{
    const qreal ratio = picture.devicePixelRatio();
    const QPoint origin = meter.mapTo(&window, QPoint(0, 0));
    const int y = qRound((origin.y() + meter.height() / 2.0) * ratio);

    QList<QColor> row;
    row.reserve(qRound(meter.width() * ratio));
    for (int x = qRound(origin.x() * ratio); x < qRound((origin.x() + meter.width()) * ratio); ++x) {
        row.append(picture.pixelColor(x, y));
    }
    return row;
}

/** How many runs of `colour` stand in `row` — one run is one bar. */
int runsOf(const QList<QColor> &row, const QColor &colour)
{
    int runs = 0;
    bool inside = false;
    for (const QColor &pixel : row) {
        const bool hit = pixel == colour;
        if (hit && !inside) {
            ++runs;
        }
        inside = hit;
    }
    return runs;
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        qFatal("usage: levelshots <target directory>");
    }

    // A configuration directory of its own, and a real colour scheme in it
    // **before** QApplication: without a kdeglobals the platform theme and
    // KColorScheme read two different sources, and the picture then shows a
    // fault of the runner (CLAUDE.md, finding 38).
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());
    QFile::copy(QStringLiteral("/usr/share/color-schemes/BreezeDark.colors"),
                configuration.path() + QStringLiteral("/kdeglobals"));
    QFile scheme(configuration.path() + QStringLiteral("/kdeglobals"));
    if (scheme.open(QIODevice::Append)) {
        scheme.write("\n[General]\nColorScheme=BreezeDark\n");
        scheme.close();
    }
    QFile plasma(configuration.path() + QStringLiteral("/plasmarc"));
    if (plasma.open(QIODevice::WriteOnly)) {
        plasma.write("[Theme]\nname=breeze-dark\n");
        plasma.close();
    }

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("denkzettel"));
    // Without the domain no catalogue is ever consulted, and the run writes an
    // English picture whatever LANGUAGE says (#138).
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // The domain alone does not make a catalogue findable, and a missing one
    // shows up as a picture in the wrong language that looks finished. So the
    // run stops instead of writing it: a line on stderr would travel the very
    // channel this defect hid behind — without QT_FORCE_STDERR_LOGGING it goes
    // into the journal (finding 25) — while an abort withholds the picture.
    const QStringList languages = KLocalizedString::languages();
    const QSet<QString> catalogues = KLocalizedString::availableApplicationTranslations();
    QStringList catalogueNames(catalogues.cbegin(), catalogues.cend());
    // Sorted, because a QSet hands its members back in no defined order: the
    // same run printed "de en_US" here and "en_US de" in the neighbouring
    // runner, and whoever compares two logs sees a difference that is none.
    catalogueNames.sort();
    qWarning("language: %s · catalogue for: %s", qUtf8Printable(languages.join(QLatin1Char(' '))),
             qUtf8Printable(catalogueNames.join(QLatin1Char(' '))));

    // **A request for the source language is always satisfiable**, and two
    // shapes of it reach here: an empty list, because no locale variable was
    // set at all, and the C or POSIX locale, which *is* the source language.
    // std::all_of over an empty range is true, so both are covered by one
    // condition — and without it the run aborted on a picture that would have
    // come out right. Measured in the review of this change: an unset locale
    // and LANG=C.UTF-8 both ended at 134 with 0 pictures, while a runner
    // without this guard drew the correct English window in the same
    // environment.
    const bool wantsSourceLanguage = std::all_of(languages.cbegin(), languages.cend(), [](const QString &language) {
        return language.isEmpty() || language == QLatin1String("C") || language == QLatin1String("POSIX");
    });
    if (!wantsSourceLanguage
        && std::none_of(languages.cbegin(), languages.cend(), [&catalogues](const QString &language) {
               return catalogues.contains(language);
           })) {
        qFatal("no message catalogue for %s: every picture would come out in the source "
               "language. Install into a staging root and point XDG_DATA_DIRS at it, see the "
               "usage block above",
               qUtf8Printable(languages.join(QLatin1Char(' '))));
    }
    // ponytail: presence, not currency. The guard says a catalogue for the
    // wanted language is reachable, never which file that is — the documented
    // invocation leaves /usr/share at the end of XDG_DATA_DIRS, so a forgotten
    // staging root resolves to the installed copy, which can lag behind the
    // source (findings 21 and 57). Upgrade path: the marker readback this
    // change was proved with — a msgstr altered in the staged catalogue, read
    // back out of the run — turned into a check of its own.

    // Read back what the run really drew with, rather than trusting that the
    // variables were set (findings 28 and 38).
    qWarning("style: %s", qUtf8Printable(app.style()->objectName()));
    qWarning("palette Base %s Text %s PlaceholderText %s",
             qUtf8Printable(app.palette().color(QPalette::Base).name()),
             qUtf8Printable(app.palette().color(QPalette::Text).name()),
             qUtf8Printable(app.palette().color(QPalette::PlaceholderText).name()));

    const QString directory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(directory);

    const QTemporaryDir data;
    Store store(data.path() + QStringLiteral("/denkzettel.db"));
    if (!store.open()) {
        qFatal("store: %s", qUtf8Printable(store.lastError()));
    }

    RecordingWindow window(&store);
    if (!window.startWithoutADevice()) {
        qFatal("the recording never started: %s",
               qUtf8Printable(window.recorder()->lastError()));
    }
    if (!QTest::qWaitForWindowExposed(&window)) {
        qFatal("the recording window never reached the screen");
    }

    const QWidget *meter = window.findChild<QWidget *>(QStringLiteral("levelMeter"));
    if (meter == nullptr) {
        qFatal("no widget named levelMeter — the meter cannot be counted");
    }
    QSignalSpy levels(window.recorder(), &AudioRecorder::levelChanged);

    // The two colours, from the two ends where the answer cannot be in doubt.
    feed(*window.recorder(), 0);
    const QColor unlit = barColours(window, *meter, window.grab().toImage()).constFirst();
    feed(*window.recorder(), 1);
    const QColor lit = barColours(window, *meter, window.grab().toImage()).constFirst();
    if (lit == unlit) {
        qFatal("silence and full scale draw the same colour — nothing can be counted");
    }
    // The rectangle a picture difference is read against, in device pixels and
    // set from outside the picture — a difference box that begins inside the
    // widget looks exactly like a widget that has moved (finding 64).
    const QPoint origin = meter->mapTo(&window, QPoint(0, 0));
    const qreal ratio = window.devicePixelRatioF();
    qWarning("meter at x=%d y=%d w=%d h=%d · device x %d..%d y %d..%d · dpr %.2f · lit %s · "
             "unlit %s",
             origin.x(), origin.y(), meter->width(), meter->height(),
             qRound(origin.x() * ratio), qRound((origin.x() + meter->width()) * ratio),
             qRound(origin.y() * ratio), qRound((origin.y() + meter->height()) * ratio), ratio,
             qUtf8Printable(lit.name()), qUtf8Printable(unlit.name()));

    // From a quiet room to full scale. The amplitudes are the run's own; what
    // is measured against them is the level the recorder reports and the bars
    // the picture shows.
    const QList<qreal> amplitudes{0, 0.005, 0.02, 0.05, 0.1, 0.2, 0.5, 1};
    for (const qreal amplitude : amplitudes) {
        levels.clear();
        feed(*window.recorder(), amplitude);
        const qreal reported = levels.isEmpty() ? -1 : levels.takeLast().constFirst().toReal();

        const QImage picture = window.grab().toImage();
        const QList<QColor> row = barColours(window, *meter, picture);
        const int litBars = runsOf(row, lit);
        const int bars = litBars + runsOf(row, unlit);

        const QString name = QStringLiteral("pegel-%1.png")
                                 .arg(qRound(amplitude * 1000), 4, 10, QLatin1Char('0'));
        if (!picture.save(directory + QLatin1Char('/') + name)) {
            qFatal("could not write %s", qUtf8Printable(name));
        }
        qWarning("amplitude %.3f · level %.4f · %.2f dBFS · %d of %d bars · %s", amplitude,
                 reported, amplitude > 0 ? 20 * std::log10(amplitude) : -999.0, litBars, bars,
                 qUtf8Printable(name));
    }

    // A recorder taken down mid-recording deletes its file, and nothing here
    // wants the note it would otherwise make.
    window.recorder()->cancel();
    QTest::qWait(300);
    return 0;
}
