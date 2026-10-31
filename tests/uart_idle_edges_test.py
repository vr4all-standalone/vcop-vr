"""Differentially exercise skipped edges against actual i8251/serial code."""
import argparse,os,subprocess,tempfile
from pathlib import Path
from native_clip_fastpath_test import function
def harness(uart,serial):
    text=Path(uart).read_text();base=Path(serial).read_text()
    methods=''.join(function(text,s) for s in ['unsigned i8251_device::idle_clock_deadline(', 'void i8251_device::skip_idle_clock_edges(', 'void i8251_device::write_txc(', 'void i8251_device::write_rxc(', 'void i8251_device::receive_clock('])
    methods+=function(base,'void device_serial_interface::receive_register_update_bit(').replace('device_serial_interface::','i8251_device::')
    transmit=function(text,'void i8251_device::transmit_clock(')
    # The original prefix must return before any meaningful divider event.
    methods+=transmit[:transmit.index('\tm_txc_count = 0;')]+'assert(false && "Skipped a meaningful TX edge");\n}\n'
    cpp=r'''
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <tuple>
using u32=uint32_t;
#define BIT(n,b) (((n)>>(b))&1)
#define LOGMASKED(...) ((void)0)
struct i8251_device {
 enum {I8251_NEXT_COMMAND=0,I8251_STATUS_PARITY_ERROR=8,I8251_STATUS_FRAMING_ERROR=32,
 RECEIVE_REGISTER_WAITING_FOR_START_BIT=1,RECEIVE_REGISTER_SYNCHRONISED=2,RECEIVE_REGISTER_FULL=4};
 int m_br_factor=16,m_flags{},m_sync_byte_count{},m_txc{},m_rxc{},m_txc_count{},m_rxc_count=1,m_rxd=1;
 unsigned m_command=4,m_status{};
 uint16_t m_rcv_register_data=0x1234;unsigned m_rcv_flags=1,m_rcv_bit_count_received{},m_rcv_bit_count=10,m_df_min_rx_stop_bit_count=1;
 bool m_rcv_framing_error{},m_rcv_parity_error{};
 bool is_receive_register_synchronized()const{return m_rcv_flags&2;}
 bool is_receive_register_full()const{return m_rcv_flags&4;}
 bool is_receive_parity_error()const{return m_rcv_parity_error;}
 bool is_receive_framing_error()const{return m_rcv_framing_error;}
 void receive_register_extract(){assert(false);}uint8_t get_received_char(){return 0;}void receive_character(uint8_t){assert(false);}
 void sync1_rxc(){assert(false);}void sync2_rxc(){assert(false);}
 unsigned idle_clock_deadline(int)const;void skip_idle_clock_edges(unsigned,int);
 void write_txc(int);void write_rxc(int);void transmit_clock();void receive_clock();void receive_register_update_bit(int);
 auto state()const{return std::tuple(m_txc,m_rxc,m_txc_count,m_rxc_count,m_rcv_register_data,m_rcv_flags,m_rcv_bit_count_received,m_rcv_framing_error,m_rcv_parity_error);}
};
METHODS
int main(){unsigned cases=0;
 for(int level:{0,1})for(int tx=0;tx<16;++tx)for(int rx=1;rx<=24;++rx)for(bool enabled:{false,true})for(bool synced:{false,true})for(int line:{0,1})for(uint16_t data:{uint16_t(0x0123),uint16_t(0x8421),uint16_t(0xffff)}){
  i8251_device a;a.m_txc=a.m_rxc=level;a.m_txc_count=tx;a.m_rxc_count=rx;a.m_command=enabled?4:0;a.m_rcv_flags=synced?2:1;a.m_rxd=line;a.m_rcv_register_data=data;
  auto deadline=a.idle_clock_deadline(level);assert(deadline>=1&&deadline<=32);
  for(unsigned edges=0;edges<deadline;++edges){auto reference=a,folded=a;int l=level;
   for(unsigned edge=0;edge<edges;++edge){l^=1;reference.write_txc(l);reference.write_rxc(l);}
   folded.skip_idle_clock_edges(edges,level);if(reference.state()!=folded.state()){std::cerr<<"Edge differential failed at"<<edges<<" of"<<deadline<<'\n';return 1;}++cases;
  }
 }
 i8251_device expected;expected.m_txc_count=3;expected.m_rxc_count=12;expected.m_rcv_flags=2;assert(expected.idle_clock_deadline(0)==23);
 for(int factor:{1,64}){expected.m_br_factor=factor;assert(expected.idle_clock_deadline(0)==1);}expected.m_br_factor=16;
 expected.m_sync_byte_count=1;assert(expected.idle_clock_deadline(0)==1);expected.m_sync_byte_count=0;expected.m_flags=1;assert(expected.idle_clock_deadline(0)==1);
 std::cout<<cases<<" actual i8251/serial idle-edge comparisons and unsupported-mode fallbacks passed\n";
}
'''.replace('METHODS',methods)
    return cpp

def run(uart,serial,cxx):
    cpp=harness(uart,serial)
    env=os.environ.copy();env['PATH']=str(Path(cxx).parent)+os.pathsep+env.get('PATH','')
    with tempfile.TemporaryDirectory(prefix='.uart-proof-',dir=Path(__file__).parent) as directory:
        root=Path(directory).resolve();env['TMP']=env['TEMP']=str(root)
        src=root/'uart.cpp';src.write_text(cpp);exe=root/'uart.exe'
        subprocess.run([str(cxx),'-std=c++20','-O2','-UNDEBUG',str(src),'-o',str(exe)],env=env,check=True);subprocess.run([str(exe)],env=env,check=True)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--uart',type=Path,required=True);p.add_argument('--serial',type=Path,required=True);p.add_argument('--cxx',type=Path,required=True);run(**vars(p.parse_args()))
