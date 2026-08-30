#pragma once

#include <QList>
#include <QObject>
#include <QString>

#include <cstdint>

namespace KWallet
{
class Wallet;
}

/**
 * The API keys of SPEC 5.2, and the one road they take.
 *
 * SPEC 5.2 says where they may lie and where they may not: in KWallet, never
 * in plain text in a configuration file. So there is no `kcfg_` widget and no
 * skeleton item for a key anywhere in this program — a key that passed through
 * `Settings` would stand in `denkzettelrc` on the next save, and nothing on
 * screen would say so. Whoever needs a key asks here, and only here.
 *
 * **Asynchronous, because opening a wallet is what takes the time.** The
 * wallet service may have to ask the user for a password, and the daemon has
 * one event loop that the capture window lives in as well. So every call below
 * returns at once and is answered by a signal; a call made while the wallet is
 * still opening waits in a queue and is answered from it.
 *
 * **What happens when the wallet stays shut** (the decision this class makes,
 * issue #37): the request is answered with an error and with nothing else. No
 * waiting, no dialog of ours, and above all no second place the key could be
 * kept — a fallback into the configuration file would be exactly the sentence
 * of SPEC 5.2 undone. The caller shows the error where it asked: the settings
 * page beside the field, a provider through the error channel it already
 * carries. The wallet is opened again on the next request, so a user who
 * unlocks it afterwards is served without restarting anything.
 *
 * The wallet is opened on the **first** request and not before: a user who
 * never enters a key is never asked for a wallet password.
 *
 * `provider` is the entry key inside a folder of ours — the names of SPEC 7.1
 * ("openrouter", "openai"). One key per provider, which is what S25b and S26
 * ask for and all they ask for.
 */
class KeyStore : public QObject
{
    Q_OBJECT

public:
    /**
     * The one instance, built on first use and living as long as the process.
     *
     * One and not one per caller, because a `KWallet::Wallet` is a handle the
     * service counts: three providers each opening their own would be three
     * handles and, on a locked wallet, three password prompts for one key.
     */
    static KeyStore *self();

    /** Answered by keyRead(); a key that was never stored is empty, not an error. */
    void readKey(const QString &provider);

    /** Answered by keyStored(). */
    void storeKey(const QString &provider, const QString &key);

    /** Answered by keyStored(); removing what is not there is not an error. */
    void removeKey(const QString &provider);

Q_SIGNALS:
    /** `error` is empty exactly when the wallet answered — `key` may still be empty. */
    void keyRead(const QString &provider, const QString &key, const QString &error);

    /** The answer to storeKey() and to removeKey(); `error` empty means it happened. */
    void keyStored(const QString &provider, const QString &error);

private:
    KeyStore();

    enum class Action : std::uint8_t {
        Read,
        Write,
        Remove,
    };

    struct Request {
        QString provider;
        /** The key to write, and empty for the other two actions. */
        QString value;
        Action action;
    };

    void enqueue(Request request);
    void openWallet();
    void run(const Request &request);
    void answerAll(const QString &error);
    void answer(const Request &request, const QString &key, const QString &error);

    /** Null until the first request, and again after an open that failed. */
    KWallet::Wallet *m_wallet = nullptr;
    /** What came in while the wallet was opening. */
    QList<Request> m_pending;
};
