#pragma once
template <typename T>
class Singleton
{
public:
    template<typename... Args>
    static void Initialize(Args&&... args)
    {
        assert(s_Instance == nullptr);
        s_Instance = new T(std::forward<Args>(args)...);
    }

    static void Shutdown()
    {
        assert(s_Instance != nullptr);
        delete s_Instance;
        s_Instance = nullptr;
    }

    static T& Get()
    {
        assert(s_Instance != nullptr);
        return *s_Instance;
    }

    static T* GetPtr()
    {
        return s_Instance;
    }

    static bool IsInitialized()
    {
        return s_Instance != nullptr;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

protected:
    Singleton() = default;
    virtual ~Singleton() = default;

private:
    inline static T* s_Instance = nullptr;
};