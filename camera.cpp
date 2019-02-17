
#include "Camera.h"

Camera::Camera(void)
{
	// sphere
	/*x = 6;
	y = 5;
	z = 150;*/

	// Alphabet
	/*
	x = 0.0;
	y = 0.7;
	z = 4.5;*/

	//test
	x = 0.0;
	y = 20.0;
	z = 200;
	
	position =
		::vec3(x, y, z);
	//horizontalAngle = 3.14f;
	horizontalAngle = 0.0f;
	verticalAngle = 0.0f;
	initialFoV = 45.0f;

	speed = 5.0f; // 3 units / second
	mouseSpeed = 0.0005f;
}

Camera::~Camera(void)
{
}


glm::mat4 Camera::getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 Camera::getProjectionMatrix(){
	return ProjectionMatrix;
}
void Camera::computeMatricesFromInputs(){
	// Get mouse position

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
		);

	position =
		::vec3(x, y, z);

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
		);

	// Up vector
	glm::vec3 up = glm::cross(right, direction);


	float FoV = initialFoV; //- 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45?Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 1024.0f / 600.0f, 0.01f, 10000.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		vec3(0,0,0), // and looks here : at the same position, plus "direction"
		vec3(0,1,0)                  // Head is up (set to 0,-1,0 to look upside-down)
		);

}
