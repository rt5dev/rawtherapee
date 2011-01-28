/*
 * filttonecurve.cc
 *
 *  Created on: Sep 16, 2010
 *      Author: gabor
 */

#include "filttonecurve.h"
#include "rtengine.h"
#include "macros.h"
#include "iccstore.h"
#include "improcfuns.h"
#include <string.h>
#include "filterchain.h"
#include "curves.h"
#include "util.h"

#define TC_LUTSIZE 	2*65536
#define TC_LUTSCALE 65536

namespace rtengine {

class PreToneCurveFilterDescriptor : public FilterDescriptor {
    public:
        PreToneCurveFilterDescriptor ()
            : FilterDescriptor ("PreToneCurve", MultiImage::RGB, MultiImage::RGB) {
            addTriggerEvent (EvAutoExp);
            addTriggerEvent (EvClip);
        }
        void createAndAddToList (Filter* tail) const {}
};

ToneCurveFilterDescriptor toneCurveFilterDescriptor;
PreToneCurveFilterDescriptor preToneCurveFilterDescriptor;

ToneCurveFilterDescriptor::ToneCurveFilterDescriptor ()
	: FilterDescriptor ("ToneCurve", MultiImage::RGB, MultiImage::RGB) {

    addTriggerEvent (EvBrightness);
	addTriggerEvent (EvContrast);
    addTriggerEvent (EvExpComp);
    addTriggerEvent (EvBlack);
    addTriggerEvent (EvHLCompr);
    addTriggerEvent (EvSHCompr);
    addTriggerEvent (EvToneCurve);
}

void ToneCurveFilterDescriptor::getDefaultParameters (ProcParams& defProcParams) const {

	defProcParams.setBoolean ("ToneCurveAutoExp", true);
	defProcParams.setFloat   ("ToneCurveClip", 0.001);
	defProcParams.setFloat   ("ToneCurveExpComp", 0);
	defProcParams.setFloat   ("ToneCurveBrightness", 0);
	defProcParams.setFloat   ("ToneCurveContrast", 0);
	defProcParams.setFloat   ("ToneCurveBlack", 0);
	defProcParams.setFloat   ("ToneCurveHLCompr", 100);
	defProcParams.setFloat   ("ToneCurveSHCompr", 100);
	FloatList tcurve;
	defProcParams.setFloatList ("ToneCurveCustomCurve", tcurve);
}

void ToneCurveFilterDescriptor::createAndAddToList (Filter* tail) const {

	PreToneCurveFilter* ptcf = new PreToneCurveFilter ();
    tail->addNext (ptcf);
    ptcf->addNext (new ToneCurveFilter (ptcf));
}

PreToneCurveFilter::PreToneCurveFilter ()
    : Filter (&preToneCurveFilterDescriptor), histogram (NULL) {
}

PreToneCurveFilter::~PreToneCurveFilter () {

    delete [] histogram;
}

void PreToneCurveFilter::process (const std::set<ProcEvent>& events, MultiImage* sourceImage, MultiImage* targetImage, Buffer<float>* buffer) {

    Filter* p = getParentFilter ();
    if (!p) {

        // update histogram histogram
        if (!histogram)
            histogram = new unsigned int [TC_LUTSIZE];

    	String workingProfile = procParams->getString ("ColorManagementWorkingProfile");

        Matrix33 wprof = iccStore->workingSpaceMatrix (workingProfile);
        float mulr = wprof.data[1][0];
        float mulg = wprof.data[1][1];
        float mulb = wprof.data[1][2];

        memset (histogram, 0, TC_LUTSIZE*sizeof(int));
        for (int i=0; i<sourceImage->height; i++)
            for (int j=0; j<sourceImage->width; j++) {
                int y = TC_LUTSCALE * (mulr * sourceImage->r[i][j] + mulg * sourceImage->g[i][j] + mulb * sourceImage->b[i][j]);
                histogram[CLIPTO(y,0,TC_LUTSIZE-1)]++;
            }

    	bool autoexp = procParams->getBoolean ("ToneCurveAutoExp");
    	float clip   = procParams->getFloat ("ToneCurveClip");

    	// calculate auto exposure parameters
        if (autoexp) {
            unsigned int aehist[65536]; int aehistcompr;
            ImageSource* imgsrc = getFilterChain ()->getImageSource ();
            imgsrc->getAEHistogram (aehist, aehistcompr);
            float expcomp, black;
            ImProcFunctions::calcAutoExp (aehist, aehistcompr, clip, expcomp, black);
        	procParams->setFloat ("ToneCurveExpComp", expcomp);
        	procParams->setFloat ("ToneCurveBlack", black*100.0);
        }
    }

    // we have to copy image data if input and output are not the same
    if (sourceImage != targetImage)
        targetImage->copyFrom (sourceImage);
}

unsigned int* PreToneCurveFilter::getHistogram () {

    return histogram;
}

ToneCurveFilter::ToneCurveFilter (PreToneCurveFilter* ptcf)
	: Filter (&toneCurveFilterDescriptor), curve (NULL), ptcFilter (ptcf), bchistogram (NULL) {
}

ToneCurveFilter::~ToneCurveFilter () {

    delete [] curve;
    delete [] bchistogram;
}

void ToneCurveFilter::process (const std::set<ProcEvent>& events, MultiImage* sourceImage, MultiImage* targetImage, Buffer<float>* buffer) {

	Filter* p = getParentFilter ();
    ImageSource* imgsrc = getFilterChain ()->getImageSource ();

    float* myCurve;

    // curve is only generated once: in the root filter chain
    if (!p) {
    	float expcomp = procParams->getFloat ("ToneCurveExpComp");
    	float brightness = procParams->getFloat ("ToneCurveBrightness");
    	float contrast = procParams->getFloat ("ToneCurveContrast");
    	float black = procParams->getFloat ("ToneCurveBlack");
    	float hlcompr = procParams->getFloat ("ToneCurveHLCompr");
    	float shcompr = procParams->getFloat ("ToneCurveSHCompr");
    	FloatList tcurve = procParams->getFloatList ("ToneCurveCustomCurve");

    	if (!curve) {
            curve = new float [TC_LUTSIZE];
            CurveFactory::complexCurve (expcomp, black/100.0, hlcompr, shcompr, brightness, contrast, imgsrc->isRaw() ? 2.2 : 0.0 , true, tcurve, ptcFilter->getHistogram(), TC_LUTSIZE, TC_LUTSCALE, curve, NULL, getScale ()==1 ? 1 : 16);
        }
        else if (isTriggerEvent (events) || ptcFilter->isTriggerEvent (events))
            CurveFactory::complexCurve (expcomp, black/100.0, hlcompr, shcompr, brightness, contrast, imgsrc->isRaw() ? 2.2 : 0.0 , true, tcurve, ptcFilter->getHistogram(), TC_LUTSIZE, TC_LUTSCALE, curve, NULL, getScale ()==1 ? 1 : 16);
        myCurve = curve;
    }
    else {
        Filter* root = p;
        while (root->getParentFilter())
            root = root->getParentFilter();
        myCurve = ((ToneCurveFilter*)root)->curve;
    }

    // apply curve
    #pragma omp parallel for if (multiThread)
	for (int i=0; i<sourceImage->height; i++) {
		for (int j=0; j<sourceImage->width; j++) {
			targetImage->r[i][j] = lutInterp<float,TC_LUTSIZE> (myCurve, TC_LUTSCALE*sourceImage->r[i][j]);
			targetImage->g[i][j] = lutInterp<float,TC_LUTSIZE> (myCurve, TC_LUTSCALE*sourceImage->g[i][j]);
			targetImage->b[i][j] = lutInterp<float,TC_LUTSIZE> (myCurve, TC_LUTSCALE*sourceImage->b[i][j]);
		}
	}
}

}
