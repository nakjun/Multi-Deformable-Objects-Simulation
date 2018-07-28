#include "glsl.h"
#include "DeformableObject.h"
#include "camera.h"
#include "pairwise.h"
class CRender{
public:
	CRender();
	~CRender();

	void init(float width, float height);
	void render();

	void invoke_compute_shader();
	void invoke_updateBB_shader();
	void invoke_collisionBB_shader();

	GLuint baseShader;
	GLuint computeShader;
	GLuint springShader;
	GLuint BoundingBoxShader;
	GLuint computeProgNorm;

	void SetFaceList();

	void resetNodeSpringTableSSBO();
	void resetPositionSSBO();
	void resetSpringSSBO();
	void resetVelocitySSBO();
	void resetSpringForceSSBO();
	void resetNodeForceMapSSBO();
	void resetNodeForceSummationSSBO();
	void resetObjectInformationSSBO();
	void resetBoundingBoxSSBO1();	
	void resetFaceSSBO();
	void resetNormalSSBO();
	void resetParticleNormalSSBO();
	void resetBoundingBoxResultSSBO();
	void resetFacePairWise();
	void resetBBFaceSSBO();
	void resetMaskSSBO();
	void resetFaceListSSBO();
	void resetBBLineSSBO();

	GLuint compileShader(std::string path_to_file, GLenum shader_type);
	void printProgramLog(GLuint shader);
	void printShaderLog(GLuint program);
	void check();
	void check2();
	void check3();
	int BBIntersection(vec3 minA,vec3 maxA,vec3 minB,vec3 maxB);
	void generateShaders();
	void generateBuffers();
	void generateMVPMatrix();
	void setRenderFlag();
	void setLineArray();
	void setFacePair();
public:
	FILE *mfile;
	CMassSpringSystem *mSpringSystem;
	vector<CDeformable *> mDefList;

	GLFWwindow* Scenewindow;

	float mTick;
	float mRest;
	float mMass;
	
	int workingGroups;

	float mKS; float mKD;
	float Width; float Height;

	//Shader Handler
	uint32_t render_program_handle = 0;
	uint32_t compute_program_handle[3];
	uint32_t BB_program_handle[3];

	uint32_t particle_position_vao_handle = 0;
	uint32_t particle_buffer_handle = 0;

	GLuint computeshader;

	GLuint MatrixID;

	GLuint uniform_loc;
	
	//SSBO ID
	GLuint SSBOPos;
	GLuint SSBONormal;
	GLuint SSBOSpring;
	GLuint SSBOVel;
	GLuint SSBOForce;
	GLuint SSBOBoundingBoxMin;	
	GLuint SSBONodeForceMap;
	GLuint SSBONodeForceSummation;	
	GLuint SSBOFace;
	GLuint SSBOBBFace;	
	GLuint SSBOBoundingBoxResult;	
	GLuint SSBOObjectInformation;
	GLuint SSBONodeSPringTable;
	GLuint SSBOFacePairWise;
	GLuint SSBOMaskBoundingBox;	
	GLuint SSBOMaskTable;
	GLuint SSBOFaceList;
	GLuint SSBOBBLineData;
	GLuint VAO;

	vec3 Center;
	vector<int> vCountList;
	vector<int> fCountList;
	vector<int> sCountList;
	vector<CFacePair *> *facePair;
	int spring_count;
	Camera *cam;
	int VerticesCount;
	int FaceCount;
	int ObjectCount;
	bool isRender;
	int fcnt;
	int BBFaceCount;
	mat4 Projection;
	mat4 View;
	mat4 Model;
	vector<ivec2> *faceList;
};
