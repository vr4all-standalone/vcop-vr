#include "../native/cabinet_io.h"
#include "../native/game_memory.h"
#include <cassert>
#include <cmath>
#include <unordered_map>
using namespace vcop;
int main(){
 Cabinet c;assert(c.x[0]==380&&c.x[1]==377&&c.y[0]==230&&c.y[1]==232);
 c.coins=3;c.starts=3;c.test=c.service=true;assert(in0(255,c)==0xc0);
 c.trigger={true,true};assert(in1(255,c)==0xfc);
 c.x={0x123,0x234};c.y={0x345,0x156};
 assert(serial_byte(0,c)==0x45&&serial_byte(1,c)==3&&serial_byte(2,c)==0x23&&serial_byte(3,c)==1);
 assert(serial_byte(4,c)==0x56&&serial_byte(5,c)==1&&serial_byte(6,c)==0x34&&serial_byte(7,c)==2);
 assert(offscreen(0,155,230));assert(!offscreen(0,156,230));assert(offscreen(0,606,230));assert(!offscreen(0,605,230));
 assert(offscreen(1,152,232));assert(!offscreen(1,153,232));assert(offscreen(1,603,232));
 assert(offscreen(0,380,55)&&!offscreen(0,380,56));assert(offscreen(1,377,58)&&!offscreen(1,377,59));
 // Original fresh VC1 backup1 dump: little endian UINT16 xmin,ymin,xmax,ymax.
 std::unordered_map<uint32_t,uint32_t> mem{{0x1d00024,131|(36u<<16)},{0x1d00028,630|(425u<<16)},{0x1d0002c,128|(39u<<16)},{0x1d00030,627|(425u<<16)}};
 auto read=[&](uint32_t a){return mem[a];};
 for(unsigned p=0;p<2;p++){
  auto cal=read_gun_calibration(read,p);assert(cal.valid());
  for(uint16_t x:{uint16_t(0),uint16_t(1000),uint16_t(16384),uint16_t(32768),uint16_t(49151),uint16_t(65535)}){
   auto raw=calibrated_gun_position(x,x,cal);
   const double expectedx=x*495./65535.,expectedy=x*383./65535.;
   assert(std::abs(native_gun_axis(raw[0],cal.xmin,cal.xmax,496)-expectedx)<.51);
   assert(std::abs(native_gun_axis(raw[1],cal.ymin,cal.ymax,384)-expectedy)<.51);
  }
 }
 mem[0x1d00028]=131|(425u<<16);assert(!read_gun_calibration(read,0).valid());
 assert(!read_gun_calibration(read,2).valid());
}
