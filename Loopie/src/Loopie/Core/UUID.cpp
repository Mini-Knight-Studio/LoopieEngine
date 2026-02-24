#include "Loopie/Core/UUID.h"
#include "Loopie/Core/Random.h"
#include "Loopie/Core/Assert.h"

namespace Loopie {

    UUID::UUID() : m_id(Generate()) {
    }

    UUID::UUID(const std::string& id)
        : m_id(id) {
        ASSERT(m_id.size() != UUID_SIZE, "UUID id does not have a correct size of {0}, current size is {1}", UUID_SIZE, m_id.size());
    }

    const std::string& UUID::Get() const {
        return m_id;
    }

    std::string UUID::Generate() {
        const char* v = "0123456789abcdef";
        const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

        std::string res;
        res.reserve(UUID_SIZE);

        for (int i = 0; i < 16; i++) {
            if (dash[i]) res += '-';
            res += v[Random::Get(0, 15)];
            res += v[Random::Get(0, 15)];
        }
        return res;
    }

    bool UUID::IsValid(const std::string& id)
    {
        if (id.size() != UUID_SIZE)
            return false;
        return true;
    }

    bool UUID::operator==(const UUID& other) const {
        return m_id == other.m_id;
    }

}
