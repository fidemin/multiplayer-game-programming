
#include <cstdint>
#include <vector>
#include <string>
#include "GameObject.cpp"
#include "OutputMemoryStream.cpp"
#include "InputMemoryStream.cpp"


class RoboCat: public GameObject {
    public:
        RoboCat(): mHealth(10), mMeowCount(3), mHomeBase(0) {}

        void SetHealth(int32_t inHealth) { mHealth = inHealth; }
        void SetMeowCount(int32_t inMeowCount) { mMeowCount = inMeowCount; }
        void SetName(const std::string& inName) { mName = inName; }

        std::string GetDescription() const;
        void Write(OutputMemoryStream& inStream) const;
        void Read(InputMemoryStream& inStream);

    private:
        int32_t mHealth;
        int32_t mMeowCount;
        GameObject* mHomeBase;
        std::string mName;
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
    inStream.WriteStr(mName);
    // TODO: mMiceIndices need to be handled
}

void RoboCat::Read(InputMemoryStream& inStream) {
    inStream.Read(mHealth);
    inStream.Read(mMeowCount);
    // TODO: mHomeBase need to be handled
    inStream.ReadStr(mName);
    // TODO: mMiceIndices need to be handled
}