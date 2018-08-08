#pragma once
#include <vector>

class Feature {
public:
    bool hasLoaded;
};

class Features {
public:
    std::vector<Feature*> list;

public:
    Features();
    template <typename T = Feature>
    void AddFeature(T** featurePtr)
    {
        *featurePtr = new T();
        if (*featurePtr) {
            (*featurePtr)->hasLoaded = true;
        }
        this->list.push_back(*featurePtr);
    }
    template <typename T = Module>
    void RemoveFeature(T** featurePtr)
    {
        this->list.erase(*featurePtr);
    }
    void DeleteAll();
    ~Features();
};
