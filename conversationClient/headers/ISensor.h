#pragma once

class ISensor {
public:
    virtual ~ISensor() = default;
    virtual void update() = 0;
};