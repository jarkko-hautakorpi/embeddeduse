﻿// Copyright (C) 2019, Burkhard Stubert (DBA Embedded Use)

#include <algorithm>

#include <QMetaType>
#include <QtDebug>

#include "canbusext.h"
#include "mockcanutils.h"
#include "mocksocketcandevice.h"

MockSocketCanDevice::MockSocketCanDevice(const QString &name, QObject *parent)
    : QCanBusDevice{parent}
    , m_interface{name}
{
    // Required to use QCanBusDevice::CanBusError in asynchronous signal-slot connections and
    // in QSignalSpys.
    qRegisterMetaType<QCanBusDevice::CanBusError>();

    setConfigurationParameter(QCanBusDevice::ConfigurationKey::ReceiveOwnKey, false);
}

MockSocketCanDevice::~MockSocketCanDevice()
{
    close();
}

QList<QCanBusDeviceInfo> MockSocketCanDevice::interfaces()
{
    QList<QCanBusDeviceInfo> result;
    result.append(createDeviceInfo("mcan0"));
    result.append(createDeviceInfo("mcan1"));
    return result;
}

void MockSocketCanDevice::setConfigurationParameter(int key, const QVariant &value)
{
    QCanBusDevice::setConfigurationParameter(key, value);
    if (key == int(MockConfigurationKey::ExpectedCanIo)) {
        m_frameIndex = 0;
        m_frameCount = expectedCanIo(this).size();
        checkForResponses();
    }
}

bool MockSocketCanDevice::writeFrame(const QCanBusFrame &frame)
{
    if (m_frameIndex >= m_frameCount) {
        qWarning() << "Expected no frame, but got " << frame.toString();
    }
    else {
        auto expectedFrame = QCanBusFrame{expectedCanIo(this)[m_frameIndex]};
        ++m_frameIndex;
        if (expectedFrame != frame) {
            qWarning() << "Expected " << expectedFrame.toString()
                       << ", but got " << frame.toString();
        }
    }
    appendActualIoFrame(this, MockCanFrame{MockCanFrame::Type::Outgoing, frame});
    emit framesWritten(1);
    checkForResponses();
    return true;
}

QString MockSocketCanDevice::interpretErrorFrame(const QCanBusFrame &errorFrame)
{
    Q_UNUSED(errorFrame)
    return {};
}

bool MockSocketCanDevice::open()
{
    const auto &interfaces = MockSocketCanDevice::interfaces();
    auto pos = std::find_if(interfaces.cbegin(), interfaces.cend(),
                            [this](const QCanBusDeviceInfo &info) {
                                return info.name() == m_interface;
                            });
    if (pos == interfaces.cend()) {
        close();
        return false;
    }
    setState(QCanBusDevice::ConnectedState);
    return true;
}

void MockSocketCanDevice::close()
{
    setState(QCanBusDevice::UnconnectedState);
}

bool MockSocketCanDevice::isReceiveOwnFrameEnabled() const
{
    return configurationParameter(QCanBusDevice::ConfigurationKey::ReceiveOwnKey).toBool();
}

void MockSocketCanDevice::checkForResponses()
{
    auto expectedFrameColl = expectedCanIo(this);
    auto incomingFrameColl = CanBusFrameCollection{};
    while (m_frameIndex < m_frameCount) {
        auto expectedFrame = expectedFrameColl[m_frameIndex];
        if (expectedFrame.isOutgoing()) {
            break;
        }
        if (!expectedFrame.isOwnIncoming() || isReceiveOwnFrameEnabled()) {
            appendActualIoFrame(this, expectedFrame);
        }
        else {
            qWarning() << "Received own frame " << QCanBusFrame{expectedFrame}.toString()
                       << ", although ReceiveOwnKey disabled.";
        }
        if (expectedFrame.isDeviceError()) {
            setError(expectedFrame.deviceErrorString(), expectedFrame.deviceError());
        }
        else if (expectedFrame.isIncoming()) {
            incomingFrameColl.append(expectedFrame);
        }
        ++m_frameIndex;
    }
    enqueueReceivedFrames(incomingFrameColl);
}