#include "store/keystore.h"

#include <KLocalizedString>
#include <KWallet>

#include <QLatin1StringView>
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
    if (m_wallet != nullptr && m_wallet->isOpen()) {
        run(request);
        return;
    }

    m_pending.append(std::move(request));
    // The first request in the queue is the one that opens; everything that
    // arrives while the open is out rides along on the same one.
    if (m_pending.size() == 1) {
        openWallet();
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

    // Which wallet the user keeps their secrets in. `LocalWallet()` and not
    // `NetworkWallet()`, measured on KWallet 6.29, whose `kwalletd6` is a
    // compatibility shim over the Secret Service: the network name comes back
    // **empty** there while the local one answers `kdewallet`. An empty name is
    // not a wallet, and handing it on is what makes the service ask the user
    // about a wallet that does not exist — measured, that request then never
    // completes.
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
        const QList<Request> pending = std::exchange(m_pending, {});
        for (const Request &request : pending) {
            run(request);
        }
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

void KeyStore::run(const Request &request)
{
    const QString failed = i18n("The password store refused the API key.");

    if (!m_wallet->hasFolder(WalletFolder)) {
        // Nothing was ever stored. For a read and for a removal that is the
        // answer rather than a failure; only a write has to make the folder.
        if (request.action != Action::Write) {
            answer(request, QString(), QString());
            return;
        }
        if (!m_wallet->createFolder(WalletFolder)) {
            answer(request, QString(), failed);
            return;
        }
    }

    if (!m_wallet->setFolder(WalletFolder)) {
        answer(request, QString(), failed);
        return;
    }

    // Asked before every read and every removal, because "there is no such
    // entry" is not a failure — the user has simply not entered a key yet —
    // and the caller has to be able to tell that from a store that is broken.
    // `readPassword()` and `removeEntry()` return one non-zero code for
    // everything that went wrong (`kwallet.h:398` and `:509`), so on their
    // answer alone the two cannot be told apart.
    const bool present = m_wallet->hasEntry(request.provider);

    switch (request.action) {
    case Action::Read: {
        QString key;
        if (present && m_wallet->readPassword(request.provider, key) != 0) {
            answer(request, QString(), failed);
            return;
        }
        answer(request, key, QString());
        return;
    }
    case Action::Write:
        answer(request, QString(), m_wallet->writePassword(request.provider, request.value) == 0 ? QString() : failed);
        return;
    case Action::Remove:
        answer(request, QString(), !present || m_wallet->removeEntry(request.provider) == 0 ? QString() : failed);
        return;
    }
}
