/*
** EPITECH PROJECT, 2021
** Serialization
** File description:
** FNSer
*/

#ifndef BAS_HPP_
#define BAS_HPP_

#include <algorithm>
#include <cstring>
#include <vector>

#include <iostream>

////////////////////////////////////////////

#define _BAS_SIZE_BYTES_ 2 // Number of bytes used to store var size and array size in the payload,
#define _BAS_ARRAY_SIZE_ 2 // MAX is your OS size_t byte size (4 is max recommended as it will be compatible in most cases)

////////////////////////////////////////////

using uchar = unsigned char;
using uint = unsigned int;
namespace bas {

class SerializedObject {
public:
    SerializedObject() = default;

    inline SerializedObject(const char* data, size_t size)
    {
        _data.reserve(size);
        for (size_t i = 0; i < size; i++)
            _data.push_back((uchar)data[i]);
    }

    inline SerializedObject(const SerializedObject& other)
        : _data(other._data)
    {
    }

    ~SerializedObject() = default;

    template <typename T>
    inline void pushData(const T& data_, uint array_size = 1)
    {
        const uchar* data = (const uchar*)&data_;
        uint size = sizeof(T);

        for (size_t i = 0; i < _BAS_SIZE_BYTES_; i++)
            _data.push_back((size >> (i * 8)) & 0xFF);

        for (size_t i = 0; i < _BAS_ARRAY_SIZE_; i++)
            _data.push_back((array_size >> (i * 8)) & 0xFF);

        for (size_t j = 0; j < array_size; j++) {
            for (size_t i = 0; i < size; i++) {
                _data.push_back(data[j * size + i]);
            }
        }
    }

    template <typename T>
    inline void pushData(const T* data_, uint array_size = 1)
    {
        const uchar* data = (const uchar*)data_;
        uint size = sizeof(T);

        for (size_t i = 0; i < _BAS_SIZE_BYTES_; i++)
            _data.push_back((size >> (i * 8)) & 0xFF);

        for (size_t i = 0; i < _BAS_ARRAY_SIZE_; i++)
            _data.push_back((array_size >> (i * 8)) & 0xFF);

        for (size_t j = 0; j < array_size; j++) {
            for (size_t i = 0; i < size; i++) {
                _data.push_back(data[j * size + i]);
            }
        }
    }

    template <typename T>
    inline T popData()
    {
        uint size = 0;
        uint array_size = 0;
        T var;

        for (size_t i = 0; i < _BAS_SIZE_BYTES_; i++) {
            size |= (_data.front() << (i * 8));
            _data.erase(_data.begin());
        }

        for (size_t i = 0; i < _BAS_ARRAY_SIZE_; i++) {
            array_size |= (_data.front() << (i * 8));
            _data.erase(_data.begin());
        }

        for (size_t j = 0; j < array_size; j++) {
            std::memcpy(&var + j, _data.data(), size);
            for (size_t i = 0; i < size; i++) {
                _data.erase(_data.begin());
            }
        }
        return var;
    }

    template <typename T>
    inline T* popDataArray()
    {
        uint size = 0;
        uint array_size = 0;
        T* var;

        for (size_t i = 0; i < _BAS_SIZE_BYTES_; i++) {
            size |= (_data.front() << (i * 8));
            _data.erase(_data.begin());
        }

        for (size_t i = 0; i < _BAS_ARRAY_SIZE_; i++) {
            array_size |= (_data.front() << (i * 8));
            _data.erase(_data.begin());
        }

        var = new T[array_size];

        for (size_t j = 0; j < array_size; j++) {
            std::memcpy(var + j, _data.data(), size);
            for (size_t i = 0; i < size; i++) {
                _data.erase(_data.begin());
            }
        }
        return var;
    }

    inline const uchar* payload() const
    {
        return _data.data();
    }

    inline size_t size() const
    {
        return _data.size();
    }

    inline void clear() {
        _data.clear();
    }

    inline SerializedObject& operator=(const SerializedObject& other)
    {
        _data = other._data;
        return *this;
    }

private:
    std::vector<uchar> _data;
};

class Serializable {
public:
    virtual ~Serializable() = default;
    virtual SerializedObject serialize() = 0;
    virtual void unserialize(SerializedObject obj) = 0;

protected:

    inline SerializedObject concludeSerialization()
    {
        SerializedObject obj = _obj;
        _obj.clear();
        return obj;
    }

    template <typename T>
    inline void pushData(const T& data, uint array_size = 1)
    {
        _obj.pushData(data, array_size);
    }

    template <typename T>
    inline void pushData(const T* data, uint array_size = 1)
    {
        _obj.pushData(data, array_size);
    }

    SerializedObject _obj;

private:
};

}

#endif /* !BAS_HPP_ */
