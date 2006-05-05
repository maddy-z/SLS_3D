#ifndef	_POINT_GREY_CAMERA_H_
#define	_POINT_GREY_CAMERA_H_

#include <PGR_FlyCapture\PGRFlyCapture.h>
#include <PGR_FlyCapture\pgrflycaptureplus.h>

#define		_VIDEOMODE		FLYCAPTURE_VIDEOMODE_ANY
#define		_FRAMERATE		FLYCAPTURE_FRAMERATE_ANY

#define PG_HANDLE_ERROR( error, function )															\
	if ( error != FLYCAPTURE_OK ) {																			\
		cout << function << " : " << flycaptureErrorToString( error ) << endl;			\
		return -1;																										\
	}

#define _HANDLE_ERROR( error, function )																\
	if ( error != FLYCAPTURE_OK ) {																			\
		cout << function << " : " << flycaptureErrorToString( error ) << endl;			\
		return -1;																										\
	}

#define		_BUFFERS			2

#define		DEFAULT_COLOR_PROCESSING_METHOD		FLYCAPTURE_EDGE_SENSING

#define		DEFAULT_WIDTH										640
#define		DEFAULT_HEIGHT										480
#define		DEFAULT_FRAME_RATE								60.0f

#define		INITIALIZE													0x000
#define		TRIGGER_INQ												0x530
#define		CAMERA_POWER										0x610
#define		SOFTWARE_TRIGGER									0x62C
#define		SOFT_ASYNC_TRIGGER								0x102C

// 
// Point Grey Camera
// 

class	PointGreyCamera 
{

public:
	
	// 内部类
	class	Property 
	{
	
	private:
		
		FlyCaptureContext	context;
		FlyCaptureProperty	property;

		bool	is_present;
		bool	is_one_push;
		bool	is_on_off;
		bool	is_auto;
		bool	is_available_one_push;
		bool	is_available_read_out;
		bool	is_available_on_off;
		bool	is_available_auto;
		bool	is_available_manual;
		
		int		value_a;
		int		value_b;
		int		min;
		int		max;

	public:

		Property( const FlyCaptureContext n_context,
					   const FlyCaptureProperty n_property );
		~Property();

		int	Get( void );										// 从驱动程序中获得属性的各个成员值
		int	GetRatio( float& va, float& vb );			// 获得属性值 A 和 B 在其取值范围内所占的比例, va, vb 分别为输出的参数
		int	GetRange( void );								// 获得该属性的最大和最小值
		
		int Set ( const bool one_push, const bool on_off, const bool auto_c, const int va, const int vb );					// 设置对应属性的值
		int SetRatio( const bool one_push, const bool on_off, const bool auto_c, const float va, const float vb );		// 设置属性值所占的当前比例
		
		// 打印出摄像头该属性的值
		int Print( int num = 1 );
	};
	
protected:

	// 摄像头指标，区别多个该摄像头
	int			camera_index;
	int			width;
	int			height;
	float		frame_rate;
	bool		is_capturing;

	FlyCaptureContext		context;   
	FlyCaptureError				error;

	// Multiple capture regions
	FlyCaptureImagePlus		g_arImageplus;
	FlyCaptureInfoEx			g_arInfo;
	FlyCaptureContext		g_arContext;
	unsigned char *				g_arpBuffers[_BUFFERS];

	FlyCaptureFrameRate		f;
	FlyCaptureVideoMode	v;

	Property	* brightness;
	Property	* exposure;
	Property	* shutter;
	Property	* gain;
	Property	* white_balance;

	void	ReportCameraInfo( const FlyCaptureInfoEx* pinfo );

#if 0	
	int		GetPropertyRangeEx( FlyCaptureProperty p );
	int		GetPropertyRange( FlyCaptureProperty p, Property& property );
	int		GetProperty( FlyCaptureProperty p, Property& property );
#endif	
	
public:

	PointGreyCamera( const int index,
								const int n_width = DEFAULT_WIDTH,
								const int n_height = DEFAULT_HEIGHT,
								const float n_frame_rate = DEFAULT_FRAME_RATE );
	virtual ~PointGreyCamera();

	// 打印出所有的属性值
	void	PrintProperties( void );

	void	SetBrightness( const int value );											// 设置亮度值
	void	SetExposure( const int value );											// 设置曝光值
	void	SetShutter( const int value );												// 设置快门值	
	void	SetGain( const int value );													// 设置增益值
	void	SetWhiteBalance( const int va, const int vb );						// 设置白平衡值，该属性有两个参数
	void	SetBrightnessRatio( const float value );								// 设置亮度比例值，根据比例设定具体值
	void	SetExposureRatio( const float value );									// 设置曝光比例值
	void	SetShutterRatio( const float value );									// 设置快门比例值
	void	SetGainRatio( const float value );											// 设置增益比例值
	void	SetWhiteBalanceRatio( const float va, const float vb );			// 设置白平衡比例值，该属性有两个参数

#if 0	
	int		GetBrightness( const int value );
	int		GetExposure( const int value );
	int		GetShutter( const int value );
	int		GetGain( const int value );
	void	GetWhiteBalance( int& va, int& vb );
#endif	

	// 获得 XX 属性值
	float	GetBrightnessRatio( void );
	float	GetExposureRatio( void );
	float	GetShutterRatio( void );
	float	GetGainRatio( void );
	void	GetWhiteBalanceRatio( float& va, float& vb );
	
	// 自动调节 XX 属性值
	void	AutoBrightness( void );
	void	AutoExposure( void );
	void	AutoShutter( void );
	void	AutoGain( void );
	void	AutoWhiteBalance( void );

	// 更亮通过调节其中的两个属性值完成
	void	Brighter( void );
	// 更淡
	void	Darker( void );
	
	// 初始化
	virtual int Init( void );
	virtual int InitSynchronized( void);

	// 根据宽高 和 帧率设置 摄像头的捕捉模式开始捕捉
	virtual int Start( void );
	virtual int StartSynchronized(  int mode = 0 );

	// 捕获一帧图像，缓冲区指针为 BUF
	virtual int Capture( unsigned char * buf );
	virtual int CaptureSingle( unsigned char * buf );
	virtual int CaptureMultiple( unsigned char ** buf , int count = _BUFFERS );
	virtual int SynchronizedCapture( unsigned char * buf );

	int SendSoftwareTrigger();
	int EmptyImageBuffer();
	
	// 停止
	virtual int Stop( void );
	int checkTriggerReady(FlyCaptureContext context );

	// 设置颜色处理方法
	inline	 void SetColorProcessingMethod( const FlyCaptureColorMethod method ) 
	{
		flycaptureSetColorProcessingMethod( context, method );
	}
};

#endif	// __POINT_GREY_CAMERA_H__

		
		
	

