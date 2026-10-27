// Application and integration: assigned to Njugi Elijah Muiru.
// Scene/interface: Kevin. Mathematical algorithms: Billiart. Documentation/tests: Samuel.
#include "raylib.h"
#include "simulation.hpp"
#include "graphics.hpp"
#include "scene.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr int CANVAS_W = 1400, CANVAS_H = 960;
const Color BG{241,245,243,255}, INK{20,48,57,255}, MUTED{91,113,119,255};
const Color TEAL{17,126,106,255}, MINT{220,244,234,255}, BORDER{216,228,222,255};
const Color PAPER{255,255,255,255}, UI_GOLD{209,145,52,255};
Font uiFont{};
struct Input { Vector2 mouse{-1,-1}; bool pressed{}, down{}; } input;
struct App {
    smartflow::TrafficSimulation sim;
    bool running{false}, routes{false}, labels{true}, bounds{false};
    float speed{1}, zoom{1};
    int view{}, selected{-1}, mode{}, nextEndpoint{};
    int x0{4},y0{6},x1{35},y1{24},cx{20},cy{15},radius{10};
    void reset() { sim.reset(); running=false; selected=-1; }
    void setView(int v) { if (v != view) running=false; view=v; }
    void resetLab() { x0=4; y0=6; x1=35; y1=24; cx=20; cy=15; radius=10; nextEndpoint=0; }
};
std::string fixed(double n,int digits=1) { std::ostringstream s; s<<std::fixed<<std::setprecision(digits)<<n; return s.str(); }
std::string approach(char c) { switch(c){case 'N':return "North";case 'E':return "East";case 'S':return "South";default:return "West";} }
void text(const std::string& s,float x,float y,float size=18,Color color=INK) { DrawTextEx(uiFont,s.c_str(),{x,y},size,.35f,color); }
float width(const std::string& s,float size=18) { return MeasureTextEx(uiFont,s.c_str(),size,.35f).x; }
void wrap(const std::string& s,float x,float y,float w,float size=17,Color color=MUTED,float line=24) {
    std::istringstream stream(s); std::string word,current;
    while(stream>>word) { auto candidate=current.empty()?word:current+" "+word;
        if(width(candidate,size)>w && !current.empty()) {text(current,x,y,size,color); y+=line;current=word;} else current=candidate;
    } if(!current.empty()) text(current,x,y,size,color);
}
void panel(Rectangle r,Color fill=PAPER) { DrawRectangleRounded(r,.04f,8,fill); DrawRectangleRoundedLinesEx(r,.04f,8,1,BORDER); }
bool hit(Rectangle r) { return CheckCollisionPointRec(input.mouse,r); }
bool button(Rectangle r,const std::string& label,bool active=false,bool enabled=true) {
    const bool hover=hit(r)&&enabled;
    Color fill=active?TEAL:(hover?MINT:PAPER);
    if(!enabled)fill=BG;
    DrawRectangleRounded(r,.18f,7,fill);
    DrawRectangleRoundedLinesEx(r,.18f,7,1,active?TEAL:BORDER);
    text(label,r.x+(r.width-width(label,17))/2,r.y+(r.height-17)/2-1,17,!enabled?MUTED:(active?WHITE:INK));
    return hover && input.pressed;
}
bool slider(float x,float y,float w,const std::string& label,float& value,float min,float max,const std::string& suffix="") {
    text(label,x,y,18); std::string display=fixed(value,0)+suffix; text(display,x+w-width(display,17),y,17,TEAL);
    Rectangle track{x,y+34,w,6}; bool changed=false;
    if(input.down && hit({x-8,y+22,w+16,30})) {value=std::round(min+(max-min)*std::clamp((input.mouse.x-x)/w,0.f,1.f));changed=true;}
    DrawRectangleRounded(track,1,8,BORDER);
    float progress=(value-min)/(max-min);
    DrawRectangleRounded({x,y+34,std::max(1.f,w*progress),6},1,8,TEAL);
    DrawCircleV({x+w*progress,y+37},8,TEAL); DrawCircleV({x+w*progress,y+37},3,WHITE);
    return changed;
}
void toggle(float x,float y,const std::string& label,bool& value) {
    if(input.pressed&&hit({x,y,236,29}))value=!value;
    DrawRectangleRounded({x,y+2,38,21},1,8,value?TEAL:BORDER);
    DrawCircleV({x+(value?27.f:11.f),y+12.5f},7.5f,WHITE);text(label,x+51,y+2,17);
}
void rule(float x,float y,float w) {DrawLineEx({x,y},{x+w,y},1,BORDER);}
void metric(float x,float y,float w,const std::string& label,const std::string& value,const std::string& detail) {
    panel({x,y,w,91});text(label,x+18,y+12,16,MUTED);text(value,x+18,y+34,28);text(detail,x+18,y+68,13,MUTED);
}
void header(App& a) {
    DrawRectangle(0,0,CANVAS_W,81,INK);
    DrawLineEx({26,54},{26,26},5,Color{92,221,180,255});
    DrawLineEx({26,26},{40,26},5,Color{92,221,180,255});
    DrawLineEx({40,26},{40,54},5,Color{92,221,180,255});
    DrawLineEx({40,54},{54,54},5,Color{92,221,180,255});
    DrawLineEx({54,54},{54,26},5,Color{92,221,180,255});
    text("SmartFlow",72,17,28,WHITE);text("TRAFFIC & GRAPHICS LAB",74,50,11,Color{159,198,189,255});
    if(button({400,21,150,39},"Simulation",a.view==0))a.setView(0);
    if(button({562,21,150,39},"Graphics lab",a.view==1))a.setView(1);
    if(button({724,21,150,39},"Project brief",a.view==2))a.setView(2);
    text("C++  /  NATIVE DESKTOP",1120,33,16,Color{188,219,209,255});
    rule(24,925,1352);
    text("SMARTFLOW   /   COMPUTER GRAPHICS",24,937,12,MUTED);
    text("1 Simulation    2 Graphics lab    3 Project brief    F11 Fullscreen",890,937,12,MUTED);
}
void inspector(App& a,Rectangle area) {
    auto it=std::find_if(a.sim.vehicles.begin(),a.sim.vehicles.end(),[&](const auto& v){return v.id==a.selected;});
    if(it==a.sim.vehicles.end()){a.selected=-1;return;}
    Rectangle r{area.x+area.width-265,area.y+16,248,198};panel(r);
    text("Vehicle "+std::to_string(it->id),r.x+17,r.y+14,21);
    if(button({r.x+r.width-39,r.y+10,28,28},"x"))a.selected=-1;
    text(approach(it->approach)+" / "+it->turn,r.x+17,r.y+49,17,TEAL);
    text("Position    "+fixed(it->x,0)+", "+fixed(it->y,0),r.x+17,r.y+81,16);
    text("Rotation   "+fixed(it->angle)+" deg",r.x+17,r.y+107,16);
    text("Speed       "+fixed(it->speed*.54)+" km/h",r.x+17,r.y+133,16);
    text("Stopped    "+fixed(it->wait)+" seconds",r.x+17,r.y+159,16);
}
void simulationView(App& a) {
    text("Traffic simulation",24,98,27);text("Seed 42  /  reproducible arrivals",1070,105,16,MUTED);
    panel({24,144,284,763});text("Control room",44,166,22);text(a.running?"RUNNING":"PAUSED",226,170,12,a.running?TEAL:MUTED);
    if(button({44,210,170,42},a.running?"Pause":"Start",true))a.running=!a.running;
    if(button({222,210,65,42},"Reset"))a.reset();
    if(button({44,264,243,34},"Step forward  + 1 second",false,!a.running))a.sim.step(1);
    rule(44,319,243);
    float density=static_cast<float>(a.sim.density);
    if(slider(44,339,243,"Traffic demand",density,4,60," / min"))a.sim.setDensity(density);
    if(button({44,395,76,32},"Quiet",density==10))a.sim.setDensity(10);
    if(button({126,395,76,32},"Normal",density==24))a.sim.setDensity(24);
    if(button({208,395,79,32},"Rush",density==48))a.sim.setDensity(48);
    float green=static_cast<float>(a.sim.greenDuration);
    if(slider(44,454,243,"Green duration",green,6,30," s"))a.sim.setGreenDuration(green);
    text("Applies at the next green phase.",44,510,14,MUTED);
    text("Playback speed",44,550,18);
    const float speeds[]{.5f,1,2,4};const char* labels[]{"0.5x","1x","2x","4x"};
    for(int i=0;i<4;i++)if(button({44.f+i*62,581,57,33},labels[i],a.speed==speeds[i]))a.speed=speeds[i];
    rule(44,637,243);text("Scene layers",44,657,18);
    toggle(44,695,"Route paths",a.routes);toggle(44,731,"Scene labels",a.labels);toggle(44,767,"Junction boundary",a.bounds);
    wrap("Click a vehicle to inspect its route, position and motion.",44,817,243,16,MUTED,22);
    text("Space Play / pause     R Reset",44,878,13,MUTED);
    metric(330,144,246,"On the road",std::to_string(a.sim.stats.active),"active vehicles");
    metric(592,144,246,"Waiting",std::to_string(a.sim.stats.queued),std::to_string(a.sim.stats.pending)+" waiting offscreen");
    metric(854,144,246,"Completed trips",std::to_string(a.sim.stats.completed),"since reset");
    metric(1116,144,260,"Average wait",fixed(a.sim.stats.averageWait)+" s","completed trips only");
    panel({330,253,1046,577});text("Campus intersection",350,268,19);text("Left-hand traffic",548,271,15,MUTED);
    text("SIM TIME  "+fixed(a.sim.time)+" s",1160,271,15,MUTED);
    Rectangle scene{331,306,1044,477};
    smartflow::drawScene(a.sim,scene,a.zoom,a.routes,a.labels,a.bounds,a.selected);
    if(input.pressed&&hit(scene)) {
        Rectangle inspectBox{scene.x+scene.width-265,scene.y+16,248,198};
        if(a.selected<0||!hit(inspectBox))a.selected=smartflow::pickVehicle(a.sim,scene,a.zoom,input.mouse);
    }
    inspector(a,scene);
    text("Go",367,799,14,MUTED);DrawCircle(354,806,5,TEAL);
    text("Clearing",431,799,14,MUTED);DrawCircle(418,806,5,UI_GOLD);
    text("Stop",536,799,14,MUTED);DrawCircle(523,806,5,Color{214,91,94,255});
    text("1 world unit ~ 0.15 m",614,799,14,MUTED);
    if(button({1130,791,34,29},"-"))a.zoom=std::max(.7f,a.zoom-.1f);
    text(fixed(a.zoom*100,0)+"%",1175,797,15);
    if(button({1225,791,34,29},"+"))a.zoom=std::min(1.6f,a.zoom+.1f);
    if(button({1270,791,88,29},"Fit"))a.zoom=1;
    panel({330,846,1046,61});
    text(a.sim.phaseLabel(),348,860,20);
    const std::string detail=(a.sim.phase=="allRed"&&a.sim.phaseRemaining==0)?"Waiting for junction to clear":fixed(a.sim.phaseRemaining)+" seconds remaining";
    text(detail,348,885,13,MUTED);
    for(int i=0;i<4;i++) {char c=smartflow::APPROACHES[i];auto signal=a.sim.signal(c);Color color=signal=="green"?TEAL:(signal=="amber"?UI_GOLD:Color{194,82,85,255});DrawCircle(860+i*77,876,8,color);text(std::string(1,c),878+i*77,866,20);}
    if(button({1182,859,177,34},"Next phase"))a.sim.requestNextPhase();
}
void numberControl(float x,float y,const std::string& label,int& value,int lo,int hi) {
    text(label,x,y,15,MUTED);if(button({x,y+24,34,30},"-"))value=std::max(lo,value-1);
    text(std::to_string(value),x+52,y+30,18);if(button({x+88,y+24,34,30},"+"))value=std::min(hi,value+1);
}
void labView(App& a) {
    text("Graphics lab",24,98,27);text("C++ calculates the samples. raylib draws them.",830,105,18,MUTED);
    panel({24,144,302,763});text("Drawing algorithm",44,165,21);
    const char* labels[]{"Bresenham line","Midpoint circle","Line clipping"};
    for(int i=0;i<3;i++)if(button({44,208.f+i*45,261,36},labels[i],a.mode==i)){a.mode=i;a.nextEndpoint=0;}
    rule(44,357,261);
    if(a.mode==1){
        text("Circle geometry",44,381,21);numberControl(44,427,"Centre X",a.cx,0,40);numberControl(181,427,"Centre Y",a.cy,0,30);
        float radius=static_cast<float>(a.radius);if(slider(44,523,261,"Radius",radius,1,14," cells"))a.radius=static_cast<int>(radius);
        wrap("Click the grid to move the centre. The algorithm uses eight-way symmetry.",44,604,261,17,MUTED,25);
    }else{
        text("Line endpoints",44,381,21);numberControl(44,427,"Start X",a.x0,0,40);numberControl(181,427,"Start Y",a.y0,0,30);
        numberControl(44,512,"End X",a.x1,0,40);numberControl(181,512,"End Y",a.y1,0,30);
        wrap("Click the grid to set "+std::string(a.nextEndpoint==0?"the start point.":"the end point.")+" Clicks alternate between endpoints.",44,605,261,17,MUTED,25);
    }
    if(button({44,724,261,38},"Reset geometry"))a.resetLab();
    wrap(a.mode==0?"The error term chooses the next neighbouring cell with integer arithmetic.":a.mode==1?"The midpoint decision chooses each circle sample without calculating an angle.":"Cohen-Sutherland tests endpoint region codes and clips against four edges.",44,792,261,16,TEAL,23);
    panel({346,144,1030,763});text(a.mode==0?"Bresenham line rasterisation":a.mode==1?"Midpoint circle rasterisation":"Cohen-Sutherland line clipping",370,166,22);
    constexpr float gx=423,gy=239,cell=21; Rectangle grid{gx,gy,40*cell,30*cell};
    if(input.pressed&&hit({gx-cell/2,gy-cell/2,41*cell,31*cell})){
        int x=std::clamp(static_cast<int>(std::round((input.mouse.x-gx)/cell)),0,40);int y=std::clamp(static_cast<int>(std::round((input.mouse.y-gy)/cell)),0,30);
        if(a.mode==1){a.cx=x;a.cy=y;}else if(a.nextEndpoint==0){a.x0=x;a.y0=y;a.nextEndpoint=1;}else{a.x1=x;a.y1=y;a.nextEndpoint=0;}
    }
    for(int x=0;x<=40;x++){DrawLineEx({gx+x*cell,gy},{gx+x*cell,gy+30*cell},1,BORDER);if(x%5==0)text(std::to_string(x),gx+x*cell-7,gy-24,13,MUTED);}
    for(int y=0;y<=30;y++){DrawLineEx({gx,gy+y*cell},{gx+40*cell,gy+y*cell},1,BORDER);if(y%5==0)text(std::to_string(y),gx-32,gy+y*cell-6,13,MUTED);}
    std::vector<smartflow::graphics::Pixel> pixels;
    if(a.mode==1) pixels=smartflow::graphics::midpointCircle(a.cx,a.cy,a.radius);
    else if(a.mode==0)pixels=smartflow::graphics::bresenhamLine(a.x0,a.y0,a.x1,a.y1);
    else {
        DrawRectangleRec({gx+10*cell,gy+7*cell,20*cell,16*cell},Fade(TEAL,.06f));
        DrawRectangleLinesEx({gx+10*cell,gy+7*cell,20*cell,16*cell},2,TEAL);
        auto clipped=smartflow::graphics::clipLine(a.x0,a.y0,a.x1,a.y1,{10,7,30,23});
        if(clipped){
            DrawLineEx({gx+static_cast<float>(clipped->x0)*cell,gy+static_cast<float>(clipped->y0)*cell},{gx+static_cast<float>(clipped->x1)*cell,gy+static_cast<float>(clipped->y1)*cell},6,TEAL);
            text("Accepted segment shown in green",981,171,15,TEAL);
        }else text("Segment rejected",1150,171,15,UI_GOLD);
    }
    BeginScissorMode(static_cast<int>(grid.x-cell/2),static_cast<int>(grid.y-cell/2),static_cast<int>(grid.width+cell),static_cast<int>(grid.height+cell));
    for(const auto& p:pixels) if(p.x>=0&&p.x<=40&&p.y>=0&&p.y<=30)DrawRectangleRounded({gx+p.x*cell-8,gy+p.y*cell-8,16,16},.15f,4,Fade(TEAL,.65f));
    if(a.mode==1)DrawCircleLines(static_cast<int>(gx+a.cx*cell),static_cast<int>(gy+a.cy*cell),a.radius*cell,UI_GOLD);
    else {DrawLineEx({gx+a.x0*cell,gy+a.y0*cell},{gx+a.x1*cell,gy+a.y1*cell},2,UI_GOLD);DrawCircleV({gx+a.x0*cell,gy+a.y0*cell},5,INK);DrawCircleV({gx+a.x1*cell,gy+a.y1*cell},5,INK);}
    EndScissorMode();
    if(a.mode!=2)text(std::to_string(pixels.size())+" calculated samples",1110,171,15,TEAL);
    text("Green: calculated result     Gold: ideal geometry     Coordinates: +x right, +y down",427,886,14,MUTED);
}
void briefView() {
    text("Project brief",24,98,27);text("One intersection. A practical graphics study.",770,105,20,MUTED);
    panel({24,144,656,205});text("The idea and the audience",46,165,23);
    wrap("SmartFlow helps computer graphics students explore how geometry and traffic rules create an animated scene. Change demand and signal timing, then observe the queues and vehicle transformations.",46,206,610,19,INK,28);
    panel({700,144,676,205});text("A manageable scope",723,165,23);
    wrap("One junction with left-hand traffic, twelve routes, timed signals and interactive graphics algorithms. This teaching model uses conservative junction reservations. It is not calibrated to predict real road traffic.",723,206,626,19,INK,28);
    panel({24,369,1352,135});text("Graphics you can explain",46,390,23);
    wrap("Primitive shapes and colour  /  Translation and rotation  /  Scaling and viewport clipping  /  Cubic Bezier curves and path sampling  /  Fixed-step animation  /  Bresenham lines, midpoint circles and Cohen-Sutherland clipping",46,433,1295,19,INK,29);
    panel({24,524,1352,283});text("Five members and assigned responsibilities",46,545,23);
    const char* ids[]{"165899","164099","146412","168377","152710"};
    const char* names[]{"Macharia Grace Wairimu","Mwangi Billiart B. Muthoni","Njugi Elijah Muiru","Kihumba Kevin Renana","Kisotu Samuel Lemayian"};
    const char* roles[]{"Concept, purpose and introduction","Mathematics, route geometry and drawing algorithms","Lead programming, simulation and integration","Scene, interface and feature planning","Documentation, testing and demonstration preparation"};
    for(int i=0;i<5;i++){float y=597.f+i*38; if(i%2==0)DrawRectangleRec({43,y-5,1310,35},BG);text(ids[i],54,y,17,TEAL);text(names[i],160,y,18);text(roles[i],608,y,18);}
    panel({24,827,1352,80});text("Development sequence",46,844,20);
    text("Concept and design  >  Core graphics  >  Advanced graphics  >  Integration and refinement",330,846,20,TEAL);
    text("Each member should understand their assigned area and record their actual contributions through GitHub.",46,878,16,MUTED);
}
void render(App& a,RenderTexture2D canvas) {
    BeginTextureMode(canvas);ClearBackground(BG);header(a);
    if(a.view==0)simulationView(a);else if(a.view==1)labView(a);else briefView();
    EndTextureMode();
}
void capture(RenderTexture2D canvas,const std::filesystem::path& file) {
    Image image=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&image);
    if(!ExportImage(image,file.string().c_str())){UnloadImage(image);throw std::runtime_error("Screenshot export failed");}
    UnloadImage(image);
}
void require(bool condition,const std::string& message) {if(!condition)throw std::runtime_error(message);}
void smokeTest(App& a,RenderTexture2D canvas,const std::filesystem::path& out) {
    std::filesystem::create_directories(out);
    auto click=[&](float x,float y){input={{x,y},true,true};render(a,canvas);input={};};
    render(a,canvas);click(100,230);require(a.running,"Start button");click(100,230);require(!a.running,"Pause button");
    click(120,282);require(std::abs(a.sim.time-1)<1e-8,"Step button");click(251,230);require(a.sim.time==0,"Reset button");
    click(240,410);require(a.sim.density==48,"Rush preset");click(287,376);require(a.sim.density==60,"Demand slider");
    click(287,491);require(a.sim.greenDuration==30,"Green slider");click(267,596);require(a.speed==4,"Speed control");
    click(1260,875);require(a.sim.phase=="amber","Next phase transition");
    click(1240,804);require(a.zoom>1,"Zoom control");click(1310,804);require(a.zoom==1,"Fit control");
    click(65,706);click(65,778);require(a.routes&&a.bounds,"Scene layers");
    a.sim.setDensity(48);a.sim.setGreenDuration(10);a.reset();a.sim.step(100);
    const Rectangle area{331,306,1044,477};
    const float scale=std::min(area.width/1000,area.height/800);
    for(const auto& v:a.sim.vehicles){
        if(v.x<0||v.x>1000||v.y<0||v.y>800)continue;
        Vector2 mouse{area.x+area.width/2+static_cast<float>(v.x-500)*scale,area.y+area.height/2+static_cast<float>(v.y-400)*scale};
        if(CheckCollisionPointRec(mouse,area)){click(mouse.x,mouse.y);require(a.selected>=0,"Vehicle selection");break;}
    }
    require(a.selected>=0,"A visible vehicle must be selected during the UI check");
    render(a,canvas);capture(canvas,out/"simulation.png");
    a.running=true;click(620,40);require(a.view==1&&!a.running,"Lab navigation pauses traffic");
    click(528,449);require(a.x0==5&&a.y0==10,"Grid endpoint input");render(a,canvas);capture(canvas,out/"lab-line.png");
    click(120,267);require(a.mode==1,"Circle mode");click(864,596);require(a.cx==21&&a.cy==17,"Circle centre input");render(a,canvas);capture(canvas,out/"lab-circle.png");
    click(125,750);require(a.radius==10&&a.cx==20,"Lab reset");click(120,314);require(a.mode==2,"Clip mode");render(a,canvas);capture(canvas,out/"lab-clipping.png");
    click(800,40);require(a.view==2,"Brief navigation");render(a,canvas);capture(canvas,out/"project-brief.png");
    click(450,40);require(a.view==0,"Simulation navigation");
    std::ofstream(out/"ui-results.txt")<<"PASS: native UI start, pause, step, reset, presets, sliders, speed, signal advance, zoom, layers, vehicle selection, all lab modes, coordinate input, geometry reset, navigation and pause-on-tab-change.\n";
}
} // namespace

int main(int argc,char** argv) {
    bool smoke=argc==3&&std::string(argv[1])=="--smoke-test";
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT|(smoke?FLAG_WINDOW_HIDDEN:0));
    SetTraceLogLevel(smoke?LOG_WARNING:LOG_ERROR);
    InitWindow(1260,864,"SmartFlow - C++ Traffic and Graphics Lab");
    if(!IsWindowReady())return 1;
    SetWindowMinSize(1050,720);SetTargetFPS(60);SetExitKey(KEY_NULL);
    uiFont=GetFontDefault();bool loadedFont=false;
    // Read the installed Windows UI font; no proprietary font is redistributed.
    const char* windowsFont="C:/Windows/Fonts/segoeui.ttf";
    if(FileExists(windowsFont)){uiFont=LoadFontEx(windowsFont,40,nullptr,0);loadedFont=uiFont.texture.id!=GetFontDefault().texture.id;}
    SetTextureFilter(uiFont.texture,TEXTURE_FILTER_BILINEAR);
    RenderTexture2D canvas=LoadRenderTexture(CANVAS_W,CANVAS_H);SetTextureFilter(canvas.texture,TEXTURE_FILTER_BILINEAR);
    int result=0;
    try {
        App app;
        bool wasFocused=IsWindowFocused();
        if(smoke)smokeTest(app,canvas,argv[2]);
        else while(!WindowShouldClose()) {
            float scale=std::min(static_cast<float>(GetScreenWidth())/CANVAS_W,static_cast<float>(GetScreenHeight())/CANVAS_H);
            Vector2 offset{(GetScreenWidth()-CANVAS_W*scale)/2,(GetScreenHeight()-CANVAS_H*scale)/2};
            Vector2 mouse=GetMousePosition();input={{(mouse.x-offset.x)/scale,(mouse.y-offset.y)/scale},IsMouseButtonPressed(MOUSE_BUTTON_LEFT),IsMouseButtonDown(MOUSE_BUTTON_LEFT)};
            if(IsKeyPressed(KEY_ONE))app.setView(0);
            if(IsKeyPressed(KEY_TWO))app.setView(1);
            if(IsKeyPressed(KEY_THREE))app.setView(2);
            if(IsKeyPressed(KEY_F11))ToggleBorderlessWindowed();
            if(IsKeyPressed(KEY_ESCAPE))app.selected=-1;
            if(app.view==0){
                if(IsKeyPressed(KEY_SPACE))app.running=!app.running;
                if(IsKeyPressed(KEY_R))app.reset();
                if(IsKeyPressed(KEY_S)&&!app.running)app.sim.step(1);
                if(IsKeyPressed(KEY_N))app.sim.requestNextPhase();
                if(IsKeyPressed(KEY_L))app.routes=!app.routes;
                if(IsKeyPressed(KEY_H))app.labels=!app.labels;
                if(IsKeyPressed(KEY_B))app.bounds=!app.bounds;
                if(IsKeyPressed(KEY_EQUAL)||IsKeyPressed(KEY_KP_ADD))app.zoom=std::min(1.6f,app.zoom+.1f);
                if(IsKeyPressed(KEY_MINUS)||IsKeyPressed(KEY_KP_SUBTRACT))app.zoom=std::max(.7f,app.zoom-.1f);
                if(app.running&&wasFocused&&IsWindowFocused())app.sim.step(std::min(static_cast<double>(GetFrameTime()),.1)*app.speed);
            }
            wasFocused=IsWindowFocused();
            render(app,canvas);
            BeginDrawing();ClearBackground(INK);
            DrawTexturePro(canvas.texture,{0,0,static_cast<float>(CANVAS_W),-static_cast<float>(CANVAS_H)},{offset.x,offset.y,CANVAS_W*scale,CANVAS_H*scale},{0,0},0,WHITE);
            EndDrawing();
        }
    }catch(const std::exception& e){std::ofstream("SmartFlow-error.log")<<e.what()<<'\n';result=1;}
    UnloadRenderTexture(canvas);if(loadedFont)UnloadFont(uiFont);CloseWindow();return result;
}
