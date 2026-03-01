
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

    while (!mPendingCommands.empty() && maxCommandsPerPacket-- > 0) {
         if (mPendingCommands.empty()) {
             ErrorUtil::ReportError(L"ReplicationManagerServer::ProcessPendingCommands - No more pending commands to process\n");
             break;
         }
         ReplicationCommand command = mPendingCommands.front();

         GameObject* gameObject = mLinkingContext->GetGameObject(command.networkId);
         if (!gameObject) {
             ErrorUtil::ReportError((L"ReplicationManagerServer::ProcessPendingCommands - GameObject not found for network ID: " + std::to_wstring(command.networkId)).c_str());   
             mPendingCommands.pop(); // Remove the command from the queue even if the game object is not found to prevent infinite loop
             continue;
         }

         switch (command.action) {
             case RA_Create:
                 ReplicateCreate(outStream, gameObject);
                 break;
             case RA_Update:
                 ReplicateUpdate(outStream, gameObject);
                 break;
             case RA_Destroy:
                 ReplicateDestroy(outStream, gameObject);
                 break;
             default:
                 ErrorUtil::ReportError((L"ReplicationManagerServer::ProcessPendingCommands - Unknown replication action: " + std::to_wstring(command.action)).c_str());
                 break;
         }
         mPendingCommands.pop();
    }
}
