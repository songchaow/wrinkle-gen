#pragma once
#include <cctype>
#include <memory>

class HeightMap {
    uint32_t _size;
    std::unique_ptr<float[]> _data;
public:
    HeightMap(uint32_t size) : _size(size), _data(new float[size*size]) {}
    void Set(uint32_t x, uint32_t y, float val) {
        _data[y*_size+x] = val;
    }
    void Add(uint32_t x, uint32_t y, float val) {
        _data[y*_size+x] += val;
    }
    uint32_t size() { return _size; }
    float* data() { return _data.get(); }
};