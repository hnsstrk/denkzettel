// Probe for the vorprüfung of #76: does the const suggested by
// misc-const-correctness for main.cpp still compile? Nothing is changed in the
// project; this file reproduces the pattern of src/main.cpp:59/61 in isolation.
#include <QObject>

class Sender : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void request();
};

class Receiver : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void act() {}
};

int main()
{
    Sender tray;
#ifdef PROBE_CONST
    const Receiver library;
#else
    Receiver library;
#endif
    QObject::connect(&tray, &Sender::request, &library, &Receiver::act);
    return 0;
}
#include "constprobe.moc"
