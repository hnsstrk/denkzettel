#include "store/keystore.h"

#include <KLocalizedString>
#include <KWallet>

#include <QLatin1StringView>
#include <QMap>
#include <QMetaObject>

#include <utility>

namespace
{
/**
 * The folder our entries live in, one entry per provider.
 *
 * Its own folder and not `Wallet::PasswordFolder()`: that one is the shared
 * drawer every KDE application writes into, and a key named "openai" there
 * would be indistinguishable from anybody else's. In a folder of our own the
 * wallet manager shows what this program keeps, and removing the program's
 * secrets is removing one folder.
 */
const QLatin1StringView WalletFolder{"denkzettel"};
}

KeyStore *KeyStore::self()
{
    static KeyStore instance;
    return &instance;
}

KeyStore::KeyStore() = default;

void KeyStore::readKey(const QString &provider)
{
    enqueue({provider, QString(), Action::Read});
}

void KeyStore::storeKey(const QString &provider, const QString &key)
{
    enqueue({provider, key, Action::Write});
}

void KeyStore::removeKey(const QString &provider)
{
    enqueue({provider, QString(), Action::Remove});
}

void KeyStore::enqueue(Request request)
{
    const bool ready = m_wallet != nullptr && m_wallet->isOpen();
    const QString provider = request.provider;
    m_pending.append(std::move(request));

    if (!ready) {
        // The wallet has to be opened first, and that is the one step here
        // that can take as long as a person needs to type a password. Say so —
        // queued, for the same reason everything else below is queued.
        QMetaObject::invokeMethod(
            this,
            [this, provider] {
                Q_EMIT waitingForWallet(provider);
            },
            Qt::QueuedConnection);
    }

    // The first request in the queue is the one that sets things going;
    // everything that arrives while that is out rides along on the same one.
    if (m_pending.size() > 1) {
        return;
    }

    if (ready) {
        // Through the event loop even with the wallet standing open, and for
        // the reason openWallet() gives below: the caller has not returned
        // from its own call yet. Run straight from here, the interface would
        // answer after the call on the first request of a session and inside
        // it on every one after that — two timings for one method, and the
        // five stories that spend this interface would each meet whichever
        // one they happened to test against.
        QMetaObject::invokeMethod(
            this,
            [this] {
                runPending();
            },
            Qt::QueuedConnection);
        return;
    }

    openWallet();
}

void KeyStore::runPending()
{
    const QList<Request> pending = std::exchange(m_pending, {});
    for (const Request &request : pending) {
        run(request);
    }
}

void KeyStore::openWallet()
{
    // A handle that is not open is of no further use — the wallet was closed
    // under us, or the last open failed. Either way the next attempt starts
    // from nothing. Disconnected first and deleted afterwards, because a
    // caller may well ask again from the very error handler this class calls
    // it in: that road leads back here from inside the old wallet's own
    // signal, where deleting it outright would pull the ground from under the
    // emission, and where a second walletOpened() from it would answer a queue
    // that has moved on.
    if (m_wallet != nullptr) {
        m_wallet->disconnect(this);
        m_wallet->deleteLater();
        m_wallet = nullptr;
    }

    // Which wallet the user keeps their secrets in. Both this and
    // `NetworkWallet()` beside it come back **empty** when no wallet service is
    // reachable on the session — that is what the check below catches, and it
    // is the reason it carries. `LocalWallet()` and not the other one because
    // it is the one that answered a usable name in every state measured, while
    // `NetworkWallet()` answered `kdewallet` on one machine and the empty
    // string on another whose Secret Service held no collection for
    // `kwalletd6` to read (KWallet 6.29, where `kwalletd6` is a compatibility
    // shim over the Secret Service). An empty name is not a wallet, and handing
    // it on is what makes the service ask the user about a wallet that does not
    // exist — measured, that request then never completes.
    //
    // ponytail: this call and openWallet() below each spend one D-Bus round
    // trip on the stack — 85 to 90 ms once, while the wallet service is being
    // activated, and 0 ms from then on. KWallet offers no asynchronous form of
    // either, and moving the name lookup alone off the event loop buys nothing:
    // with the name written out by hand, openWallet() pays the same 90 ms
    // itself. The upgrade path, if a wedged wallet service ever holds a window
    // for the D-Bus reply timeout, is a thread of its own.
    const QString wallet = KWallet::Wallet::LocalWallet();
    if (!wallet.isEmpty()) {
        // Asynchronous is the whole point of this class: the call returns
        // before the service has answered, and the answer arrives as
        // walletOpened(). The window id is 0 because the daemon has no window
        // of its own to hang a password dialog on.
        m_wallet = KWallet::Wallet::openWallet(wallet, 0, KWallet::Wallet::Asynchronous);
    }

    if (m_wallet == nullptr) {
        // No wallet system on this session at all. Answering here and now
        // would hand the caller its answer before its own call had returned,
        // so the failure goes through the event loop like every other one.
        QMetaObject::invokeMethod(
            this,
            [this] {
                answerAll(i18n("There is no password store on this session, so API keys cannot be kept."));
            },
            Qt::QueuedConnection);
        return;
    }

    connect(m_wallet, &KWallet::Wallet::walletOpened, this, [this](bool opened) {
        if (!opened) {
            answerAll(i18n("The password store could not be opened, so API keys cannot be kept."));
            return;
        }
        runPending();
    });
}

void KeyStore::answerAll(const QString &error)
{
    const QList<Request> pending = std::exchange(m_pending, {});
    for (const Request &request : pending) {
        answer(request, QString(), error);
    }
}

void KeyStore::answer(const Request &request, const QString &key, const QString &error)
{
    if (request.action == Action::Read) {
        Q_EMIT keyRead(request.provider, key, error);
    } else {
        Q_EMIT keyStored(request.provider, error);
    }
}

QString KeyStore::failureMessage(Action action)
{
    // One sentence per verb. The read and the removal offer no key at all, so
    // a single "the store refused the API key" over all three would name a
    // thing that never left this process.
    switch (action) {
    case Action::Read:
        return i18n("The API key could not be read from the password store.");
    case Action::Write:
        return i18n("The API key could not be stored in the password store.");
    case Action::Remove:
        return i18n("The API key could not be removed from the password store.");
    }
    return {};
}

void KeyStore::run(const Request &request)
{
    const QString failed = failureMessage(request.action);

    // A wallet that broke after it was opened — the service died, the handle
    // is still there — answers every call with a D-Bus error, and
    // `QDBusReply<bool>` then hands back the value it was default-constructed
    // with. So `hasFolder()`, `hasEntry()` and `setFolder()` all say `false`
    // for "it is not there" **and** for "the call failed", and read through
    // that a broken store is indistinguishable from an empty one: the read
    // hands out an empty key with no error, and the removal reports success
    // over a secret that is still in the wallet.
    //
    // `hasFolder()` is therefore asked for one thing only — to skip a create
    // that is not needed — and its `false` is never itself an answer. From
    // here on every step carries a failure of its own.
    if (!m_wallet->hasFolder(WalletFolder) && !m_wallet->createFolder(WalletFolder)) {
        answer(request, QString(), failed);
        return;
    }
    // And the folder is made for a read as well, not only for a write, which
    // costs an empty folder in the wallet on a first read and buys the one
    // thing worth more: with setFolder() left to fail quietly, the wallet's
    // **global** folder stays current — the one every application may write
    // into — and the passwordList() below would answer out of entries that are
    // not ours.
    if (!m_wallet->setFolder(WalletFolder)) {
        answer(request, QString(), failed);
        return;
    }

    if (request.action == Action::Write) {
        answer(request, QString(), m_wallet->writePassword(request.provider, request.value) == 0 ? QString() : failed);
        return;
    }

    // The one call of this interface with an error channel of its own: `ok`
    // says whether the wallet answered at all, and only then does the map say
    // whether the entry is there. `passwordList()` rather than the
    // `entriesList()` beside it, because it hands the value back as the
    // QString it was written as — one call where a `hasEntry()` and a
    // `readPassword()` stood before, and neither of those two could tell a
    // missing entry from a broken store.
    bool ok = false;
    const QMap<QString, QString> entries = m_wallet->passwordList(&ok);
    if (!ok) {
        answer(request, QString(), failed);
        return;
    }

    if (request.action == Action::Read) {
        // A key that was never stored is an empty key and not a failure: the
        // user has simply not entered one yet.
        answer(request, entries.value(request.provider), QString());
        return;
    }

    // Likewise for the removal — what is not there needs no removing, and
    // saying so is not the same as saying the wallet refused.
    if (!entries.contains(request.provider)) {
        answer(request, QString(), QString());
        return;
    }
    answer(request, QString(), m_wallet->removeEntry(request.provider) == 0 ? QString() : failed);
}
