
#pragma once
#include <memory>

class DeliveryNotificationManager;

class TransmissionData {
    public:
        virtual void HandleDeliverySuccess(DeliveryNotificationManager* inDeliveryNotificationManager) = 0;
        virtual void HandleDeliveryFailure(DeliveryNotificationManager* inDeliveryNotificationManager) = 0;
};

using TransmissionDataPtr = std::shared_ptr<TransmissionData>;