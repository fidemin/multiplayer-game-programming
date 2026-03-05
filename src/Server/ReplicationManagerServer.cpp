
#pragma once
#include <queue>
#include "../LinkingContext.cpp"
#include "../ReplicationManager.cpp"
#include "../ErrorUtil.cpp"
#include "../BitsHelper.cpp"
#include "../InFlightPacket.cpp"

struct ReplicationCommand {
    uint32_t networkId;
    ReplicationAction action;
};

// ReplicationManagerServer: one per server-client connection. Each ClientProxy has its own ReplicationManagerServer to manage replication commands for that client. 
class ReplicationManagerServer: public ReplicationManager {
    public:
        ReplicationManagerServer(LinkingContext* inLinkingContext) : ReplicationManager(inLinkingContext) {}
        void AddReplicationCommand(uint32_t networkId, ReplicationAction action);        
        void WritePendingCommands(OutputMemoryBitStream& inStream, InFlightPacket* inFlightPacket = nullptr);
    private:
        std::queue<ReplicationCommand> mPendingCommands;
};

void ReplicationManagerServer::AddReplicationCommand(uint32_t networkId, ReplicationAction action) {
    mPendingCommands.push({networkId, action});
}

void ReplicationManagerServer::WritePendingCommands(OutputMemoryBitStream& outStream, InFlightPacket* inFlightPacket) {
    // TODO: InFlightPacket can be used to track replication data sent with this packet.

    int maxCommandsPerPacket = 10;

    // Collect valid commands first so we know the exact count before writing it
    std::vector<std::pair<ReplicationCommand, GameObject*>> validCommands;
    int processed = 0;
    while (!mPendingCommands.empty() && processed++ < maxCommandsPerPacket) {
        ReplicationCommand command = mPendingCommands.front();
        mPendingCommands.pop();
        GameObject* gameObject = mLinkingContext->GetGameObject(command.networkId);
        if (gameObject) {
            validCommands.push_back({command, gameObject});
        } else {
            ErrorUtil::ReportError((L"ReplicationManagerServer::WritePendingCommands - GameObject not found for network ID: " + std::to_wstring(command.networkId)).c_str());
        }
    }

    outStream.Write(static_cast<uint32_t>(validCommands.size()));

    for (auto& pair : validCommands) {
        switch (pair.first.action) {
            case RA_Create:
                ReplicateCreate(outStream, pair.second);
                break;
            case RA_Update:
                ReplicateUpdate(outStream, pair.second);
                break;
            case RA_Destroy:
                ReplicateDestroy(outStream, pair.second);
                break;
            default:
                ErrorUtil::ReportError((L"ReplicationManagerServer::WritePendingCommands - Unknown replication action: " + std::to_wstring(pair.first.action)).c_str());
                break;
        }
    }
}
