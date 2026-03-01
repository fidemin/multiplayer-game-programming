
#pragma once
#include <queue>
#include "../LinkingContext.cpp"
#include "../ReplicationManager.cpp"
#include "../ErrorUtil.cpp"
#include "../BitsHelper.cpp"

struct ReplicationCommand {
    uint32_t networkId;
    ReplicationAction action;
};

class ReplicationManagerServer: public ReplicationManager {
    public:
        ReplicationManagerServer(LinkingContext* inLinkingContext) : ReplicationManager(inLinkingContext) {}
        void AddReplicationCommand(uint32_t networkId, ReplicationAction action);        
        void ProcessPendingCommands(OutputMemoryBitStream& inStream);
    private:
        std::queue<ReplicationCommand> mPendingCommands;
};

void ReplicationManagerServer::AddReplicationCommand(uint32_t networkId, ReplicationAction action) {
    mPendingCommands.push({networkId, action});
}

void ReplicationManagerServer::ProcessPendingCommands(OutputMemoryBitStream& outStream) {
    outStream.WriteBits(PT_ReplicationData, GetRequiredBits(PT_MAX));

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
            ErrorUtil::ReportError((L"ReplicationManagerServer::ProcessPendingCommands - GameObject not found for network ID: " + std::to_wstring(command.networkId)).c_str());
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
                ErrorUtil::ReportError((L"ReplicationManagerServer::ProcessPendingCommands - Unknown replication action: " + std::to_wstring(pair.first.action)).c_str());
                break;
        }
    }
}
