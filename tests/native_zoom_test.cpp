// SPDX-License-Identifier: GPL-2.0-or-later
#include "native_zoom.h"
#include "camera_state.h"
#include "model2_scene.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <unordered_map>
#define CHECK(x) do{if(!(x)){std::cerr<<"Failed " #x " line "<<__LINE__<<'\n';return 1;}}while(0)
using namespace vcop;
int main(int argc,char**argv){
 CHECK(native_zoom_factor(15,30,false)==1&&native_zoom_factor(30,30,true)==1);
 CHECK(native_zoom_factor(15,30,true)>2.035&&native_zoom_factor(15,30,true)<2.036);
 CHECK(native_zoom_factor(60,30,true)<.465&&native_zoom_factor(60,30,true)>.464);
 double previous=0;
 for(float fov:{5.f,4.f,2.f,1.f}){auto z=native_zoom_factor(fov,30,true);CHECK(z>previous&&native_zoom_factor(fov,30,false)==1);previous=z;}
 for(float invalid:{0.f,-1.f,151.f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}){
  CHECK(native_zoom_factor(invalid,30,true)==1&&native_zoom_factor(15,invalid,true)==1);
 }
 CHECK(native_zoom_factor(std::numeric_limits<float>::denorm_min(),30,true)==1);
 Vec3 original{2,-3,-12};auto enlarged=zoomed_world_position(original,2);auto inverse=unzoomed_world_position(enlarged,2);
 CHECK(enlarged.x==4&&enlarged.y==-6&&enlarged.z==-12&&inverse.x==2&&inverse.y==-3&&inverse.z==-12);
 NativeFrame recycled;recycled.native_fov=15;recycled.native_reference_fov=30;
 SceneCapture capture;capture.begin(1,0,std::move(recycled));auto fresh=capture.take();
 CHECK(fresh.native_fov==0&&fresh.native_reference_fov==0&&frame_zoom(fresh,true)==1);
 capture.begin(2,1);capture.camera_zoom(15,30);auto frame=capture.take();
 CHECK(frame_zoom(frame,false)==1&&frame_zoom(frame,true)>2.035f);
 frame.raster_only=true;CHECK(frame_zoom(frame,true)==1);
 if(argc==2){
  std::ifstream in(argv[1],std::ios::binary);std::vector<uint8_t> rom{std::istreambuf_iterator<char>(in),{}};CHECK(rom.size()==0x200000);
  std::unordered_map<uint32_t,uint32_t> ram{{0x5016cc,11},{0x5016d4,7},{0x50f3f4,std::bit_cast<uint32_t>(15.f)}};
  auto read=[&](uint32_t a){if(a<rom.size())return uint32_t(rom[a])|(uint32_t(rom[a+1])<<8)|(uint32_t(rom[a+2])<<16)|(uint32_t(rom[a+3])<<24);return ram[a];};
  auto z=read_native_camera_zoom(read);CHECK(z.fov==15&&z.reference_fov==30);
  ram[0x50f3f4]=std::bit_cast<uint32_t>(4.f);CHECK(read_native_camera_zoom(read).fov==4);
  ram[0x5016d4]=2;CHECK(read_native_camera_zoom(read).fov==0);ram[0x5016d4]=7;
  ram[0x559330]=1;CHECK(read_native_camera_zoom(read).fov==0);ram[0x559330]=0;
  ram[0x50f3f4]=std::bit_cast<uint32_t>(std::numeric_limits<float>::quiet_NaN());CHECK(read_native_camera_zoom(read).fov==0);
  ram[0x50f3f4]=std::bit_cast<uint32_t>(15.f);rom[0x6dd9c]^=1;CHECK(read_native_camera_zoom(read).fov==0);
 }
 std::cout<<"VC1 native zoom, depth/inverse mapping, recycled snapshots and owned-ROM phase guards passed\n";
}
