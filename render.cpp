#pragma once
#pragma warning(disable:4996)
#include "render.h"
#include "DeformableObject.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include "Ocpart.h"
using namespace std;

#define BBCOUNT 64

vec3 lightPos = vec3(0.0f, 15.0f, 1.0f);
vec3 external_Force = vec3(9.8f, 0.0f, 0.0f);
int faceCount2;
int currentCNT = 0;
struct vertex4f {
	GLfloat x, y, z;
	GLfloat index;
};
struct mface
{
	vertex4f faceindex;
	vertex4f nodefaceresponseindex;
};
struct spring
{
	vertex4f springInformation;	// 
	vertex4f SSBOIndex;			// 
};
struct Result {
	int res;
};
struct BBFace
{
	int faceindex;
	int bbindex;
};
struct FacePair {
	int x, y, z;
};
struct indices{
	GLint x, y, z;
}; 
struct boundingBox{
	vertex4f min;
	vertex4f max;
	vertex4f data;	
};
struct TwoFacesPair{
	GLint x,y,z;
};

float framenum = 0;

vector<int> FACEARRAY;
vector<int> FACEARRAY2;
vector<ivec2> LINEARRAY;
int facearray_count = 0;

string readFile(const char *fileName)
{
	string fileContent;
	ifstream fileStream(fileName, ios::in);
	if (!fileStream.is_open()) {
		printf("File %s not found\n", fileName);
		return "";
	}
	string line = "";
	while (!fileStream.eof()) {
		getline(fileStream, line);
		fileContent.append(line + "\n");
	}
	fileStream.close();
	return fileContent;
}

CRender::CRender()
{
	mDefList.clear();
	printf("--------------------------\n");
	printf("Deformable Obj List Clear\n\n");

	isRender = false;
	vCountList.clear();
	VerticesCount = 0;
	cam = new Camera();
	fcnt = 0;
}

CRender::~CRender()
{
	printf("%d\n", fcnt);
	
}
void CRender::init(float width, float height)
{
	this->Width = width;
	this->Height = height;
}

void CRender::generateMVPMatrix()
{
	cam->computeMatricesFromInputs();
	Projection = cam->getProjectionMatrix();
	View = cam->getViewMatrix();
	
	MatrixID = glGetUniformLocation(render_program_handle, "model");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Model[0][0]);

	MatrixID = glGetUniformLocation(render_program_handle, "view");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &View[0][0]);

	MatrixID = glGetUniformLocation(render_program_handle, "projection");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Projection[0][0]);
}

// CALL UPDATE BOUNDING BOX COMPUTE SHADER
void CRender::invoke_updateBB_shader(){
	
	// BB_program_handle[1] : BBUpdate.cshader	
	
	glUseProgram(BB_program_handle[1]);

	uniform_loc = glGetUniformLocation(BB_program_handle[1], "ObjectCount");
	workingGroups = BBCOUNT * ObjectCount / 32;
	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, BBCOUNT);
	}
	uniform_loc = glGetUniformLocation(BB_program_handle[1], "FaceListOffset");

	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, mDefList.at(0)->sum);
	}
	uniform_loc = glGetUniformLocation(BB_program_handle[1], "FaceOffset");

	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, fCountList.at(0));
	}
	glDispatchCompute(workingGroups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Memory Barrier : Thread Merge
}

// CALL BOUNDING BOX COLLISION and FACE-FACE INTERSECTION COMPUTE SHADER
void CRender::invoke_collisionBB_shader()
{
	// BB_program_handle[0] : BBCollision.cshader
	// BB_program_handle[2] : FaceFaceIntersection.cshader
	
	glUseProgram(BB_program_handle[0]);
	
	uniform_loc = glGetUniformLocation(BB_program_handle[0], "FaceOffset");

	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, fCountList.at(0));
	}
	
	workingGroups = BBCOUNT * ObjectCount / 32;

	glDispatchCompute(workingGroups, workingGroups, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Memory Barrier : Thread Merge
	
	// Face-Face Intersection : N' * M' Invoke
	glUseProgram(BB_program_handle[2]);
	uniform_loc = glGetUniformLocation(BB_program_handle[2], "Offset");

	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, mDefList.at(0)->sum);
	}
	uniform_loc = glGetUniformLocation(BB_program_handle[2], "FaceOffset");

	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, fCountList.at(0));
	}
	uniform_loc = glGetUniformLocation(BB_program_handle[2], "obj2facecount");

	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, faceCount2);
	}

	int workingGroups = mDefList.at(0)->sum / 32;
	int workingGroups2 = mDefList.at(1)->sum / 32;
	glDispatchCompute(workingGroups + 1, workingGroups2 + 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Memory Barrier : Thread Merge	
}

void CRender::invoke_collisionHandling_shader()
{	
	// Calculate Reflecting Velocity : N' * M' Invoke
	glUseProgram(CH_program_handle[1]);
	uniform_loc = glGetUniformLocation(CH_program_handle[1], "Offset");

	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, mDefList.at(0)->sum);
	}
	uniform_loc = glGetUniformLocation(CH_program_handle[1], "FaceOffset");

	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, fCountList.at(0));
	}
	uniform_loc = glGetUniformLocation(CH_program_handle[1], "obj2facecount");

	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, faceCount2);
	}
	int workingGroups = mDefList.at(0)->sum / 32;
	int workingGroups2 = mDefList.at(1)->sum / 32;
	glDispatchCompute(workingGroups + 1, workingGroups2 + 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Memory Barrier : Thread Merge

	// Back Positioning
	glUseProgram(CH_program_handle[0]);
	uniform_loc = glGetUniformLocation(CH_program_handle[0], "VertexCount");
	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, VerticesCount);
	}

	uniform_loc = glGetUniformLocation(CH_program_handle[0], "ObjectCount");
	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, ObjectCount);
	}

	workingGroups = VerticesCount / 512;
	glDispatchCompute(workingGroups + 1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Memory Barrier : Thread Merge
}


void CRender::invoke_compute_shader(){
	//Spring Force Update by Hook's Raw
	glUseProgram(compute_program_handle[1]);

	workingGroups = spring_count / 512;
	uniform_loc = glGetUniformLocation(compute_program_handle[1], "VertexCount");
	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, VerticesCount);
	}
	uniform_loc = glGetUniformLocation(compute_program_handle[1], "SpringCount");
	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, spring_count);
	}
	glDispatchCompute(workingGroups + 1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Memory Barrier : Thread Merge

	//Node Force Summation
	glUseProgram(compute_program_handle[2]);
	uniform_loc = glGetUniformLocation(compute_program_handle[2], "VertexCount");
	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, VerticesCount);
	}

	uniform_loc = glGetUniformLocation(compute_program_handle[2], "ObjectCount");
	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, ObjectCount);
	}

	workingGroups = VerticesCount / 512;
	glDispatchCompute(workingGroups + 1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Memory Barrier : Thread Merge

	//Node Shader Use and Node Pos Update(Using Numerical Integration)
	glUseProgram(compute_program_handle[0]);
	uniform_loc = glGetUniformLocation(compute_program_handle[0], "VertexCount");
	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, VerticesCount);
	}

	uniform_loc = glGetUniformLocation(compute_program_handle[0], "ObjectCount");
	if (uniform_loc != unsigned int(-1))
	{
		glUniform1i(uniform_loc, ObjectCount);
	}


	uniform_loc = glGetUniformLocation(compute_program_handle[0], "extForce");
	if (uniform_loc != unsigned int(-1))
	{
		if (currentCNT>400)
			glUniform1f(uniform_loc, 1.0f);
		else
			glUniform1f(uniform_loc, 0.0f);
	}


	workingGroups = VerticesCount / 512;
	glDispatchCompute(workingGroups + 1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Memory Barrier : Thread Merge

	currentCNT++;
}

void CRender::render()
{
	//glClear(GL_COLOR_BUFFER_BIT);
	
	//glEnable(GL_DEPTH_TEST);
	glClearColor(0.1, 0.1, 0.1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(render_program_handle);
	generateMVPMatrix();	
	//glDrawElements(GL_TRIANGLES, FACEARRAY.size(), GL_UNSIGNED_INT, &FACEARRAY[0]);
	glDrawElements(GL_LINES, LINEARRAY.size() * 2, GL_UNSIGNED_INT, &LINEARRAY[0]);

	glfwSwapBuffers(Scenewindow);
	fcnt++;

	glfwPollEvents();
}

GLuint CRender::compileShader(std::string path_to_file, GLenum shader_type)
{
	GLuint shader_handle = 0;
	std::ifstream shader_file(path_to_file, std::ios::ate | std::ios::binary);
	if (!shader_file)
	{
		throw std::runtime_error("shader file load error");
	}
	size_t shader_file_size = static_cast<size_t>(shader_file.tellg());
	std::vector<char> shader_code(shader_file_size);
	shader_file.seekg(0);
	shader_file.read(shader_code.data(), shader_file_size);
	shader_file.close();
	//must be nul terminated
	shader_code.push_back('\0');

	char* shader_code_ptr = shader_code.data();
	shader_handle = glCreateShader(shader_type);
	glShaderSource(shader_handle, 1, &shader_code_ptr, NULL);
	glCompileShader(shader_handle);

	int32_t is_compiled = 0;
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &is_compiled);
	if (is_compiled == GL_FALSE)
	{
		int32_t len = 0;
		glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &len);
		std::vector<GLchar> log(len);
		glGetShaderInfoLog(shader_handle, len, &len, &log[0]);
		for (auto c : log)
		{
			std::cout << c;
		}
		throw std::runtime_error("shader compile error");
	}
	return shader_handle;
}

void CRender::printProgramLog(GLuint shader_program_handle){
	int32_t is_linked = 0;
	glGetProgramiv(shader_program_handle, GL_LINK_STATUS, &is_linked);
	if (is_linked == GL_FALSE)
	{
		int32_t len = 0;
		glGetProgramiv(shader_program_handle, GL_INFO_LOG_LENGTH, &len);
		std::vector<GLchar> log(len);
		glGetProgramInfoLog(shader_program_handle, len, &len, &log[0]);

		for (auto c : log)
		{
			std::cout << c;
		}
		throw std::runtime_error("shader link error");
	}
}
void CRender::printShaderLog(GLuint program){

}

void CRender::generateShaders(){
	uint32_t vertex_shader_handle = compileShader("vertex.shader", GL_VERTEX_SHADER);
	uint32_t fragment_shader_handle = compileShader("fragment.shader", GL_FRAGMENT_SHADER);

	render_program_handle = glCreateProgram();
	glAttachShader(render_program_handle, fragment_shader_handle);
	glAttachShader(render_program_handle, vertex_shader_handle);
	glLinkProgram(render_program_handle);
	printProgramLog(render_program_handle);

	glDeleteShader(vertex_shader_handle);
	glDeleteShader(fragment_shader_handle);

	compute_shader_handle = compileShader("node.cshader", GL_COMPUTE_SHADER);
	compute_program_handle[0] = glCreateProgram();
	glAttachShader(compute_program_handle[0], compute_shader_handle);
	glLinkProgram(compute_program_handle[0]);
	printProgramLog(compute_program_handle[0]);
	glDeleteShader(compute_shader_handle);

	compute_shader_handle = compileShader("spring.cshader", GL_COMPUTE_SHADER);
	compute_program_handle[1] = glCreateProgram();
	glAttachShader(compute_program_handle[1], compute_shader_handle);
	glLinkProgram(compute_program_handle[1]);
	printProgramLog(compute_program_handle[1]);
	glDeleteShader(compute_shader_handle);

	compute_shader_handle = compileShader("NodeForce.cshader", GL_COMPUTE_SHADER);
	compute_program_handle[2] = glCreateProgram();
	glAttachShader(compute_program_handle[2], compute_shader_handle);
	glLinkProgram(compute_program_handle[2]);
	printProgramLog(compute_program_handle[2]);
	glDeleteShader(compute_shader_handle);

	//BoundingBox Collision CS
	BB_compute_shader_handle = compileShader("BBCollision.cshader", GL_COMPUTE_SHADER);
	BB_program_handle[0] = glCreateProgram();
	glAttachShader(BB_program_handle[0], BB_compute_shader_handle);
	glLinkProgram(BB_program_handle[0]);
	printProgramLog(BB_program_handle[0]);
	glDeleteShader(BB_compute_shader_handle);

	//BoundingBox Update CS
	BB_compute_shader_handle = compileShader("BBUpdate.cshader", GL_COMPUTE_SHADER);
	BB_program_handle[1] = glCreateProgram();
	glAttachShader(BB_program_handle[1], BB_compute_shader_handle);
	glLinkProgram(BB_program_handle[1]);
	printProgramLog(BB_program_handle[1]);
	glDeleteShader(BB_compute_shader_handle);

	//Face-Face Intersection CS
	BB_compute_shader_handle = compileShader("FaceFaceIntersection.cshader", GL_COMPUTE_SHADER);
	BB_program_handle[2] = glCreateProgram();
	glAttachShader(BB_program_handle[2], BB_compute_shader_handle);
	glLinkProgram(BB_program_handle[2]);
	printProgramLog(BB_program_handle[2]);
	glDeleteShader(BB_compute_shader_handle);

	//Collision Handling CS : BackPositioning
	CH_compute_shader_handle = compileShader("BackPosition.cshader", GL_COMPUTE_SHADER);
	CH_program_handle[0] = glCreateProgram();
	glAttachShader(CH_program_handle[0], CH_compute_shader_handle);
	glLinkProgram(CH_program_handle[0]);
	printProgramLog(CH_program_handle[0]);
	glDeleteShader(CH_compute_shader_handle);

	//Collision Handling CS : Calculate Reflect Velocity
	CH_compute_shader_handle = compileShader("CollisionResponse.cshader", GL_COMPUTE_SHADER);
	CH_program_handle[1] = glCreateProgram();
	glAttachShader(CH_program_handle[1], CH_compute_shader_handle);
	glLinkProgram(CH_program_handle[1]);
	printProgramLog(CH_program_handle[1]);
	glDeleteShader(CH_compute_shader_handle);

	//Collision Handling CS : Summation Reflect Velocity
	CH_compute_shader_handle = compileShader("ReflectVelocity.cshader", GL_COMPUTE_SHADER);
	CH_program_handle[2] = glCreateProgram();
	glAttachShader(CH_program_handle[2], CH_compute_shader_handle);
	glLinkProgram(CH_program_handle[2]);
	printProgramLog(CH_program_handle[2]);
	glDeleteShader(CH_compute_shader_handle);
}
void CRender::generateBuffers(){
	// 0 : Node Position, 1 : Node Velocity, 2 : Spring Information, 3 : Node Force, 4 : BBInformation, 5 : BBResult, 6 : Normal, 7 : Face Information

	//Node Position Generate [0]
	if (glIsBuffer(SSBOPos)) {
		glDeleteBuffers(1, &SSBOPos);
	};
	glGenBuffers(1, &SSBOPos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOPos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, VerticesCount * sizeof(vertex4f), NULL, GL_STATIC_DRAW);
	resetPositionSSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBOPos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//Node Velocity Generate [1]
	if (glIsBuffer(SSBOVel)) {
		glDeleteBuffers(1, &SSBOVel);
	};
	glGenBuffers(1, &SSBOVel);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOVel);
	glBufferData(GL_SHADER_STORAGE_BUFFER, VerticesCount * sizeof(vertex4f), NULL, GL_STATIC_DRAW);
	resetVelocitySSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBOVel);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//Spring Information Generate [2]
	if (glIsBuffer(SSBOSpring)) {
		glDeleteBuffers(1, &SSBOSpring);
	};
	glGenBuffers(1, &SSBOSpring);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOSpring);
	glBufferData(GL_SHADER_STORAGE_BUFFER, spring_count * sizeof(spring), NULL, GL_STATIC_DRAW);
	resetSpringSSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, SSBOSpring);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//Node Force Summation Generate [3]
	if (glIsBuffer(SSBONodeForceMap)) {
		glDeleteBuffers(1, &SSBONodeForceMap);
	};
	glGenBuffers(1, &SSBONodeForceMap);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBONodeForceMap);
	glBufferData(GL_SHADER_STORAGE_BUFFER, VerticesCount * 30 * sizeof(vertex4f), NULL, GL_STATIC_DRAW);
	resetNodeForceSummationSSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, SSBONodeForceMap);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	// Bounding Box Inforamtion Generate [4]
	if (glIsBuffer(SSBOBoundingBoxMin)) {
		glDeleteBuffers(1, &SSBOBoundingBoxMin);
	};
	glGenBuffers(1, &SSBOBoundingBoxMin);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOBoundingBoxMin);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ObjectCount * BBCOUNT * sizeof(boundingBox), NULL, GL_STATIC_DRAW);
	resetBoundingBoxSSBO1();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SSBOBoundingBoxMin);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Bounding Box Inforamtion Generate [5]
	if (glIsBuffer(SSBOBoundingBoxResult)) {
		glDeleteBuffers(1, &SSBOBoundingBoxResult);
	};
	glGenBuffers(1, &SSBOBoundingBoxResult);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOBoundingBoxResult);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 64 * 64 * sizeof(vertex4f), NULL, GL_STATIC_DRAW);
	resetBoundingBoxResultSSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBOBoundingBoxResult);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Face Normal Generate [6]
	if (glIsBuffer(SSBONormal)) {
		glDeleteBuffers(1, &SSBONormal);
	};
	glGenBuffers(1, &SSBONormal);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBONormal);
	glBufferData(GL_SHADER_STORAGE_BUFFER, VerticesCount * sizeof(vertex4f), NULL, GL_STATIC_DRAW);
	resetNormalSSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, SSBONormal);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	// Face Information Generate [7]
	if (glIsBuffer(SSBOFace)) {
		glDeleteBuffers(1, &SSBOFace);
	};
	glGenBuffers(1, &SSBOFace);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOFace);
	glBufferData(GL_SHADER_STORAGE_BUFFER, FaceCount * sizeof(mface), NULL, GL_STATIC_DRAW);
	resetFaceSSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, SSBOFace);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Object Information Generate [8]
	if (glIsBuffer(SSBOObjectInformation)) {
		glDeleteBuffers(1, &SSBOObjectInformation);
	};
	glGenBuffers(1, &SSBOObjectInformation);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOObjectInformation);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ObjectCount * sizeof(vertex4f), NULL, GL_STATIC_DRAW);
	resetObjectInformationSSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, SSBOObjectInformation);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// SSBO FaceList Generate [9]
	if (glIsBuffer(SSBOFaceList)) {
		glDeleteBuffers(1, &SSBOFaceList);
	};
	glGenBuffers(1, &SSBOFaceList);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOFaceList);	
	glBufferData(GL_SHADER_STORAGE_BUFFER, BBFaceCount * sizeof(vertex4f), NULL, GL_STATIC_DRAW);
	resetFaceListSSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, SSBOFaceList);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (glIsBuffer(SSBOFacePairWise)) {
		glDeleteBuffers(1, &SSBOFacePairWise);
	};
	glGenBuffers(1, &SSBOFacePairWise);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOFacePairWise);
	glBufferData(GL_SHADER_STORAGE_BUFFER, facePair->size() * sizeof(vertex4f), NULL, GL_STATIC_DRAW);
	resetFacePairWise();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, SSBOFacePairWise);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (glIsBuffer(SSBOMaskBoundingBox)) {
		glDeleteBuffers(1, &SSBOMaskBoundingBox);
	};
	glGenBuffers(1, &SSBOMaskBoundingBox);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOMaskBoundingBox);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 64 * 64 * ObjectCount * ObjectCount * sizeof(int), NULL, GL_STATIC_DRAW);
	resetMaskSSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, SSBOMaskBoundingBox);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (glIsBuffer(SSBOPrevPos)) {
		glDeleteBuffers(1, &SSBOPrevPos);
	};
	glGenBuffers(1, &SSBOPrevPos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOPrevPos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, VerticesCount * sizeof(vertex4f), NULL, GL_STATIC_DRAW);
	resetPrevPositionSSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, SSBOPrevPos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (glIsBuffer(SSBOPrevVel)) {
		glDeleteBuffers(1, &SSBOPrevVel);
	};
	glGenBuffers(1, &SSBOPrevVel);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOPrevVel);
	glBufferData(GL_SHADER_STORAGE_BUFFER, VerticesCount * sizeof(vertex4f), NULL, GL_STATIC_DRAW);
	resetPrevVelocitySSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, SSBOPrevVel);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (glIsBuffer(SSBONodeFaceResponseTable)) {
		glDeleteBuffers(1, &SSBONodeFaceResponseTable);
	};
	glGenBuffers(1, &SSBONodeFaceResponseTable);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBONodeFaceResponseTable);
	glBufferData(GL_SHADER_STORAGE_BUFFER, VerticesCount * nodefacemaxSize * sizeof(vertex4f), NULL, GL_STATIC_DRAW);
	resetNodeFaceResponseForceSSBO();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, SSBONodeFaceResponseTable);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	// Allocation Node Position and Normal Buffer to VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, SSBOPos);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, SSBONormal);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

}

void CRender::resetNodeFaceResponseForceSSBO()
{
	struct vertex4f* NodeFaceResponse = (struct vertex4f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, VerticesCount * nodefacemaxSize * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < VerticesCount * nodefacemaxSize; i++)
	{
		NodeFaceResponse[i].x = 0.0;
		NodeFaceResponse[i].y = 0.0;
		NodeFaceResponse[i].z = 0.0;
	 	NodeFaceResponse[i].index = 0.0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}



void CRender::resetPrevPositionSSBO()
{
	struct vertex4f* PrevPos = (struct vertex4f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, VerticesCount * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for (int i = 0; i < VerticesCount; i++)
	{
		PrevPos[i].x = 0.0;
		PrevPos[i].y = 0.0;
		PrevPos[i].z = 0.0;
		PrevPos[i].index = 0.0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void CRender::resetParticleNormalSSBO(){
	struct vertex4f* ParticleNormal = (struct vertex4f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, VerticesCount * FaceCount * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for (int i = 0; i < FaceCount*VerticesCount; i++)
	{
			ParticleNormal[i].x = 0.0;
			ParticleNormal[i].y = 0.0;
			ParticleNormal[i].z = 0.0;
			ParticleNormal[i].index = 0.0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}
void CRender::resetObjectInformationSSBO()
{
	struct vertex4f* ObjectInformation = (struct vertex4f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ObjectCount * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	int offset = 0;
	for (int i = 0; i < ObjectCount; i++)
	{	
		ObjectInformation[i].x = i;
		ObjectInformation[i].y = mDefList.at(i)->mSrpingSystem->GetNumParticles();
		ObjectInformation[i].z = mDefList.at(i)->mSrpingSystem->mFaceArray.size();
		ObjectInformation[i].index = offset;

		offset += mDefList.at(i)->mSrpingSystem->GetNumParticles();
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}
void CRender::resetNormalSSBO(){
	struct vertex4f* VertexForece = (struct vertex4f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, VerticesCount * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for (int i = 0; i < VerticesCount; i++)
	{
		VertexForece[i].x = 1.0;
		VertexForece[i].y = 1.0;
		VertexForece[i].z = 1.0;
		VertexForece[i].index = 0.0;
	}
	//printf("Size of Vertex Normal is : [%d]", VerticesCount);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}
void CRender::resetPositionSSBO(){
	//printf("Start Setting PositionSSBO\n");
	//Offset 지정 SSBO 매핑 기법
	CParticle *particle_temp;
	CDeformable *temp;
	int Offset = 0;
	struct vertex4f* VertexPos;
	for (int n = 0; n < ObjectCount; n++){
		printf("vCountList[%d]=%d\n", n, vCountList.at(n));
		VertexPos = (struct vertex4f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, Offset, vCountList.at(n) * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		Offset += vCountList.at(n) * sizeof(vertex4f);

		temp = mDefList.at(n);
		int new_VerticesCount = (*temp->curMSS)->GetNumParticles();
		for (int i = 0; i < new_VerticesCount; i++)
		{
			particle_temp = temp->mSrpingSystem->GetParticle(i);
			VertexPos[i].x = particle_temp->mPos.x;
			VertexPos[i].y = particle_temp->mPos.y;
			VertexPos[i].z = particle_temp->mPos.z;
			VertexPos[i].index = (float)n;
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
}

void CRender::resetBoundingBoxSSBO1(){

	struct boundingBox* ObjectBB = (struct boundingBox*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
		ObjectCount * BBCOUNT * sizeof(boundingBox), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	int currentCount = 0;
	CDeformable *temp;
	int curr = 0;
	for (int i = 0; i < ObjectCount; i++){

		//First : Level 1 AABB
		temp = mDefList.at(i);		
		
		for (int j = 0; j < 8; j++)
		{
			Cnode *l1 = temp->Octree->root->children[j];
			for (int n = 0; n < 8; n++)
			{
				Cnode *l2 = l1->children[n];
				ObjectBB[curr].min.x = l2->MinX;
				ObjectBB[curr].min.y = l2->MinY;
				ObjectBB[curr].min.z = l2->MinZ;
				ObjectBB[curr].min.index = 0.0;

				ObjectBB[curr].max.x = l2->MaxX;
				ObjectBB[curr].max.y = l2->MaxY;
				ObjectBB[curr].max.z = l2->MaxZ;
				ObjectBB[curr].max.index = 0.0;

				ObjectBB[curr].data.x = (float)i;
				ObjectBB[curr].data.y = 0.0;
				ObjectBB[curr].data.z = l2->offset;
				ObjectBB[curr].data.index = l2->facelist2->size();

				curr++;
			}			
		}
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	printf("curr %d\n", curr);
	//check();
}

void CRender::resetNodeForceSummationSSBO(){
	//printf("start PARTICLE force \n");

	struct vertex4f* VertexForece = (struct vertex4f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, VerticesCount * 30 * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for (int i = 0; i < VerticesCount * 30; i++)
	{
		VertexForece[i].x = 0.0;
		VertexForece[i].y = 0.0;
		VertexForece[i].z = 0.0;
		VertexForece[i].index = 1.0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void CRender::resetPrevVelocitySSBO(){
	//printf("Start Setting VelocitySSBO\n");

	CParticle *particle_temp;
	CDeformable *temp;
	int Offset = 0;
	struct vertex4f* VertexVel;
	for (int n = 0; n < ObjectCount; n++){
		VertexVel = (struct vertex4f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, Offset, vCountList.at(n) * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		Offset += vCountList.at(n) * sizeof(vertex4f);

		temp = mDefList.at(n);
		int new_VerticesCount = (*temp->curMSS)->GetNumParticles();
		for (int i = 0; i < new_VerticesCount; i++)
		{
			particle_temp = temp->mSrpingSystem->GetParticle(i);
			VertexVel[i].x = 0.0;
			VertexVel[i].y = 0.0;
			VertexVel[i].z = 0.0;
			VertexVel[i].index = (float)n;
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
}

void CRender::resetVelocitySSBO(){
	//printf("Start Setting VelocitySSBO\n");

	CParticle *particle_temp;
	CDeformable *temp;
	int Offset = 0;
	struct vertex4f* VertexVel;
	for (int n = 0; n < ObjectCount; n++){
		VertexVel = (struct vertex4f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, Offset, vCountList.at(n) * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		Offset += vCountList.at(n) * sizeof(vertex4f);

		temp = mDefList.at(n);
		int new_VerticesCount = (*temp->curMSS)->GetNumParticles();
		for (int i = 0; i < new_VerticesCount; i++)
		{
			particle_temp = temp->mSrpingSystem->GetParticle(i);
			VertexVel[i].x = 0.0;
			VertexVel[i].y = 0.0;
			VertexVel[i].z = 0.0;
			VertexVel[i].index = (float)n;
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
}
void CRender::resetSpringSSBO(){

	CDeformable *temp;
	CMassSpringSystem *mTemp;
	CSpring *springTemp;
	int Offset = 0;

	
	int previous = 0;
	struct spring* SpringIndex;
	for (int n = 0; n < ObjectCount; n++){
		SpringIndex = (struct spring*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, Offset, sCountList.at(n) * sizeof(spring), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		Offset += sCountList.at(n) * sizeof(spring);
		
		temp = mDefList.at(n);
		mTemp = (*temp->curMSS);
		
		int new_SpringCount = mTemp->GetNumSpring();
		for (int i = 0; i < new_SpringCount; i++)
		{
			springTemp = mTemp->GetSpring(i);
			SpringIndex[i].springInformation.x = (float)springTemp->mIndex1 + previous;
			SpringIndex[i].springInformation.y = (float)springTemp->mIndex2 + previous;
			SpringIndex[i].springInformation.z = springTemp->mRestLen;
			SpringIndex[i].springInformation.index = 0.0;

			SpringIndex[i].SSBOIndex.x = (float)springTemp->forceindex1 + (previous * 30);
			SpringIndex[i].SSBOIndex.y = (float)springTemp->forceindex2 + (previous * 30);
			SpringIndex[i].SSBOIndex.z = 0.0;
			SpringIndex[i].springInformation.index = 0.0;

			//fprintf(fp2, "[%d] %f %f %f\n", i, SpringIndex[i].x, SpringIndex[i].y, SpringIndex[i].z);
		}
		previous += vCountList.at(n);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}	
}

void CRender::resetSpringForceSSBO(){
	
	CDeformable *temp;
	CMassSpringSystem *mTemp;
	CSpring *spring;
	int Offset = 0;
	struct vertex4f* SpringForce;
	for (int n = 0; n < ObjectCount; n++){
		SpringForce = (struct vertex4f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, Offset, sCountList.at(n) * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		Offset += sCountList.at(n) * sizeof(vertex4f);

		temp = mDefList.at(n);
		mTemp = (*temp->curMSS);
		for (int i = 0; i < sCountList.at(n); i++)
		{
			SpringForce[i].x = 0.0;
			SpringForce[i].y = 0.0;
			SpringForce[i].z = 0.0;
			SpringForce[i].index = n;
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
}
void CRender::resetFaceSSBO(){	
	struct mface* faceBuff;
	int curr = 0;
	//int offset = 0;
	int Offset = 0;
	CDeformable *temp;
	CMassSpringSystem *mTemp;

	for (int n = 0; n < ObjectCount; n++){
		faceBuff = (struct mface*) (struct mface*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, Offset, fCountList.at(n) * sizeof(mface), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		Offset += fCountList.at(n) * sizeof(mface);

		temp = mDefList.at(n);
		mTemp = (*temp->curMSS);
		for (int i = 0; i < fCountList.at(n); i++)
		{
			faceBuff[i].faceindex.x = mTemp->mFaceArray.at(i).x;
			faceBuff[i].faceindex.y = mTemp->mFaceArray.at(i).y;
			faceBuff[i].faceindex.z = mTemp->mFaceArray.at(i).z;
			faceBuff[i].faceindex.index = (float)n;

			faceBuff[i].nodefaceresponseindex.x = mTemp->mFaceArray2.at(i).x;
			faceBuff[i].nodefaceresponseindex.y = mTemp->mFaceArray2.at(i).y;
			faceBuff[i].nodefaceresponseindex.z = mTemp->mFaceArray2.at(i).z;
			faceBuff[i].nodefaceresponseindex.index = (float)n;
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}


	/*for (int n = 0; n < mDefList.size(); n++){
		temp = mDefList.at(n);
		mTemp = *temp->curMSS;

		for (int i = offset; i < mTemp->mFaceArray.size(); i++)
		{
			faceBuff[i].faceindex.x = mTemp->mFaceArray.at(curr).x;
			faceBuff[i].faceindex.y = mTemp->mFaceArray.at(curr).y;
			faceBuff[i].faceindex.z = mTemp->mFaceArray.at(curr).z;
			faceBuff[i].faceindex.index = (float)n;

			faceBuff[i].nodefaceresponseindex.x = mTemp->mFaceArray2.at(curr).x;
			faceBuff[i].nodefaceresponseindex.y = mTemp->mFaceArray2.at(curr).y;
			faceBuff[i].nodefaceresponseindex.z = mTemp->mFaceArray2.at(curr).z;
			faceBuff[i].nodefaceresponseindex.index = (float)n;

			curr++;
		}
		offset += mTemp->mFaceArray.size();
		curr = 0;
	}*/

	//for (int n = 0; n < fCountList.size(); n++)
	//{
	//	for (int i = offset; i < offset + fCountList.at(n); i++){
	//		/*faceBuff[i].faceindex.x = (float)FACEARRAY.at((3*i));
	//		faceBuff[i].faceindex.y = (float)FACEARRAY.at((3 * i) + 1);
	//		faceBuff[i].faceindex.z = (float)FACEARRAY.at((3 * i) + 2);
	//		faceBuff[i].faceindex.index = (float)n;

	//		faceBuff[i].nodefaceresponseindex.x = (float)FACEARRAY2.at((3 * i));
	//		faceBuff[i].nodefaceresponseindex.y = (float)FACEARRAY2.at((3 * i) + 1);
	//		faceBuff[i].nodefaceresponseindex.z = (float)FACEARRAY2.at((3 * i) + 2);
	//		faceBuff[i].nodefaceresponseindex.index = (float)n;*/
	//	}
	//	
	//}
	
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	printf("face ssbo setup end\n");
}

void CRender::resetFacePairWise()
{
	struct vertex4f* FacePair = (struct vertex4f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, facePair->size() * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for (int i = 0; i < facePair->size(); i++){
		FacePair[i].x = (float)facePair->at(i)->faceIndex1;
		FacePair[i].y = (float)facePair->at(i)->faceIndex2;
		FacePair[i].z = -1.0;
		FacePair[i].index = -1.0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void CRender::setLineArray()
{	
	CDeformable *temp;
	CMassSpringSystem *mTemp;
	int vSize = 0;
	for (int n = 0; n < mDefList.size(); n++){
		temp = mDefList.at(n);
		mTemp = *temp->curMSS;

		int size = mTemp->mFaceArray.size();

		for (int i = 0; i < size; i++)
		{
			ivec3 faceTmp = mTemp->mFaceArray.at(i);			
			ivec2 arrayTmp[3];

			arrayTmp[0] = ivec2(faceTmp.x + vSize, faceTmp.y + vSize);
			arrayTmp[1] = ivec2(faceTmp.x + vSize, faceTmp.z + vSize);
			arrayTmp[2] = ivec2(faceTmp.y + vSize, faceTmp.z + vSize);

			LINEARRAY.push_back(arrayTmp[0]);
			LINEARRAY.push_back(arrayTmp[1]);
			LINEARRAY.push_back(arrayTmp[2]);
		}
		vSize += vCountList.at(n);
	}
}

void CRender::resetFaceListSSBO()
{		
	struct vertex4f* faceListBuff = (struct vertex4f*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, BBFaceCount * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);;
	int offset = 0;
	for (int i = 0; i < mDefList.size(); i++){
		CDeformable *DeformTemp = mDefList.at(i);
		
		for (int j = 0; j < DeformTemp->faceList->size(); j++)
		{
			faceListBuff[offset + j].x = (float)DeformTemp->faceList->at(j).x;
			faceListBuff[offset + j].y = (float)DeformTemp->faceList->at(j).y+(64*i);
			faceListBuff[offset + j].z = -1.0;
			faceListBuff[offset + j].index = -1.0;
		}		
		offset = offset + DeformTemp->faceList->size();
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	check2();
}

void CRender::SetFaceList(){
	CDeformable *temp;
	CMassSpringSystem *mTemp;
	fCountList.clear();
	FaceCount = 0;
	FACEARRAY.clear();
	FACEARRAY2.clear();
	int vSize = 0;
	for (int n = 0; n < mDefList.size(); n++){
		temp = mDefList.at(n);
		mTemp = *temp->curMSS;
		int size = mTemp->mFaceArray.size();
		FaceCount += size;
		fCountList.push_back(size);
		for (int i = 0; i < size; i++){
			ivec3 arrayTmp = temp->mSrpingSystem->mFaceArray.at(i);
			ivec3 arrayTmp2 = temp->mSrpingSystem->mFaceArray2.at(i);
			if (n == 0){
				FACEARRAY.push_back(arrayTmp.x);
				FACEARRAY.push_back(arrayTmp.y);
				FACEARRAY.push_back(arrayTmp.z);

				FACEARRAY2.push_back(arrayTmp2.x);
				FACEARRAY2.push_back(arrayTmp2.y);
				FACEARRAY2.push_back(arrayTmp2.z);
			}
			else if (n>0){
				FACEARRAY.push_back(arrayTmp.x + vSize);
				FACEARRAY.push_back(arrayTmp.y + vSize);
				FACEARRAY.push_back(arrayTmp.z + vSize);

				FACEARRAY2.push_back(arrayTmp2.x);
				FACEARRAY2.push_back(arrayTmp2.y);
				FACEARRAY2.push_back(arrayTmp2.z);
			}
		}
		vSize += vCountList.at(n);
	}
}
void CRender::resetBoundingBoxResultSSBO(){
	struct vertex4f* ObjectBB = (struct vertex4f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 64 * 64 * sizeof(vertex4f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for (int i = 0; i < 64 * 64; i++){
		ObjectBB[i].x = -1.0;
		ObjectBB[i].y = 1.0;
		ObjectBB[i].z = 1.0;
		ObjectBB[i].index = (float)i;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void CRender::setFacePair()
{
	CDeformable *temp1;
	CDeformable *temp2;
	CMassSpringSystem *mTemp;

	CFacePair *tmp;

	facePair = new vector<CFacePair *>();
	facePair->clear();

	temp1 = mDefList.at(0);
	temp2 = mDefList.at(1);

	int cnt1 = temp1->mSrpingSystem->mFaceArray.size();
	int cnt2 = temp2->mSrpingSystem->mFaceArray.size();

	faceCount2 = cnt2;

	for (int i = 0; i < cnt1; i++)
	{
		for (int j = 0; j < cnt2; j++)
		{
			tmp = new CFacePair(i, j, -1);
			facePair->push_back(tmp);
		}
	}
}

void CRender::resetMaskSSBO()
{
	int* Mask = (int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 64 * 64 * ObjectCount * ObjectCount * sizeof(int), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for (int i = 0; i < 64 * 64 * ObjectCount * ObjectCount ; i++){
		Mask[i] = -1.0;		
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void CRender::check(){
	
	FILE *fpmm = fopen("newMinMax.txt", "w");
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, SSBOBoundingBoxMin);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOBoundingBoxMin);

	struct boundingBox *ptr;
	ptr = (struct boundingBox*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

	vector<boundingBox> array;
	
	array.clear();
	for (int i = 0; i < ObjectCount * BBCOUNT; i++){
		array.push_back(*ptr++);
	}

	for (int i = 0; i < ObjectCount * BBCOUNT; i++){		
		fprintf(fpmm, "%d [Object %d Min] %f %f %f %f\n", fcnt, i, array[i].min.x, array[i].min.y, array[i].min.z, array[i].min.index);
		fprintf(fpmm, "%d [Object %d Max] %f %f %f %f\n", fcnt, i, array[i].max.x, array[i].max.y, array[i].max.z, array[i].max.index);
		fprintf(fpmm, "%d [Object %d Index] %f %f %f %f\n\n", fcnt, i, array[i].data.x, array[i].data.y, array[i].data.z, array[i].data.index);
	}
	fclose(fpmm);
	
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void CRender::check2()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 20, SSBOPos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOPos);

	struct vertex4f *ptr;
	ptr = (struct vertex4f*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

	FILE *fpbb = fopen("PositionTest.txt", "w");
	vector<vertex4f> array;
	array.clear();

	int VERTEXSIZE = 0;

	for (int i = 0; i < vCountList.size(); i++)
	{
		VERTEXSIZE += vCountList.at(i);
	}

	for (int i = 0; i < VERTEXSIZE; i++){
		array.push_back(*ptr++);
	}
	printf("array Size : %d\n", array.size());

	for (int i = 0; i < VERTEXSIZE; i++){
		//fprintf(fpbb, "%d번째 FaceList 결과 : %f / [BB : %f] %f %f\n", i, array[i].x, array[i].y, array[i].z, array[i].index);
		fprintf(fpbb, "%d,%f,%f,%f,%f\n", i, array[i].x, array[i].y, array[i].z, array[i].index);
	}
	fclose(fpbb);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void CRender::check3()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, SSBOMaskBoundingBox);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOMaskBoundingBox);

	int *ptr;
	ptr = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

	//FILE *fp = fopen("BBCollisionResult.txt", "w");
	FILE *fpbb = fopen("BBMASKRESULT.txt", "w");
	vector<int> array;
	array.clear();
	for (int i = 0; i < 64*64; i++){
		array.push_back(*ptr++);
	}

	for (int i = 0; i < 64*64; i++){		
		fprintf(fpbb,"%d번째 Mask 결과 : %d\n", i,array[i]);		
	}
	fclose(fpbb);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}
