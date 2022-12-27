#ifndef __WYD_RUPTURE_HPP__
#define __WYD_RUPTURE_HPP__

#include <cstdint>
#include <cstdlib>

class Wyd_Rupture
{
private:
    Wyd_Rupture(/* arguments */);
    virtual ~Wyd_Rupture() {}
    Wyd_Rupture(const Wyd_Rupture &) = delete;
    Wyd_Rupture &operator=(const Wyd_Rupture &) = delete;

public:
    static bool Decode(uint8_t *buffer, int32_t offset, int32_t length);
    static bool Encode(uint8_t *buffer, int32_t offset, int32_t length);
};

#endif // !__WYD_RUPTURE_HPP__