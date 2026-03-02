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
        const std::vector<T>& Paste()
        {
            return m_data;
        };

        const T& PasteFirst()
        {
            return m_data.front();
        };

    private:
        std::vector<T> m_data;
    };
}