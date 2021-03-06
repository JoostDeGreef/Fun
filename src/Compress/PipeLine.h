#pragma once

#include <tuple>

#include "ICompress.h"

// pipeline several compressors

template<class... Objects>
class PipeLineCommon
{
private:
    std::tuple<Objects...> m_objects;

private:
    template <size_t ...I>
    struct reversed_index_sequence : public std::index_sequence<I...> {};

    template <size_t N, size_t ...I>
    struct make_reversed_index_sequence : public make_reversed_index_sequence<N - 1, I..., N - 1> {};

    template <size_t ...I>
    struct make_reversed_index_sequence<0, I...> : public reversed_index_sequence<I...> {};

    template<class... ObjectsToIndex>
    using reversed_index_sequence_for = make_reversed_index_sequence<sizeof...(ObjectsToIndex)>;

    template<typename Func>
    void ApplyImplementation(Func&& /*func*/)
    {}
    template<typename Func, class ApplyObject, class... ApplyObjects>
    void ApplyImplementation(Func&& func, ApplyObject& object, ApplyObjects& ... objects)
    {
        func(object);
        ApplyImplementation(func, objects...);
    }
    template<typename Func, typename ...T, size_t ...I>
    void ApplyImplementation(Func&& func, std::tuple<T...> &objects, std::index_sequence<I...>)
    {
        ApplyImplementation(func, std::get<I>(objects) ...);
    }
protected:
    template<typename Func>
    void Apply(Func&& func, const bool reverse)
    {
        if (reverse)
        {
            ApplyImplementation(func, m_objects, reversed_index_sequence_for<Objects...>());
        }
        else
        {
            ApplyImplementation(func, m_objects, std::index_sequence_for<Objects...>());
        }
    }
};

template<class... Compressors>
class PipeLineCompressor : public ICompressor, PipeLineCommon<Compressors...>
{
public:
    void Compress(std::vector<unsigned char>& ioBuffer) override
    {
        this->Apply([&ioBuffer](ICompressor& compressor) {compressor.Compress(ioBuffer); }, false);
    }
    void Finish(std::vector<unsigned char>& ioBuffer) override
    {
        this->Apply([&ioBuffer](ICompressor& compressor) {compressor.Finish(ioBuffer); }, false);
    }
};

template<class... DeCompressors>
class PipeLineDeCompressor : public IDeCompressor, PipeLineCommon<DeCompressors...>
{
public:
    void DeCompress(std::vector<unsigned char>& ioBuffer) override
    {
        this->Apply([&ioBuffer](IDeCompressor& decompressor) {decompressor.DeCompress(ioBuffer); }, true);
    }
    void Finish(std::vector<unsigned char>& ioBuffer) override
    {
        this->Apply([&ioBuffer](IDeCompressor& decompressor) {decompressor.Finish(ioBuffer); }, true);
    }
};

