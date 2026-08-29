#pragma once

#include <QWidget>

/**
 * The page "Analysis" of SPEC 13: when an analysis run starts, and how far
 * apart the runs are when they are periodic (SPEC 7.2).
 */
class AnalysisPage : public QWidget
{
    Q_OBJECT

public:
    explicit AnalysisPage(QWidget *parent = nullptr);
};
