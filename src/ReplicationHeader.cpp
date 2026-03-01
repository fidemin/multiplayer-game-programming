#pragma once
#include <cstdint>
#include "OutputMemoryBitStream.cpp"
#include "InputMemoryBitStream.cpp"
#include "BitsHelper.cpp"

enum ReplicationAction {
    RA_Create,
    RA_Update,
    RA_Destroy,
    RA_MAX,
};

class ReplicationHeader {
    public:
        ReplicationHeader() {};
        ReplicationHeader(ReplicationAction inAction, uint32_t inNetworkId, uint32_t inClassId=0) : 
        mAction(inAction), mNetworkId(inNetworkId), mClassId(inClassId) 
        {}

        void Write(OutputMemoryBitStream& inStream) const {
            inStream.WriteBits(mAction, GetRequiredBits(RA_MAX));
            inStream.Write(mNetworkId);

            // For Create and Update actions, we need to include the class ID to know how to deserialize the object on the receiving end. 
            // For Destroy action, we only need the network ID to identify which object to destroy. (use GameObject's Destroy() method)
            if (mAction != RA_Destroy) {
                inStream.Write(mClassId);
            }
        }

        void Read(InputMemoryBitStream& inStream) {
            // initialize with zeros to all bytes of mAction before partial bit read
            // Because enum size is usually 4 bytes, but the actual bits received for mAction may be less than 32 bits
            // we need to clear the remaining bits to avoid having garbage values in mAction after reading from the stream. 
            mAction = RA_Create; 
            inStream.ReadBits(&mAction, GetRequiredBits(RA_MAX));
            inStream.Read(mNetworkId);

            if (mAction != RA_Destroy) {
                inStream.Read(mClassId);
            }
        }

        ReplicationAction GetAction() const { return mAction; }
        uint32_t GetNetworkId() const { return mNetworkId; }
        uint32_t GetClassId() const { return mClassId; }

    private:
        ReplicationAction mAction;
        uint32_t mNetworkId;
        uint32_t mClassId;
};