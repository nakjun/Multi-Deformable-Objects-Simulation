#include "glsl.h"
#include "render.h"
#include "DeformableObject.h"
#include <chrono>

using namespace chrono;
CRender *render;
CDeformable *obj;
float width, height;
int vertex_count , face_count , spring_count;
system_clock::time_point t1;
system_clock::time_point t2;
double summation=0.0;
double update = 0.0;
double collision = 0.0;
double rendering = 0.0;
bool flag;
void init()
{
	flag = true;
	width = 1024; height = 600;
	render = new CRender();
	render->init(width, height);
	render->ObjectCount = 0;
	render->VerticesCount = 0;
	render->FaceCount = 0;
	render->spring_count = 0;
	render->BBFaceCount = 0;
	render->vCountList.clear();
	render->sCountList.clear();	
		
	int v_cnt;	
	int ttt = 0;	
	int max = 0;		
		
	for (int i = 0; i < 2; i++)
	{
		obj = new CDeformable(1);
		v_cnt = (*obj->curMSS)->GetNumParticles();
		if (max < obj->maxFaceListSize)
		{
			max = obj->maxFaceListSize;
		}

		render->nodefacemaxSize = max;

		printf("max is %d\n", render->nodefacemaxSize);

		obj->SetBoundingBox(render->VerticesCount);
		obj->setFaceList();
		render->mDefList.push_back(obj);
		ttt = obj->sum;
		render->BBFaceCount += ttt;
		printf("%d\n", render->BBFaceCount);
		render->VerticesCount += v_cnt;
		render->spring_count += obj->mSrpingSystem->mNumSprings;
		render->ObjectCount++;
		render->vCountList.push_back(v_cnt);
		render->sCountList.push_back(obj->mSrpingSystem->mNumSprings);
	}
}

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		render->cam->x++;
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		render->cam->x--;
	}
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		render->cam->y++;
	}
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		render->cam->y--;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		render->cam->z++;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		render->cam->z--;
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{		
		if (flag == true) flag = false;
		else flag = true;
	}
	printf("x : %d y : %d z : %d\n", render->cam->x, render->cam->y, render->cam->z);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{

}
static void resize_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

static void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	std::string msgSource;
	switch (source){
	case GL_DEBUG_SOURCE_API:
		msgSource = "WINDOW_SYSTEM";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		msgSource = "SHADER_COMPILER";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		msgSource = "THIRD_PARTY";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		msgSource = "APPLICATION";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		msgSource = "OTHER";
		break;
	}

	std::string msgType;
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		msgType = "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		msgType = "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		msgType = "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		msgType = "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		msgType = "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		msgType = "OTHER";
		break;
	}

	std::string msgSeverity;
	switch (severity){
	case GL_DEBUG_SEVERITY_LOW:
		msgSeverity = "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		msgSeverity = "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		msgSeverity = "HIGH";
		break;
	}

	printf("glDebugMessage:\n%s \n type = %s source = %s severity = %s\n", message, msgType.c_str(), msgSource.c_str(), msgSeverity.c_str());
}

int main(void)
{
	//Set the error callback
	int counttt = 0;
	glfwSetErrorCallback(error_callback);

	//Initialize GLFW
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	//Request an OpenGL 4.3 core context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Declare a window object
	GLFWwindow* window;

	//Create a window and create its OpenGL context
	//window = glfwCreateWindow(1920, 1200, "OpenGL Compute Shader Particle System", glfwGetPrimaryMonitor(), NULL);
	window = glfwCreateWindow(1024, 600, "OpenGL Compute Shader Particle System", NULL, NULL);
	//window = glfwCreateWindow(1920, 1200, "OpenGL Compute Shader Particle System", NULL, NULL);

	//If the window couldn't be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//This function makes the context of the specified window current on the calling thread. 
	glfwMakeContextCurrent(window);

	//Set callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	//If GLEW hasn't initialized
	if (err != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}

	glEnable(GL_DEBUG_OUTPUT);

	//Set a background color

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	double lastFPStime = glfwGetTime();
	int frameCounter = 0;
		
	//printf("Deformable Load");

	init(); // Object Loading

	render->Scenewindow = window;

	resize_callback(render->Scenewindow, 1024, 600);
	render->setFacePair();
	render->setLineArray();
	render->SetFaceList();	   // Surface Mesh Initializing
	
	render->generateShaders(); // Shader Compile
	render->generateBuffers(); // Buffer Initializing
	
	update = 0.0;
	collision = 0.0;
	summation = 0.0;
	//Main Loop
	do
	{	
		if (flag)
		{
			render->invoke_compute_shader();
			render->invoke_updateBB_shader();
			render->invoke_collisionBB_shader();
			//render->invoke_collisionHandling_shader();
		}
		render->render();		
	} //Check if the ESC key had been pressed or if the window had been closed
	
	while (!glfwWindowShouldClose(window));		
	//Close OpenGL window and terminate GLFW
	glfwDestroyWindow(window);
	//Finalize and clean up GLFW
	glfwTerminate();

	exit(EXIT_SUCCESS);
}