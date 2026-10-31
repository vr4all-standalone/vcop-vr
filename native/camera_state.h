// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
namespace vcop {
inline constexpr uint32_t CameraFocusAddress=0x50f404;
// Original VC1 camera producer1ce30 performs CPU tanrl and stores one derived
// focal value at50f404. GEO1decc and native pickup/hit25114..251e0 consume it.
// Animated FOV50f3f4 remains unchanged; VC1 has no VC2 tangent coefficient.
template<class Read32> bool camera_revision_matches(Read32 read) {
 constexpr uint32_t words[][2]={{0x1ce30,0x90983000},{0x1ce34,0x0050f3f4},{0x1ce94,0x69a01714},{0x1ce98,0x79a01594},{0x1cea4,0x92a03000},{0x1cea8,0x0050f404},{0x1decc,0x90a03000},{0x1ded0,0x0050f404},{0x1def4,0x9ab03000},{0x25114,0x90a03000},{0x25118,0x0050f404},{0x2512c,0x78450604},{0x25130,0x90a03000},{0x25134,0x0050f404}};
 for(const auto&w:words)if(uint32_t(read(w[0]))!=w[1])return false;return true;
}
template<class Read32> bool camera_world_phase(Read32 read) {
 const uint32_t mode=read(0x5016cc),submode=read(0x5016d4);
 const uint32_t service=read(0x559330)&255;
 return !service&&submode<=15&&(mode==4||(mode==11&&submode!=2));
}
inline uint32_t widened_camera_write(uint32_t address,uint32_t value,uint32_t mask,float fov,bool world) {
 if(!world||mask!=0xffffffffu||address!=CameraFocusAddress||!std::isfinite(fov)||fov<35||fov>150)return value;
 const float source=std::bit_cast<float>(value);
 if(!std::isfinite(source)||source<1||source>100000)return value;
 const float focus=248.f/std::tan(fov*(3.14159265358979323846f/360));
 return std::bit_cast<uint32_t>(std::min(source,focus));
}
template<class Read32> uint32_t camera_write(uint32_t address,uint32_t value,uint32_t mask,Read32 read,float fov) {
 if(!camera_revision_matches(read))return value;
 return widened_camera_write(address,value,mask,fov,camera_world_phase(read));
}
struct NativeCameraZoom {float fov{},reference_fov{};};
template<class Read32> NativeCameraZoom read_native_camera_zoom(Read32 read) {
 // VC1's original scene setup loads 30 degrees and directly stores it in the
 // animated FOV. This is not VC2's stage-indexed 34/38-degree table.
 constexpr uint32_t setup[][2]={{0x6dd98,0x8ca83000},{0x6dd9c,0x41f00000},
                               {0x6dda0,0x92a83000},{0x6dda4,0x0050f3f4}};
 if(!camera_revision_matches(read)||!camera_world_phase(read))return {};
 for(const auto& word:setup)if(uint32_t(read(word[0]))!=word[1])return {};
 const float fov=std::bit_cast<float>(uint32_t(read(0x50f3f4)));
 if(!std::isfinite(fov)||fov<=0||fov>150)return {};
 return {fov,std::bit_cast<float>(uint32_t(read(0x6dd9c)))};
}
// VC1's native CPU submission tests use independent fixed half-angle cones:
// props/effects22.5deg, actors36deg. Preserve the actor's13.5deg overscan.
inline uint32_t widened_cull_read(uint32_t address,uint32_t value,uint32_t mask,float fov,bool world) {
 if(!world||mask!=0xffffffffu||!std::isfinite(fov)||fov<35||fov>150)return value;
 const bool actor=address==0x320fc||address==0x32104;
 const bool low=address==0xb8b0||address==0x320fc||address==0x506a0;
 const bool high=address==0xb8b8||address==0x32104||address==0x506a8;
 if(!low&&!high)return value;
 const uint32_t original=low?(actor?0x306f3395u:0xfc8b007au):(actor?0x3fe41b2fu:0x3fd921fau);
 if(value!=original)return value;
 const double original_angle=actor?36:22.5;
 const double target=double(fov)*.5+(actor?13.5:0);
 if(target<=original_angle)return value;
 const auto bits=std::bit_cast<uint64_t>(target*(3.14159265358979323846/180));
 return uint32_t(low?bits:bits>>32);
}
template<class Read32> bool camera_cull_revision_matches(Read32 read) {
 constexpr uint32_t words[][2]={{0xb8ac,0x8c403000},{0xb8b0,0xfc8b007a},{0xb8b4,0x8c483000},{0xb8b8,0x3fd921fa},{0xb8bc,0x69a01688},{0xb8c0,0x69052a80},{0x320f8,0x8c403000},{0x320fc,0x306f3395},{0x32100,0x8c483000},{0x32104,0x3fe41b2f},{0x32108,0x69a01688},{0x3210c,0x69052a80},{0x5069c,0x8c403000},{0x506a0,0xfc8b007a},{0x506a4,0x8c483000},{0x506a8,0x3fd921fa},{0x506ac,0x69a01688},{0x506b0,0x69052a80}};
 for(const auto&w:words)if(uint32_t(read(w[0]))!=w[1])return false;return true;
}
}
