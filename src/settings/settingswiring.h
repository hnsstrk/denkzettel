#pragma once

class AnalysisScheduler;
class Embedder;
class OllamaProvider;
class OriginWatcher;
class Suggester;
class Transcriber;

/**
 * Hands what the settings dialog has just written to the objects that are
 * already running (SPEC 13).
 *
 * **Here and not in main.cpp, and that is the whole of issue #123.** For
 * several features these connections *are* the feature: without one of them the
 * user changes a setting, the dialog writes it, the page confirms it, and the
 * running program goes on with the old value — the silent fault issue #119 was
 * built against. A misspelled pointer to a member function is a compile error;
 * a line that was deleted or never written is not, and `main.cpp` is linked by
 * no library, so no test set could reach it. Measured on 29.08.2026: the three
 * connections of issue #119 removed, rebuilt, `ctest` **14/14 green** — the
 * same output as the unchanged state.
 *
 * **The other side of the assurance is `settingstest`**, and it is named here
 * rather than claimed: that set links `denkzettelsettings`, which is the
 * library this file is built into, and
 * `SettingsTest::everySettingReachesItsRunningObject()` reads every connection
 * below back one by one (CLAUDE.md, finding 48).
 *
 * **What belongs here** is every connection whose sender is `Settings::self()`
 * *and* whose receiver outlives the settings dialog — a value out of
 * `denkzettelrc` reaching an object that lives as long as the daemon does. Not
 * every connection on that signal: a dialog page hangs on it too
 * (`voicenotespage.cpp`), and a page is built and destroyed per opening, so it
 * wires itself and nothing here could. Everything else main.cpp connects stays
 * where it is: those are signals of our own objects to one another, and each of
 * them carries a check or a window that would notice.
 *
 * ponytail: the assurance reaches the seven connections in this function and
 * **not the one call that runs it**. Measured on 30.08.2026: with
 * `connectSettingsToRunningObjects()` deleted from main.cpp the build succeeds
 * and `ctest` stays at 14/14, because main.cpp is linked by no library and no
 * test set reaches it whatever stands in it. **The ceiling is therefore one
 * silent hole instead of seven**, and it is the call site. The way up is the
 * other reading of issue #123 — a run that starts the daemon, writes a setting
 * and reads the arrival off its behaviour; the first end-to-end harness in this
 * project, which is why it was not built here.
 *
 * A free function and not a class: it holds nothing between two calls, and a
 * class whose whole body ran in its constructor would be an object nobody ever
 * asks anything.
 */
void connectSettingsToRunningObjects(Transcriber *transcriber,
                                     OriginWatcher *origins,
                                     OllamaProvider *provider,
                                     Embedder *embedder,
                                     Suggester *suggester,
                                     AnalysisScheduler *analysis);
