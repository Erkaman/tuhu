#include "gui.hpp"
#include "gui_enum.hpp"
#include "gui_listener.hpp"

#include <imgui.h>

#include "ewa/log.hpp"
#include "ewa/file.hpp"
#include "ewa/keyboard_state.hpp"
#include "ewa/mouse_state.hpp"

#include "nfd.h"


#include <imgui.h>

#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library

#include <stdio.h>

float deltaScroll;

#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif


constexpr int MIN_CURSOR_SIZE = 10;
constexpr int MAX_CURSOR_SIZE = 40;
constexpr int DEFAULT_RADIUS = 35;

constexpr float MIN_NOISE_SCALE = 0.004;
constexpr float MAX_NOISE_SCALE = 0.1;
constexpr float DEFAULT_NOISE_SCALE = 0.04;


// Data
static GLFWwindow*  g_Window = NULL;
static double       g_Time = 0.0f;
static bool         g_MousePressed[3] = { false, false, false };
static float        g_MouseWheel = 0.0f;
static GLuint       g_FontTexture = 0;
static int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
static int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
static unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_ImplGlfwGL3_RenderDrawLists(ImDrawData* draw_data)
{
    // Backup GL state
    GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
    GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
    GLint last_blend_src; glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
    GLint last_blend_dst; glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
    GLint last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
    GLint last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    // Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
    ImGuiIO& io = ImGui::GetIO();
    float fb_height = io.DisplaySize.y * io.DisplayFramebufferScale.y;
    float fb_width = io.DisplaySize.x * io.DisplayFramebufferScale.x;

    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);

    const float ortho_projection[4][4] =
	{
	    { 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
	    { 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
	    { 0.0f,                  0.0f,                  -1.0f, 0.0f },
	    { -1.0f,                  1.0f,                   0.0f, 1.0f },
	};
    glUseProgram(g_ShaderHandle);
    glUniform1i(g_AttribLocationTex, 0);
    glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
    glBindVertexArray(g_VaoHandle);

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
	const ImDrawList* cmd_list = draw_data->CmdLists[n];
	const ImDrawIdx* idx_buffer_offset = 0;

	glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&cmd_list->VtxBuffer.front(), GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), (GLvoid*)&cmd_list->IdxBuffer.front(), GL_STREAM_DRAW);

	for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++)
	{
	    if (pcmd->UserCallback)
	    {
		pcmd->UserCallback(cmd_list, pcmd);
	    }
	    else
	    {
		glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
		glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
		glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
	    }
	    idx_buffer_offset += pcmd->ElemCount;
	}
    }

    // Restore modified GL state
    glUseProgram(last_program);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
    glBindVertexArray(last_vertex_array);
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFunc(last_blend_src, last_blend_dst);
    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
}

static const char* ImGui_ImplGlfwGL3_GetClipboardText()
{
    return glfwGetClipboardString(g_Window);
}

static void ImGui_ImplGlfwGL3_SetClipboardText(const char* text)
{
    glfwSetClipboardString(g_Window, text);
}

void ImGui_ImplGlfwGL3_MouseButtonCallback(GLFWwindow*, int button, int action, int /*mods*/)
{
    if (action == GLFW_PRESS && button >= 0 && button < 3)
	g_MousePressed[button] = true;
}

void ImGui_ImplGlfwGL3_ScrollCallback(GLFWwindow*, double /*xoffset*/, double yoffset)
{

    deltaScroll = (float)yoffset;


    g_MouseWheel += (float)yoffset; // Use fractional mouse wheel, 1.0 unit 5 lines.
}

void ImGui_ImplGlfwGL3_KeyCallback(GLFWwindow*, int key, int, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS)
	io.KeysDown[key] = true;
    if (action == GLFW_RELEASE)
	io.KeysDown[key] = false;

    (void)mods; // Modifiers are not reliable across systems
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
}

void ImGui_ImplGlfwGL3_CharCallback(GLFWwindow*, unsigned int c)
{
    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000)
	io.AddInputCharacter((unsigned short)c);
}

bool ImGui_ImplGlfwGL3_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);

    return true;
}

bool ImGui_ImplGlfwGL3_CreateDeviceObjects()
{
    // Backup GL state
    GLint last_texture, last_array_buffer, last_vertex_array;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

    const GLchar *vertex_shader =
	"#version 330\n"
	"uniform mat4 ProjMtx;\n"
	"in vec2 Position;\n"
	"in vec2 UV;\n"
	"in vec4 Color;\n"
	"out vec2 Frag_UV;\n"
	"out vec4 Frag_Color;\n"
	"void main()\n"
	"{\n"
	"	Frag_UV = UV;\n"
	"	Frag_Color = Color;\n"
	"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
	"}\n";

    const GLchar* fragment_shader =
	"#version 330\n"
	"uniform sampler2D Texture;\n"
	"in vec2 Frag_UV;\n"
	"in vec4 Frag_Color;\n"
	"out vec4 Out_Color;\n"
	"void main()\n"
	"{\n"
	"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
	"}\n";

    g_ShaderHandle = glCreateProgram();
    g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
    g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(g_VertHandle, 1, &vertex_shader, 0);
    glShaderSource(g_FragHandle, 1, &fragment_shader, 0);
    glCompileShader(g_VertHandle);
    glCompileShader(g_FragHandle);
    glAttachShader(g_ShaderHandle, g_VertHandle);
    glAttachShader(g_ShaderHandle, g_FragHandle);
    glLinkProgram(g_ShaderHandle);

    g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
    g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
    g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
    g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
    g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

    glGenBuffers(1, &g_VboHandle);
    glGenBuffers(1, &g_ElementsHandle);

    glGenVertexArrays(1, &g_VaoHandle);
    glBindVertexArray(g_VaoHandle);
    glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
    glEnableVertexAttribArray(g_AttribLocationPosition);
    glEnableVertexAttribArray(g_AttribLocationUV);
    glEnableVertexAttribArray(g_AttribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
    glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
    glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

    ImGui_ImplGlfwGL3_CreateFontsTexture();

    // Restore modified GL state
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindVertexArray(last_vertex_array);

    return true;
}

void    ImGui_ImplGlfwGL3_InvalidateDeviceObjects()
{
    if (g_VaoHandle) glDeleteVertexArrays(1, &g_VaoHandle);
    if (g_VboHandle) glDeleteBuffers(1, &g_VboHandle);
    if (g_ElementsHandle) glDeleteBuffers(1, &g_ElementsHandle);
    g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;

    glDetachShader(g_ShaderHandle, g_VertHandle);
    glDeleteShader(g_VertHandle);
    g_VertHandle = 0;

    glDetachShader(g_ShaderHandle, g_FragHandle);
    glDeleteShader(g_FragHandle);
    g_FragHandle = 0;

    glDeleteProgram(g_ShaderHandle);
    g_ShaderHandle = 0;

    if (g_FontTexture)
    {
	glDeleteTextures(1, &g_FontTexture);
	ImGui::GetIO().Fonts->TexID = 0;
	g_FontTexture = 0;
    }
}

bool    ImGui_ImplGlfwGL3_Init(GLFWwindow* window, bool install_callbacks)
{
    g_Window = window;

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                         // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.RenderDrawListsFn = ImGui_ImplGlfwGL3_RenderDrawLists;       // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    io.SetClipboardTextFn = ImGui_ImplGlfwGL3_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplGlfwGL3_GetClipboardText;
#ifdef _WIN32
    io.ImeWindowHandle = glfwGetWin32Window(g_Window);
#endif

    if (install_callbacks)
    {
	glfwSetMouseButtonCallback(window, ImGui_ImplGlfwGL3_MouseButtonCallback);
	glfwSetScrollCallback(window, ImGui_ImplGlfwGL3_ScrollCallback);
	glfwSetKeyCallback(window, ImGui_ImplGlfwGL3_KeyCallback);
	glfwSetCharCallback(window, ImGui_ImplGlfwGL3_CharCallback);
    }

    return true;
}

void ImGui_ImplGlfwGL3_Shutdown()
{
    ImGui_ImplGlfwGL3_InvalidateDeviceObjects();
    ImGui::Shutdown();
}

void ImGui_ImplGlfwGL3_NewFrame(float guiVerticalScale)
{
    if (!g_FontTexture)
	ImGui_ImplGlfwGL3_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(g_Window, &w, &h);
    glfwGetFramebufferSize(g_Window, &display_w, &display_h);

    float SCALE = guiVerticalScale;
    w *= SCALE;
    display_w *= SCALE;

    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    // Setup time step
    double current_time = glfwGetTime();
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
    g_Time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    if (glfwGetWindowAttrib(g_Window, GLFW_FOCUSED))
    {
	double mouse_x, mouse_y;
	glfwGetCursorPos(g_Window, &mouse_x, &mouse_y);
	io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);   // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
    }
    else
    {
	io.MousePos = ImVec2(-1, -1);
    }

    for (int i = 0; i < 3; i++)
    {
	io.MouseDown[i] = g_MousePressed[i] || glfwGetMouseButton(g_Window, i) != 0;    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
	g_MousePressed[i] = false;
    }

    io.MouseWheel = g_MouseWheel;
    g_MouseWheel = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    glfwSetInputMode(g_Window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

    // Start the frame
    ImGui::NewFrame();
}


using std::string;

Gui::Gui(GLFWwindow* window) {

    m_guiMode = ModifyTerrainMode;
    m_drawTextureType = GrassTexture;
    m_inputMode = InputNoneMode;
    m_axisMode = NoneAxis;
    m_translation = Vector3f(0);
    m_rotation = Vector3f(0);
    m_cursorSize = DEFAULT_RADIUS;
    m_strength = 10;
    m_noiseScale = DEFAULT_NOISE_SCALE;

    m_terrainMode = SmoothMode;

    // init gui:
    if(ImGui_ImplGlfwGL3_Init(window, true)) {
	LOG_I("IMGUI initialization succeeded");
    } else {
	LOG_E("IMGUI initialization failed");
    }
}


Gui::~Gui() {
    ImGui_ImplGlfwGL3_InvalidateDeviceObjects();
    ImGui::Shutdown();
}


void Gui::NewFrame(const float guiVerticalScale) {
    ImGui_ImplGlfwGL3_NewFrame(guiVerticalScale);
}

void Gui::RadiusSlider() {
	int oldCursorSize = m_cursorSize;
	ImGui::SliderInt("Radius", &m_cursorSize, MIN_CURSOR_SIZE, MAX_CURSOR_SIZE);
	if(oldCursorSize != m_cursorSize) {
	    SetCursorSize(m_cursorSize);
	}
}

void Gui::Render(int windowWidth, int windowHeight) {
    static float f = 0.0f;

    ImGuiWindowFlags window_flags = 0;
//    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_ShowBorders;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove;
    //window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_MenuBar;

    float bg_alpha = 1.0f; // <0: default

    bool p_opened = true;

    ImGui::Begin("ImGui Demo", &p_opened,
		 ImVec2(windowWidth,windowHeight), bg_alpha, window_flags);

    ImGui::SetWindowPos(ImVec2(0,0) );

    ImGui::Text("Global mode:");

    ImGui::RadioButton("MT", &m_guiMode, ModifyTerrainMode); ImGui::SameLine();
    ImGui::RadioButton("DT", &m_guiMode, DrawTextureMode);  ImGui::SameLine();
    ImGui::RadioButton("M", &m_guiMode, ModelMode);

    if(m_guiMode == ModifyTerrainMode) {

	ImGui::Text("Local mode:");

	ImGui::RadioButton("Elevation", &m_terrainMode, ModifyElevationMode);
	ImGui::RadioButton("Distort", &m_terrainMode, DistortMode);
	ImGui::RadioButton("Smooth", &m_terrainMode, SmoothMode);

	RadiusSlider();


	ImGui::SliderInt("Strength", &m_strength, 1, 35);

	ImGui::SliderFloat("Noise Scale", &m_noiseScale, MIN_NOISE_SCALE, MAX_NOISE_SCALE);


    } else if(m_guiMode == DrawTextureMode) {

	RadiusSlider();


        ImGui::Combo("Texture", &m_drawTextureType, "Grass\0Dirt\0Rock\0Eraser\0\0");   // Combo using values packed in a single
    } else {

	ImGui::Text("lol:");

	string xs = "x: ";
	xs += std::to_string(m_translation.x);
	ImGui::Text(xs.c_str() );

	string mode = "state: ";
	if(m_inputMode == InputNoneMode) {
	    mode += "none";
	}  else if(m_inputMode == InputTranslateMode) {
	    mode += "translate: ";

	    mode += AxisModeToStr(m_axisMode);
	}  else if(m_inputMode == InputRotateMode) {
	    mode += "rotate: ";

	    mode += AxisModeToStr(m_axisMode);
	}

	ImGui::Text(mode.c_str() );

#if !defined(_WIN32)
	if (ImGui::Button("Add Model")) {

	    nfdchar_t *outPath = NULL;

	    nfdresult_t result = NFD_OpenDialog( "eob", "obj/", &outPath );

	    if ( result == NFD_OKAY ) {

		string path(outPath);

		size_t lastSlashIndex = path.rfind("/");

		size_t secondToLastSlashIndex = path.rfind("/", lastSlashIndex-1);

		string objDir = path.substr(secondToLastSlashIndex+1, lastSlashIndex-secondToLastSlashIndex-1);

		// we only allow files to be loaded from the directory obj/
		if(objDir == "obj" ) {

		    string objPath =
			path.substr(secondToLastSlashIndex+1, string::npos);

		    for(GuiListener* listener : m_listeners) {
			listener->ModelAdded(objPath);
		    }

		} else {
 		    LOG_W("You can only load obj files from obj/");
		}

//		LOG_I("found path: %s",
//		      .c_str() );


	    }
	}
#else

	LOG_W("This feature is not yet supported on windows");
#endif


    }



    ImGui::End();

    ImGui::Render();

}
//             ImGui::LogButtons();


int Gui::GetGuiMode()const {
    return m_guiMode;
}


int Gui::GetDrawTextureType()const {
    return m_drawTextureType;
}


void Gui::ResetModelMode() {

    m_translation = Vector3f(0);
    m_rotation = Vector3f(0);

    m_inputMode = InputNoneMode;
    m_axisMode = NoneAxis;
}

void Gui::Update() {

    KeyboardState& kbs = KeyboardState::GetInstance();
    MouseState& ms = MouseState::GetInstance();


    if(m_guiMode == ModelMode) {

	if(m_inputMode == InputNoneMode) {

	    if( kbs.WasPressed(GLFW_KEY_G) ) {
		m_inputMode = InputTranslateMode;
	    } else if( kbs.WasPressed(GLFW_KEY_R) ) {
		m_inputMode = InputRotateMode;
	    }

	}else if(m_inputMode == InputRotateMode) {

	    if(kbs.IsPressed(GLFW_KEY_BACKSPACE) ) {
		ResetModelMode();

	    } else if(kbs.IsPressed(GLFW_KEY_ENTER) ) {

		for(GuiListener* listener : m_listeners) {
		    listener->RotationAccepted();
		}

		ResetModelMode();

	    } else {

		if(m_axisMode == NoneAxis) {

		    // at this point, you can input which axis you wish to translate for.

		    if( kbs.WasPressed(GLFW_KEY_X) ) {
			m_axisMode = XAxis;
		    } else if( kbs.WasPressed(GLFW_KEY_Y) ) {
			m_axisMode = YAxis;
		    } else if( kbs.WasPressed(GLFW_KEY_Z) ) {
			m_axisMode = ZAxis;
		    }

		} else {
		    // get keyboard number input.

		    float* tr = (float*)&m_rotation;

		    tr[m_axisMode] += ms.GetDeltaX() * 0.01;
		}
	    }



	}else if(m_inputMode == InputTranslateMode) {

	    if(kbs.IsPressed(GLFW_KEY_BACKSPACE) ) {
		ResetModelMode();

	    } else if(kbs.IsPressed(GLFW_KEY_ENTER) ) {

		for(GuiListener* listener : m_listeners) {
		    listener->TranslationAccepted();
		}

		ResetModelMode();

	    } else {

		if(m_axisMode == NoneAxis) {

		    // at this point, you can input which axis you wish to translate for.

		    if( kbs.WasPressed(GLFW_KEY_X) ) {
			m_axisMode = XAxis;
		    } else if( kbs.WasPressed(GLFW_KEY_Y) ) {
			m_axisMode = YAxis;
		    } else if( kbs.WasPressed(GLFW_KEY_Z) ) {
			m_axisMode = ZAxis;
		    }

		} else {
		    // get keyboard number input.

		    float* tr = (float*)&m_translation;

		    tr[m_axisMode] += ms.GetDeltaX() * 0.1;
		}
	    }
	}
    } else if(m_guiMode == ModifyTerrainMode) {

	float diff = deltaScroll;

	SetCursorSize( GetCursorSize() - 1.5 * diff  );

	deltaScroll = 0;

    }

}

Vector3f Gui::GetTranslation()const {
    return m_translation;
}

Vector3f Gui::GetRotation()const {
    return m_rotation;
}

void Gui::AddListener(GuiListener* listener) {
    m_listeners.push_back(listener);
}


int Gui::GetCursorSize()const {
    return m_cursorSize;
}


void Gui::SetCursorSize(int cursorSize) {

    if(cursorSize < MIN_CURSOR_SIZE) {
	cursorSize = MIN_CURSOR_SIZE;
    }

    if(cursorSize > MAX_CURSOR_SIZE) {
	cursorSize = MAX_CURSOR_SIZE;
    }

    m_cursorSize = cursorSize;

    for(GuiListener* listener : m_listeners) {
	listener->CursorSizeChanged();
    }

}

float Gui::GetStrength()const {
    return 1.0f / (40.0 - m_strength);
}

int Gui::GetTerrainMode()const {
    return m_terrainMode;
}


float Gui::GetNoiseScale()const {
    return m_noiseScale;
}
