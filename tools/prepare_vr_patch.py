"""Generate pinned continuous Model 2 VR host hooks after diagnostic patch."""
import argparse,difflib,subprocess
from pathlib import Path
from prepare_capture_patch import COMMIT,modify as capture_modify

def replace(s,a,b):
    if s.count(a)!=1:raise ValueError('Expected one hook: '+a[:100]+' count='+str(s.count(a)))
    return s.replace(a,b)

def video(s):
    s=replace(s,'#include "../../lib/vcop/model2_trace.h"','#include "../../lib/vcop/model2_trace.h"\n#include "../../lib/vcop/model2_live.h"')
    s=replace(s,'vcop::trace_begin(m_screen->frame_number(), machine().time().as_double());','vcop::trace_begin(m_screen->frame_number(), machine().time().as_double());\n        vcop::live_begin(m_screen->frame_number(), machine().time().as_double());')
    s=replace(s,'if (vcop::trace_active()) {\n        vcop::NativePolygon','if (vcop::trace_active() || vcop::live_active()) {\n        vcop::NativePolygon')
    s=replace(s,'        vcop::trace_capture().append(native);','        if(vcop::trace_active())vcop::trace_capture().append(native);\n        if(vcop::live_active())vcop::live_capture().append(native);')
    s=s.replace('if(vcop::trace_active())vcop::trace_object()','if(vcop::trace_active()||vcop::live_active())vcop::trace_object()')
    anchor='\t/* tell the rasterizer we\'re starting a frame */'
    s=replace(s,anchor,'''    if(vcop::live_pending()) {
        auto& capture=vcop::live_capture();
        const int width=m_screen->visible_area().width(),height=m_screen->visible_area().height();
        capture.display(m_crtc_xoffset,m_crtc_yoffset,width,height);
        capture.bytes("texture0",&m_textureram0[0],m_textureram0.bytes());
        capture.bytes("texture1",&m_textureram1[0],m_textureram1.bytes());
        capture.bytes("palette",m_palram.get(),0x4000);
        capture.bytes("colorxlat",m_colorxlat.get(),0xc000);
        capture.bytes("luma",m_lumaram.get(),0x8000);
        capture.bytes("gamma",m_gamma_table,sizeof(m_gamma_table));
        auto frame=std::make_unique<vcop::NativeFrame>(capture.take());
        frame->complete=false;
        vcop::copy_pixels(frame->background,width,height,[&](int x,int y){return bitmap.pix(y,x);});
        vcop::live_raster()=std::move(frame);vcop::live_pending()=false;
    }
'''+anchor)
    anchor='\tcopybitmap_trans(bitmap, m_sys24_bitmap, 0, 0, 0, 0, cliprect, 0);\n    if(!vcop::trace_pending_raster().empty())'
    s=replace(s,anchor,anchor.split('\n')[0]+'''
    if(auto& frame=vcop::live_raster();frame) {
        vcop::copy_pixels(frame->foreground,frame->width,frame->height,[&](int x,int y){return m_sys24_bitmap.pix(y,x);});
        vcop::copy_pixels(frame->native,frame->width,frame->height,[&](int x,int y){return bitmap.pix(y,x);});
        frame->complete=true;
        vcop::shared_runtime().scene.store(std::shared_ptr<const vcop::NativeFrame>(frame.release()));
    }
    if(!vcop::trace_pending_raster().empty())''')
    anchor='    if(vcop::trace_active()) {\n        vcop::trace_geometry_pending()=true;'
    s=replace(s,anchor,'    if(vcop::live_active()){vcop::live_pending()=true;vcop::live_active()=false;}\n'+anchor)
    return s

def driver(s):
    s=replace(s,'#include "model2.h"','#include "model2.h"\n#include "../../lib/vcop/host_bridge.h"')
    s=replace(s,'\tm_framenum = m_screen->frame_number();','''    vcop::native_tick([&](uint32_t address){return m_maincpu->space(AS_PROGRAM).read_dword(address);});
    if(vcop::vr_requested()) {
        const auto* poly=reinterpret_cast<const uint32_t*>(memregion("polygons")->base());
        const auto* tex=reinterpret_cast<const uint16_t*>(memregion("textures")->base());
        vcop::native_weapons([&](uint32_t address){return m_maincpu->space(AS_PROGRAM).read_dword(address);},
            [&](uint32_t index){return poly[index];},[&](uint32_t index){return tex[index];},
            memregion("polygons")->bytes()/4,memregion("textures")->bytes()/2);
    }
\tm_framenum = m_screen->frame_number();''')
    anchor='\tioboard.in_callback<0>().set_ioport("IN0");\n\tioboard.in_callback<1>().set_ioport("IN1");\n\tioboard.in_callback<2>().set_ioport("IN2");\n\tioboard.output_callback().set(FUNC(model2_state::lamp_output_w));\n\tioboard.set_lightgun_p1x_tag("P1_X");'
    s=replace(s,anchor,anchor.replace('ioboard.in_callback<0>().set_ioport("IN0");','ioboard.in_callback<0>().set([this](){return vcop::system_input(ioport("IN0")->read());});').replace('ioboard.in_callback<1>().set_ioport("IN1");','ioboard.in_callback<1>().set([this](){return vcop::trigger_input(ioport("IN1")->read());});'))
    return s

def osd(s):
    s=replace(s,'#include "emu.h"','#include "emu.h"\n#include "vcop/host_bridge.h"')
    s=replace(s,'void osd_common_t::update(bool skip_redraw)\n{','void osd_common_t::update(bool skip_redraw)\n{\n    vcop::poll_host(machine());')
    s=replace(s,'void osd_common_t::osd_exit()\n{','void osd_common_t::osd_exit()\n{\n    vcop::stop_host();')
    return s

def original_io(s):
    s=replace(s,'#include "model1io2.h"','#include "model1io2.h"\n#include "../../lib/vcop/host_bridge.h"')
    return replace(s,'\t// lightgun coordinates\n\tif (offset < 8)', '    // Feed physical gun samples only after the native FPGA upload completes.\n    if(vcop::vr_requested() && std::string_view(machine().system().name)=="vcop") {\n        if(offset<8)return vcop::lightgun_byte(offset);\n        if(offset==8)return vcop::offscreen_input(0,0xfc);\n    }\n\t// lightgun coordinates\n\tif (offset < 8)')

def generate(source):
    patch=''
    for rel,fn,base in [('src/mame/sega/model2_v.cpp',video,capture_modify),('src/mame/sega/model2.cpp',driver,lambda s:s),('src/mame/sega/model1io2.cpp',original_io,lambda s:s),('src/osd/modules/lib/osdobj_common.cpp',osd,lambda s:s)]:
        old=base(subprocess.check_output(['git','-C',str(source),'show',f'{COMMIT}:{rel}'],text=True))
        new=fn(old)
        patch+=''.join(difflib.unified_diff(old.splitlines(True),new.splitlines(True),fromfile='a/'+rel,tofile='b/'+rel))
    return patch
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--source',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    args.output.write_text(generate(args.source),encoding='utf8',newline='\n')
