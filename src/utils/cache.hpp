#pragma once

template <typename ID, typename Type>
class Cache : public NoCopyNoMove {
public:
    Cache() = default;

    template <typename... Args>
    Type& get_or_create(ID id, Args&&... args);

private:
    struct Storage {
        ID id;
        Type type;

        template <typename... Args>
        explicit Storage(ID id, Args&&... args)
            : id(std::move(id))
            , type(std::forward<Args>(args)...)
        {
        }
    };

    std::deque<Storage> m_storage;
};

template <typename ID, typename Type>
template <typename... Args>
Type& Cache<ID, Type>::get_or_create(ID id, Args&&... args)
{
    for (Storage& object : m_storage) {
        if (id == object.id) {
            return object.type;
        }
    }

    m_storage.emplace_back(id, std::forward<Args>(args)...);
    return m_storage.back().type;
}