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
	
	// �ڲ���
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

		int	Get( void );										// �����������л�����Եĸ�����Աֵ
		int	GetRatio( float& va, float& vb );			// �������ֵ A �� B ����ȡֵ��Χ����ռ�ı���, va, vb �ֱ�Ϊ����Ĳ���
		int	GetRange( void );								// ��ø����Ե�������Сֵ
		
		int Set ( const bool one_push, const bool on_off, const bool auto_c, const int va, const int vb );					// ���ö�Ӧ���Ե�ֵ
		int SetRatio( const bool one_push, const bool on_off, const bool auto_c, const float va, const float vb );		// ��������ֵ��ռ�ĵ�ǰ����
		
		// ��ӡ������ͷ�����Ե�ֵ
		int Print( int num = 1 );
	};
	
protected:

	// ����ͷָ�꣬������������ͷ
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

	// ��ӡ�����е�����ֵ
	void	PrintProperties( void );

	void	SetBrightness( const int value );											// ��������ֵ
	void	SetExposure( const int value );											// �����ع�ֵ
	void	SetShutter( const int value );												// ���ÿ���ֵ	
	void	SetGain( const int value );													// ��������ֵ
	void	SetWhiteBalance( const int va, const int vb );						// ���ð�ƽ��ֵ������������������
	void	SetBrightnessRatio( const float value );								// �������ȱ���ֵ�����ݱ����趨����ֵ
	void	SetExposureRatio( const float value );									// �����ع����ֵ
	void	SetShutterRatio( const float value );									// ���ÿ��ű���ֵ
	void	SetGainRatio( const float value );											// �����������ֵ
	void	SetWhiteBalanceRatio( const float va, const float vb );			// ���ð�ƽ�����ֵ������������������

#if 0	
	int		GetBrightness( const int value );
	int		GetExposure( const int value );
	int		GetShutter( const int value );
	int		GetGain( const int value );
	void	GetWhiteBalance( int& va, int& vb );
#endif	

	// ��� XX ����ֵ
	float	GetBrightnessRatio( void );
	float	GetExposureRatio( void );
	float	GetShutterRatio( void );
	float	GetGainRatio( void );
	void	GetWhiteBalanceRatio( float& va, float& vb );
	
	// �Զ����� XX ����ֵ
	void	AutoBrightness( void );
	void	AutoExposure( void );
	void	AutoShutter( void );
	void	AutoGain( void );
	void	AutoWhiteBalance( void );

	// ����ͨ���������е���������ֵ���
	void	Brighter( void );
	// ����
	void	Darker( void );
	
	// ��ʼ��
	virtual int Init( void );
	virtual int InitSynchronized( void);

	// ���ݿ�� �� ֡������ ����ͷ�Ĳ�׽ģʽ��ʼ��׽
	virtual int Start( void );
	virtual int StartSynchronized(  int mode = 0 );

	// ����һ֡ͼ�񣬻�����ָ��Ϊ BUF
	virtual int Capture( unsigned char * buf );
	virtual int CaptureSingle( unsigned char * buf );
	virtual int CaptureMultiple( unsigned char ** buf , int count = _BUFFERS );
	virtual int SynchronizedCapture( unsigned char * buf );

	int SendSoftwareTrigger();
	int EmptyImageBuffer();
	
	// ֹͣ
	virtual int Stop( void );
	int checkTriggerReady(FlyCaptureContext context );

	// ������ɫ������
	inline	 void SetColorProcessingMethod( const FlyCaptureColorMethod method ) 
	{
		flycaptureSetColorProcessingMethod( context, method );
	}
};

#endif	// __POINT_GREY_CAMERA_H__

		
		
	

