#include <stdio.h>
#include <stdlib.h>

#include <glm\glm.hpp>
#include <glm\gtc\/matrix_transform.hpp>
#include <glm\gtc/quaternion.hpp>

using namespace glm;

//#include <EGL/egl.h>
#include <vector>
#include <algorithm>
using namespace std;


class Camera
{
public:
	Camera(void);
	~Camera(void);

	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
	glm::vec3 position;
	float horizontalAngle;
	float verticalAngle;
	float initialFoV;
	float speed;
	float mouseSpeed;


	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();
	void computeMatricesFromInputs();

	int x, y, z;

private:

};