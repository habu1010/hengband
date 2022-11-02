#pragma once

template <typename T>
class Singleton {
public:
    static T &get_instance()
    {
        static T t{};

        // 参照に形だけアクセスすることでプログラム実行前に確実にインスタンスを実体化させるようにする
        use(instance);

        return t;
    }

    ~Singleton() = default;
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;
    Singleton(Singleton &&) = delete;
    Singleton &operator=(Singleton &&) = delete;

protected:
    Singleton() = default;

private:
    static void use(const T &){};
    static T &instance;
};

template <typename T>
T &Singleton<T>::instance = Singleton<T>::get_instance();
