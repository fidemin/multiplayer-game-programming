#pragma once
#include <queue>
#include "InFlightPacket.cpp"
#include "OutputMemoryBitStream.cpp"
#include "InputMemoryBitStream.cpp"

class AckRange {
    public:
        AckRange() : mStart(0), mCount(0) {}
        AckRange(PacketSequenceNumber inStart) 
            : mStart(inStart) {
                mCount = 1;
            }

        bool ExtendIfAdjacent(PacketSequenceNumber inSequenceNumber) {
            if (inSequenceNumber == mStart + mCount) {
                mCount++;
                return true;
            }
            return false;
        }        

        void Write(OutputMemoryBitStream& inOutputStream) const {
            inOutputStream.Write(mStart);
            bool hasCount = (mCount > 1);
            inOutputStream.Write(hasCount);

            if (hasCount) {
                uint32_t countMinusOne = mCount - 1;
                uint8_t countToAck = (countMinusOne > 255) ? 255 : static_cast<uint8_t>(countMinusOne);
                inOutputStream.Write(countToAck);
            }
        }

        void Read(InputMemoryBitStream& inInputStream) {
            inInputStream.Read(mStart);
            bool hasCount;
            inInputStream.Read(hasCount);
            if (hasCount) {
                uint8_t countToAck;
                inInputStream.Read(countToAck);
                mCount = countToAck + 1;
            } else {
                // default to 1 for single ack
                mCount = 1;
            }
        }

        PacketSequenceNumber GetStart() const { return mStart; }
        uint32_t GetCount() const { return mCount; }
    private:
        PacketSequenceNumber mStart;
        u_int32_t mCount;
};


// DeliveryNotificationManager per server-client connection.
class DeliveryNotificationManager {
    public:
        InFlightPacket* WriteSequenceNumber(OutputMemoryBitStream& inOutputStream);
        bool ProcessSequenceNumber(InputMemoryBitStream& inInputStream);
        void WritePendingAcks(OutputMemoryBitStream& inOutputStream);
        void ProcessAcks(InputMemoryBitStream& inInputStream);
    private:
        PacketSequenceNumber mNextOutgoingSequenceNumber = 1;
        PacketSequenceNumber mNextExpectedSequenceNumber;
        uint32_t mDispatchedPacketCount = 0;
        uint32_t mAckedPacketCount = 0;
        uint32_t mDroppedPacketCount = 0;
        std::queue<InFlightPacket> mInFlightPackets;
        std::queue<AckRange> mPendingAcks;

        void AddPendingAck(PacketSequenceNumber inSequenceNumber);
        void HandlePacketDeliverySuccess(InFlightPacket& inFlightPacket);
        void HandlePacketDeliveryFailure(InFlightPacket& inFlightPacket);
};


InFlightPacket* DeliveryNotificationManager::WriteSequenceNumber(OutputMemoryBitStream& inOutputStream) {
    PacketSequenceNumber sequenceNumber = mNextOutgoingSequenceNumber++;
    inOutputStream.Write(sequenceNumber);

    mDispatchedPacketCount++;

    mInFlightPackets.emplace(sequenceNumber);
    return &mInFlightPackets.back();
}

bool DeliveryNotificationManager::ProcessSequenceNumber(InputMemoryBitStream& inInputStream) {
    PacketSequenceNumber sequenceNumber;
    inInputStream.Read(sequenceNumber);

    if (sequenceNumber < mNextExpectedSequenceNumber) {
        // This is an old packet, so ignore it
        return false;
    }

    mNextExpectedSequenceNumber = sequenceNumber + 1;
    AddPendingAck(sequenceNumber);
    return true;
};

void DeliveryNotificationManager::AddPendingAck(PacketSequenceNumber inSequenceNumber) {
    if (mPendingAcks.empty() || !mPendingAcks.back().ExtendIfAdjacent(inSequenceNumber)) {
        mPendingAcks.emplace(inSequenceNumber);
    }
}


void DeliveryNotificationManager::WritePendingAcks(OutputMemoryBitStream& inOutputStream) {
    bool hasAcks = !mPendingAcks.empty();
    inOutputStream.Write(hasAcks);

    if (hasAcks) {
        while (!mPendingAcks.empty()) {
            mPendingAcks.front().Write(inOutputStream);
            mPendingAcks.pop();
        }
    }
}

void DeliveryNotificationManager::ProcessAcks(InputMemoryBitStream& inInputStream) {
    bool hasAcks;
    inInputStream.Read(hasAcks);

    if (hasAcks) {
        AckRange ackRange;
        ackRange.Read(inInputStream);

        PacketSequenceNumber packetToCheck = ackRange.GetStart();
        uint32_t packetToCheckEnd = packetToCheck + ackRange.GetCount();

        while (packetToCheck < packetToCheckEnd && !mInFlightPackets.empty()) {
            InFlightPacket& nextInFlightPacket = mInFlightPackets.front();
            if (nextInFlightPacket.GetSequenceNumber() == packetToCheck) {
                // Acked packet
                HandlePacketDeliverySuccess(nextInFlightPacket);

                mInFlightPackets.pop();
                return;
            } else if (nextInFlightPacket.GetSequenceNumber() < packetToCheck) {
                // missed inflight packet, so treat it as a failure

                // Note: we need to make a copy of the packet before popping it from the queue
                // because handling delivery failure may involve accessing data in the packet that would be destroyed after popping
                auto copyOfNextInFlightPacket = nextInFlightPacket;
                mInFlightPackets.pop();

                HandlePacketDeliveryFailure(copyOfNextInFlightPacket);
            } else {
                // inflight packet is newer than the acked packet, so move on to the next ack
                // some acks are removed before processed for some reason (Maybe, timeout?)
                packetToCheck = nextInFlightPacket.GetSequenceNumber();
            }
        }
    }
}    

void DeliveryNotificationManager::HandlePacketDeliverySuccess(InFlightPacket& inFlightPacket) {
    mAckedPacketCount++;
    inFlightPacket.HandleDeliverySuccess(this);
}

void DeliveryNotificationManager::HandlePacketDeliveryFailure(InFlightPacket& inFlightPacket) {
    mDroppedPacketCount++;
    inFlightPacket.HandleDeliveryFailure(this);
}
