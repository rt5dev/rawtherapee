/*
 * stdimagesource.h
 *
 *  Created on: Aug 19, 2010
 *      Author: gabor
 */

#ifndef STDIMAGESOURCE_H_
#define STDIMAGESOURCE_H_

#include "imagesource.h"
#include "image.h"
#include "imagedata.h"
#include "colortemp.h"
#include "imageview.h"
#include "multiimage.h"

namespace rtengine {

class StdImageSource : public ImageSource {

    private:
        String fileName;
        Image* img;
        ImageData* idata;
        ColorTemp autoWB;
        bool autoWBComputed;
        cmsHPROFILE embProfile;

	public:

        StdImageSource ();
        virtual ~StdImageSource ();

		// inherited from InitialImage
		String		 		 getFileName () { return fileName; }
		cmsHPROFILE 		 getEmbeddedProfile ();
		const ImageMetaData* getMetaData () { return idata; }

      // inherited from ImageSource
        int 		load (const String& fileName, ProgressListener* listener = NULL);

        ColorTemp   getCamWB    ();
        ColorTemp   getAutoWB   ();
        ColorTemp   getSpotWB   (std::vector<Coord2D> red, std::vector<Coord2D> green, std::vector<Coord2D>& blue);
        void        getAEHistogram (unsigned int* histogram, int& histcompr);
        Matrix33	getCamToRGBMatrix ();
        Matrix33	getRGBToCamMatrix ();

    	Dim  getFullImageSize ();
    	void getImage (const ImageView& targetImageView, MultiImage* targetImage);

        bool isRaw ()       { return false; }
        bool isThumbnail () { return false; }
        double getScale ()  { return 1.0; }
};

}
#endif /* STDIMAGESOURCE_H_ */
