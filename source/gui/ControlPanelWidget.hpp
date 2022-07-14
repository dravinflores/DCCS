#pragma once

#include <QWidget>
#include <QPushButton>

class ControlPanelWidget : public QWidget
{
    Q_OBJECT

public:
    ControlPanelWidget(QWidget* parent = nullptr);

public slots:
    void connectionChanged(bool status);
    void executionChanged(bool status);

signals:
    void requestToConnect();
    void requestToDisconnect();
    void requestToStart();
    void requestToStop();

private:
    QPushButton* connection;
    QPushButton* execution;
};