#ifndef BOOLEANRESULT_H
#define BOOLEANRESULT_H

#include <QObject>
#include <QQmlEngine>

class BooleanResult : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Only for accessing created instances.")

public:
    BooleanResult(QObject *parent = nullptr) : QObject(parent) { }
    ~BooleanResult() { }

    Q_PROPERTY(bool value READ value WRITE setValue NOTIFY valueChanged)
    void setValue(bool val)
    {
        if (m_value == val)
            return;
        m_value = val;
        emit valueChanged();
    }
    bool value() const { return m_value; }
    Q_SIGNAL void valueChanged();

private:
    bool m_value = false;
};

#endif // BOOLEANRESULT_H
