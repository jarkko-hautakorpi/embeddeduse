// Copyright (C) 2019, Burkhard Stubert (DBA Embedded Use)

#include "j1939_frame.h"
#include "mockcanbusrouter.h"

MockCanBusRouter::MockCanBusRouter(int canId, const QString &interface, QObject *parent)
    : CanBusRouter{canId, "mockcan", interface, parent}
{
    ::setActualCanFrames(m_device, {});
    ::setExpectedCanFrames(m_device, {});
}

MockCanBusRouter::~MockCanBusRouter()
{
}

void MockCanBusRouter::setWriteErrorInterval(int interval)
{
    m_device->setConfigurationParameter(static_cast<int>(MockConfigurationKey::WriteErrorInterval),
                                        interval);
}

void MockCanBusRouter::expectWriteFrame(const QCanBusFrame &frame)
{
    auto frames = ::expectedCanFrames(m_device);
    frames.append(MockCanFrame{MockCanFrame::Type::Outgoing, frame});
    ::setExpectedCanFrames(m_device, frames);
}

void MockCanBusRouter::expectWriteFrames(const QVector<QCanBusFrame> &frames)
{
    auto expectedFrames = ::expectedCanFrames(m_device);
    for (const auto &frame : frames)
    {
        expectedFrames.append(MockCanFrame{MockCanFrame::Type::Outgoing, frame});
    }
    ::setExpectedCanFrames(m_device, expectedFrames);
}

void MockCanBusRouter::expectReadFrame(const QCanBusFrame &frame)
{
    auto frames = ::expectedCanFrames(m_device);
    frames.append(MockCanFrame{MockCanFrame::Type::Incoming, frame});
    ::setExpectedCanFrames(m_device, frames);
}

void MockCanBusRouter::expectReadFrames(const QVector<QCanBusFrame> &frames)
{
    auto expectedFrames = ::expectedCanFrames(m_device);
    for (const auto &frame : frames)
    {
        expectedFrames.append(MockCanFrame{MockCanFrame::Type::Incoming, frame});
    }
    ::setExpectedCanFrames(m_device, expectedFrames);
}

void MockCanBusRouter::expectReadOwnFrame(const QCanBusFrame &frame)
{
    auto frames = ::expectedCanFrames(m_device);
    frames.append(MockCanFrame{MockCanFrame::Type::OwnIncoming, frame});
    ::setExpectedCanFrames(m_device, frames);
}

void MockCanBusRouter::expectError(QCanBusDevice::CanBusError deviceError,
                                   MockCanFrame::ErrorNo errorNo)
{
    auto frames = ::expectedCanFrames(m_device);
    frames.append(MockCanFrame{deviceError, errorNo});
    ::setExpectedCanFrames(m_device, frames);
}


MockCanFrameCollection MockCanBusRouter::actualCanFrames() const
{
    return ::actualCanFrames(m_device);
}

MockCanFrameCollection MockCanBusRouter::expectedCanFrames() const
{
    return ::expectedCanFrames(m_device);
}

