#include "../native/controller_labels.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// Capture the renderer boundary, keeping Labels::draw, its layout/cache, and
// its emitted line/dot transforms real. Text upload needs no graphics context.
namespace {
struct Quad {glm::mat4 model;unsigned texture;float hx,hy;};
std::vector<Quad> quads;
std::vector<std::string> words;
int failures=0;
void check(bool ok,const std::string& why) {
    if(!ok){std::cerr<<"FAIL: "<<why<<'\n';++failures;}
}
bool near(glm::vec3 a,glm::vec3 b) {return glm::length(a-b)<.000002f;}
}
namespace vr::xr {
unsigned textTexture(const std::string& text,float,const glm::vec3&,int& width,int& height) {
    words.push_back(text);width=int(text.size())*20;height=40;
    return unsigned(words.size());
}
void drawTexture(const glm::mat4&,const glm::mat4& model,unsigned texture,float hx,float hy,const glm::vec4&,bool) {
    quads.push_back({model,texture,hx,hy});
}
}
namespace {
// Independent landmarks on the squeeze-button surface after the bundled GLB
// node rotation is baked by loadGlb (raw x,y,z -> x,-z,y). Mirroring precedes
// controller placement; raw mesh coordinates are not renderer coordinates.
const glm::vec3 grip[2]={{-.013753f,-.028802f,-.026018f},{.013753f,-.028802f,-.026018f}};
const glm::vec3 other[2][5]={
    {{-.0155f,.0067f,.0028f},{-.0049f,.0046f,-.0097f},{.0114f,.0021f,-.0109f},{.0063f,.0112f,.007f},{-.0142f,-.0159f,.0254f}},
    {{.0154f,.0067f,.0028f},{.0048f,.0046f,-.0097f},{0,0,0},{-.0065f,.0111f,.007f},{.0142f,-.0159f,.0254f}}
};
void dump(std::ostream& out,int hand,const glm::mat4& model) {
    auto point=[&](glm::vec3 p){out<<'['<<p.x<<','<<p.y<<','<<p.z<<']';};
    const int count=hand==0?6:5;
    out<<"{\"hand\":"<<hand<<",\"model\":[";
    for(int c=0;c<4;c++)for(int r=0;r<4;r++){if(c||r)out<<',';out<<model[c][r];}
    out<<"],\"labels\":[";
    for(int i=0;i<count;i++) {
        if(i)out<<',';
        const auto& line=quads[i*2];const auto& dot=quads[i*2+1];const auto& text=quads[count*2+i];
        auto start=glm::vec3(dot.model[3]);
        auto end=glm::vec3(line.model[3])*2.f-start;
        out<<"{\"text\":\""<<words[text.texture-1]<<"\",\"from\":";point(start);
        out<<",\"to\":";point(end);out<<",\"centre\":";point(glm::vec3(text.model[3]));
        out<<",\"hx\":"<<text.hx<<",\"hy\":"<<text.hy<<'}';
    }
    out<<"]}";
}
void verify(int hand,const glm::mat4& model,int startPlayer,int gunPlayer) {
    const int count=hand==0?6:5,gripIndex=hand==0?3:2;
    check(quads.size()==size_t(count*3),"each visible help label emits its line, dot and text");
    if(quads.size()!=size_t(count*3))return;
    for(int i=0;i<count;i++) {
        const auto& line=quads[i*2];const auto& dot=quads[i*2+1];const auto& text=quads[count*2+i];
        const auto from=glm::vec3(dot.model[3]);
        const int ordinary=i<gripIndex?i:i+(hand?0:-1);
        const auto expected=glm::vec3(model*glm::vec4(i==gripIndex?grip[hand]:other[hand][ordinary],1));
        const auto name=words[text.texture-1];
        check(near(from,expected),"hand "+std::to_string(hand)+" "+name+" dot follows its physical button through the controller model transform");
        auto endA=glm::vec3(line.model[3])-glm::vec3(line.model[0])*line.hx;
        auto endB=glm::vec3(line.model[3])+glm::vec3(line.model[0])*line.hx;
        check(near(from,endA)||near(from,endB),name+" leader begins at the visible button dot");
        if(i==gripIndex) {
            check(name=="Grip - Toggle zoom","both hands retain the Grip zoom instruction");
            const auto local=glm::vec3(glm::inverse(model)*glm::vec4(from,1));
            check(local.y>-.04f&&local.y<-.02f&&local.z<-.02f,"Grip callout reaches the side squeeze surface, clear of the index trigger");
        }
        if(i==0)check(name.find("P"+std::to_string(gunPlayer+1)+" Reload")!=std::string::npos,"reload ownership survives player hand swaps");
        if(i==1)check(name.find("P"+std::to_string(startPlayer+1)+" Start")!=std::string::npos,"start ownership survives player hand swaps");
    }
}
}
int main(int argc,char** argv) {
    std::ofstream output;if(argc==3&&std::string(argv[1])=="--dump"){output.open(argv[2]);output<<std::setprecision(9)<<'[';}
    const auto vp=glm::perspective(glm::radians(70.f),1.f,.05f,100.f);
    for(int hand=0;hand<2;hand++)for(int pose=0;pose<3;pose++)for(int start=0;start<2;start++)for(int gun=0;gun<2;gun++) {
        auto model=glm::translate(glm::mat4(1),glm::vec3(hand==0?-.17f:.17f,0,-.6f));
        if(pose)model=glm::rotate(model,glm::radians(pose==1?55.f:-32.f),glm::normalize(glm::vec3(.3f,1,.2f)));
        if(pose==2)model=glm::scale(model,glm::vec3(.8f,1.2f,1.1f));
        vcop::controller_help::Labels labels;words.clear();quads.clear();labels.draw(vp,hand,model,start,gun);
        if(output&&pose==0&&start==0&&gun==0){if(hand)output<<',';dump(output,hand,model);}
        verify(hand,model,start,gun);
        const auto first=quads;quads.clear();labels.draw(vp,hand,model,start,gun);
        check(quads.size()==first.size(),"cached stereo layout retains all callouts");
        if(quads.size()==first.size())for(size_t i=0;i<quads.size();i++)check(quads[i].model==first[i].model,"cached stereo layout keeps leader and text transforms stable");
        quads.clear();labels.draw(vp,hand,model,1-start,1-gun);
        verify(hand,model,1-start,1-gun);
    }
    if(output)output<<"]\n";
    std::cout<<"controller grip origin: "<<(failures?"FAIL":"PASS")<<" (24 hand/pose/player cases plus cached redraws and player swaps)\n";
    return failures?1:0;
}
