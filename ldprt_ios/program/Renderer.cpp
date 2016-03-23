#include "Renderer.h"


LIGHTING_TYPE Renderer::lightType = LIGHTING_TYPE_SH_UNSHADOWED;
SHOW_TYPE Renderer::showType = SHOW_TYPE_ORIGINAL;
//double Renderer::theta = 101.7f;
//double Renderer::phi = 90.7f;
double Renderer::theta = 167.1f;
double Renderer::phi = 186.2f;
double Renderer::preMouseX = 0.0f;
double Renderer::preMouseY = 0.0f;
bool Renderer::isNeedRotate = false;
bool Renderer::isLeftButtonPress = false;

Renderer::Renderer(int _width, int _height) : width(_width), height(_height)
{
    initGL();
}

void Renderer::initGL()
{
    glViewport(0, 0, width, height);
    /* Set the background */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    /* Depth buffer setup */
    glClearDepthf(1.0f);
    /* Enables Depth Testing */
    glEnable(GL_DEPTH_TEST);
    /* The Type Of Depth Test To Do */
    glDepthFunc(GL_LEQUAL);
    
    modelMatrix = mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, vec3(0.2f, 0.2f, 0.2f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), vec3(0.0, 1.0, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(25.0f), vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, vec3(0.0f, -360.0f, 0.0f));
    
    viewMatrix = glm::lookAt(vec3(0.0f, 0.0f, 10.0f), vec3(0.0f, 0.0f, -10.0f), vec3(0.0f, 1.0f, 0.0f));
}

void Renderer::renderSceneWithLight(Scene &scene, Light &light)
{

    this->precomputeColor(scene, light);

    unsigned nObjects = scene.objects.size();

    GLuint vao, vbo, ebo;
    

    for (unsigned i = 0; i < nObjects; ++i)
    {
        Object *curObject = scene.objects[i];
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)* curObject->vertices.size(), &curObject->vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* curObject->indices.size(), &curObject->indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, litColor));
        glEnableVertexAttribArray(1);

        glDrawElements(GL_TRIANGLES, curObject->indices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }
}



void Renderer::compileShaderFromFile(const char *verFileName, const char *fragFileName)
{
	bool ret = prog.compileShaderFromFile(verFileName, GLSLShader::VERTEX);
	if (!ret)
	{
		cerr << "Compile Vertex Shader Error: " << prog.log() << endl;
		exit(-1);
	}
	ret = prog.compileShaderFromFile(fragFileName, GLSLShader::FRAGMENT);
	if (!ret)
	{
		cerr << "Compile Fragment Shader Error: " << prog.log() << endl;
		exit(-1);
	}

	prog.bindAttribLocation(0, "position");
	prog.bindAttribLocation(1, "color");

	ret = prog.link();
	if (!ret)
	{
		cerr << "Link Error: " << prog.log() << endl;
		exit(-1);
	}

	prog.use();
}

void Renderer::setUniform()
{
	mat4 projection = glm::perspective(45.0f, float(width) / height, 0.1f, 1000.0f);
	mat4 mv = viewMatrix * modelMatrix;

	prog.setUniform("MVP", projection * mv);
}

void Renderer::precomputeColor(Scene &scene, Light &light)
{
	lightType = LIGHTING_TYPE_SH_SHADOWED;
	//
	unsigned nObjects = scene.objects.size();
	unsigned nFuncs = BAND_NUM * BAND_NUM;

	LFLOAT rmsr = 0.0, rmsg = 0.0, rmsb = 0.0;

	SHEvalFunc SHEval[] = { SHEval3, SHEval3, SHEval3, SHEval3, SHEval4, SHEval5, SHEval6, SHEval7, SHEval8, SHEval9, SHEval10 };

	LFLOAT *tApproximateCoeffs = new LFLOAT[nFuncs];
	LFLOAT *tShValues = new LFLOAT[nFuncs];

	if (lightType == LIGHTING_TYPE_SH_UNSHADOWED)
	{
		for (unsigned objIdx = 0; objIdx < nObjects; ++objIdx)
		{
			Object *curObject = scene.objects[objIdx];

			unsigned nVertices = curObject->vertices.size();

			for (unsigned verIdx = 0; verIdx < nVertices; ++verIdx)
			{
				Vertex &curVertex = curObject->vertices[verIdx];
				
				vec3 litColor = vec3(0.0f);
				for (unsigned l = 0; l < nFuncs; ++l)
				{
					litColor.r += light.rotatedCoeffs[0][l] * curVertex.unshadowedCoeffs[l];
					litColor.g += light.rotatedCoeffs[1][l] * curVertex.unshadowedCoeffs[l];
					litColor.b += light.rotatedCoeffs[2][l] * curVertex.unshadowedCoeffs[l];
				}

				curVertex.litColor = litColor;
			}
		}
	}
	else if (lightType == LIGHTING_TYPE_SH_SHADOWED)
	{
		for (unsigned objIdx = 0; objIdx < nObjects; ++objIdx)
		{
			Object *curObject = scene.objects[objIdx];

			unsigned nVertices = curObject->vertices.size();

			for (unsigned verIdx = 0; verIdx < nVertices; ++verIdx)
			{
				Vertex &curVertex = curObject->vertices[verIdx];

				memset(tApproximateCoeffs, 0, nFuncs * sizeof(LFLOAT));
				//fit SH coeffs
				for (unsigned lobeIdx = 0; lobeIdx < LOBE_NUM; ++lobeIdx)
				{
					LFLOAT *lobe = curVertex.lobes[lobeIdx];
					SHEval[BAND_NUM](lobe[0], lobe[1], lobe[2], tShValues);

					for (unsigned l = 0; l < nFuncs; ++l)
					{
						tApproximateCoeffs[l] += curVertex.lobes[lobeIdx][l + 3] * tShValues[l];
					}
				}

				vec3 litColor = vec3(0.0f, 0.0f, 0.0f);
				LFLOAT ocr = 0.0, ocg = 0.0, ocb = 0.0;

				for (unsigned l = 0; l < nFuncs; ++l)
				{
					ocr += light.rotatedCoeffs[0][l] * curVertex.shadowedCoeffs[0][l];
					ocg += light.rotatedCoeffs[1][l] * curVertex.shadowedCoeffs[0][l];
					ocb += light.rotatedCoeffs[2][l] * curVertex.shadowedCoeffs[0][l];
				}
				for (unsigned l = 0; l < nFuncs; ++l)
				{
					litColor.r += light.rotatedCoeffs[0][l] * tApproximateCoeffs[l];
					litColor.g += light.rotatedCoeffs[1][l] * tApproximateCoeffs[l];
					litColor.b += light.rotatedCoeffs[2][l] * tApproximateCoeffs[l];
				}

				rmsr += (litColor.r - ocr) * (litColor.r - ocr);
				rmsg += (litColor.g - ocg) * (litColor.g - ocg);
				rmsb += (litColor.b - ocb) * (litColor.b - ocb);


				if (showType == SHOW_TYPE_ORIGINAL)
				{
					litColor = vec3(ocr, ocg, ocb);
				}
				

				curVertex.litColor = litColor;				
			}
		}
		static bool mark = true;
		if (mark)
		{
			rmsr = sqrt(rmsr / scene.numAllVertices);
			rmsg = sqrt(rmsg / scene.numAllVertices);
			rmsb = sqrt(rmsg / scene.numAllVertices);

			cout << endl;
			cout << "RMS_R: " << rmsr << endl;
			cout << "RMS_G: " << rmsg << endl;
			cout << "RMS_B: " << rmsb << endl;
			mark = false;
		}
		
	}
	else
	{
		int bounceTime = lightType - 1;
		for (unsigned objIdx = 0; objIdx < nObjects; ++objIdx)
		{
			Object *curObject = scene.objects[objIdx];

			unsigned nVertices = curObject->vertices.size();

			for (unsigned verIdx = 0; verIdx < nVertices; ++verIdx)
			{
				Vertex &curVertex = curObject->vertices[verIdx];

				vec3 litColor = vec3(0.0f, 0.0f, 0.0f);

				unsigned nFuncs = BAND_NUM * BAND_NUM;
				for (unsigned l = 0; l < nFuncs; ++l)
				{
					litColor.r += light.rotatedCoeffs[0][l] * curVertex.shadowedCoeffs[bounceTime][l];
					litColor.g += light.rotatedCoeffs[1][l] * curVertex.shadowedCoeffs[bounceTime][l];
					litColor.b += light.rotatedCoeffs[2][l] * curVertex.shadowedCoeffs[bounceTime][l];
				}

				curVertex.litColor = litColor;
			}
		}
	}
}

Renderer::~Renderer()
{

}