#include "tuhu_application.hpp"

#include "ewa/camera.hpp"
#include "ewa/common.hpp"
#include "ewa/font.hpp"
#include "ewa/keyboard_state.hpp"

#include "ewa/gl/depth_fbo.hpp"
#include "ewa/gl/texture.hpp"

#include "ewa/audio/sound.hpp"
#include "ewa/audio/wave_loader.hpp"

#include "skydome.hpp"
#include "height_map.hpp"
#include "grass.hpp"
#include "particle_system.hpp"
#include "smoke_effect.hpp"
#include "snow_effect.hpp"
#include "fire_effect.hpp"

using namespace std;

constexpr int SHADOW_MAP_SIZE = 1024;

void ToClipboard(const std::string& str) {
    std::string command = "echo '" + str + "' | pbcopy";
    system(command.c_str());
}

//(0.705072, 0.0758142, 0.705072)
TuhuApplication::TuhuApplication(int argc, char *argv[]):Application(argc, argv), m_camera(NULL), m_heightMap(NULL),m_skydome(NULL), m_lightDirection(

    -0.705072, -0.458142, -0.705072,
//    -0.705072f, -0.0758142f, -0.705072f ,

    0.0f){ }

TuhuApplication::~TuhuApplication() {
    MY_DELETE(m_camera);
    MY_DELETE(m_heightMap);
    MY_DELETE(m_skydome);
    MY_DELETE(m_windSound);
}

void TuhuApplication::Init() {
    m_smoke = new SmokeEffect(Vector3f(11.5,-3,10));
    m_smoke->Init();

    ::SetDepthTest(true);
    ::SetCullFace(true);

    const Vector3f pos =

	Vector3f(17.328205, 15.360136, 14.091190);




    Vector3f(5.997801, 5.711470, -3.929811);
    m_camera = new Camera(GetWindowWidth(),GetWindowHeight(),




			  pos,
Vector3f(-0.597377, -0.590989, -0.542100)

			  , true);



    m_snow = new SnowEffect(pos);
    m_snow->Init();



    m_fire = new FireEffect(Vector3f(12,-3,10));
    m_fire->Init();


    // m_heightMap = new HeightMap("img/combined.png");



    //                    128000
    m_skydome = new Skydome(1, 10, 10);

//    m_grass = new Grass(Vector2f(10,10), m_heightMap);


    m_stoneFloor = new GeometryObject();
    m_stoneFloor->Init("obj/rock_floor.eob");
    m_stoneFloor->SetModelMatrix(
	Matrix4f::CreateTranslation(Vector3f(0,0,0)));

    m_flatWoodFloor = new GeometryObject();
    m_flatWoodFloor->Init("obj/flat_wood_floor.eob");
    m_flatWoodFloor->SetModelMatrix(
	Matrix4f::CreateTranslation(Vector3f(10,0,0)));

    m_woodFloor = new GeometryObject();
    m_woodFloor->Init("obj/wood_floor.eob");
    m_woodFloor->SetModelMatrix(
	Matrix4f::CreateTranslation(Vector3f(-10,0,0)));

    m_sphere = new GeometryObject();
    m_sphere->Init("obj/sunball.eob");
    m_sphere->SetModelMatrix(
	Matrix4f::CreateTranslation(Vector3f(0,3,0)));


    m_plane = new GeometryObject();
    m_plane->Init("obj/color.eob");
    m_plane->SetModelMatrix(

	Matrix4f::CreateScale(Vector3f(10,1.0,10))   *
	Matrix4f::CreateTranslation(Vector3f(0,-2.5,0))
	);

    m_tree = new GeometryObject();
    m_tree->Init("obj/tree.eob");
    m_tree->SetModelMatrix(
	Matrix4f::CreateTranslation(Vector3f(10,-2.5,10)));

    m_wall = new GeometryObject();
    m_wall->Init("obj/wall.eob");
    m_wall->SetModelMatrix(
	Matrix4f::CreateTranslation(Vector3f(-5,-2.5,-5)));

    m_wall2 = new GeometryObject();
    m_wall2->Init("obj/wall.eob");
    m_wall2->SetModelMatrix(
	Matrix4f::CreateScale(Vector3f(1,0.4,1))   *
	Matrix4f::CreateTranslation(Vector3f(20,-6.5,-5)));


    m_ball2 = new GeometryObject();
    m_ball2->Init("obj/sunball.eob");
    m_ball2->SetModelMatrix(
    Matrix4f::CreateTranslation(Vector3f(20,-1.0,-7)));

/*    Matrix4f m = Matrix4f::CreateScale(Vector3f(10,1.0,10))   *
      Matrix4f::CreateTranslation(Vector3f(0,-2.0,0));
*/
    // LOG_I("m:\n %s", string(m).c_str() );

    m_depthFbo = new DepthFBO();
m_depthFbo->Init(9, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);


    /*
      OpenAL::Initp();

      m_windSound = new Sound("audio/wind2.wav");
      m_windSound->SetGain(1.0f);
      m_windSound->SetLooping(true);
    */

/*
    Vector3f lightInvDir = Vector3f(0.5f,2,2);

    Matrix4f depthViewMatrix = Matrix4f::CreateLookAt(lightInvDir, Vector3f(0,0,0), Vector3f(0,1,0));


    LOG_I("depthviewmatrix: %s", string(depthViewMatrix).c_str() );


    Matrix4f projMatrix = Matrix4f::CreateOrthographic(-10,10,-10,10,-10,20);

    LOG_I("projmatrix: %s", string(projMatrix).c_str() );

    Matrix4f prod = projMatrix * depthViewMatrix;

    LOG_I("mvo: %s", string(prod).c_str() );

    Matrix4f bias = Matrix4f(
	0.5f, 0.0f, 0.0f, 0.5f,
	0.0f, 0.5f, 0.0f, 0.5f,
	0.0f, 0.0f, 0.5f, 0.5f,
	0.0f, 0.0f, 0.0f, 1.0f
	);

    Matrix4f a = bias * prod;

    LOG_I("a: %s", string(a).c_str() );


     exit(1);
*/
}


void TuhuApplication::RenderShadowMap() {

    m_depthFbo->Bind();
    {
	::SetViewport(0,0,SHADOW_MAP_SIZE,SHADOW_MAP_SIZE);

	Clear(0.0f, 1.0f, 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_lightViewMatrix = Matrix4f::CreateLookAt(
	    -Vector3f(m_lightDirection),
	    Vector3f(0.0f, 0.0f, 0.0f),
	    Vector3f(0.0, 1.0, 0.0)
	    );

	m_lightProjectionMatrix = Matrix4f::CreateOrthographic(-30, 30, -12, 12, -20, 50);

	Matrix4f vp = m_lightProjectionMatrix * m_lightViewMatrix;

/*
  m_stoneFloor->RenderShadowMap(vp);

  m_flatWoodFloor->RenderShadowMap(vp);

  m_woodFloor->RenderShadowMap(vp);
*/
	m_sphere->RenderShadowMap(vp);

	m_tree->RenderShadowMap(vp);

	m_wall->RenderShadowMap(vp);
	m_wall2->RenderShadowMap(vp);

	m_ball2->RenderShadowMap(vp);

    }
   m_depthFbo->Unbind();



}

void TuhuApplication::RenderScene() {

    // set the viewport to the size of the window.
    SetViewport();

    Clear(0.0f, 1.0f, 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //m_skydome->Draw(*m_camera);

//   m_heightMap->Render(*m_camera, m_lightDirection);

//    m_grass->Draw(*m_camera, m_lightDirection);

    //  m_smoke->Render(m_camera->GetMvpFromM(), m_camera->GetPosition());


    // m_snow->Render(m_camera->GetMvpFromM(), m_camera->GetPosition());

    //  m_fire->Render(m_camera->GetMvpFromM(), m_camera->GetPosition());


/*
  m_stoneFloor->Render(*m_camera, m_lightDirection);

  m_flatWoodFloor->Render(*m_camera, m_lightDirection);

  m_woodFloor->Render(*m_camera, m_lightDirection);
*/

    Matrix4f biasMatrix(
	0.5f, 0.0f, 0.0f, 0.5f,
	0.0f, 0.5f, 0.0f, 0.5f,
	0.0f, 0.0f, 0.5f, 0.5f,
	0.0f, 0.0f, 0.0f, 1.0f
	);

    Matrix4f lightVp =  biasMatrix *  m_lightProjectionMatrix * m_lightViewMatrix;

    m_tree->Render(*m_camera, m_lightDirection, lightVp, *m_depthFbo);

      m_sphere->Render(*m_camera, m_lightDirection, lightVp, *m_depthFbo);


     m_plane->Render(*m_camera, m_lightDirection, lightVp, *m_depthFbo);


    m_sphere->SetModelMatrix(
	Matrix4f::CreateTranslation(  Vector3f(-30 * m_lightDirection) ));
    m_sphere->Render(*m_camera, m_lightDirection, lightVp, *m_depthFbo);
    m_sphere->SetModelMatrix(
	Matrix4f::CreateTranslation(Vector3f(0,3,0)));

     m_wall->Render(*m_camera, m_lightDirection, lightVp, *m_depthFbo);
     m_wall2->Render(*m_camera, m_lightDirection, lightVp, *m_depthFbo);

     m_ball2->Render(*m_camera, m_lightDirection, lightVp, *m_depthFbo);

}

void TuhuApplication::Render() {

  RenderShadowMap();


  RenderScene();
}

void TuhuApplication::Update(const float delta) {

    m_camera->HandleInput(delta);

    m_smoke->Update(delta);
    //   m_snow->Update(delta);
    m_fire->Update(delta);

    m_skydome->Update(delta);

//      m_grass->Update(delta);

    const KeyboardState& kbs = KeyboardState::GetInstance();

    if( kbs.IsPressed(GLFW_KEY_P) ) {

	string out = "Vector3f" +tos(m_camera->GetPosition()) + ",";
	out += "Vector3f" + tos(m_camera->GetViewDir());
	ToClipboard(out);
    }

    static bool b= false;

    if( kbs.IsPressed(GLFW_KEY_B) && !b ) {
	LOG_I("play sound");
	m_windSound->Play();

	b = true;
    }
}

void TuhuApplication::RenderText()  {

    m_font->DrawString(*m_fontShader, 600,150, "hello world" );
}
