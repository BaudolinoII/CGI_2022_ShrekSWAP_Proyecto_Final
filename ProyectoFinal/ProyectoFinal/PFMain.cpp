#include <iostream>
#include <cmath>
#include <Windows.h>
#include <mmsystem.h>
#include <thread>
// GLEW GLfW
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//Load Models
#include "SOIL2/SOIL2.h"
#include "stb_image.h"
// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"

// Function prototypes
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement();

// Window dimensions
const GLuint WIDTH = 1200, HEIGHT = 800;
int SCREEN_WIDTH, SCREEN_HEIGHT;
float currLight = 0.7f, targetLight = 0.7f;
// Camera
Camera  camera(glm::vec3(-12.5f, 8.0f, 45.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true, active = false, one_shot_0 = true, one_shot_1 = true;

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(-1.0f, 0.7f, -12.5f),
	glm::vec3(0.0f,0.0f, 0.0f),
	glm::vec3(0.0f,0.0f, 0.0f),
	glm::vec3(0.0f,0.0f, 0.0f)
};
GLfloat skyboxVertices[] = {
	// Positions
	-1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,-1.0f,  1.0f, 1.0f, -1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,-1.0f, -1.0f, 1.0f,  1.0f, -1.0f,  1.0f
};
// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

void playSoundTrack(int nOST) {
	switch (nOST) {
	case 0:
		PlaySound(TEXT("Audio/All_Star.wav"), NULL, SND_SYNC);
		PlaySound(TEXT("Audio/Ambientacion.wav"), NULL, SND_LOOP || SND_NOSTOP);
		break;
	case 1:
		PlaySound(TEXT("Audio/Vieja_Muerta.wav"), NULL, SND_SYNC);
	case 2:
		PlaySound(TEXT("Audio/Ambientacion.wav"), NULL, SND_LOOP||SND_NOSTOP);
		break;
	}
}

typedef struct _frame {//Variables para GUARDAR Key Frames
	float* var;
	struct _frame() { var = nullptr; }
	void setComplexity(const unsigned int complexity) {
		var = new float[complexity];
		for (unsigned int i = 0; i < complexity; i++) var[i] = 0;
	}
}FRAME;
typedef struct _routine {
	FRAME* KF;
	unsigned int complexity, currDetail, nFrames, detail;
	int currFrame;
	float* delta;
	float* value;
	bool play, cicle;
	struct _routine(const unsigned int complexity, const unsigned int nFrames, const unsigned int detail, const bool cicle = false) {
		this->nFrames = nFrames;
		this->complexity = complexity;
		KF = new FRAME[this->nFrames];
		for (unsigned int i = 0; i < this->nFrames; i++) KF[i].setComplexity(this->complexity); 
		delta = new float[this->complexity];
		value = new float[this->complexity];
		for (unsigned int i = 0; i < this->complexity; i++) { value[i] = 0; delta[i] = 0; }
		currFrame = 0;
		currDetail = 0;
		this->detail = detail;
		play = false; this->cicle = cicle;
	}
	void interpolation() {
		for (unsigned int i = 0; i < complexity; i++)
			delta[i] = (KF[currFrame + 1].var[i] - KF[currFrame].var[i]) / detail;
	}
	void setAtCero() {
		for (unsigned int i = 0; i < complexity; i++) value[i] = KF[0].var[i];
		currFrame = 0;
		currDetail = 0;
		interpolation();
	}
	void animacion() {//Movimiento del personaje
		if (play)
			if (currDetail >= detail) {//end of animation between frames?
				if (currFrame < nFrames - 2) {//Next frame interpolations
					currDetail = 0; //Reset counter
					currFrame++;
					interpolation();
				}else if (cicle) //end of total animation?
						  setAtCero();
					  else
						  play = false;
			}else{
				for (unsigned int i = 0; i < complexity; i++)//Cambio de las variables
					value[i] += delta[i];
				currDetail++;
			}
	}
}RT;


RT rt_Pos_Shrek(4, 13, 600, false);//Rutina Principal
RT rt_WK(10, 9, 90, true), rt_SB(10, 4, 240, false), rt_VM(10, 4, 450, false);//Rutinas Extremidades Shrek
RT rt_Puerta_Bano(4, 5, 120, false), rt_Puerta_Casa(1, 4, 300, false), rt_Ataud(3, 4, 300, false), rt_Sillas(2, 3, 300, false);//Rutinas del entorno
void setAnim() {
	////////////////////////////////////////////////////////ANIMACION DE MOVIMIENTO SHREK////////////////////////////////////////////////////////////
	rt_Pos_Shrek.KF[0].var[0] = 27.0f; rt_Pos_Shrek.KF[0].var[1] = 10.3f; rt_Pos_Shrek.KF[0].var[2] = 23.0f; rt_Pos_Shrek.KF[0].var[3] = -130.0f;//Inicio
	rt_Pos_Shrek.KF[1].var[0] = 26.5f; rt_Pos_Shrek.KF[1].var[1] = 10.3f; rt_Pos_Shrek.KF[1].var[2] = 22.5f; rt_Pos_Shrek.KF[1].var[3] = -130.0f;//Somebody
	rt_Pos_Shrek.KF[2].var[0] = 26.0f; rt_Pos_Shrek.KF[2].var[1] = 10.3f; rt_Pos_Shrek.KF[2].var[2] = 22.0f; rt_Pos_Shrek.KF[2].var[3] = -130.0f;
	rt_Pos_Shrek.KF[3].var[0] = 25.7f; rt_Pos_Shrek.KF[3].var[1] = 10.3f; rt_Pos_Shrek.KF[3].var[2] = 21.1f; rt_Pos_Shrek.KF[3].var[3] = -120.0f;//Caminata
	rt_Pos_Shrek.KF[4].var[0] = 19.0f; rt_Pos_Shrek.KF[4].var[1] =  8.7f; rt_Pos_Shrek.KF[4].var[2] = 17.0f; rt_Pos_Shrek.KF[4].var[3] =  -90.0f;
	rt_Pos_Shrek.KF[5].var[0] = 14.0f; rt_Pos_Shrek.KF[5].var[1] =  5.7f; rt_Pos_Shrek.KF[5].var[2] = 13.0f; rt_Pos_Shrek.KF[5].var[3] = -110.0f;
	rt_Pos_Shrek.KF[6].var[0] =  9.0f; rt_Pos_Shrek.KF[6].var[1] =  3.7f; rt_Pos_Shrek.KF[6].var[2] = 10.0f; rt_Pos_Shrek.KF[6].var[3] = -130.0f;
	rt_Pos_Shrek.KF[7].var[0] = 4.34f; rt_Pos_Shrek.KF[7].var[1] =  2.6f; rt_Pos_Shrek.KF[7].var[2] =  8.4f; rt_Pos_Shrek.KF[7].var[3] = -150.0f;
	rt_Pos_Shrek.KF[8].var[0] = 0.02f; rt_Pos_Shrek.KF[8].var[1] =  2.4f; rt_Pos_Shrek.KF[8].var[2] =  7.0f; rt_Pos_Shrek.KF[8].var[3] = -175.0f;
	rt_Pos_Shrek.KF[9].var[0] =-0.03f; rt_Pos_Shrek.KF[9].var[1] = 2.36f; rt_Pos_Shrek.KF[9].var[2] = 0.76f; rt_Pos_Shrek.KF[9].var[3] = -180.0f;//Entrada a la Casa
	rt_Pos_Shrek.KF[10].var[0] =-0.73f; rt_Pos_Shrek.KF[10].var[1] = 2.36f; rt_Pos_Shrek.KF[10].var[2] = 0.76f; rt_Pos_Shrek.KF[10].var[3] = -145.0f;
	rt_Pos_Shrek.KF[11].var[0] = -6.0f; rt_Pos_Shrek.KF[11].var[1] = 2.36f; rt_Pos_Shrek.KF[11].var[2] =-2.36f; rt_Pos_Shrek.KF[11].var[3] = -135.0f;
	rt_Pos_Shrek.KF[12].var[0] = -4.7f; rt_Pos_Shrek.KF[12].var[1] = 1.95f; rt_Pos_Shrek.KF[12].var[2] = -6.5f; rt_Pos_Shrek.KF[12].var[3] = -270.0f;//Empujar
	rt_Pos_Shrek.setAtCero();////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////ANIMACION DEL ENTORNO////////////////////////////////////////////////////////////////////
	rt_Puerta_Bano.KF[0].var[0] = 26.55f; rt_Puerta_Bano.KF[0].var[1] = 10.0f; rt_Puerta_Bano.KF[0].var[2] = 21.16f; rt_Puerta_Bano.KF[0].var[3] = 0.0f;
	rt_Puerta_Bano.KF[1].var[0] = 13.27f; rt_Puerta_Bano.KF[1].var[1] = 15.0f; rt_Puerta_Bano.KF[1].var[2] = 10.30f; rt_Puerta_Bano.KF[1].var[3] = 180.0f;
	rt_Puerta_Bano.KF[2].var[0] =   0.0f; rt_Puerta_Bano.KF[2].var[1] = 20.0f; rt_Puerta_Bano.KF[2].var[2] =  0.18f; rt_Puerta_Bano.KF[2].var[3] = 360.0f;
	rt_Puerta_Bano.KF[3].var[0] =-13.27f; rt_Puerta_Bano.KF[3].var[1] = 15.0f; rt_Puerta_Bano.KF[3].var[2] =-10.30f; rt_Puerta_Bano.KF[3].var[3] = 440.0f;
	rt_Puerta_Bano.KF[4].var[0] =-23.77f; rt_Puerta_Bano.KF[4].var[1] = 10.0f; rt_Puerta_Bano.KF[4].var[2] =-21.01f; rt_Puerta_Bano.KF[4].var[3] = 800.0f;
	rt_Puerta_Bano.setAtCero();///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	rt_Puerta_Casa.KF[0].var[0] = 0.0f; rt_Puerta_Casa.KF[1].var[0] = -120.0f; rt_Puerta_Casa.KF[2].var[0] = -120.0f; 
	rt_Puerta_Casa.setAtCero();///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	rt_Ataud.KF[0].var[0] = 0.0f; rt_Ataud.KF[0].var[1] = 3.4f; rt_Ataud.KF[0].var[2] = 0.0f;
	rt_Ataud.KF[1].var[0] = 5.0f; rt_Ataud.KF[1].var[1] = 3.4f; rt_Ataud.KF[1].var[2] =-22.0f;
	rt_Ataud.KF[2].var[0] =10.0f; rt_Ataud.KF[2].var[1] = 3.4f; rt_Ataud.KF[2].var[2] =-45.0f;
	rt_Ataud.KF[3].var[0] =10.0f; rt_Ataud.KF[3].var[1] = 1.0f; rt_Ataud.KF[3].var[2] = 0.0f;
	rt_Ataud.setAtCero();/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	rt_Sillas.KF[0].var[0] = 0.8f; rt_Sillas.KF[0].var[1] = 0.0f;
	rt_Sillas.KF[1].var[0] = 0.8f; rt_Sillas.KF[1].var[1] = 45.0f;
	rt_Sillas.KF[2].var[0] = 0.4f; rt_Sillas.KF[2].var[1] = 90.0f;
	rt_Sillas.setAtCero();/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////ANIMACION DE EXTREMIDADES/////////////////////////////////////////////////////////////////
	/// ANIMACION SOMEBODY DESDE LOS CUADROS [0 - 2] DE LA ANIMACION rt_Pos_Shrek
	///BRAZO IZQ Y//BRAZO IZQ Z//ANTBRAZO IZQ Y//BRAZO DER Y//BRAZO DER Z//ANTBRAZO DER Z///
	rt_SB.KF[0].var[0] = 0.0f; rt_SB.KF[0].var[1]=-65.0f; rt_SB.KF[0].var[2] = 0.0f; rt_SB.KF[0].var[3] = 0.0f; rt_SB.KF[0].var[4]= 65.0f; rt_SB.KF[0].var[5] = 0.0f;
	rt_SB.KF[1].var[0] =60.0f; rt_SB.KF[1].var[1]=-65.0f; rt_SB.KF[1].var[2]=-115.0f;rt_SB.KF[1].var[3] =60.0f; rt_SB.KF[1].var[4]= 65.0f; rt_SB.KF[1].var[5]=-115.0f;
	rt_SB.KF[2].var[0] = 0.0f; rt_SB.KF[2].var[1]=-65.0f; rt_SB.KF[2].var[2] = 0.0f; rt_SB.KF[2].var[3] = 0.0f; rt_SB.KF[2].var[4]= 65.0f; rt_SB.KF[2].var[5] = 0.0f;
	rt_SB.KF[3].var[0] = 0.0f; rt_SB.KF[3].var[1]=-65.0f; rt_SB.KF[3].var[2] = 0.0f; rt_SB.KF[3].var[3] = 0.0f; rt_SB.KF[3].var[4]= 65.0f; rt_SB.KF[3].var[5] = 0.0f;
	///PIERNA IZQ X//ANTPIERNA IZQ X//PIERNA DER X//ANTPIERNA X///
	rt_SB.KF[0].var[6] = -5.0f; rt_SB.KF[0].var[7] = 65.0f; rt_SB.KF[0].var[8] = 0.0f; rt_SB.KF[0].var[9] = 0.0f;
	rt_SB.KF[1].var[6] =-55.0f; rt_SB.KF[1].var[7] =  0.0f; rt_SB.KF[1].var[8] = 0.0f; rt_SB.KF[1].var[9] = 0.0f;
	rt_SB.KF[2].var[6] =-20.0f; rt_SB.KF[2].var[7] =-30.0f; rt_SB.KF[2].var[8] = 0.0f; rt_SB.KF[2].var[9] = 0.0f;
	rt_SB.KF[3].var[6] =  0.0f; rt_SB.KF[3].var[7] =  0.0f; rt_SB.KF[3].var[8] = 0.0f; rt_SB.KF[3].var[9] = 0.0f;
	rt_SB.setAtCero();////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// ANIMACION CAMINATA DESDE LOS CUADROS [3 - 8]U[10 - 11] DE LA ANIMACION rt_Pos_Shrek CICLICO
	///BRAZO IZQ Y//BRAZO IZQ Z//ANTBRAZO IZQ Y//BRAZO DER Y//BRAZO DER Z//ANTBRAZO DER Z///
	rt_WK.KF[0].var[0]=-70.0f; rt_WK.KF[0].var[1]=-65.0f; rt_WK.KF[0].var[2]=-40.0f; rt_WK.KF[0].var[3]= 70.0f; rt_WK.KF[0].var[4] =65.0f; rt_WK.KF[0].var[5] =40.0f;
	rt_WK.KF[1].var[0]=-35.0f; rt_WK.KF[1].var[1]=-65.0f; rt_WK.KF[1].var[2]=-30.0f; rt_WK.KF[1].var[3]= 35.0f; rt_WK.KF[1].var[4] =65.0f; rt_WK.KF[1].var[5] =30.0f;
	rt_WK.KF[2].var[0]=  0.0f; rt_WK.KF[2].var[1]=-65.0f; rt_WK.KF[2].var[2]=-20.0f; rt_WK.KF[2].var[3]=  0.0f; rt_WK.KF[2].var[4] =65.0f; rt_WK.KF[2].var[5] =20.0f;
	rt_WK.KF[3].var[0]= 35.0f; rt_WK.KF[3].var[1]=-65.0f; rt_WK.KF[3].var[2]=-10.0f; rt_WK.KF[3].var[3]=-35.0f; rt_WK.KF[3].var[4] =65.0f; rt_WK.KF[3].var[5] =10.0f;
	rt_WK.KF[4].var[0]= 70.0f; rt_WK.KF[4].var[1]=-65.0f; rt_WK.KF[4].var[2]=  0.0f; rt_WK.KF[4].var[3]=-70.0f; rt_WK.KF[4].var[4] =65.0f; rt_WK.KF[4].var[5] = 0.0f;
	rt_WK.KF[5].var[0]= 35.0f; rt_WK.KF[5].var[1]=-65.0f; rt_WK.KF[5].var[2]=-10.0f; rt_WK.KF[5].var[3]=-35.0f; rt_WK.KF[5].var[4] =65.0f; rt_WK.KF[5].var[5] =10.0f;
	rt_WK.KF[6].var[0]=  0.0f; rt_WK.KF[6].var[1]=-65.0f; rt_WK.KF[6].var[2]=-20.0f; rt_WK.KF[6].var[3]=  0.0f; rt_WK.KF[6].var[4] =65.0f; rt_WK.KF[6].var[5] =20.0f;
	rt_WK.KF[7].var[0]=-35.0f; rt_WK.KF[7].var[1]=-65.0f; rt_WK.KF[7].var[2]=-30.0f; rt_WK.KF[7].var[3]= 35.0f; rt_WK.KF[7].var[4] =65.0f; rt_WK.KF[7].var[5] =30.0f;
	rt_WK.KF[8].var[0]=-70.0f; rt_WK.KF[8].var[1]=-65.0f; rt_WK.KF[8].var[2]=-40.0f; rt_WK.KF[8].var[3]= 70.0f; rt_WK.KF[8].var[4] =65.0f; rt_WK.KF[8].var[5] =40.0f;
	///PIERNA IZQ X//ANTPIERNA IZQ X//PIERNA DER X//ANTPIERNA X///
	rt_WK.KF[0].var[6] = -35.0f; rt_WK.KF[0].var[7] = 54.0f; rt_WK.KF[0].var[8] =  12.0f; rt_WK.KF[0].var[9] = 12.0f;
	rt_WK.KF[1].var[6] = -19.0f; rt_WK.KF[1].var[7] =  0.0f; rt_WK.KF[1].var[8] =   0.0f; rt_WK.KF[1].var[9] = 0.0f;
	rt_WK.KF[2].var[6] =   0.0f; rt_WK.KF[2].var[7] =  0.0f; rt_WK.KF[2].var[8] = -19.0f; rt_WK.KF[2].var[9] = 0.0f;
	rt_WK.KF[3].var[6] =  12.0f; rt_WK.KF[3].var[7] = 12.0f; rt_WK.KF[3].var[8] = -35.0f; rt_WK.KF[3].var[9] = 54.0f;
	rt_WK.KF[4].var[6] =  30.0f; rt_WK.KF[4].var[7] = 20.0f; rt_WK.KF[4].var[8] = -19.0f; rt_WK.KF[4].var[9] = 0.0f;
	rt_WK.KF[5].var[6] =  12.0f; rt_WK.KF[5].var[7] = 12.0f; rt_WK.KF[5].var[8] =   0.0f; rt_WK.KF[5].var[9] = 0.0f;
	rt_WK.KF[6].var[6] =   0.0f; rt_WK.KF[6].var[7] =  0.0f; rt_WK.KF[6].var[8] =  12.0f; rt_WK.KF[6].var[9] = 12.0f;
	rt_WK.KF[7].var[6] = -19.0f; rt_WK.KF[7].var[7] =  0.0f; rt_WK.KF[7].var[8] =  30.0f; rt_WK.KF[7].var[9] = 20.0f;
	rt_WK.KF[8].var[6] = -35.0f; rt_WK.KF[8].var[7] = 54.0f; rt_WK.KF[8].var[8] =  12.0f; rt_WK.KF[8].var[9] = 12.0f;
	rt_WK.setAtCero();////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// ANIMACION VIEJA MUERTA DESDE EL CUADRO 12 DE LA ANIMACION rt_Pos_Shrek
	///BRAZO IZQ Y//BRAZO IZQ Z//ANTBRAZO IZQ Y//BRAZO DER Y//BRAZO DER Z//ANTBRAZO DER Z///
	rt_VM.KF[0].var[0] = 0.0f; rt_VM.KF[0].var[1]=-65.0f; rt_VM.KF[0].var[2]=-60.0f; rt_VM.KF[0].var[3] = 0.0f; rt_VM.KF[0].var[4] =65.0f; rt_VM.KF[0].var[5] =60.0f;
	rt_VM.KF[1].var[0] =90.0f; rt_VM.KF[1].var[1] = 0.0f; rt_VM.KF[1].var[2] = 0.0f; rt_VM.KF[1].var[3]=-90.0f; rt_VM.KF[1].var[4] = 0.0f; rt_VM.KF[1].var[5] = 0.0f;
	rt_VM.KF[2].var[0] =90.0f; rt_VM.KF[2].var[1] = 0.0f; rt_VM.KF[2].var[2] = 0.0f; rt_VM.KF[2].var[3]=-90.0f; rt_VM.KF[2].var[4] = 0.0f; rt_VM.KF[2].var[5] = 0.0f;
	rt_VM.KF[3].var[0] = 0.0f; rt_VM.KF[3].var[1]=-65.0f; rt_VM.KF[3].var[2] = 0.0f; rt_VM.KF[3].var[3] = 0.0f; rt_VM.KF[3].var[4] =65.0f; rt_VM.KF[3].var[5] = 0.0f;
	///PIERNA IZQ X//ANTPIERNA IZQ X//PIERNA DER X//ANTPIERNA X///
	rt_VM.KF[0].var[6] =   0.0f; rt_VM.KF[0].var[7] =  0.0f; rt_VM.KF[0].var[8] =   0.0f; rt_VM.KF[0].var[9] = 0.0f;
	rt_VM.KF[1].var[6] =   0.0f; rt_VM.KF[1].var[7] =  0.0f; rt_VM.KF[1].var[8] =   0.0f; rt_VM.KF[1].var[9] = 0.0f;
	rt_VM.KF[2].var[6] = -90.0f; rt_VM.KF[2].var[7] = 90.0f; rt_VM.KF[2].var[8] = -90.0f; rt_VM.KF[2].var[9] = 90.0f;
	rt_VM.KF[3].var[6] = -90.0f; rt_VM.KF[3].var[7] = 90.0f; rt_VM.KF[3].var[8] = -90.0f; rt_VM.KF[3].var[9] = 90.0f;
	rt_VM.setAtCero();////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

const float degree(const float current, const float goal) {
	if (current > goal)
		return current - 0.0001f;
	else if (current < goal)
		return current + 0.0001f;
	return current;
}
void resetScene() {
	active = false; one_shot_0 = true; one_shot_1 = true; currLight = 0.7f; targetLight = 0.7f;
	rt_Pos_Shrek.play = false; rt_Pos_Shrek.setAtCero();
	rt_Puerta_Bano.play = false; rt_Puerta_Bano.setAtCero();
	rt_Puerta_Casa.play = false; rt_Puerta_Casa.setAtCero();
	rt_Sillas.play = false; rt_Sillas.setAtCero();
	rt_Ataud.play = false; rt_Ataud.setAtCero();
	rt_SB.play = false; rt_SB.setAtCero();
	rt_WK.play = false; rt_WK.setAtCero();
	rt_VM.play = false; rt_VM.setAtCero();
}

int main(){
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);*/

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "315104271_ProyectoFinal_GPO07", nullptr, nullptr);

	if (nullptr == window){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLFW Options
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit()){
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	Shader single("Shaders/single.vs", "Shaders/single.frag");
	Shader standar("Shaders/standar.vs", "Shaders/standar.frag");
	Shader skybox("Shaders/SkyBox.vs", "Shaders/SkyBox.frag");

	Model Suelo((char*)"Models/Shrek/Suelo.obj");

	Model Ataud((char*)"Models/Shrek/Ataud.obj");
	Model Cristal((char*)"Models/Shrek/Ataud_Cristal.obj");
	Model Bano((char*)"Models/Shrek/Bano.obj");
	Model Casa((char*)"Models/Shrek/Casa.obj");
	Model Fuego((char*)"Models/Shrek/Fuego.obj");
	Model Fogata((char*)"Models/Shrek/Fogata.obj");
	Model Letrero((char*)"Models/Shrek/Letrero.obj");
	Model Mesa((char*)"Models/Shrek/Mesa.obj");
	Model Puerta_Bano((char*)"Models/Shrek/Puerta_Bano.obj");
	Model Puerta_Casa((char*)"Models/Shrek/Puerta_Casa.obj");
	Model Silla((char*)"Models/Shrek/Silla.obj");
	Model Sillon((char*)"Models/Shrek/Sillon.obj");

	Model Antebrazo((char*)"Models/Shrek/Ogro/Shrek_Antebrazo.obj");
	Model Antepierna((char*)"Models/Shrek/Ogro/Shrek_Antepierna.obj");
	Model Brazo((char*)"Models/Shrek/Ogro/Shrek_Brazo.obj");
	Model Cabeza((char*)"Models/Shrek/Ogro/Shrek_Cabeza.obj");
	Model Torso((char*)"Models/Shrek/Ogro/Shrek_Torso.obj");
	Model Pierna((char*)"Models/Shrek/Ogro/Shrek_Pierna.obj");

	// Set texture units
	
	standar.Use();
	glUniform1i(glGetUniformLocation(standar.Program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(standar.Program, "material.specular"),1);
	glUniform1f(glGetUniformLocation(standar.Program, "alpha"), 1.0f);
	// Directional light
	glUniform3f(glGetUniformLocation(standar.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
	glUniform3f(glGetUniformLocation(standar.Program, "dirLight.diffuse"), 0.2f, 0.2f, 0.2f);
	glUniform3f(glGetUniformLocation(standar.Program, "dirLight.specular"), 0.4f, 0.4f, 0.4f);
	// Point light 1
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[0].ambient"), 0.02f, 0.02f, 0.02f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[0].diffuse"), 0.9f, 0.72f, 0.53f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[0].specular"), 0.9f, 0.72f, 0.53f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[0].constant"), 1.0f);
	// Point light 2
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[1].ambient"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[1].diffuse"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[1].specular"), 0.0f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[1].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[1].linear"), 0.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[1].quadratic"), 0.0f);
	// Point light 3
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[2].ambient"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[2].diffuse"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[2].specular"), 0.0f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[2].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[2].linear"), 0.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[2].quadratic"), 0.0f);
	// Point light 4
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[3].position"), pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[3].ambient"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[3].diffuse"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[3].specular"), 0.0f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[3].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[3].linear"), 0.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[3].quadratic"), 0.0f);

	// SpotLight
	glUniform3f(glGetUniformLocation(standar.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
	glUniform3f(glGetUniformLocation(standar.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
	glUniform3f(glGetUniformLocation(standar.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(standar.Program, "spotLight.diffuse"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(standar.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight.linear"), 0.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight.quadratic"), 0.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight.cutOff"), glm::cos(glm::radians(0.0f)));
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(0.0f)));

	// Set material properties
	glUniform1f(glGetUniformLocation(standar.Program, "material.shininess"), 16.0f);

	GLuint skyboxVBO, skyboxVAO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	vector<const GLchar*> faces;
	faces.push_back("SkyBox/right.png");
	faces.push_back("SkyBox/left.png");
	faces.push_back("SkyBox/top.png");
	faces.push_back("SkyBox/bottom.png");
	faces.push_back("SkyBox/back.png");
	faces.push_back("SkyBox/front.png");

	GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);
	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);
	setAnim();
	
	float random = 0.0f, deg = 0.025f;
	float rot_limbs[10];
	for (unsigned int i = 0; i < 10; i++)
		rot_limbs[i] = rt_SB.value[i];

	// Game loop
	std::thread soundtrack(&playSoundTrack, 2);
	soundtrack.detach();
	while (!glfwWindowShouldClose(window)){
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		DoMovement();
		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	   
		// OpenGL options
		glEnable(GL_DEPTH_TEST);
		if (active) { //Animacion general de la escena
			rt_Pos_Shrek.animacion();//Animacion Maestra
			
			if (one_shot_0) {
				std::thread soundtrack(&playSoundTrack, 0);
				soundtrack.detach();
				one_shot_0 = false;
			}
			if (rt_Pos_Shrek.currFrame > 0) {
				rt_Puerta_Bano.animacion();
				rt_SB.animacion();
			}
			if (rt_Pos_Shrek.currFrame > 2) {
				rt_WK.animacion();
			}
				
			if (rt_Pos_Shrek.currFrame > 7) {
				rt_Puerta_Casa.animacion();
			}
				
			if (rt_Pos_Shrek.currFrame > 9) {
				if (one_shot_1) {
					std::thread quote(&playSoundTrack, 1);
					quote.detach();
					one_shot_1 = false;
				}
				//std::cout << rt_Pos_Shrek.currFrame << std::endl;
				rt_VM.animacion();
			}
			if (rt_Pos_Shrek.currFrame == 11) {
				rt_Sillas.animacion();
				rt_Ataud.animacion();
				//std::cout << "A la vieja muerta me la bajan de la mesa" << std::endl;
			}
			switch (rt_Pos_Shrek.currFrame) {
			case 0:
			case 1:
			case 2:
				for (unsigned int i = 0; i < 10; i++)
					rot_limbs[i] = rt_SB.value[i];
				break;
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				targetLight = 0.2f;
				for (unsigned int i = 0; i < 10; i++)
					rot_limbs[i] = rt_WK.value[i];
				break;
			case 9:
				for (unsigned int i = 0; i < 10; i++)
					rot_limbs[i] = rt_SB.KF[2].var[i];
				break;
			case 10:
				for (unsigned int i = 0; i < 10; i++)
					rot_limbs[i] = rt_WK.value[i];
				break;
			case 11:
			case 12:
				for (unsigned int i = 0; i < 10; i++)
					rot_limbs[i] = rt_VM.value[i];
				break;
			}
		}
		
		//Load Model
		// Use cooresponding shader when setting uniforms/drawing objects
		standar.Use();
		GLint viewPosLoc = glGetUniformLocation(standar.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		
		if (abs((0.0125f + random) - degree(deg, 0.0125f + random)) < 0.00001)
			random = (float)(std::rand() % 1000) / 10000.0f;
		else
			deg = degree(deg, 0.0125 + random);
		glUniform1f(glGetUniformLocation(standar.Program, "pointLights[0].linear"), deg);
		glUniform1f(glGetUniformLocation(standar.Program, "pointLights[0].quadratic"), deg / 2.0f);
		// Global Light
		currLight = degree(currLight, targetLight);
		glUniform3f(glGetUniformLocation(standar.Program, "dirLight.ambient"), currLight, currLight, currLight);

		// Create camera transformations
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model(1); glm::mat4 temp(1);
		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(standar.Program, "model");
		GLint viewLoc = glGetUniformLocation(standar.Program, "view");
		GLint projLoc = glGetUniformLocation(standar.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Casa.Draw(standar);
		Suelo.Draw(standar);
		model = glm::translate(model, glm::vec3(-3.145f, 2.0f, 1.3f));
		model = glm::rotate(model, glm::radians(rt_Puerta_Casa.value[0]), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Puerta_Casa.Draw(standar);

		////////////////////Dibujo de Shrek////////////////////
		model = glm::translate(glm::mat4(1), glm::vec3(rt_Pos_Shrek.value[0], rt_Pos_Shrek.value[1], rt_Pos_Shrek.value[2]));
		model = glm::rotate(model, glm::radians(rt_Pos_Shrek.value[3]), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Torso.Draw(standar);
		temp = glm::translate(model, glm::vec3(0.0f, 0.9f, -0.1f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(temp));
		Cabeza.Draw(standar);
		///BRAZO IZQ Y//BRAZO IZQ Z//ANTBRAZO IZQ Z//BRAZO DER Y//BRAZO DER Z//ANTBRAZO DER Z///PIERNA IZQ X//ANTPIERNA IZQ X//PIERNA DER X//ANTPIERNA X///
		//Brazo Izq
		temp = glm::translate(model, glm::vec3(0.43f, 0.5f, -0.18f));
		temp = glm::rotate(temp, glm::radians(rot_limbs[0]), glm::vec3(0.0f, 1.0f, 0.0f));
		temp = glm::rotate(temp, glm::radians(rot_limbs[1]), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(temp));
		Brazo.Draw(standar);
		//Ant Brazo Izq
		temp = glm::translate(temp, glm::vec3(0.75f, 0.0f, 0.0f));
		temp = glm::rotate(temp, glm::radians(rot_limbs[2]), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(temp));
		Antebrazo.Draw(standar);
		//Brazo Der
		temp = glm::translate(model, glm::vec3(-0.43f, 0.5f, -0.18f));
		temp = glm::rotate(temp, glm::radians(rot_limbs[3]), glm::vec3(0.0f,-1.0f, 0.0f));
		temp = glm::rotate(temp, glm::radians(rot_limbs[4]), glm::vec3(0.0f, 0.0f, 1.0f));
		temp = glm::scale(temp, glm::vec3(-1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(temp));
		Brazo.Draw(standar);
		//Ant Brazo Izq
		temp = glm::translate(temp, glm::vec3(0.75f, 0.0f, 0.0f));
		temp = glm::rotate(temp, glm::radians(rot_limbs[5]), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(temp));
		Antebrazo.Draw(standar);
		//Pierna Izq
		temp = glm::translate(model, glm::vec3(0.25f, -0.7f, 0.0f));
		temp = glm::rotate(temp, glm::radians(rot_limbs[6]), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(temp));
		Pierna.Draw(standar);
		//Ant Pierna Izq
		temp = glm::translate(temp, glm::vec3(0.0f, -0.5f, 0.0f));
		temp = glm::rotate(temp, glm::radians(rot_limbs[7]), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(temp));
		Antepierna.Draw(standar);
		//Pierna Der
		temp = glm::translate(model, glm::vec3(-0.25f, -0.7f, 0.0f));
		temp = glm::rotate(temp, glm::radians(rot_limbs[8]), glm::vec3(1.0f, 0.0f, 0.0f));
		temp = glm::scale(temp, glm::vec3(-1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(temp));
		Pierna.Draw(standar);
		//Ant Pierna Der
		temp = glm::translate(temp, glm::vec3(0.0f, -0.5f, 0.0f));
		temp = glm::rotate(temp, glm::radians(rot_limbs[9]), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(temp));
		Antepierna.Draw(standar);
		////////////////////Dibujando resto del entorno////////////////////
		model = glm::translate(glm::mat4(1), glm::vec3(27.0f, 8.0f, 23.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Bano.Draw(standar);
		model = glm::translate(glm::mat4(1), glm::vec3(rt_Puerta_Bano.value[0], rt_Puerta_Bano.value[1], rt_Puerta_Bano.value[2]));
		model = glm::rotate(model, glm::radians(rt_Puerta_Bano.value[3]), glm::vec3(1.0f, 0.0f, -1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Puerta_Bano.Draw(standar);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, -12.5f))));
		Fogata.Draw(standar);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(1), glm::vec3(-15.3f, 3.0f, 38.7f))));
		Letrero.Draw(standar);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(1), glm::vec3(8.0f, 0.0f, -4.0f))));
		Sillon.Draw(standar);
		model = glm::translate(glm::mat4(1), glm::vec3(-5.0f, 0.8f, -6.75f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Silla.Draw(standar);
		model = glm::translate(glm::mat4(1), glm::vec3( 2.0f, rt_Sillas.value[0], -6.75f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rt_Sillas.value[1]), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Silla.Draw(standar);
		model = glm::translate(glm::mat4(1), glm::vec3(-1.5f, 0.0f, -6.5f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Mesa.Draw(standar);
		model = glm::translate(model, glm::vec3(rt_Ataud.value[0], rt_Ataud.value[1], 0.0f));
		model = glm::rotate(model, glm::radians(rt_Ataud.value[2]), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Ataud.Draw(standar);

		glEnable(GL_BLEND); //Avtiva la funcionalidad para trabajar el canal alfa
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1f(glGetUniformLocation(standar.Program, "alpha"), 0.75f);
		model = glm::translate(model, glm::vec3(0.0f, 0.33f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Cristal.Draw(standar);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(1), glm::vec3(-1.0f, 0.7f, -12.5f))));
		Fuego.Draw(standar);
		glUniform1f(glGetUniformLocation(standar.Program, "alpha"), 1.0f);
		glDisable(GL_BLEND);  //Desactiva el canal alfa 
		glBindVertexArray(0);

		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		skybox.Use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
		glUniformMatrix4fv(glGetUniformLocation(skybox.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skybox.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default
	
		// Swap the screen buffers
		glfwSwapBuffers(window);
	}
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement(){
	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
		camera.ProcessKeyboard(FORWARD, deltaTime);

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
		camera.ProcessKeyboard(BACKWARD, deltaTime);

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
		camera.ProcessKeyboard(LEFT, deltaTime);

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (keys[GLFW_KEY_F]) {
		active = true;
		rt_Pos_Shrek.play = true;
		rt_Puerta_Bano.play = true;
		rt_Puerta_Casa.play = true;
		rt_Sillas.play = true;
		rt_Ataud.play = true;
		rt_SB.play = true;
		rt_WK.play = true;
		rt_VM.play = true;
	}
	if (keys[GLFW_KEY_R])
		resetScene();
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode){
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024){
		if (action == GLFW_PRESS){
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE){
			keys[key] = false;
		}
	}
}

void MouseCallback(GLFWwindow *window, double xPos, double yPos){
	if (firstMouse){
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}
	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left
	lastX = xPos;
	lastY = yPos;
	camera.ProcessMouseMovement(xOffset, yOffset);
}