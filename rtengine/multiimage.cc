#include "multiimage.h"
#include <string.h>
#include "iccstore.h"
#include <math.h>
#include "macros.h"
#include "image16.h"

#undef XYZ_MAXVAL
#define XYZ_MAXVAL 2*65536-1

namespace rtengine {

int* MultiImage::cacheL;
int* MultiImage::cacheab;
int* MultiImage::ycache;
int* MultiImage::xzcache;
bool MultiImage::labConversionCacheInitialized = false;

MultiImage::MultiImage (int w, int h, ColorSpace cs)
	: data(NULL), width(w), height(h), colorSpace(cs), allocWidth(w), allocHeight(h) {

    ch[0] = ch[1] = ch[2] = NULL;

	initArrays ();
	if (!labConversionCacheInitialized)
		initLabConversionCache ();
}

MultiImage::MultiImage (const MultiImage& other)
	: data(NULL), width(other.width), height(other.height), colorSpace(other.colorSpace), allocWidth(other.allocWidth), allocHeight(other.allocHeight) {

    ch[0] = ch[1] = ch[2] = NULL;

	initArrays ();
	if (colorSpace==Raw)
		memcpy (data, other.data, allocWidth*allocHeight*sizeof(unsigned short));
	else if (colorSpace==RGB || colorSpace==Lab)
		memcpy (data, other.data, 3*allocWidth*allocHeight*sizeof(unsigned short));
}

void MultiImage::initArrays () {

    delete [] data;
    for (int i=0; i<3; i++)
        delete [] ch[i];

    data = new unsigned short[allocWidth*allocHeight*sizeof(unsigned short)*3];
    for (int i=0; i<3; i++) {
        ch[i] = new unsigned short* [allocHeight];
        for (int j=0; j<allocHeight; j++)
            ch[i][j] = data + i*allocWidth*allocHeight + j*allocWidth;
    }

    r = x = cieL = raw = ch[0];
    g = y = ch[1];
    b = z = ch[2];
    ciea = (short**)ch[1];
    cieb = (short**)ch[2];
}

MultiImage::~MultiImage () {

	delete [] data;

	for (int i=0; i<3; i++)
        delete [] ch[i];
}

bool MultiImage::setDimensions (int w, int h) {

	if (w > allocWidth || h > allocHeight)
		return false;
	else {
		width = w;
		height = h;
	}
}

bool MultiImage::copyFrom (MultiImage* other) {

    if (width!=other->width || height!=other->height || colorSpace!=other->colorSpace)
        return false;
    else {
        int channels = colorSpace==Raw ? 1 : 3;

        if (allocWidth==other->allocWidth && allocHeight==other->allocHeight)
            memcpy (data, other->data, channels*allocWidth*allocHeight*sizeof(unsigned short));
        else
            for (int k=0; k<channels; k++)
                for (int i=0; i<height; i++)
                    memcpy (ch[k][i], other->ch[k][i], width * sizeof(unsigned short));
        return true;
    }
}

bool MultiImage::copyFrom (MultiImage* other, int ofsx, int ofsy, int skip) {

	if (ofsx<0 || ofsy<0 || ofsx + (width-1)*skip >= other->width || ofsy + (height-1)*skip >= other->height || skip<1)
		return false;

	if (colorSpace!=other->colorSpace) {
        colorSpace = other->colorSpace;
	    initArrays ();
	}
    int channels = colorSpace==Raw ? 1 : 3;
    for (int k=0; k<channels; k++)
        for (int i=0; i<height; i++)
            for (int j=0; j<width; j++)
                ch[k][i][j] = other->raw[ofsy + i*skip][ofsx + j*skip];
	return true;
}


Buffer<unsigned short> MultiImage::getBufferView (unsigned short** channel) {

    return Buffer<unsigned short> (width, height, data, channel);
}

Buffer<short> MultiImage::getBufferView (short** channel) {

    return Buffer<short> (width, height, (short*)data, channel);
}

void MultiImage::convertTo (ColorSpace cs, bool multiThread, std::string workingColorSpace) {

	if (colorSpace==cs || colorSpace==Invalid || cs==Invalid)
		return;

	if (colorSpace==Raw) {
		// convert to RGB by simple bilinear demosaicing
		unsigned short* tmpData = new unsigned short[allocWidth*allocHeight*sizeof(unsigned short)*3];
		unsigned short** tmpch[3];
	    for (int i=0; i<3; i++) {
	        tmpch[i] = new unsigned short* [allocHeight];
	        for (int j=0; j<allocHeight; j++)
	            tmpch[i][j] = tmpData + i*allocWidth*allocHeight + j*allocWidth;
	    }
		// bilinear demosaicing of the center of the image
		#pragma omp parallel for if (multiThread)
		for (int i=1; i<allocHeight-1; i++)
			for (int j=1; j<allocWidth-1; j++)
				if (raw_isRed(i,j)) {
				    tmpch[0][i][j] = raw[i][j];
				    tmpch[1][i][j] = (raw[i-1][j] + raw[i+1][j] + raw[i][j-1] + raw[i][j+1]) >> 2;
				    tmpch[2][i][j] = (raw[i-1][j-1] + raw[i+1][j-1] + raw[i-1][j+1] + raw[i+1][j+1]) >> 2;
				}
				else if (raw_isBlue(i,j)) {
				    tmpch[0][i][j] = (raw[i-1][j-1] + raw[i+1][j-1] + raw[i-1][j+1] + raw[i+1][j+1]) >> 2;
				    tmpch[1][i][j] = (raw[i-1][j] + raw[i+1][j] + raw[i][j-1] + raw[i][j+1]) >> 2;
				    tmpch[2][i][j] = raw[i][j];
				}
		#pragma omp parallel for if (multiThread)
		for (int i=1; i<allocHeight-1; i++)
			for (int j=1; j<allocWidth-1; j++)
				if (raw_isGreen(i,j)) {
				    tmpch[0][i][j] = (tmpch[0][i-1][j] + tmpch[0][i+1][j] + tmpch[0][i][j-1] + tmpch[0][i][j+1]) >> 2;
					tmpch[1][i][j] = raw[i][j];
					tmpch[2][i][j] = (tmpch[2][i-1][j] + tmpch[2][i+1][j] + tmpch[2][i][j-1] + tmpch[2][i][j+1]) >> 2;
				}
		// demosaicing borders less efficiently
		#pragma omp parallel for if (multiThread)
		for (int i=0; i<allocHeight; i++)
			for (int j=0; j<allocWidth; j++)
				if (i==0 || j==0 || i==allocHeight-1 || j==allocWidth-1) {
					int r_ = 0, g_ = 0, b_ = 0, rn = 0, gn = 0, bn = 0;
					for (int x=-1; x<=1; x++)
						for (int y=-1; y<=1; y++)
							if (i+x>=0 && j+y>=0 && i+x<allocHeight && j+y<allocWidth) {
								if (raw_isRed(i+x,j+y))
									r_ += raw[i+x][j+y], rn++;
								else if (raw_isGreen(i+x,j+y))
									g_ += raw[i+x][j+y], gn++;
								else if (raw_isBlue(i+x,j+y))
									b_ += raw[i+x][j+y], bn++;
							}
                    tmpch[0][i][j] = r_ / rn;
                    tmpch[1][i][j] = g_ / rn;
                    tmpch[2][i][j] = b_ / rn;
				}
		delete [] data;
        data = tmpData;
        for (int i=0; i<3; i++) {
            delete [] ch[i];
            ch[i] = tmpch[i];
        }
        r = x = cieL = raw = ch[0];
        g = y = ch[1];
        b = z = ch[2];
        ciea = (short**)ch[1];
        cieb = (short**)ch[2];
		// convert to the desired color space
		convertTo (cs, multiThread, workingColorSpace);
	}
	else if (cs==Raw) {
		// convert to rgb first
		convertTo (RGB, multiThread, workingColorSpace);
		// Do mosaicing
		#pragma omp parallel for if (multiThread)
		for (int i=0; i<allocHeight; i++)
			for (int j=0; j<allocWidth; j++)
				if (raw_isRed(i,j))
				    raw[i][j] = r[i][j];
				else if (raw_isGreen(i,j))
				    raw[i][j] = g[i][j];
				else if (raw_isBlue(i,j))
				    raw[i][j] = b[i][j];
	}
	else if (colorSpace==RGB && cs==Lab) {
	    TMatrix wprof = iccStore.workingSpaceMatrix (workingColorSpace);
	    // calculate white point tristimulus
	    double xn = wprof[0][0] + wprof[1][0] + wprof[2][0];
	    double yn = wprof[0][1] + wprof[1][1] + wprof[2][1];
	    double zn = wprof[0][2] + wprof[1][2] + wprof[2][2];
	    int toxyz[3][3] = {								// white point: D50 (tristimulus: 0.96422, 1.0, 0.82521)
	        floor(32768.0 * wprof[0][0] / xn),
	        floor(32768.0 * wprof[0][1] / yn),
	        floor(32768.0 * wprof[0][2] / zn),
	        floor(32768.0 * wprof[1][0] / xn),
	        floor(32768.0 * wprof[1][1] / yn),
	        floor(32768.0 * wprof[1][2] / zn),
	        floor(32768.0 * wprof[2][0] / xn),
	        floor(32768.0 * wprof[2][1] / yn),
	        floor(32768.0 * wprof[2][2] / zn)};

		#pragma omp parallel for if (multiThread)
	    for (int i=0; i<height; i++)
        	for (int j=0; j<width; j++) {
        		int r_ = r[i][j], g_ = g[i][j], b_ = b[i][j];
				int x_ = (toxyz[0][0] * r_ + toxyz[1][0] * g_ + toxyz[2][0] * b_) >> 15;
				int y_ = (toxyz[0][1] * r_ + toxyz[1][1] * g_ + toxyz[2][1] * b_) >> 15;
				int z_ = (toxyz[0][2] * r_ + toxyz[1][2] * g_ + toxyz[2][2] * b_) >> 15;

				x_ = CLIPTO(x_,0,2*65536-1);
				y_ = CLIPTO(y_,0,2*65536-1);
				z_ = CLIPTO(z_,0,2*65536-1);

				cieL[i][j] = cacheL[y_];
				ciea[i][j] = cacheab[x_] - cacheab[y_];
				cieb[i][j] = cacheab[y_] - cacheab[z_];
        	}
	}
    else if (colorSpace==RGB && cs==XYZ) {
        TMatrix wprof = iccStore.workingSpaceMatrix (workingColorSpace);
        // calculate white point tristimulus
        double xn = wprof[0][0] + wprof[1][0] + wprof[2][0];
        double yn = wprof[0][1] + wprof[1][1] + wprof[2][1];
        double zn = wprof[0][2] + wprof[1][2] + wprof[2][2];
        int toxyz[3][3] = {                             // white point: D50 (tristimulus: 0.96422, 1.0, 0.82521)
            floor(32768.0 * wprof[0][0] / xn),
            floor(32768.0 * wprof[0][1] / yn),
            floor(32768.0 * wprof[0][2] / zn),
            floor(32768.0 * wprof[1][0] / xn),
            floor(32768.0 * wprof[1][1] / yn),
            floor(32768.0 * wprof[1][2] / zn),
            floor(32768.0 * wprof[2][0] / xn),
            floor(32768.0 * wprof[2][1] / yn),
            floor(32768.0 * wprof[2][2] / zn)};

        #pragma omp parallel for if (multiThread)
        for (int i=0; i<height; i++)
            for (int j=0; j<width; j++) {
                int r_ = r[i][j], g_ = g[i][j], b_ = b[i][j];
                int x_ = (toxyz[0][0] * r_ + toxyz[1][0] * g_ + toxyz[2][0] * b_) >> 15;
                int y_ = (toxyz[0][1] * r_ + toxyz[1][1] * g_ + toxyz[2][1] * b_) >> 15;
                int z_ = (toxyz[0][2] * r_ + toxyz[1][2] * g_ + toxyz[2][2] * b_) >> 15;

                x[i][j] = CLIP(x_);
                y[i][j] = CLIP(y_);
                z[i][j] = CLIP(z_);
            }
    }
	else if (colorSpace==Lab && cs==RGB) {
	    TMatrix wprof = iccStore.workingSpaceMatrix (workingColorSpace);
	    // calculate the white point tristimulus
	    double xn = wprof[0][0] + wprof[1][0] + wprof[2][0];
	    double yn = wprof[0][1] + wprof[1][1] + wprof[2][1];
	    double zn = wprof[0][2] + wprof[1][2] + wprof[2][2];
	    TMatrix iwprof = iccStore.workingSpaceInverseMatrix (workingColorSpace);
	    int torgb[3][3] = {								// white point: D50 (tristimulus: 0.96422, 1.0, 0.82521)
	        floor(32768.0 * iwprof[0][0] * xn),
	        floor(32768.0 * iwprof[0][1] * xn),
	        floor(32768.0 * iwprof[0][2] * xn),
	        floor(32768.0 * iwprof[1][0] * yn),
	        floor(32768.0 * iwprof[1][1] * yn),
	        floor(32768.0 * iwprof[1][2] * yn),
	        floor(32768.0 * iwprof[2][0] * zn),
	        floor(32768.0 * iwprof[2][1] * zn),
	        floor(32768.0 * iwprof[2][2] * zn)};

		#pragma omp parallel for if (multiThread)
		for (int i=0; i<height; i++) {
			for (int j=0; j<width; j++) {
				int L_ = cieL[i][j], y_;

				int x_ = 141558 + (L_ + 10486 + 464 * ciea[i][j] / 100);
				int z_ = 141558 + (L_ + 10486 - 464 * cieb[i][j] / 100);

				x_ = xzcache[x_];
				y_ = ycache[L_];
				z_ = xzcache[z_];

				/* XYZ to RGB */
				int r_ = (torgb[0][0] * x_ + torgb[1][0] * y_ + torgb[2][0] * z_) >> 15;
				int g_ = (torgb[0][1] * x_ + torgb[1][1] * y_ + torgb[2][1] * z_) >> 15;
				int b_ = (torgb[0][2] * x_ + torgb[1][2] * y_ + torgb[2][2] * z_) >> 15;

				r[i][j] = CLIPTO(r_,0,65535);
				g[i][j] = CLIPTO(g_,0,65535);
				b[i][j] = CLIPTO(b_,0,65535);
			}
		}
	}
    else if (colorSpace==Lab && cs==XYZ) {
        #pragma omp parallel for if (multiThread)
        for (int i=0; i<height; i++) {
            for (int j=0; j<width; j++) {
                int L_ = cieL[i][j], y_;

                int x_ = 141558 + (L_ + 10486 + 464 * ciea[i][j] / 100);
                int z_ = 141558 + (L_ + 10486 - 464 * cieb[i][j] / 100);

                x[i][j] = xzcache[x_];
                y[i][j] = ycache[L_];
                z[i][j] = xzcache[z_];
            }
        }
    }
    else if (colorSpace==XYZ && cs==RGB) {
        TMatrix wprof = iccStore.workingSpaceMatrix (workingColorSpace);
        // calculate the white point tristimulus
        double xn = wprof[0][0] + wprof[1][0] + wprof[2][0];
        double yn = wprof[0][1] + wprof[1][1] + wprof[2][1];
        double zn = wprof[0][2] + wprof[1][2] + wprof[2][2];
        TMatrix iwprof = iccStore.workingSpaceInverseMatrix (workingColorSpace);
        int torgb[3][3] = {                             // white point: D50 (tristimulus: 0.96422, 1.0, 0.82521)
            floor(32768.0 * iwprof[0][0] * xn),
            floor(32768.0 * iwprof[0][1] * xn),
            floor(32768.0 * iwprof[0][2] * xn),
            floor(32768.0 * iwprof[1][0] * yn),
            floor(32768.0 * iwprof[1][1] * yn),
            floor(32768.0 * iwprof[1][2] * yn),
            floor(32768.0 * iwprof[2][0] * zn),
            floor(32768.0 * iwprof[2][1] * zn),
            floor(32768.0 * iwprof[2][2] * zn)};

        #pragma omp parallel for if (multiThread)
        for (int i=0; i<height; i++) {
            for (int j=0; j<width; j++) {
                int r_ = (torgb[0][0] * x[i][j] + torgb[1][0] * y[i][j] + torgb[2][0] * z[i][j]) >> 15;
                int g_ = (torgb[0][1] * x[i][j] + torgb[1][1] * y[i][j] + torgb[2][1] * z[i][j]) >> 15;
                int b_ = (torgb[0][2] * x[i][j] + torgb[1][2] * y[i][j] + torgb[2][2] * z[i][j]) >> 15;

                r[i][j] = CLIPTO(r_,0,65535);
                g[i][j] = CLIPTO(g_,0,65535);
                b[i][j] = CLIPTO(b_,0,65535);
            }
        }
    }
    else if (colorSpace==XYZ && cs==Lab) {
        #pragma omp parallel for if (multiThread)
        for (int i=0; i<height; i++)
            for (int j=0; j<width; j++) {
                cieL[i][j] = cacheL[y[i][j]];
                ciea[i][j] = cacheab[x[i][j]] - cacheab[y[i][j]];
                cieb[i][j] = cacheab[y[i][j]] - cacheab[z[i][j]];
            }
    }

	colorSpace = cs;
}

void MultiImage::switchTo  (ColorSpace cs) {

	if (colorSpace==cs || colorSpace==Invalid || cs==Invalid)
		return;

	colorSpace = cs;
}

void MultiImage::initLabConversionCache () {

    cacheL = new int[XYZ_MAXVAL+1];
    cacheab = new int[XYZ_MAXVAL+1];

    const int threshold = (int)(0.008856*65535);
    for (int i=0; i<XYZ_MAXVAL; i++)
        if (i>threshold) {
            cacheL[i] = (int)round(655.35 * (116.0 * exp(1.0/3.0 * log(i/65535.0)) - 16.0));
            cacheab[i] = (int)round(16384.0 * exp(1.0/3.0 * log(i/65535.0)));
        }
        else {
            cacheL[i] = (int)round(655.35 * 903.3 * i/65535.0);
            cacheab[i] = (int)round(16384.0 * (7.787*i/65535.0+16.0/116.0));
        }

    double fY;
    ycache = new int[65536];
    for (int i=0; i<65536; i++)
        ycache[i] = (int)round(65535.0 * ((fY=(i/655.35+16.0)/116.0) > 0.206893034 ? fY*fY*fY : 0.001107019*i/655.35));

    double fX;
    xzcache = new int[369623];
    for (int i=-141558; i<228066; i++)
        xzcache[i+141558] = (int)round(65535.0 * ((fX=i/76020.6) > 0.206893034 ? fX*fX*fX : 0.128414183*fX - 0.017712301));

	labConversionCacheInitialized = true;
}

Image16* MultiImage::createImage () {

    if (colorSpace == RGB) {
        Image16* img = new Image16 (width, height);
        for (int i=0; i<height; i++) {
            memcpy (img->r[i], r[i], width * sizeof(unsigned short));
            memcpy (img->g[i], g[i], width * sizeof(unsigned short));
            memcpy (img->b[i], b[i], width * sizeof(unsigned short));
        }
        return img;
    }
    else
        return NULL;
}

}
