"""Run the actual sound-ready and acknowledge callbacks with an IRQ sink."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile
from native_clip_fastpath_test import function

def run(source,cxx):
    text=Path(source).read_text(encoding='utf8')
    callbacks=function(text,'void model2_state::sound_ready_w(')+function(text,'void model2_state::irq_ack_w(')
    cpp=r'''
#include <cassert>
#include <cstdint>
#include <iostream>
using u32=uint32_t;
struct Uart{bool tx{},rx{};bool txrdy_r(){return tx;}bool rxrdy_r(){return rx;}};
struct model2_state {
 u32 m_intena{},m_intreq{};Uart uart;Uart* m_uart=&uart;unsigned updates{};
 void irq_update(){++updates;}
 void sound_ready_w(int);void irq_ack_w(u32);
};
CALLBACKS
int main(){
 model2_state s;s.m_intena=0x400;
 s.sound_ready_w(1);assert(s.m_intreq==0&&s.updates==0);
 s.uart.tx=true;s.sound_ready_w(1);assert(s.m_intreq==0x400&&s.updates==1);
 for(int n=0;n<100000;++n)s.sound_ready_w(1);
 if(s.updates!=1){std::cerr<<"Already asserted sound IRQ redundantly synchronized "<<s.updates<<" times\n";return 1;}
 s.irq_ack_w(~0x400u);assert(s.m_intreq==0&&s.updates==2);
 // Still-ready UART must reassert after ACK, including a low callback from
 // the other ready pin: this callback samples both actual pin levels.
 s.sound_ready_w(0);assert(s.m_intreq==0x400&&s.updates==3);
 s.irq_ack_w(~0x400u);s.uart.tx=false;s.uart.rx=true;s.sound_ready_w(1);assert(s.m_intreq==0x400&&s.updates==5);
 s.irq_ack_w(~0x400u);s.m_intena=0;s.sound_ready_w(1);assert(s.m_intreq==0&&s.updates==6);
 s.m_intreq=0x80;s.m_intena=0x400;s.sound_ready_w(1);assert(s.m_intreq==0x480&&s.updates==7);
 std::cout<<"Sound IRQ ACK/reassert, both UART ready pins, disabled mask and100000 redundant callbacks passed\n";
}
'''.replace('CALLBACKS',callbacks)
    env=os.environ.copy();env['PATH']=str(Path(cxx).parent)+os.pathsep+env.get('PATH','')
    with tempfile.TemporaryDirectory(prefix='.sound-irq-',dir=Path(__file__).parent) as directory:
        root=Path(directory).resolve();env['TMP']=env['TEMP']=str(root)
        source=root/'sound.cpp';source.write_text(cpp);exe=root/'sound.exe'
        subprocess.run([str(cxx),'-std=c++20','-O2','-UNDEBUG',str(source),'-o',str(exe)],env=env,check=True)
        subprocess.run([str(exe)],env=env,check=True)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source',type=Path,required=True);p.add_argument('--cxx',type=Path,required=True)
    run(**vars(p.parse_args()))
