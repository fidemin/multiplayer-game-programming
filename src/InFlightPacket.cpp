
#pragma once
#include <vector>
#include "TransmissionData.cpp"

class DeliveryNotificationManager;

using PacketSequenceNumber = uint16_t;

class InFlightPacket {
    public:
        InFlightPacket(PacketSequenceNumber inSequenceNumber) : mSequenceNumber(inSequenceNumber) {}
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
    private:
        std::vector<TransmissionDataPtr> mTransmissionDataList;
        PacketSequenceNumber mSequenceNumber;
};