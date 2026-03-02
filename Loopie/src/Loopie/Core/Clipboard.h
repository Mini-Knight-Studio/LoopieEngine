#include <string>
#include <vector>

namespace Loopie
{
    template <typename T>
    class Clipboard
    {
    public:
        Clipboard() = default;
        ~Clipboard() = default;

        template<typename... Args>
        void Copy(Args&&... args)
        {
            m_data.clear();
            m_data.reserve(sizeof...(args));
            (m_data.emplace_back(std::forward<Args>(args)), ...);
        };
        std::vector<T> Paste()
        {
            return m_data;
        };
    private:
        std::vector<T> m_data;
    };
}