/*
 *  This file is part of RawTherapee.
 *
 *  Copyright (c) 2004-2010 Gabor Horvath <hgabor@rawtherapee.com>
 *
 *  RawTherapee is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  RawTherapee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RawTherapee.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _PROCPARAMS_H_
#define _PROCPARAMS_H_

#include <glibmm.h>
#include <vector>
#include "rtcommon.h"
#include <map>
#include <vector>

namespace rtengine {

///**
//  * Parameters on what order the image processing functions are applied
//  */
//class FilterOrderParams {
//
//	public:
//		bool					 custom;
//		std::vector<Glib::ustring> filterlist;
//};

/**
  * Parameters of the demosaicing
  */
/*class DemosaicingParams {

	public:
		Glib::ustring method;
		int			  colorcorrectionsteps;
};
*/
/**
  * Parameters of the tone curve 
  */
class ToneCurveParams {

    public:
        bool        autoexp;
        double      clip;
        double      expcomp;
        std::vector<double>   curve;
        int         brightness;
        int         black;
        int         contrast;
        int         shcompr;
        int         hlcompr;
};

/**
  * Parameters of the luminance curve
  */
/*class LCurveParams {

    public:
        std::vector<double>   curve;
        int      	brightness;
        int         contrast;
};
*/
/**
  * Parameters of the sharpening
  */
class SharpeningParams {

    public:
        bool    enabled;
        double  radius;
        int     amount;
        int     threshold;
        bool    edgesonly;
        double  edges_radius;
        int     edges_tolerance;
        bool    halocontrol;
        int     halocontrol_amount;
        
        Glib::ustring method;
        int     deconvamount;
        double  deconvradius;
        int     deconviter;
        int     deconvdamping;
};

/**
  * Parameters of the color boost
  */
/*class ColorBoostParams {

    public: 
        int     amount;
        bool    avoidclip;
        bool    enable_saturationlimiter;
        double  saturationlimit;
};
*/
/**
  * Parameters of the white balance adjustments
  */
class WBParams {

    public:
        Glib::ustring   	method;
        int             temperature;
        double          green;
};

/**
  * Parameters of the color shift
  */
/*class ColorShiftParams {

    public:
        double  a;
        double  b;
};
*/
/**
  * Parameters of the luminance denoising
  */
/*class LumaDenoiseParams {

    public:
        bool    enabled;
        double  radius;
        int     edgetolerance;
};*/

/**
  * Parameters of the color denoising
  */
/*class ColorDenoiseParams {

    public:
        bool    enabled;
        double  radius;
        int     edgetolerance;
        bool    edgesensitive;
        int		amount;
};
*/
/**
  * Parameters of the shadow/highlight enhancement
  */
class SHParams {

    public:
        bool    enabled;
        bool    hq;
        int     highlights;
        int     htonalwidth;
        int     shadows;
        int     stonalwidth;
        int     localcontrast;
        int     radius;
};

/**
  * Parameters of the cropping
  */
class CropParams {

    public:
        bool    enabled;
        int     x;
        int     y;
        int     w;
        int     h;
        bool    fixratio;
        Glib::ustring   ratio;
        Glib::ustring   orientation;
        Glib::ustring   guide;
};

/**
  * Parameters of the coarse transformations like 90 deg rotations and h/v flipping
  */
/*class CoarseTransformParams {

    public:
        int     rotate;
        bool    hflip;
        bool    vflip;
};
*/
/**
  * Common transformation parameters
  */
class CommonTransformParams {

	public:
		bool autofill;
};

/**
  * Parameters of the rotation
  */
class RotateParams {
    
    public:
        double  degree;
};

/**
  * Parameters of the distortion correction
  */
class DistortionParams {

    public:
		bool    uselensfun;
        double  amount;
};

/**
  * Parameters of the perspective correction
  */
class PerspectiveParams {

    public:
		int  horizontal;
		int  vertical;
};

/**
  * Parameters of the vignetting correction
  */
class VignettingParams {

    public:
        int  amount;
        int  radius;
};

///**
//  * Parameters of the color mixer
//  */
//class ChannelMixerParams {
//
//    public:
//        int red[3];
//        int green[3];
//        int blue[3];
//};

/**
  * Parameters of the c/a correction
  */
class CACorrParams {

    public:
        double red;
        double blue;
};

/**
  * Parameters of the highlight recovery
  */
class HRecParams {

    public:
        bool enabled;
        Glib::ustring method;
};

/**
  * Parameters of the resizing
  */
class ResizeParams {

    public:
        bool enabled;
        double scale;
        Glib::ustring method;
        int dataspec;
        int width;
        int height;
};

/**
  * Parameters of the color spaces used during the processing
  */
class ColorManagementParams {

    public:
		Glib::ustring input;
        bool        gammaOnInput;
        Glib::ustring working;
        Glib::ustring output;
};

/**
  * A class representing a key/value for the exif metadata information
  */
class ExifPair {

    public:
		Glib::ustring field;
		Glib::ustring value;
};

/**
  * The IPTC key/value pairs
  */
class IPTCPair {

    public:
		Glib::ustring field;
        std::vector<Glib::ustring> values;
};

/**
  * This class holds all the processing parameters applied on the images
  */

class ProcParams {

		std::map<String,double> floatParams;
		std::map<String,int> 	 intParams;
		std::map<String,bool> 	 boolParams;
		std::map<String,String> stringParams;
		std::map<String,FloatList> floatListParams;
		std::map<String,IntList> intListParams;
		std::map<String,StringList> stringListParams;

    public:

		void   setFloat   (const String& key, float value);
		float  getFloat   (const String& key);

		void   setInteger (const String& key, int value);
		int    getInteger (const String& key);

		void   setBoolean (const String& key, bool value);
		bool   getBoolean (const String& key);

		void   setString  (const String& key, const String& value);
		String getString  (const String& key);

		void   		setFloatList (const String& key, const FloatList& value);
		FloatList&  getFloatList (const String& key);

		void   		setIntegerList (const String& key, const IntList& value);
		IntList& 	getIntegerList (const String& key);

		void   		setStringList (const String& key, const StringList& value);
		StringList& getStringList (const String& key);

		// --------------8<------------------ to be removed when all filters are rewritten ------

//		DemosaicingParams		demosaicing;	///< Demosaicing parameters
//		FilterOrderParams		filterOrder;	///< Order of image processing functions
		ToneCurveParams         toneCurve;      ///< Tone curve parameters
//        LCurveParams            lumaCurve;      ///< CIELAB luminance curve parameters
        SharpeningParams        sharpening;     ///< Sharpening parameters
//        ColorBoostParams        colorBoost;     ///< Color boost parameters
        WBParams                wb;             ///< White balance parameters
//        ColorShiftParams        colorShift;     ///< Color shift parameters
//        LumaDenoiseParams       lumaDenoise;    ///< Luminance denoising parameters
//        ColorDenoiseParams      colorDenoise;   ///< Color denoising parameters
        SHParams                sh;             ///< Shadow/highlight enhancement parameters
        CropParams              crop;           ///< Crop parameters
//        CoarseTransformParams   coarse;         ///< Coarse transformation (90, 180, 270 deg rotation, h/v flipping) parameters
        CommonTransformParams	commonTrans;	///< Common transformation parameters (autofill)
        RotateParams            rotate;         ///< Rotation parameters
        DistortionParams        distortion;     ///< Lens distortion correction parameters
        PerspectiveParams       perspective;    ///< Perspective correction parameters
        CACorrParams            cacorrection;   ///< Lens c/a correction parameters
        VignettingParams        vignetting;     ///< Lens vignetting correction parameters
//        ChannelMixerParams      chmixer;        ///< Channel mixer parameters
        HRecParams              hlrecovery;     ///< Highlight recovery parameters
        ResizeParams            resize;         ///< Resize parameters
        ColorManagementParams   icm;            ///< profiles/color spaces used during the image processing
        std::vector<ExifPair> exif;             ///< List of modifications appplied on the exif tags of the input image
        std::vector<IPTCPair> iptc;             ///< The IPTC tags and values to be saved to the output image
        int version;                            ///< Version of the file from which the parameters have been read
        
      /**
        * The constructor only sets the hand-wired defaults.
        */
        ProcParams          ();
      /**
        * Sets the hand-wired defaults parameters.
        */
        void    setDefaults ();
      /**
        * Saves the parameters to a file.
        * @param fname the name of the file
        * @return Error code (=0 if no error)
        */
        int     save        (Glib::ustring fname) const;
      /**
        * Loads the parameters from a file.
        * @param fname the name of the file
        * @return Error code (=0 if no error)
        */
        int     load        (Glib::ustring fname);

      /** Creates a new instance of ProcParams.
        * @return a pointer to the new ProcParams instance. */
        static ProcParams* create  ();

      /** Destroys an instance of ProcParams.
        * @param pp a pointer to the ProcParams instance to destroy. */
        static void        destroy (ProcParams* pp);

        bool operator== (const ProcParams& other);
        bool operator!= (const ProcParams& other);
};

extern ProcParams defaultProcParams;
}
#endif
