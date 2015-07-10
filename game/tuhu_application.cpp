#include "tuhu_application.hpp"
#include "gl/shader_program.hpp"
#include "gl/vbo.hpp"
#include "gl/vao.hpp"

#include "math/matrix4f.hpp"

#include "gl/texture2d.hpp"
#include "gl/fbo.hpp"

#include "camera.hpp"
#include "height_map.hpp"
#include "tree.hpp"
#include "skybox.hpp"
#include "plane.hpp"
#include "quad.hpp"

#include "font.hpp"


#include "common.hpp"


using namespace std;

TuhuApplication::TuhuApplication() { }

TuhuApplication::~TuhuApplication() {
    delete camera;
    delete heightMap;
    delete skybox;
    delete tree;
    delete plane;
    delete fbo;
}

void TuhuApplication::Init() {

    LOG_I("init");

    GL_C(glEnable (GL_DEPTH_TEST)); // enable depth-testing
     GL_C(glEnable (GL_CULL_FACE)); // enable depth-testing

	LOG_I("making camera");

    camera = new Camera(GetWindowWidth(),GetWindowHeight(),Vector3f(0,5.1f,0), Vector3f(1.0f,-0.5f,1.0f));

	LOG_I("making height map");


    heightMap = new HeightMap("img/combined.png");

/*	LOG_I("making tree");

    tree = make_unique<Tree>(Vector3f(0,2,0));
*/
    skybox = new Skybox(
	"img/bluecloud_ft.png",
	"img/bluecloud_bk.png",
	"img/bluecloud_lf.png",
	"img/bluecloud_rt.png",
	"img/bluecloud_up.png",
	"img/bluecloud_dn.png"
	);



    plane = new Plane(Vector3f(1,4,1));


    LOG_I("done init");

    fbo = new FBO(10, 100,100);


}

void TuhuApplication::Render() {
    SetViewport();
    GL_C(glClearColor(0.0f, 0.0f, 1.0f, 1.0f));
    GL_C(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    skybox->Draw(*camera);

    heightMap->SetWireframe(false);

    Vector4f lightPosition(93,10.0f,93, 1.0f);
    heightMap->Draw(*camera, lightPosition);


//    tree->Draw(*camera, lightPosition);

      plane->Draw(*camera, lightPosition);

}

void TuhuApplication::Update(const float delta) {
    static float speed = 1.0f;


    if(GetKey( GLFW_KEY_W ) == GLFW_PRESS) {
	camera->Walk(delta * speed);
    }  else if(GetKey( GLFW_KEY_S ) == GLFW_PRESS) {
	camera->Walk(-delta * speed);
    }

    if(GetKey( GLFW_KEY_A ) == GLFW_PRESS) {
	camera->Stride(-delta * speed);
    }else if(GetKey( GLFW_KEY_D ) == GLFW_PRESS) {
	camera->Stride(+delta * speed);
    }

    if(GetKey( GLFW_KEY_O ) == GLFW_PRESS) {
	camera->Fly(+delta * speed);
    }else if(GetKey( GLFW_KEY_L ) == GLFW_PRESS) {
	camera->Fly(-delta * speed);
    }

    if(GetKey( GLFW_KEY_M ) == GLFW_PRESS) {
	speed = 5.0f;
    }else {
	speed = 1.0f;
    }

    camera->HandleInput();


    SetWindowTitle(tos(camera->GetPosition()) );

}

void TextureDeleter::operator()(Texture *p){ delete p;}

void TuhuApplication::RenderText()  {

/*    fbo->Bind();

    GL_C(glViewport(0, 0, 100, 100));
    GL_C(glClearColor(0.0f, 0.0f, 1.0f, 1.0f));
    GL_C(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    fbo->Unbind();

    fbo->GetRenderTargetTexture().WriteToFile("out.png");
    exit(2);*/

    m_font->DrawString(*m_fontShader, 600,150, "hello world" );

}
