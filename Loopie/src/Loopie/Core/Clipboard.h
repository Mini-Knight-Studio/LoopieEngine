#include <string>

namespace Loopie
{
    class Clipboard
    {
    public:
        Clipboard() = default;
        ~Clipboard() = default;

        virtual void Copy(std::string uuid);
        virtual std::string Paste();
    private:
        std::string m_uuid;
    };
}