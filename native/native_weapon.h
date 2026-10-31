// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <vector>
namespace vcop {
struct NativeWeaponVertex {float x{},y{},z{},u{},v{};};
struct NativeWeaponPolygon {
    unsigned count{};uint32_t attr{};
    std::array<uint16_t,4> header{};
    std::array<float,3> normal{};
    std::array<NativeWeaponVertex,4> vertices{};
};
struct NativeWeapon {
    bool valid{};
    uint32_t weapon{},model{},object{};
    std::vector<NativeWeaponPolygon> polygons;
};
inline const char* native_weapon_name(unsigned weapon) {
    // Native VC1 HUD string table0x281de10 names equipment1..6.
    // Slot1 explicitly identifies the original scene-pistol cosmetic.
    constexpr const char* names[]={"Unavailable","Original scene pistol (cosmetic)","Original Rifle pickup","Original Automatic pickup","Original Machine Gun pickup","Original Shot Gun pickup","Original Magnum pickup"};
    return weapon<7?names[weapon]:names[0];
}
// Original VC1 RevisionB: scene pistol3b5 is cosmetic for standard play;
// pickup equipment2..6 uses its own3cf/3ca/3cc/3d0/3cd native gun models.
// Native draw24ce8 uses table member+0 for guns; +4 is floating TEXT.
// ROM model records24c8000 are object,tp,th,count; count excludes terminator.
template<class ReadProgram,class ReadPolygon,class ReadTexture>
NativeWeapon extract_native_weapon(ReadProgram read_program,ReadPolygon read_polygon,ReadTexture read_texture,unsigned weapon,
                                   uint32_t polygon_words,uint32_t texture_words) {
    if(weapon<1 || weapon>6 || !polygon_words || !texture_words)return {};
    if(read_program(0xbae4)!=0x8ca03a10||read_program(0xbae8)!=0x024c8000||
       read_program(0x131b0)!=0x8c8003b5||read_program(0x131b4)!=0x09ff88bc||
       read_program(0x24ce8)!=0x90803a14||read_program(0x24cec)!=0x0281db30||
       read_program(0x24bdc)!=0x90803a14||read_program(0x24be0)!=0x0281db34)return {};
    constexpr unsigned expected[]={0,0x3b5,0x3cf,0x3ca,0x3cc,0x3d0,0x3cd};
    const uint32_t model=weapon==1?0x3b5:read_program(0x281db30+weapon*16);
    if(model!=expected[weapon])return {};
    const uint32_t entry=0x24c8000+model*16;
    const uint32_t object=read_program(entry),count=read_program(entry+12);
    uint32_t tp=read_program(entry+4),th=read_program(entry+8);
    if((object&0x01800000)!=0x00800000 || tp&0x800000 || th&0x800000)return {};
    uint32_t at=object&0x7fffff;
    if(at+6>polygon_words)return {};
    auto point=[&](uint32_t index){
        return std::array<float,3>{std::bit_cast<float>(uint32_t(read_polygon(index))),std::bit_cast<float>(uint32_t(read_polygon(index+1))),std::bit_cast<float>(uint32_t(read_polygon(index+2)))};
    };
    auto p0=point(at),p1=point(at+3);at+=6;
    NativeWeapon result;result.weapon=weapon;result.model=model;result.object=object;
    for(uint32_t link=0;link<4096 && link<=(count?count:4095);++link) {
        if(at>=polygon_words)return {};
        const uint32_t attr=read_polygon(at++);
        if(!(attr&3)){result.valid=!result.polygons.empty();return result;}
        if(at+9>polygon_words)return {};
        const auto normal=point(at);auto p2=point(at+3),p3=point(at+6);at+=9;
        const unsigned n=(attr&1)?4:3;if(n==3)p3=p2;
        if(tp+n*2>texture_words || th+4>texture_words)return {};
        NativeWeaponPolygon polygon;polygon.count=n;polygon.attr=attr;polygon.normal=normal;
        for(float value:normal)if(!std::isfinite(value))return {};
        const std::array<std::array<float,3>,4> points={p1,p0,p2,p3};
        for(unsigned i=0;i<4;++i)polygon.header[i]=uint16_t(read_texture(th+i));
        for(unsigned i=0;i<n;++i) {
            const auto& p=points[i];
            for(float value:p)if(!std::isfinite(value) || std::abs(value)>1000000)return {};
            polygon.vertices[i]={p[0],p[1],p[2],float(read_texture(tp+i*2+1)),float(read_texture(tp+i*2))};
        }
        tp+=n*2;int offset=int((attr>>12)&31);if(offset&16)offset-=32;
        const int64_t next=int64_t(th)+offset*4;if(next<0 || next>=texture_words)return {};th=uint32_t(next);
        const unsigned type=(attr>>8)&3;
        if(type)result.polygons.push_back(polygon); // link type 0 is native non-rendering setup
        if(type==0 || type==2){p0=p2;p1=p3;}else if(type==1)p1=p2;else p0=p3;
    }
    return {}; // Every supported original model must have a bounded terminator.
}
}
