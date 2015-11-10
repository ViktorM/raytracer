#pragma once

#include "common/common.h"
#include "FreeImage.h"
// Image Writer Class
// Use the FreeImage library to write an image to a file
// Assume (0, 0) is the top left of the image.
class ImageWriter
{
public:
    ImageWriter(std::string, int, int);
    ~ImageWriter();

	// No copy yet just initializing
	ImageWriter& operator = (ImageWriter other)
	{
		delete[] mHDRData;
		FreeImage_DeInitialise();

		mWidth = other.mWidth;
		mHeight = other.mHeight;

		// Initialize Free Image and get it ready to do stuff
		FreeImage_Initialise();

		// Create Bitmap and check if it's valid
		// Hard-code bits per pixel to 24 for now since we're just doing RBG (no alpha)
		m_pOutBitmap = FreeImage_Allocate(mWidth, mHeight, 24);
		mHDRData = new glm::vec3[mWidth * mHeight];

		if (!m_pOutBitmap)
		{
			throw std::runtime_error("ERROR: Bitmap failed to initialize.");
			*this;
		}

		m_sFileName = other.m_sFileName;

		return *this;
	}

    glm::vec3 GetHDRPixelColor(int inX, int inY) const;
    // this function will stored in a float array to support HDR.
    void SetPixelColor(glm::vec3, int, int);

    void CopyHDRToBitmap();
    // Assume color will be passed in as a 0-1 float
    void SetFinalPixelColor(glm::vec3, int, int);

    // Explicit Call to Finish and Save File -- Otherwise done at destructor
    void SaveImage();

private:
    // File name that we want to output to
    std::string m_sFileName;
    int mWidth;
    int mHeight;

    // Float data
    glm::vec3*	mHDRData;

    // Bitmap file
    FIBITMAP*	m_pOutBitmap;
};
