
#include <cstdint>
#include <vector>
#include <cstring>
#include "GameObject.cpp"
#include "OutputMemoryStream.cpp"
#include "InputMemoryStream.cpp"


class RoboCat: public GameObject {
    public:
        RoboCat(): mHealth(10), mMeowCount(3), mHomeBase(0) {
            mName[0] = '\0';
        }

        void SetHealth(int32_t inHealth) { mHealth = inHealth; }
        void SetMeowCount(int32_t inMeowCount) { mMeowCount = inMeowCount; }
        void SetName(const char* inName) {
            size_t nameLength = strlen(inName);
            if (nameLength >= sizeof(mName)) {
                wprintf(L"RoboCat::SetName - name too long, truncating to fit\n");
                nameLength = sizeof(mName) - 1;
            }

            memcpy(mName, inName, nameLength);
            mName[nameLength] = '\0'; // ensure null-termination
        }

        std::string GetDescription() const;
        void Write(OutputMemoryStream& inStream) const;
        void Read(InputMemoryStream& inStream);
    private:
        int32_t mHealth;
        int32_t mMeowCount;
        GameObject* mHomeBase;
        char mName[128];
        std::vector<int32_t> mMiceIndices;
};

std::string RoboCat::GetDescription() const {
    // TODO: implement a more complete description of the RoboCat's state, including all information
    return std::string("RoboCat: health=") + std::to_string(mHealth) +
            ", meowCount=" + std::to_string(mMeowCount) +
            ", name=" + mName +
            ", miceCount=" + std::to_string(mMiceIndices.size());
}

void RoboCat::Write(OutputMemoryStream& inStream) const {
    inStream.Write(mHealth);
    inStream.Write(mMeowCount);
    // TODO: mHomeBase need to be handled
    inStream.Write(mName, sizeof(mName));
    // TODO: mMiceIndices need to be handled
}

void RoboCat::Read(InputMemoryStream& inStream) {
    inStream.Read(mHealth);
    inStream.Read(mMeowCount);
    // TODO: mHomeBase need to be handled
    inStream.Read(mName, sizeof(mName));
    // TODO: mMiceIndices need to be handled
}