#define main native_replay_main
#include "../tools/replay_model2_frames.cpp"
#undef main
#include <cassert>
int main(int argc,char**argv){assert(argc==3);auto f=read_frame(argv[1],argv[2]);
 std::erase_if(f->polygons,[](const auto&p){return p.object!=0x80fbb3&&p.object!=0x80fbc8;});assert(!f->polygons.empty());
 f->background.assign(f->width*f->height,0xff000000);f->foreground.assign(f->width*f->height,0);f->native=f->background;
 GLContext gl;GLuint target,color,depth;glGenFramebuffers(1,&target);glBindFramebuffer(GL_FRAMEBUFFER,target);glGenTextures(1,&color);glBindTexture(GL_TEXTURE_2D,color);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,640,640,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,640,640);glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);assert(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);glViewport(0,0,640,640);
 GLScene scene;scene.set_immersive(true);scene.prepare(f);auto projection=glm::perspective(glm::radians(100.f),1.f,.02f,10000.f);
 auto draw=[&](float eye){glDepthMask(GL_TRUE);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);scene.draw(projection*glm::translate(glm::mat4(1),glm::vec3(-eye,0,0)),1,1.8);std::vector<uint8_t> pix(640*640*4);glReadPixels(0,0,640,640,GL_RGBA,GL_UNSIGNED_BYTE,pix.data());return pix;};
 auto zero=draw(0),left=draw(-.25),right=draw(.25);size_t lit=0;for(size_t i=0;i<zero.size();i+=4)if(zero[i]||zero[i+1]||zero[i+2])lit++;assert(lit>100);assert(zero==left&&zero==right);
 auto hit=trace_scene_aim({{0,0,0},{0,.2,-1}},*f,1,2,true,nullptr,true);assert(hit.target&&std::abs(hit.target->z+2)<1e-6);
 auto prepared_hit=scene.aim({{0,0,0},{0,.2,-1}},1,2);assert(prepared_hit.target&&std::abs(prepared_hit.target->z+2)<1e-6);
 NativePolygon wall;wall.count=4;wall.focus={100,100};wall.center={248,320};wall.viewport={0,128,496,512};wall.luma=252;wall.vertices={NativeVertex{-100000,100000,10},NativeVertex{100000,100000,10},NativeVertex{100000,-100000,10},NativeVertex{-100000,-100000,10}};
 auto together=std::make_shared<NativeFrame>(*f);together->polygons.push_back(wall);scene.prepare(together);auto occluded=draw(0);auto only=std::make_shared<NativeFrame>(*together);only->polygons={wall};scene.prepare(only);assert(draw(0)==occluded);assert(glGetError()==GL_NO_ERROR);
 std::cout<<"Actual VC1 native clouds: "<<lit<<" pixels, zero eye/lean parallax, native transparency, world occlusion and hit exclusion verified\n";
}
