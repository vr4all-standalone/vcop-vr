// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/native_flash.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#define CHECK(x) do { if (!(x)) { std::cerr << "Failed " #x " line " << __LINE__ << '\n'; return 1; } } while (0)
int main(int argc,char** argv) {
    if(argc!=2)return 2;
    std::ifstream in(argv[1],std::ios::binary);
    std::vector<uint8_t> rom{std::istreambuf_iterator<char>(in),{}};
    CHECK(rom.size()==0x200000);
    std::unordered_map<uint32_t,uint32_t> ram{{0x5016cc,11},{0x5016d4,7},{0x50ee30,1},{0x501528,1},{0x5b1e50,4}};
    for(uint32_t a=0x100c000;a<0x100cc00;a+=4)ram[a]=0xffffffffu;
    auto read=[&](uint32_t a) {
        if(a+3<rom.size())return uint32_t(rom[a])|uint32_t(rom[a+1])<<8|uint32_t(rom[a+2])<<16|uint32_t(rom[a+3])<<24;
        auto it=ram.find(a);return it!=ram.end()?it->second:0u;
    };
    const auto initial=ram;
    CHECK(vcop::native_gun_strobe(read));CHECK(ram==initial);
    // Native empty fire/reload uses the same gun read before ammo acknowledgement.
    ram[0x50ee38]=0;ram[0x5b1ea8]=0;CHECK(vcop::native_gun_strobe(read));
    ram[0x50ee30]=0;ram[0x501528]=0;ram[0x50ee34]=1;ram[0x50152c]=2;
    CHECK(vcop::native_gun_strobe(read));
    ram[0x5016d4]=2;CHECK(vcop::native_gun_strobe(read));ram[0x5016d4]=7;
    ram[0x559330]=1;CHECK(!vcop::native_gun_strobe(read));ram[0x559330]=0;
    ram[0x5016cc]=5;CHECK(!vcop::native_gun_strobe(read));ram[0x5016cc]=11;
    ram[0x5016d4]=3;CHECK(!vcop::native_gun_strobe(read));ram[0x5016d4]=7;
    for(uint32_t count:{0u,1u,3u,5u,0xffffffffu}){ram[0x5b1e50]=count;CHECK(!vcop::native_gun_strobe(read));}ram[0x5b1e50]=4;
    ram[0x50152c]=0;CHECK(!vcop::native_gun_strobe(read));ram[0x50152c]=2;
    ram[0x50ee34]=0;ram[0x50ee30]=1;CHECK(!vcop::native_gun_strobe(read));ram[0x50ee30]=0;ram[0x50ee34]=1;
    ram[0x50ee34]=2;CHECK(!vcop::native_gun_strobe(read));ram[0x50ee34]=1;
    ram[0x100c000]=0;CHECK(!vcop::native_gun_strobe(read));ram[0x100c000]=0xffffffffu;
    ram[0x100cbfc]=0xfffffff0u;CHECK(!vcop::native_gun_strobe(read));ram[0x100cbfc]=0xffffffffu;
    rom[0x584a4]^=1;CHECK(!vcop::native_gun_strobe(read));
    std::cout<<"VC1 owned-ROM gun read: shot/reload/empty fire, P1/P2, native mask, mode, Service and read-only guards pass\n";
}
