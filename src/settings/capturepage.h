#pragma once

#include <QWidget>

/**
 * The page "Capture" of SPEC 13, and the first one in the list: the privacy
 * switch „Herkunft der Notiz mitspeichern" (issue #47).
 *
 * It stands here and deliberately not on the page "Analysis", although the
 * origin feeds the classification of SPEC 7 — a switch against invisible data
 * collection whose own page nobody opens is the collection it was built
 * against (UX decision 29.08.2026).
 */
class CapturePage : public QWidget
{
    Q_OBJECT

public:
    explicit CapturePage(QWidget *parent = nullptr);
};
