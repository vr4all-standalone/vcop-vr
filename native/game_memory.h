// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
namespace vcop {
struct GunCalibration {
    uint32_t xmin{},xmax{},ymin{},ymax{};
    bool valid() const { return xmin<xmax && ymin<ymax && xmax<=1023 && ymax<=1023; }
};
// Pure bounded calibration arithmetic; VC1 memory mapping remains separate.
inline double native_gun_axis(uint32_t adc,uint32_t minimum,uint32_t maximum,unsigned scale) {
    if(minimum>=maximum || maximum>1023 || !scale)return -1;
    return double(float(std::clamp(adc,minimum,maximum)-minimum)*float(scale)/float(maximum-minimum));
}
// Invert to the closest *native integer pixel*. Native ADC quantisation means
// some screen pixels cannot be reached exactly. No five-percent compression.
inline uint16_t calibrated_gun_axis(uint16_t normalized,uint32_t minimum,uint32_t maximum,unsigned scale,unsigned pixels) {
    if(minimum>=maximum || maximum>1023 || !scale || !pixels)return 0;
    const double target=double(normalized)*(pixels-1)/65535.;
    const double ideal=minimum+target*(maximum-minimum)/scale;
    uint32_t best=std::clamp(uint32_t(ideal),minimum,maximum);
    double error=std::abs(native_gun_axis(best,minimum,maximum,scale)-target);
    for(uint32_t v=best;v<=std::min(maximum,best+2);++v) {
        const double next=std::abs(native_gun_axis(v,minimum,maximum,scale)-target);
        if(next<error){best=v;error=next;}
    }
    return uint16_t(best);
}
inline std::array<uint16_t,2> calibrated_gun_position(uint16_t x,uint16_t y,const GunCalibration& c,unsigned width=496,unsigned height=384) {
    return {calibrated_gun_axis(x,c.xmin,c.xmax,496,width),calibrated_gun_axis(y,c.ymin,c.ymax,384,height)};
}
// Original VC1 routine57f20 reads UINT16 calibration from backup SRAM.
// Its projection uses float496/384, unlike the VC2 integer division adapter.
template<class Read32> GunCalibration read_gun_calibration(Read32 read,unsigned player) {
    if(player>=2)return {};
    const auto word=[&](uint32_t a){return (uint32_t(read(a&~3u))>>((a&3)*8))&65535u;};
    const uint32_t base=0x1d00024+player*8;
    GunCalibration c{word(base),word(base+4),word(base+2),word(base+6)};
    return c.valid()?c:GunCalibration{};
}
}
