#include <iostream>
#include <cmath>
#include <Windows.h>
#include <mmsystem.h>
#include <thread>
#include <vector>
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
#include "Donut.h"
// Function prototypes
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement();

// Window dimensions
const GLuint WIDTH = 1200, HEIGHT = 800;
int SCREEN_WIDTH, SCREEN_HEIGHT;
float currLight = 0.7f, targetLight = 0.7f;
float thrX = 0.0f, thrZ = 0.0f;
// Camera
Camera  camera(glm::vec3(-12.5f, 8.0f, 45.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true, active = false, one_shot_0 = true, one_shot_1 = true;
bool lighton[] = { false, false, false }; bool dayCycle = false;

unsigned int valCamera = 0;
// Positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(-1.0f, 0.7f, -12.5f),
	glm::vec3(-7.25, 4.8f, 19.7f),
	glm::vec3(-18.0f, 6.85f, 38.5f),
	glm::vec3(21.5f, 9.8f, 24.7f)
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
RT rt_Lights(3, 4, 450, true), rt_dayCycle(1, 9, 800, true);
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
	rt_Lights.KF[0].var[0] = 0.7f; rt_Lights.KF[1].var[0] = 0.1f; rt_Lights.KF[2].var[0] = 0.1f; rt_Lights.KF[3].var[0] = 0.7f;
	rt_Lights.KF[0].var[1] = 0.0f; rt_Lights.KF[1].var[1] = 90.0f; rt_Lights.KF[2].var[1] = 180.0f; rt_Lights.KF[3].var[1] = 270.0f;
	rt_Lights.KF[0].var[2] = 180.0f; rt_Lights.KF[1].var[2] = 90.0f; rt_Lights.KF[2].var[2] = 0.0f; rt_Lights.KF[3].var[2] = -90.0f;
	rt_Lights.setAtCero();
	rt_dayCycle.KF[0].var[0] = 0.7f; rt_dayCycle.KF[1].var[0] = 0.55f; rt_dayCycle.KF[2].var[0] = 0.4f; rt_dayCycle.KF[3].var[0] = 0.2f;
	rt_dayCycle.KF[4].var[0] = 0.1f; rt_dayCycle.KF[5].var[0] = 0.2f; rt_dayCycle.KF[6].var[0] = 0.4f; rt_dayCycle.KF[7].var[0] = 0.55f;
	rt_dayCycle.KF[8].var[0] = 0.7f; rt_dayCycle.setAtCero();
}

const float degree(const float current, const float goal) {
	if (current > goal)
		return current - 0.0001f;
	else if (current < goal)
		return current + 0.0001f;
	return current;
}

void resetScene() {
	active = false; one_shot_0 = true; one_shot_1 = true; lighton[1] = false; currLight = 0.7f; targetLight = 0.7f; dayCycle = false;
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
	Shader primal("Shaders/prime.vs", "Shaders/prime.frag");

	Model Suelo((char*)"Models/Shrek/Suelo.obj");

	Model Antorcha((char*)"Models/Shrek/Antorcha.obj");
	Model Ataud((char*)"Models/Shrek/Ataud.obj");
	Model Cristal((char*)"Models/Shrek/Ataud_Cristal.obj");
	Model Bano((char*)"Models/Shrek/Bano.obj");
	Model Casa((char*)"Models/Shrek/Casa.obj");
	Model Faro((char*)"Models/Shrek/Faro.obj");
	Model Fuego((char*)"Models/Shrek/Fuego.obj");
	Model Fogata((char*)"Models/Shrek/Fogata.obj");
	Model Letrero((char*)"Models/Shrek/Letrero.obj");
	Model Linterna((char*)"Models/Shrek/Linterna.obj");
	Model Luz((char*)"Models/Shrek/Light.obj");
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
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[1].ambient"), 0.02f, 0.02f, 0.02f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[1].diffuse"), 0.9f, 0.72f, 0.53f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[1].specular"), 0.9f, 0.72f, 0.53f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[1].constant"), 1.0f);
	// Point light 3
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[2].ambient"), 0.02f, 0.02f, 0.02f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[2].diffuse"), 0.9f, 0.72f, 0.53f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[2].specular"), 0.9f, 0.72f, 0.53f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[2].constant"), 1.0f);
	// Point light 4
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[3].position"), pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[3].ambient"), 0.02f, 0.02f, 0.02f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[3].diffuse"), 0.9f, 0.72f, 0.53f);
	glUniform3f(glGetUniformLocation(standar.Program, "pointLights[3].specular"), 0.9f, 0.72f, 0.53f);
	glUniform1f(glGetUniformLocation(standar.Program, "pointLights[3].constant"), 1.0f);

	// SpotLight
	glUniform3f(glGetUniformLocation(standar.Program, "spotLight[0].position"), 5.0f, 5.0f, 4.0f);

	glUniform3f(glGetUniformLocation(standar.Program, "spotlight[0].ambient"), 0.2f, 0.2f, 0.2f);
	glUniform3f(glGetUniformLocation(standar.Program, "spotlight[0].diffuse"), 0.9f, 0.72f, 0.53f);
	glUniform3f(glGetUniformLocation(standar.Program, "spotlight[0].specular"), 0.9f, 0.72f, 0.53f);
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight[0].linear"), 0.045f);
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight[0].quadratic"), 0.075f);
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight[0].cutOff"), glm::cos(glm::radians(0.0f)));
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight[0].outerCutOff"), glm::cos(glm::radians(40.0f)));

	glUniform3f(glGetUniformLocation(standar.Program, "spotLight[1].position"), 25.0f, 13.7f, 23.3f);
	glUniform3f(glGetUniformLocation(standar.Program, "spotlight[1].ambient"), 0.2f, 0.2f, 0.2f);
	glUniform3f(glGetUniformLocation(standar.Program, "spotlight[1].diffuse"), 0.9f, 0.72f, 0.53f);
	glUniform3f(glGetUniformLocation(standar.Program, "spotlight[1].specular"), 0.9f, 0.72f, 0.53f);
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight[1].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight[1].linear"), 0.045f);
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight[1].quadratic"), 0.075f);
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight[1].cutOff"), glm::cos(glm::radians(0.0f)));
	glUniform1f(glGetUniformLocation(standar.Program, "spotLight[1].outerCutOff"), glm::cos(glm::radians(40.0f)));
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

	//Implementacion del Donut
	const unsigned int IQ = getRealSubDiv(4), FQ = getRealSubDiv(4);
	std::vector<float> vertices;
	getVertexArray(&vertices, 5.0f, 5.0f, IQ, FQ);
	std::vector<int> indices;
	getIndexArray(&indices, IQ, FQ);

	float pVert[] = {	5.0f, 0.0f, 0.0f,
						5.5f, 2.93893f, 0.0f,
						6.0f, 4.75528f, 0.0f,
						6.5f, 4.75528f, 0.0f,
						7.0f, 2.93893f, 0.0f,
						7.5f, 0.0f, 0.0f,
						8.0f, -2.93893f, 0.0f,
						8.5f, -4.75528f, 0.0f,
						9.0f, -4.75528f, 0.0f,
						9.5f, -2.93892f, 0.0f,
						4.04508f, 0.0f, 2.93893f,
						4.44959f, 2.93893f, 3.23282f,
						4.8541f, 4.75528f, 3.52671f,
						5.25861f, 4.75528f, 3.8206f,
						5.66312f, 2.93893f, 4.1145f,
						6.06763f, 0.0f, 4.40839f,
						6.47214f, -2.93893f, 4.70228f,
						6.87664f, -4.75528f, 4.99617f,
						7.28115f, -4.75528f, 5.29007f,
						7.68566f, -2.93892f, 5.58396f,
						1.54508f, 0.0f, 4.75528f,
						1.69959f, 2.93893f, 5.23081f,
						1.8541f, 4.75528f, 5.70634f,
						2.00861f, 4.75528f, 6.18187f,
						2.16312f, 2.93893f, 6.6574f,
						2.31763f, 0.0f, 7.13292f,
						2.47214f, -2.93893f, 7.60845f,
						2.62664f, -4.75528f, 8.08398f,
						2.78115f, -4.75528f, 8.55951f,
						2.93566f, -2.93892f, 9.03504f,
						-1.54509f, 0.0f, 4.75528f,
						-1.69959f, 2.93893f, 5.23081f,
						-1.8541f, 4.75528f, 5.70634f,
						-2.00861f, 4.75528f, 6.18187f,
						-2.16312f, 2.93893f, 6.6574f,
						-2.31763f, 0.0f, 7.13292f,
						-2.47214f, -2.93893f, 7.60845f,
						-2.62664f, -4.75528f, 8.08398f,
						-2.78115f, -4.75528f, 8.55951f,
						-2.93566f, -2.93892f, 9.03504f,
						-4.04509f, 0.0f, 2.93893f,
						-4.44959f, 2.93893f, 3.23282f,
						-4.8541f, 4.75528f, 3.52671f,
						-5.25861f, 4.75528f, 3.8206f,
						-5.66312f, 2.93893f, 4.1145f,
						-6.06763f, 0.0f, 4.40839f,
						-6.47214f, -2.93893f, 4.70228f,
						-6.87665f, -4.75528f, 4.99617f,
						-7.28115f, -4.75528f, 5.29007f,
						-7.68566f, -2.93892f, 5.58396f,
						-5.0f, 0.0f, 0.0f,
						-5.5f, 2.93893f, 0.0f,
						-6.0f, 4.75528f, 0.0f,
						-6.5f, 4.75528f, 0.0f,
						-7.0f, 2.93893f, 0.0f,
						-7.5f, 0.0f, 0.0f,
						-8.0f, -2.93893f, 0.0f,
						-8.5f, -4.75528f, 0.0f,
						-9.0f, -4.75528f, 0.0f,
						-9.5f, -2.93892f, 0.0f,
						-4.04508f, 0.0f, -2.93893f,
						-4.44959f, 2.93893f, -3.23282f,
						-4.8541f, 4.75528f, -3.52671f,
						-5.25861f, 4.75528f, -3.8206f,
						-5.66312f, 2.93893f, -4.1145f,
						-6.06763f, 0.0f, -4.40839f,
						-6.47214f, -2.93893f, -4.70228f,
						-6.87664f, -4.75528f, -4.99618f,
						-7.28115f, -4.75528f, -5.29007f,
						-7.68566f, -2.93892f, -5.58396f,
						-1.54509f, 0.0f, -4.75528f,
						-1.69959f, 2.93893f, -5.23081f,
						-1.8541f, 4.75528f, -5.70634f,
						-2.00861f, 4.75528f, -6.18187f,
						-2.16312f, 2.93893f, -6.6574f,
						-2.31763f, 0.0f, -7.13292f,
						-2.47214f, -2.93893f, -7.60845f,
						-2.62665f, -4.75528f, -8.08398f,
						-2.78115f, -4.75528f, -8.55951f,
						-2.93566f, -2.93892f, -9.03504f,
						1.54509f, 0.0f, -4.75528f,
						1.69959f, 2.93893f, -5.23081f,
						1.8541f, 4.75528f, -5.70634f,
						2.00861f, 4.75528f, -6.18187f,
						2.16312f, 2.93893f, -6.6574f,
						2.31763f, 0.0f, -7.13292f,
						2.47214f, -2.93893f, -7.60845f,
						2.62665f, -4.75528f, -8.08398f,
						2.78115f, -4.75528f, -8.55951f,
						2.93566f, -2.93892f, -9.03504f,
						4.04509f, 0.0f, -2.93892f,
						4.44959f, 2.93893f, -3.23282f,
						4.8541f, 4.75528f, -3.52671f,
						5.25861f, 4.75528f, -3.8206f,
						5.66312f, 2.93893f, -4.11449f,
						6.06763f, 0.0f, -4.40839f,
						6.47214f, -2.93893f, -4.70228f,
						6.87665f, -4.75528f, -4.99617f,
						7.28116f, -4.75528f, -5.29006f,
						7.68566f, -2.93892f, -5.58396f, };
	int pIndx[] =  {	0, 11, 10,
						1, 11, 0,
						1, 12, 11,
						2, 12, 1,
						2, 13, 12,
						3, 13, 2,
						3, 14, 13,
						4, 14, 3,
						4, 15, 14,
						5, 15, 4,
						5, 16, 15,
						6, 16, 5,
						6, 17, 16,
						7, 17, 6,
						7, 18, 17,
						8, 18, 7,
						8, 19, 18,
						9, 19, 8,
						9, 10, 19,
						0, 10, 9,
						10, 21, 20,
						11, 21, 10,
						11, 22, 21,
						12, 22, 11,
						12, 23, 22,
						13, 23, 12,
						13, 24, 23,
						14, 24, 13,
						14, 25, 24,
						15, 25, 14,
						15, 26, 25,
						16, 26, 15,
						16, 27, 26,
						17, 27, 16,
						17, 28, 27,
						18, 28, 17,
						18, 29, 28,
						19, 29, 18,
						19, 20, 29,
						10, 20, 19,
						20, 31, 30,
						21, 31, 20,
						21, 32, 31,
						22, 32, 21,
						22, 33, 32,
						23, 33, 22,
						23, 34, 33,
						24, 34, 23,
						24, 35, 34,
						25, 35, 24,
						25, 36, 35,
						26, 36, 25,
						26, 37, 36,
						27, 37, 26,
						27, 38, 37,
						28, 38, 27,
						28, 39, 38,
						29, 39, 28,
						29, 30, 39,
						20, 30, 29,
						30, 41, 40,
						31, 41, 30,
						31, 42, 41,
						32, 42, 31,
						32, 43, 42,
						33, 43, 32,
						33, 44, 43,
						34, 44, 33,
						34, 45, 44,
						35, 45, 34,
						35, 46, 45,
						36, 46, 35,
						36, 47, 46,
						37, 47, 36,
						37, 48, 47,
						38, 48, 37,
						38, 49, 48,
						39, 49, 38,
						39, 40, 49,
						30, 40, 39,
						40, 51, 50,
						41, 51, 40,
						41, 52, 51,
						42, 52, 41,
						42, 53, 52,
						43, 53, 42,
						43, 54, 53,
						44, 54, 43,
						44, 55, 54,
						45, 55, 44,
						45, 56, 55,
						46, 56, 45,
						46, 57, 56,
						47, 57, 46,
						47, 58, 57,
						48, 58, 47,
						48, 59, 58,
						49, 59, 48,
						49, 50, 59,
						40, 50, 49,
						50, 61, 60,
						51, 61, 50,
						51, 62, 61,
						52, 62, 51,
						52, 63, 62,
						53, 63, 52,
						53, 64, 63,
						54, 64, 53,
						54, 65, 64,
						55, 65, 54,
						55, 66, 65,
						56, 66, 55,
						56, 67, 66,
						57, 67, 56,
						57, 68, 67,
						58, 68, 57,
						58, 69, 68,
						59, 69, 58,
						59, 60, 69,
						50, 60, 59,
						60, 71, 70,
						61, 71, 60,
						61, 72, 71,
						62, 72, 61,
						62, 73, 72,
						63, 73, 62,
						63, 74, 73,
						64, 74, 63,
						64, 75, 74,
						65, 75, 64,
						65, 76, 75,
						66, 76, 65,
						66, 77, 76,
						67, 77, 66,
						67, 78, 77,
						68, 78, 67,
						68, 79, 78,
						69, 79, 68,
						69, 70, 79,
						60, 70, 69,
						70, 81, 80,
						71, 81, 70,
						71, 82, 81,
						72, 82, 71,
						72, 83, 82,
						73, 83, 72,
						73, 84, 83,
						74, 84, 73,
						74, 85, 84,
						75, 85, 74,
						75, 86, 85,
						76, 86, 75,
						76, 87, 86,
						77, 87, 76,
						77, 88, 87,
						78, 88, 77,
						78, 89, 88,
						79, 89, 78,
						79, 80, 89,
						70, 80, 79,
						80, 91, 90,
						81, 91, 80,
						81, 92, 91,
						82, 92, 81,
						82, 93, 92,
						83, 93, 82,
						83, 94, 93,
						84, 94, 83,
						84, 95, 94,
						85, 95, 84,
						85, 96, 95,
						86, 96, 85,
						86, 97, 96,
						87, 97, 86,
						87, 98, 97,
						88, 98, 87,
						88, 99, 98,
						89, 99, 88,
						89, 90, 99,
						80, 90, 89,
						90, 1, 0,
						91, 1, 90,
						91, 2, 1,
						92, 2, 91,
						92, 3, 2,
						93, 3, 92,
						93, 4, 3,
						94, 4, 93,
						94, 5, 4,
						95, 5, 94,
						95, 6, 5,
						96, 6, 95,
						96, 7, 6,
						97, 7, 96,
						97, 8, 7,
						98, 8, 97,
						98, 9, 8,
						99, 9, 98, };
	
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Enlazar  Vertex Array Object
	glBindVertexArray(VAO);

	//2.- Copiamos nuestros arreglo de vertices en un buffer de vertices para que OpenGL lo use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pVert), pVert, GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 2, vertices.data(), GL_STATIC_DRAW);
	// 3.Copiamos nuestro arreglo de indices en  un elemento del buffer para que OpenGL lo use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pIndx), pIndx, GL_STATIC_DRAW);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int) * 2, indices.data(), GL_STATIC_DRAW);

	// 4. Despues colocamos las caracteristicas de los vertices

	//Posicion
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0); // Desenlazamos de memoria el VAO

	setAnim();
	rt_Lights.play = true;
	float random[] = { 0.0f, 0.0f, 0.0f, 0.0f} , deg[] = { 0.025f, 0.025f, 0.025f, 0.025f};
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


		rt_Lights.animacion();
		rt_dayCycle.animacion();
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
				lighton[2] = true;
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
				targetLight = 0.1f;
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
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model(1); glm::mat4 temp(1);

		primal.Use();
		GLint modelLoc = glGetUniformLocation(primal.Program, "model");
		GLint viewLoc = glGetUniformLocation(primal.Program, "view");
		GLint projLoc = glGetUniformLocation(primal.Program, "projection");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		model = glm::translate(glm::mat4(1), glm::vec3(-0.65f, 2.5f, -6.5f));
		model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, indices.size()*2 , GL_UNSIGNED_INT, (GLvoid*)(0 * sizeof(GLfloat)));
		//glPointSize(15);
		//glDrawArrays(GL_POINTS, 0, vertices.size()*2 );
		glBindVertexArray(0);

		standar.Use();
		GLint viewPosLoc = glGetUniformLocation(standar.Program, "viewPos");
		switch (valCamera) {
			case 0:
				break;
			case 1:
				camera.position = glm::vec3(6.8f, 6.7f, 0.0f);
				break;
			case 2:
				camera.position = glm::vec3(-6.6f, 5.0f, -7.8f);
				break;
			case 3:
				
				camera.position = glm::vec3(rt_Pos_Shrek.value[0] - 1.0f, rt_Pos_Shrek.value[1] + 1.5f, rt_Pos_Shrek.value[1] - 2.0f);
				break;
		}
			
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		for(int i = 0; i < 4 ; i++)
			if (abs((0.0125f + random[i]) - degree(deg[i], 0.0125f + random[i])) < 0.00001)
				random[i] = (float)(std::rand() % 1000) / 10000.0f;
			else
				deg[i] = degree(deg[i], 0.0125 + random[i]);
		glUniform1f(glGetUniformLocation(standar.Program, "pointLights[0].linear"),  deg[0]);
		glUniform1f(glGetUniformLocation(standar.Program, "pointLights[0].quadratic"), deg[0] / 2.0f);
		glUniform1f(glGetUniformLocation(standar.Program, "pointLights[1].linear"), (lighton[0] ? deg[1] : 1.0f));
		glUniform1f(glGetUniformLocation(standar.Program, "pointLights[1].quadratic"), (lighton[0] ? deg[1] / 2.0f : 1.0f));
		glUniform1f(glGetUniformLocation(standar.Program, "pointLights[2].linear"), (lighton[1] ? deg[2] : 1.0f));
		glUniform1f(glGetUniformLocation(standar.Program, "pointLights[2].quadratic"), (lighton[1] ? deg[2] / 2.0f : 1.0f));
		glUniform1f(glGetUniformLocation(standar.Program, "pointLights[3].linear"), (lighton[2] ? deg[3] : 1.0f));
		glUniform1f(glGetUniformLocation(standar.Program, "pointLights[3].quadratic"), (lighton[2] ? deg[3] / 2.0f : 1.0f));

		glUniform3f(glGetUniformLocation(standar.Program, "spotLight[0].direction"), glm::radians(45.0f), glm::radians(rt_Lights.value[1]), glm::radians(45.0f));
		glUniform3f(glGetUniformLocation(standar.Program, "spotLight[1].direction"), glm::radians(45.0f), glm::radians(rt_Lights.value[2]), glm::radians(45.0f));
		
		// Global Light
		if (dayCycle)
			currLight = rt_dayCycle.value[0];
		else
			currLight = degree(currLight, targetLight);
		lighton[0] = currLight < 0.4f;
		glUniform3f(glGetUniformLocation(standar.Program, "dirLight.ambient"), currLight, currLight, currLight);

		// Create camera transformations
		model = glm::mat4(1);
		// Get the uniform locations
		modelLoc = glGetUniformLocation(standar.Program, "model");
		viewLoc = glGetUniformLocation(standar.Program, "view");
		projLoc = glGetUniformLocation(standar.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Casa.Draw(standar);
		Suelo.Draw(standar);
		model = glm::translate(glm::mat4(1), glm::vec3(-3.145f, 2.0f, 1.3f));
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
		model = glm::translate(glm::mat4(1), pointLightPositions[1]);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Faro.Draw(standar);
		model = glm::translate(glm::mat4(1), pointLightPositions[2]);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Faro.Draw(standar);
		model = glm::translate(glm::mat4(1), pointLightPositions[3]);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Antorcha.Draw(standar);
		model = glm::translate(glm::mat4(1), glm::vec3(5.0f, 5.0f, 4.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Linterna.Draw(standar);
		model = glm::translate(glm::mat4(1), glm::vec3(25.0f, 13.7f, 23.3f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Linterna.Draw(standar);
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
		if (lighton[0]) {
			model = glm::translate(glm::mat4(1), pointLightPositions[1]);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			Luz.Draw(standar);
		}
		if (lighton[1]) {
			model = glm::translate(glm::mat4(1), pointLightPositions[2]);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			Luz.Draw(standar);
		}
		if (lighton[2]) {
			model = glm::translate(glm::mat4(1), pointLightPositions[3]);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			Luz.Draw(standar);
		}
		model = glm::translate(glm::mat4(1), glm::vec3(5.0f, 5.0f, 4.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Luz.Draw(standar);
		model = glm::translate(glm::mat4(1), glm::vec3(25.0f, 13.7f, 23.3f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Luz.Draw(standar); 
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

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) {
		camera.ProcessKeyboard(LEFT, deltaTime);
		thrX += 0.25f * deltaTime;
	}
	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
		thrX -= 0.25f * deltaTime;
	}

	if (keys[GLFW_KEY_1])
		valCamera = 0;
	if (keys[GLFW_KEY_2])
		valCamera = 1;
	if (keys[GLFW_KEY_3])
		valCamera = 2;
	if (keys[GLFW_KEY_4])
		valCamera = 3;

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
	if (keys[GLFW_KEY_G])
		lighton[1] = true;
	if (keys[GLFW_KEY_H])
		lighton[1] = false;
	if (keys[GLFW_KEY_T]) {
		dayCycle = true;
		rt_dayCycle.play = true;
	}
		
	if (keys[GLFW_KEY_Y]) {
		dayCycle = false;
		rt_dayCycle.play = false;
	}
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