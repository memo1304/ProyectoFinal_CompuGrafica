#pragma once

#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// opciones de movimiento de la camara
enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// valores por defecto
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 6.0f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;

// camara con yaw/pitch y calculo de view matrix
class Camera
{
public:
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVTY), zoom(ZOOM)
	{
		this->position = position;
		this->worldUp = up;
		this->yaw = yaw;
		this->pitch = pitch;
		this->updateCameraVectors();
	}

	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVTY), zoom(ZOOM)
	{
		this->position = glm::vec3(posX, posY, posZ);
		this->worldUp = glm::vec3(upX, upY, upZ);
		this->yaw = yaw;
		this->pitch = pitch;
		this->updateCameraVectors();
	}

	// view matrix a partir de yaw y pitch
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(this->position, this->position + this->front, this->up);
	}

	// movimiento con teclado
	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->movementSpeed * deltaTime;

		if (direction == FORWARD)
		{
			this->position += this->front * velocity;
		}

		if (direction == BACKWARD)
		{
			this->position -= this->front * velocity;
		}

		if (direction == LEFT)
		{
			this->position -= this->right * velocity;
		}

		if (direction == RIGHT)
		{
			this->position += this->right * velocity;
		}
	}

	// movimiento con mouse, limitando el pitch para que no se voltee
	void ProcessMouseMovement(GLfloat xOffset, GLfloat yOffset, GLboolean constrainPitch = true)
	{
		xOffset *= this->mouseSensitivity;
		yOffset *= this->mouseSensitivity;

		this->yaw += xOffset;
		this->pitch += yOffset;

		if (constrainPitch)
		{
			if (this->pitch > this->maxPitch)
			{
				this->pitch = this->maxPitch;
			}

			if (this->pitch < this->minPitch)
			{
				this->pitch = this->minPitch;
			}
		}

		this->updateCameraVectors();
	}

	void ProcessMouseScroll(GLfloat yOffset)
	{

	}

	GLfloat GetZoom()
	{
		return this->zoom;
	}

	glm::vec3 GetPosition()
	{
		return this->position;
	}

	glm::vec3 GetFront()
	{
		return this->front;
	}

	void SetYaw(GLfloat newYaw)
	{
		this->yaw = newYaw;
		this->updateCameraVectors();
	}

	// para la camara GTA
	GLfloat GetYaw()   { return this->yaw; }
	GLfloat GetPitch() { return this->pitch; }

	// cambia el pitch respetando los limites
	void SetPitch(GLfloat newPitch)
	{
		if (newPitch > this->maxPitch) newPitch = this->maxPitch;
		if (newPitch < this->minPitch) newPitch = this->minPitch;
		this->pitch = newPitch;
		this->updateCameraVectors();
	}

	// limites de inclinacion (por defecto -89 a 89)
	void SetPitchLimits(GLfloat minP, GLfloat maxP)
	{
		this->minPitch = minP;
		this->maxPitch = maxP;
		if (this->pitch > maxP) this->pitch = maxP;
		if (this->pitch < minP) this->pitch = minP;
		this->updateCameraVectors();
	}

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat movementSpeed;
	GLfloat mouseSensitivity;
	GLfloat zoom;

	// limites de pitch
	GLfloat minPitch = -89.0f;
	GLfloat maxPitch =  89.0f;

	// recalcula front, right y up a partir de yaw y pitch
	void updateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		front.y = sin(glm::radians(this->pitch));
		front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		this->front = glm::normalize(front);
		this->right = glm::normalize(glm::cross(this->front, this->worldUp));
		this->up = glm::normalize(glm::cross(this->right, this->front));
	}
};