#ifndef	_GRAYCODE_H_
#define _GRAYCODE_H_

// 
// Class GrayCode
// 

class GrayCode

{

	friend void Display(void);
	friend int main(int , char ** );

private:
	
	int m_GBit;										// Current Bit Depth

	int m_CodeDepth;								// Bit depth of gray code
	int m_CodeResolution;						// Spatial resolution (pixel) of Gray Code

	int m_SlitWidth;									// Slit width of each projected white stripe
	int m_SlitInterval;								// Spatial interval (pixel) of multi-slit

	unsigned char * m_Diff;						// Store binarized image derived from difference between nega / positive images

	int m_CameraWidth;							// Camera Size
	int m_CameraHeight;

	int m_ProjectorWidth;						// Projector Size
	int m_ProjectorHeight;

	double * m_Sum;								// Buffers for multi-slit projection
	double * m_Num;

	// Directory Name
	char m_DirName[128];

	// bool skip_graycode;

	// Sequence of Projector Placement
	int m_ProjSeqNum;

public:

	int m_bSlit;										// Flag ( True / False: With / Without multi-slit projection )

	// C2P map ( double / int -> with / without multi-slit projection )
	int * m_C2P[2];
	double * m_C2P_DB[2];

	// C2P map ( For Displaying )
	unsigned char * m_C2P_UC[2];

	// FM / EM
	unsigned char * m_White;
	unsigned char * m_Black;

	unsigned char * m_Illuminance;
	unsigned char * m_Compensate;

	// Binalized multi-slit image ( captured )
	unsigned char * m_bSlitImg;

	// Mask image ( Remove non-projected (e.g. shadow) region )
	bool * m_Mask;

	// Threshold for normalize captured multi-slit images
	int m_NormThres;

	// "m_CurrSlitNum" - The slit is currently projected
	int m_CurrSlitNum;

	// 
	// Program mode definition 
	// 
	// DISP_IDLE														-- no task 
	// DISP_LIVE_INPUT_IMAGE_ON_SUBWINDOW		-- display live input image on subwindow
	// DISP_SLIT														-- display multi-slit projection
	// DISP_ILLUMI													-- display white image
	// DISP_GRAYCODE											-- display gray code 
	// 

	enum 
	{ 
		DISP_IDLE, 
		DISP_GRAYCODE, 
		DISP_SLIT, 
		DISP_ILLUMI 
	} m_DispMode;

	// 
	// NEGATIVE - POSITIVE Pattern Projection
	// 
	
	enum 
	{ 
		NEGATIVE, 
		POSITIVE
	} m_NPMode;

	// 
	// Horizontal / Vertical Pattern
	// 
	
	enum 
	{ 
		VERT, 
		HORI 
	} m_HVMode;

	// 
	// Binary Mode
	// 
	// AVG_MODE  --	capture usual intensity image.
	//				each color channels are averaged 
	// 
	// DIFF_MODE --	for binarized images.
	// 				captured image is compared with the other intensity image.
	//				it is used for NEGA / POSI binarizing method.

	enum 
	{ 
		AVG_MODE, 
		DIFF_MODE 
	};

public:

	//
	// Constructor & Destructor
	// 

	GrayCode ( int cameraW, int cameraH, int projectorW, int projectorH, bool bSlit, char * dirname );
	virtual ~GrayCode();

	//
	// Initialize Variables for Graycode / multi-slit projection
	// 
	
	void InitDispCode ( int nProjSeq );
	void InitDispCode ( int nProjSeq, int dispMode );

	//
	// Projection Image generation / Display projection image
	//

	//
	// Binary (normal) <-> Gray code converter
	// 
	
	unsigned int Gray2Binary ( unsigned int g );
	unsigned int Binary2Gray ( unsigned int b );

	//
	// Display code
	// - hv_mode: flag for horizontal / vertical pattern
	// 

	void DispCode ( int hv_mode, int np_mode );

	// ============================
	// Process of captured images
	// ============================

	// 
	// At first, captured images are passed through this function
	// - image: captured image
	// 

	void CaptureCode ( unsigned char * image );

	// 
	// Binalize captured image through subtaction of NEGA/POSI images (for graycode projection)
	// 
	// - color: captured image
	// - bin_mode: binarize mode (AVG_MODE / DIFF_MODE)
	// 

	void Binarize ( unsigned char * color, int bin_mode );

	//
	// Image processing for multi-slit projection
	// - color: Captured image
	// 

	void CaptureSlit ( unsigned char * color );

	// ========================
	// After Code projection
	// ========================

	// 
	// Mask Process (remove non-projected (e.g. shadow) region)
	// 

	void Mask ( int threshold );

	//
	// Get spatial code in double percision
	// - hv_mode: Horizontal / Vertical identifier
	// - cx, cy: position in camera coordinate
	// 

	double dblCode ( int hv_mode, double cx, double cy );

	//
	//	Write out C2P map
	// 

	void WriteC2P( void );
	
	int ShowProjectorIlluminace ( void );						// Calc Illumination intensity blending coefficients for projector

	void SaveIlluminace ( unsigned char * color );
	void SaveCompensate ( unsigned char * color );
	void CalcBlendCoefficient ();

};

#endif
