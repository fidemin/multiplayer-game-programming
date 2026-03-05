
#pragma once
#include <vector>
#include "TransmissionData.cpp"
#include "Timing.cpp"

class DeliveryNotificationManager;

using PacketSequenceNumber = uint16_t;

class InFlightPacket {
    public:
        InFlightPacket(PacketSequenceNumber inSequenceNumber) 
            : mSequenceNumber(inSequenceNumber), mTimeDispatched(Timing::sInstance.GetTimeMS()) {}
        void AddTransmissionData(const TransmissionDataPtr& inTransmissionData) {
            mTransmissionDataList.push_back(inTransmissionData);
        }

        void HandleDeliverySuccess(DeliveryNotificationManager* inDeliveryNotificationManager) {
            for (const auto& data : mTransmissionDataList) {
                data->HandleDeliverySuccess(inDeliveryNotificationManager);
            }
        }

        void HandleDeliveryFailure(DeliveryNotificationManager* inDeliveryNotificationManager) {
            for (const auto& data : mTransmissionDataList) {
                data->HandleDeliveryFailure(inDeliveryNotificationManager);
            }
        }

        PacketSequenceNumber GetSequenceNumber() const { return mSequenceNumber; }
        uint64_t GetTimeDispatched() const { return mTimeDispatched; }
    private:
        std::vector<TransmissionDataPtr> mTransmissionDataList;
        PacketSequenceNumber mSequenceNumber;
        uint64_t mTimeDispatched;
};