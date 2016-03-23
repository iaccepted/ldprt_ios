//**********************************************
//Singleton Texture Manager class
//Written by Ben English
//benjamin.english@oit.edu
//
//For use with OpenGL and the FreeImage library
//**********************************************

#include "TextureManager.h"

TextureManager* TextureManager::m_inst(0);

TextureManager* TextureManager::Inst()
{
	if (!m_inst)
		m_inst = new TextureManager();

	return m_inst;
}

TextureManager::TextureManager()
{
	// call this ONLY when linking with FreeImage as a static library
#ifdef FREEIMAGE_LIB
	FreeImage_Initialise();
#endif
}

//these should never be called
//TextureManager::TextureManager(const TextureManager& tm){}
//TextureManager& TextureManager::operator=(const TextureManager& tm){}

TextureManager::~TextureManager()
{
	// call this ONLY when linking with FreeImage as a static library
#ifdef FREEIMAGE_LIB
	FreeImage_DeInitialise();
#endif

	UnloadAllTextures();
	m_inst = 0;
}

bool TextureManager::LoadTexture(const char* filename, unsigned int &texID, GLenum image_format, GLint internal_format, GLint level, GLint border)
{
	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	//pointer to the image data
	BYTE* bits(0);
	//image width and height
	unsigned int width(0), height(0);


	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(filename, 0);
	//if still unknown, try to guess the file format from the file extension
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(filename);

	//if still unkown, return failure
	if (fif == FIF_UNKNOWN)
		return false;

	//check that the plugin has reading capabilities and load the file
	if (FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, filename);
	//if the image failed to load, return failure
	if (!dib)
		return false;

	//retrieve the image data
	bits = FreeImage_GetBits(dib);
	//get the image width and height
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	//if this somehow one of these failed (they shouldn't), return failure
	if ((bits == 0) || (width == 0) || (height == 0))
		return false;

	if (glIsTexture(texID) == GL_TRUE)
	{
		glDeleteTextures(1, &texID);
	}

	//generate an OpenGL texture ID for this texture
	glGenTextures(1, &texID);
	//store the texture ID mapping
	m_texID.push_back(texID);
	//bind to the new texture ID
	glBindTexture(GL_TEXTURE_2D, texID);
	//store the texture data for OpenGL use

	glTexImage2D(GL_TEXTURE_2D, level, internal_format, width, height,
		border, image_format, GL_UNSIGNED_BYTE, bits);

	//Free FreeImage's copy of the data
	FreeImage_Unload(dib);

	//return success
	return true;
}

bool TextureManager::UnloadTexture(const unsigned int texID)
{
	bool result(true);

	if (glIsTexture(texID) == GL_TRUE)
	{
		glDeleteTextures(1, &texID);
	}
	else
	{
		result = false;
	}

	return result;
}

bool TextureManager::BindTexture(const unsigned int texID)
{
	bool result(true);
	//if this texture ID mapped, bind it's texture as current
	if (glIsTexture(texID) == GL_TRUE)
	{
		glBindTexture(GL_TEXTURE_2D, texID);
	}

	//otherwise, binding failed
	else
		result = false;

	return result;
}

void TextureManager::UnloadAllTextures()
{
	//start at the begginning of the texture map
	auto i = m_texID.begin();

	//Unload the textures untill the end of the texture map is found
	while (i != m_texID.end())
		UnloadTexture(*i);

	//clear the texture map
	m_texID.clear();
}

bool TextureManager::saveImage(const int width, const int height, const int channel, const char *imagePath)
{
	unsigned char *mpixels = new unsigned char[width * height * 4];
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, mpixels);
	glReadBuffer(GL_BACK);
	for (int i = 0; i<(int)width * height * channel; i += 4)
	{

		mpixels[i] ^= mpixels[i + 2] ^= mpixels[i] ^= mpixels[i + 2];
	}
	FIBITMAP* bitmap = FreeImage_Allocate(width, height, 32, 8, 8, 8);

	for (int y = 0; y<FreeImage_GetHeight(bitmap); y++)
	{
		BYTE *bits = FreeImage_GetScanLine(bitmap, y);
		for (int x = 0; x<FreeImage_GetWidth(bitmap); x++)
		{
			bits[0] = mpixels[(y * width + x) * 4 + 0];
			bits[1] = mpixels[(y * width + x) * 4 + 1];
			bits[2] = mpixels[(y * width + x) * 4 + 2];
			bits[3] = 255;
			bits += 4;
		}

	}
	bool bSuccess = FreeImage_Save(FIF_PNG, bitmap, imagePath, PNG_DEFAULT);
	FreeImage_Unload(bitmap);

	return bSuccess;
}