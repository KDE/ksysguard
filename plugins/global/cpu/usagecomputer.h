// Helper class to calculate usage percentage values from ticks
class UsageComputer {
public:
    void setTicks(unsigned long long system, unsigned long long  user, unsigned long long wait, unsigned long long idle);
    double totalUsage = 0;
    double systemUsage = 0;
    double userUsage = 0;
    double waitUsage = 0;
private:
    unsigned long long m_totalTicks = 0;
    unsigned long long m_systemTicks = 0;
    unsigned long long m_userTicks = 0;
    unsigned long long m_waitTicks = 0;
};

