#include <opencv2\opencv.hpp>
#include "stdafx.h"
using namespace cv;
#pragma once

#ifndef CVVIMAGE_CLASS_DEF

#define CVVIMAGE_CLASS_DEF
#include <opencv2\opencv.hpp>
using namespace cv;
class  CvvImage

{

public:

    CvvImage();

    virtual ~CvvImage();



    virtual bool  Create( int width, int height, int bits_per_pixel, int image_origin = 0 );



    virtual bool  Load( const char* filename, int desired_color = 1 );



    virtual bool  LoadRect( const char* filename,

        int desired_color, CvRect r );

#if defined WIN32 || defined _WIN32

    virtual bool  LoadRect( const char* filename,

        int desired_color, RECT r )

    {

        return LoadRect( filename, desired_color,

            cvRect( r.left, r.top, r.right - r.left, r.bottom - r.top ));

    }

#endif



    virtual bool  Save( const char* filename );



    virtual void  CopyOf( CvvImage& image, int desired_color = -1 );

    virtual void  CopyOf( IplImage* img, int desired_color = -1 );

    IplImage* GetImage() { return m_img; };

    virtual void  Destroy(void);



    int Width() { return !m_img ? 0 : !m_img->roi ? m_img->width : m_img->roi->width; };

    int Height() { return !m_img ? 0 : !m_img->roi ? m_img->height : m_img->roi->height;};

    int Bpp() { return m_img ? (m_img->depth & 255)*m_img->nChannels : 0; };

    virtual void  Fill( int color );



    virtual void  Show( const char* window );



#if defined WIN32 || defined _WIN32



    virtual void  Show( HDC dc, int x, int y, int width, int height,

        int from_x = 0, int from_y = 0 );



    virtual void  DrawToHDC( HDC hDCDst, RECT* pDstRect );

#endif

protected:

    IplImage*  m_img;

};

//typedef CvvImage CImage;
namespace cv
{
    typedef CvvImage CImage;
}
#endif