/**********************************************
*author:guo hongzhi
*based on FreeImage lib
***********************************************/

#ifndef TextureManager_H
#define TextureManager_H

#include "FreeImage.h"
#include "Global.h"

using namespace std;

class TextureManager
{
public:
	static TextureManager* Inst();
	virtual ~TextureManager();

	//load a texture an make it the current texture
	//if texID is already in use, it will be unloaded and replaced with this texture
	bool LoadTexture(const char* filename,	//where to load the file from
		unsigned int &texID,			//arbitrary id you will reference the texture by
		//does not have to be generated with glGenTextures
		GLenum image_format = GL_RGB,		//format the image is in
		GLint internal_format = GL_RGB,		//format to store the image in
		GLint level = 0,					//mipmapping level
		GLint border = 0);					//border size

	//free the memory for a texture
	bool UnloadTexture(const unsigned int texID);

	//set the current texture
	bool BindTexture(const unsigned int texID);

	//free all texture memory
	void UnloadAllTextures();

	//save image
	bool saveImage(const int width, const int height, const int channel, const char *imagePath);

protected:
	TextureManager();
	TextureManager(const TextureManager& tm);
	TextureManager& operator=(const TextureManager& tm);

	static TextureManager* m_inst;
	//std::map<unsigned int, GLuint> m_texID;
	std::vector<GLuint> m_texID;
};

#endif