"""Exercise the production strict-idle guard, including every unsafe state."""
import argparse,os,subprocess,tempfile
from pathlib import Path
from native_clip_fastpath_test import function
def run(source,cxx):
    text=Path(source).read_text()
    methods=function(text,'bool i8251_device::clock_is_idle(')
    methods+=function(text,'unsigned i8251_device::idle_clock_deadline(')
    methods+=''.join(function(text,s) for s in ['void i8251_device::data_w(', 'bool i8251_device::is_tx_enabled(', 'void i8251_device::check_for_tx_start(', 'void i8251_device::start_tx(', 'void i8251_device::transmit_clock('])
    cpp=r'''
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <functional>
#include <iostream>
#define BIT(n,b) (((n)>>(b))&1)
#define LOG(...) ((void)0)
#define LOGSTAT(...) ((void)0)
#define LOGBITS(...) ((void)0)
struct i8251_device {
 enum{I8251_NEXT_COMMAND=0,I8251_STATUS_TX_READY=1,I8251_STATUS_RX_READY=2,I8251_STATUS_TX_EMPTY=4};
 int m_flags{},m_sync_byte_count{},m_br_factor=16,m_txc{},m_rxc{},m_txc_count{},m_rxc_count=1;
 unsigned m_command=5,m_status=5;bool m_delayed_tx_en{},empty=true,synchronized=false,full=false;int m_rxd=1,m_cts=0;
 bool is_transmit_register_empty()const{return empty;}bool is_receive_register_synchronized()const{return synchronized;}
 bool is_receive_register_full()const{return full;}bool is_tx_enabled()const;
 uint8_t m_tx_data{};int remaining_bits{};std::function<void(bool)> m_idle_clock_notify;
 void data_w(uint8_t);void start_tx();void check_for_tx_start();void transmit_clock();
 void update_tx_ready(){}void update_tx_empty(){}void m_txd_handler(uint8_t){}
 void transmit_register_setup(uint8_t){remaining_bits=10;empty=false;}
 uint8_t transmit_register_get_data_bit(){empty=!--remaining_bits;return 1;}
 const char* tag()const{return "fixture";}struct Time{uint64_t as_ticks(uint64_t){return 0;}};struct Machine{Time time(){return {};}};Machine machine(){return {};}
 bool clock_is_idle(int)const;unsigned idle_clock_deadline(int)const;
};
METHODS
int main(){unsigned cases=0;
 for(int level:{0,1})for(int count=0;count<16;++count){
  i8251_device good;good.m_txc=good.m_rxc=level;good.m_txc_count=count;
  assert(good.clock_is_idle(level)==(good.idle_clock_deadline(level)>1));
  if(!good.clock_is_idle(level))continue;
  auto historical_latch=good;historical_latch.m_delayed_tx_en=true;
  assert(historical_latch.clock_is_idle(level));
  for(int bad=0;bad<16;++bad){auto x=good;
   switch(bad){case 0:x.m_flags=1;break;case 1:x.m_sync_byte_count=1;break;case 2:x.m_br_factor=1;break;
    case 3:x.m_br_factor=64;break;case 4:x.m_txc^=1;break;case 5:x.m_rxc^=1;break;case 6:x.m_txc_count=16;break;
    case 7:x.m_command&=~4u;break;case 8:x.m_command|=8;break;case 9:x.m_status|=2;break;
    case 10:x.m_status&=~1u;break;case 11:x.m_status&=~4u;break;case 12:x.empty=false;break;
    case 13:x.synchronized=true;break;case 14:x.full=true;break;case 15:x.m_rxd=0;break;}
   assert(!x.clock_is_idle(level));++cases;
  }
  auto disabled=good;disabled.m_command=0;assert(!disabled.clock_is_idle(level));
  auto stopped=good;stopped.m_cts=1;assert(!stopped.clock_is_idle(level));
 }
 i8251_device lifecycle;lifecycle.data_w(0x85);assert(!lifecycle.clock_is_idle(0));
 for(int edge=0;edge<11*16;++edge)lifecycle.transmit_clock();
 assert(lifecycle.m_delayed_tx_en&&lifecycle.clock_is_idle(0));
 std::cout<<cases<<" strict-idle exclusions and actual transmit/drain/idle re-entry passed\n";
}
'''.replace('METHODS',methods)
    env=os.environ.copy();env['PATH']=str(Path(cxx).parent)+os.pathsep+env.get('PATH','')
    with tempfile.TemporaryDirectory(prefix='.uart-idle-guard-',dir=Path(__file__).parent) as directory:
        root=Path(directory).resolve();env['TMP']=env['TEMP']=str(root)
        src=root/'guard.cpp';src.write_text(cpp);exe=root/'guard.exe'
        subprocess.run([str(cxx),'-std=c++20','-O2','-UNDEBUG',str(src),'-o',str(exe)],env=env,check=True);subprocess.run([str(exe)],env=env,check=True)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source',type=Path,required=True);p.add_argument('--cxx',type=Path,required=True);run(**vars(p.parse_args()))
