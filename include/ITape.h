#ifndef ITAPE_H
#define ITAPE_H

#include <cstddef>
#include <cstdint>

class ITape
{
public:

    virtual std::int32_t read() = 0;
    virtual void write(std::int32_t value) = 0;

    virtual bool move_left() = 0;
    virtual bool move_right() = 0;

    virtual void rewind() = 0;

    virtual std::size_t size() const = 0;
    virtual std::size_t position() const = 0;

    virtual ~ITape() = default;
};

#endif