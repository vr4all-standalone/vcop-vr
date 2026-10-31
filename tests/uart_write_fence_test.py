"""Exercise actual guest write fencing and phase-aligned raw-clock restart."""
import argparse,os,subprocess,tempfile
from pathlib import Path
from native_clip_fastpath_test import function
def run(uart,model,cxx):
    text=Path(model).read_text()
    methods=''.join(function(text,s) for s in ['bool model2o_state::vcop_uart_write_fence(', 'TIMER_CALLBACK_MEMBER(model2o_state::vcop_uart_write_commit)', 'void model2o_state::schedule_vcop_uart_clock(', 'void model2o_state::vcop_uart_clock_notify(', 'void model2o_state::machine_reset('])
    methods+=function(Path(uart).read_text(),'void i8251_device::write(')
    cpp=r'''
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>
using u64=uint64_t;using u32=uint32_t;using u8=uint8_t;using offs_t=uint32_t;
int64_t now_ns{};
struct attotime {int64_t ns;static attotime from_ticks(u64 n,unsigned rate){return {int64_t(n)*(1000000000/rate)};}u64 as_ticks(unsigned rate)const{return u64(ns/(1000000000/rate));}attotime operator-(attotime b)const{return {ns-b.ns};}};
std::vector<std::tuple<int,int,int,int64_t>> writes;
struct model2o_state;
struct timer_expired_delegate{void(model2o_state::*callback)(int);model2o_state* owner;};
#define FUNC(x) &x
#define TIMER_CALLBACK_MEMBER(x) void x(int param)
struct Scheduler {
 struct Event{timer_expired_delegate cb;int data;int64_t at;};std::vector<Event> queue;
 void synchronize(timer_expired_delegate cb,int data){queue.push_back({cb,data,now_ns});}
 void drain(){auto entries=std::exchange(queue,{});std::stable_sort(entries.begin(),entries.end(),[](auto&a,auto&b){return a.at<b.at;});for(auto&[cb,data,at]:entries){now_ns=at;(cb.owner->*cb.callback)(data);}}
};
struct Machine{Scheduler s;attotime time()const{return {now_ns};}Scheduler& scheduler(){return s;}};
struct Timer{int64_t next{};void adjust(attotime after){next=now_ns+after.ns;assert(next>now_ns);}};
struct i8251_device {
 int id{},edge{};std::function<bool(offs_t,uint8_t)> m_write_fence;
 void write(offs_t,uint8_t);
 void data_w(uint8_t data){writes.emplace_back(id,0,data,now_ns);}
 void control_w(uint8_t data){writes.emplace_back(id,1,data,now_ns);}
 unsigned idle_clock_deadline(int level)const{return 32-(edge%32);}
 void skip_idle_clock_edges(unsigned n,int){assert(n<idle_clock_deadline(edge&1));edge+=n;}
};
struct Audio {i8251_device uart{1};unsigned uart_clock_deadline(int l)const{return uart.idle_clock_deadline(l);}void skip_uart_clock_edges(unsigned n,int l){uart.skip_idle_clock_edges(n,l);}void uart_write(offs_t o,u8 d){uart.write(o,d);}};
struct model2_tgp_state{unsigned resets=0;void machine_reset(){++resets;}};
struct model2o_state : model2_tgp_state {
 i8251_device root;Audio audio;i8251_device* m_uart=&root;Audio* m_m1audio=&audio;Machine m;Timer timer;Timer* m_uart_clock_timer=&timer;
 bool m_idle_uart_clock=true,m_retained_uart_clock=false,m_uart_clock_idle=true,m_uart_clock_busy=false;
 u64 m_uart_clock_edge=0,m_uart_clock_next=32,m_uart_raw_edge=0;u32 m_uart_write_generation=0;
 Machine& machine(){return m;}bool vcop_uart_write_fence(int,offs_t,u8);void vcop_uart_write_commit(int);void schedule_vcop_uart_clock();void vcop_uart_clock_notify(bool);void machine_reset();
 model2o_state(){timer.next=32000;root.m_write_fence=[this](offs_t o,u8 d){return vcop_uart_write_fence(0,o,d);};audio.uart.m_write_fence=[this](offs_t o,u8 d){return vcop_uart_write_fence(1,o,d);};}
};
METHODS
int main(){unsigned cases=0;
 for(int source:{0,1})for(int offset:{0,1})for(int phase:{15,16,31})for(int fraction:{0,40,960}){
  writes.clear();model2o_state m;now_ns=phase*1000+fraction;
  auto& uart=source?m.audio.uart:m.root;uart.write(offset,0x81);
  assert(writes.empty()&&m.m.s.queue.size()==1); // no premature register mutation
  m.m.s.drain();assert(writes.size()==1&&writes[0]==std::make_tuple(source,offset,0x81,now_ns));
  assert(!m.m_uart_clock_idle&&!m.m_uart_clock_busy&&m.m_uart_clock_edge==unsigned(phase));
  assert(m.timer.next==(phase+1)*1000&&m.m_uart_clock_next==unsigned(phase+1));
  assert(m.root.edge==phase&&m.audio.uart.edge==phase);
  uart.write(offset,0x42);assert(writes.size()==2&&m.m.s.queue.empty()); // active bursts keep raw access
  ++cases;
 }
 // An earlier maintenance edge can run while the write waits in the queue.
 writes.clear();model2o_state m;now_ns=32040;m.root.write(0,0x55);assert(writes.empty());
 m.m_uart_clock_edge=32;m.m_uart_clock_next=64;m.root.edge=m.audio.uart.edge=32;
 m.m.s.drain();assert(m.m_uart_clock_edge==32&&m.timer.next==33000&&writes.size()==1);
 for(bool busy:{false,true}){model2o_state x;x.m_uart_clock_busy=busy;x.m_idle_uart_clock=false;assert(!x.vcop_uart_write_fence(0,0,0));}
 for(bool equal:{false,true}){writes.clear();model2o_state x;
  now_ns=15960;x.root.write(0,0x11);now_ns=equal?15960:15040;x.audio.uart.write(1,0x22);assert(writes.empty());
  x.m.s.drain();assert(writes.size()==2);
  assert(std::get<0>(writes[0])==(equal?0:1)&&std::get<0>(writes[1])==(equal?1:0));
 }
 writes.clear();model2o_state reset;now_ns=15040;reset.root.write(0,0x66);reset.machine_reset();reset.m.s.drain();
 assert(writes.empty()&&reset.resets==1&&reset.m_uart_write_generation==1);
 std::cout<<cases<<" deferred source/offset/sub-edge writes plus earlier-event/raw-burst checks passed\n";
}
'''.replace('METHODS',methods)
    env=os.environ.copy();env['PATH']=str(Path(cxx).parent)+os.pathsep+env.get('PATH','')
    with tempfile.TemporaryDirectory(prefix='.uart-write-fence-',dir=Path(__file__).parent) as directory:
        root=Path(directory).resolve();env['TMP']=env['TEMP']=str(root)
        src=root/'fence.cpp';src.write_text(cpp);exe=root/'fence.exe'
        subprocess.run([str(cxx),'-std=c++20','-O2','-UNDEBUG',str(src),'-o',str(exe)],env=env,check=True);subprocess.run([str(exe)],env=env,check=True)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--uart',type=Path,required=True);p.add_argument('--model',type=Path,required=True);p.add_argument('--cxx',type=Path,required=True);run(**vars(p.parse_args()))
