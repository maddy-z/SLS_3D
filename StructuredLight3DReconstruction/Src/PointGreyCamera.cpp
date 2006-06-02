// 
// PointGreyCamera.cpp
// 

#pragma comment ( lib,	"PGRFlyCapture.lib" )
#pragma comment ( lib,	"pgrflycapturegui.lib" )

#include	<iostream>

#include	"PointGreyCamera.h"

// 
// 内部类 Property 
// 

PointGreyCamera::Property::Property( const FlyCaptureContext n_context,
														 const FlyCaptureProperty n_property )
	: fcContext ( n_context ), fcProperty ( n_property )
{
	GetRange();
}

PointGreyCamera::Property::~Property()
{
}

int PointGreyCamera::Property::Get( void )
{
	// 
	// 从驱动程序中获得属性的各个成员值
	// 

	FlyCaptureError	error;
	error = flycaptureGetCameraPropertyEx(
		fcContext,
		fcProperty,
		&is_one_push,
		&is_on_off,
		&is_auto,
		&value_a,
		&value_b );

	PG_HANDLE_ERROR( error, "flycaptureGetCameraPropertyEx()" );

	return 0;
}

int PointGreyCamera::Property::GetRatio( float& va, float& vb )
{
	Get();										// 获得属性值 A 和 B 在其取值范围内所占的比例
	GetRange();

	if ( min == max ) 
	{
		va = 0.0f;
		vb = 0.0f;
	} 
	else {
		va = ( value_a - min ) / static_cast<float>( max - min );
		vb = ( value_b - min ) / static_cast<float>( max - min );
	}

	return 0;
}

int PointGreyCamera::Property::GetRange( void )
{
	FlyCaptureError	error;						// 获得该属性的最大值和最小值
	
	error = flycaptureGetCameraPropertyRangeEx(
		fcContext,
		fcProperty,
		&is_present,
		&is_available_one_push,
		&is_available_read_out,
		&is_available_on_off,
		&is_available_auto,
		&is_available_manual,
		&min,
		&max 
		);

	PG_HANDLE_ERROR( error, "flycaptureGetCameraPropertyRangeEx()" );

	return 0;
}

int PointGreyCamera::Property::Set( const bool one_push,
													  const bool on_off,
													  const bool auto_c,
													  const int va, 
													  const int vb )
{
	FlyCaptureError	error;												// 设置对应属性的值
	error = flycaptureSetCameraPropertyEx( fcContext, fcProperty, one_push, on_off, auto_c, va, vb );

	PG_HANDLE_ERROR( error, "flycaptureSetCameraPropertyEx()" );

	return 0;
}

int PointGreyCamera::Property::SetRatio( 
	const bool one_push,
	const bool on_off,
	const bool auto_c,
	const float va, const float vb )
{
	GetRange();								// 设置属性值所占的当前比例

	int	iva, ivb;
	if ( min == max ) {
		iva = min;
		ivb = min;
	} else {
		iva = min + ( max - min ) * va;
		ivb = min + ( max - min ) * vb;
	}

	FlyCaptureError	error;
	error = flycaptureSetCameraPropertyEx( 
											fcContext, fcProperty, 
											one_push, on_off, auto_c,
											iva, ivb );

	PG_HANDLE_ERROR( error, "flycaptureSetCameraPropertyEx()" );

	return 0;
}

int PointGreyCamera::Property::Print( int num )
{
	GetRange();									// 打印出摄像头该属性的值

	bool	auto_c;
	int		va, vb;

	FlyCaptureError	error;
	error = flycaptureGetCameraPropertyEx( fcContext, fcProperty, NULL, NULL, &auto_c, &va, &vb );

	PG_HANDLE_ERROR( error, "flycaptureGetCameraPropertyEx()" );

	std::cout << va;
	if ( num == 2 ) {
		std::cout << " , " << vb;
	}

	std::cout << "(range = " << min << " : " << max << ")";
	if ( auto_c ) {
		std::cout << "[auto]";
	}
	std::cout << std::endl;
	
	return 0;
}

//
// Camera 
// 以默认参数初始化一个摄像头
// 

PointGreyCamera::PointGreyCamera(  
	const int index,
	const int n_width, 
	const int n_height,
	const float n_frame_rate ) : camera_index( index ), 
											 width( n_width ),
											 height( n_height ), 
											 frame_rate( n_frame_rate ),
											 is_capturing( false ), 
											 context( 0 ), brightness( 0 ), exposure( 0 ), shutter( 0 ),
											 gain( 0 ), white_balance( 0 )
{
}

PointGreyCamera::~PointGreyCamera()
{
	if ( is_capturing ) { flycaptureStop( context ); }
	if ( context ) { flycaptureDestroyContext( context ); }

	if ( brightness ) { delete brightness; }
	if ( exposure ) { delete exposure; }
	if ( shutter ) { delete shutter; }
	if ( gain ) { delete gain; }
	if ( white_balance ) { delete white_balance; }

	for ( int uibuffer = 0; uibuffer < _BUFFERS; ++uibuffer ) {
		if ( g_arpBuffers[uibuffer] != NULL ) { delete g_arpBuffers[uibuffer]; }
	}
}

void PointGreyCamera::PrintProperties( void )
{
	// 打印出所有的属性值
	std::cout << "brightness: ";
	brightness->Print();

	std::cout << "exposure: ";
	exposure->Print();

	std::cout << "shutter: ";
	shutter->Print();

	std::cout << "gain: ";
	gain->Print();

	std::cout << "white balance: ";
	white_balance->Print( 2 );								// 该属性有两个值
}

float PointGreyCamera::GetBrightnessRatio( void )
{
	// 获得亮度比例
	float	va, vb;
	brightness->GetRatio( va, vb );
	
	return	va;
}

float PointGreyCamera::GetExposureRatio( void )
{
	float	va, vb;
	exposure->GetRatio( va, vb );

	return	va;
}

float PointGreyCamera::GetShutterRatio( void )
{
	float	va, vb;
	shutter->GetRatio( va, vb );
	
	return	va;
}

float PointGreyCamera::GetGainRatio( void )
{
	float	va, vb;
	gain->GetRatio( va, vb );
	
	return	va;
}

void PointGreyCamera::GetWhiteBalanceRatio( float& va, float &vb )
{
	white_balance->GetRatio( va, vb );
}

void PointGreyCamera::SetBrightness( const int value )
{
	brightness->Set( false, true, false, value, 0 );
}

void PointGreyCamera::SetExposure( const int value )
{
	exposure->Set( false, true, false, value, 0 );
}

void PointGreyCamera::SetShutter( const int value )
{
	shutter->Set( false, true, false, value, 0 );
}

void PointGreyCamera::SetGain( const int value )
{
	gain->Set( false, true, false, value, 0 );
}

void PointGreyCamera::SetWhiteBalance( const int va, const int vb )
{
	white_balance->Set( false, true, false, va, vb );
}

void PointGreyCamera::SetBrightnessRatio( const float value )
{
	brightness->SetRatio( false, true, false, value, 0 );
}

void PointGreyCamera::SetExposureRatio( const float value )
{
	exposure->SetRatio( false, true, false, value, 0 );
}

void PointGreyCamera::SetShutterRatio( const float value )
{
	shutter->SetRatio( false, true, false, value, 0 );
}

void PointGreyCamera::SetGainRatio( const float value )
{
	gain->SetRatio( false, true, false, value, 0 );
}

void PointGreyCamera::SetWhiteBalanceRatio( const float va, const float vb )
{
	white_balance->SetRatio( false, true, false, va, vb );
}

void PointGreyCamera::AutoBrightness( void )
{
	float	va, vb;

	brightness->GetRatio( va, vb );
	brightness->SetRatio( false, true, true, va, vb );
}
	
void PointGreyCamera::AutoExposure( void )
{
	float	va, vb;

	exposure->GetRatio( va, vb );
	exposure->SetRatio( false, true, true, va, vb );
}
	
void PointGreyCamera::AutoShutter( void )
{
	float	va, vb;

	shutter->GetRatio( va, vb );
	shutter->SetRatio( false, true, true, va, vb );
}
	
void PointGreyCamera::AutoGain( void )
{
	float	va, vb;

	gain->GetRatio( va, vb );
	gain->SetRatio( false, true, true, va, vb );
}

void PointGreyCamera::AutoWhiteBalance( void )
{
	float	va, vb;

	white_balance->GetRatio( va, vb );
	white_balance->SetRatio( false, true, true, va, vb );
}
	
void PointGreyCamera::ReportCameraInfo( const FlyCaptureInfoEx * pinfo )
{
	std::cout	<< "Fly Capture Camera Serial number: " 
					<< pinfo->SerialNumber << std::endl;
	FlyCaptureCameraType t = pinfo->CameraType;
	if ( t == FLYCAPTURE_COLOR ) { 
		std::cout	<< "Fly Capture Camera Type: Color" << std::endl; }
	else if ( t == FLYCAPTURE_BLACK_AND_WHITE ) { 
		std::cout	<< "Fly Capture Camera Type: Black And White" << std::endl; 
	}
	std::cout	<< "Fly Capture Camera Model: " 
					<< pinfo->pszModelName << std::endl;
	std::cout	<< "Fly Capture Camera Vendor: " 
					<< pinfo->pszVendorName << std::endl;
	std::cout	<< "Fly Capture Camera Sensor Info: " 
					<< pinfo->pszSensorInfo << std::endl;
	std::cout	<< "Fly Capture Camera DCAM Compliance Level: " 
					<< pinfo->iDCAMVer / 100.0f << std::endl;
	std::cout	<< "Fly Capture Camera Bus Number: " 
					<< pinfo->iBusNum << std::endl;
	std::cout	<< "Fly Capture Camera Node Number: "
					<< pinfo->iNodeNum << std::endl;
}

#if 0
int
PointGreyCamera::GetPropertyRange( FlyCaptureProperty p, Property& property )
{
	error = flycaptureGetCameraPropertyRangeEx(
		context,
		p,
		&property.is_present,
		&property.is_available_one_push,
		&property.is_available_read_out,
		&property.is_available_on_off,
		&property.is_available_auto,
		&property.is_available_manual,
		&property.min,
		&property.max );

	PG_HANDLE_ERROR( error, "flycaptureGetCameraPropertyRangeEx()" );
}
#endif

#if 0
int
PointGreyCamera::GetProperty( FlyCaptureProperty p, Property& property )
{
	error = flycaptureGetCameraPropertyEx(
		context,
		p,
		&property.is_one_push,
		&property.is_on_off,
		&property.is_auto,
		&property.value_a,
		&property.value_b );

	PG_HANDLE_ERROR( error, "flycaptureGetCameraPropertyEx()" );
}
#endif

#if 0
int
PointGreyCamera::GetPropertyRangeEx( FlyCaptureProperty p )
{
	bool	is_present, is_onepush, is_readout, is_onoff, is_auto, is_manual;
	int		min, max;

	error = flycaptureGetCameraPropertyRangeEx(
		context,
		p,
		&is_present,
		&is_onepush,
		&is_readout,
		&is_onoff,
		&is_auto,
		&is_manual,
		&min,
		&max );
	PG_HANDLE_ERROR( error, "flycaptureGetCameraPropertyRangeEx()" );

	cout << "presence:" << is_present << endl
		 << "one push:" << is_onepush << endl
		 << "read out:" << is_readout << endl
		 << "on / off:" << is_onoff << endl
		 << "auto    :" << is_auto << endl
		 << "manual  :" << is_manual << endl
		 << "range   :" << min << "-" << max << endl;

	return 0;
}
#endif

int PointGreyCamera::Init( void )
{
	// 
	// Enumerate the cameras on the bus.
	// 

	const unsigned int	MAX_CAMS = 32;
	FlyCaptureInfoEx  ar_info[MAX_CAMS];

	unsigned int size = MAX_CAMS;
	error = flycaptureBusEnumerateCamerasEx( ar_info, &size );
	PG_HANDLE_ERROR( error, "flycaptureBusEnumerateCameras()" );

	if (size == 0) {
		std::cout << "There is no Fly Capture Camera." << std::endl;
		// return -1;
	}

#if 0	
	for ( int i = 0; i < size; i++ ) 
	{
		FlyCaptureInfoEx*	pinfo = &ar_info[ i ];
		cout << "Index " << i << " : " << pinfo->pszModelName
				<< "(" << pinfo->SerialNumber << ")" << endl;
	}
#endif	

	//
	// Create the context.
	//
   
	error = flycaptureCreateContext( &context );
	PG_HANDLE_ERROR( error, "flycaptureCreateContext()" );
   
	//
	// Initialize the camera.
	//
   
	std::cout << "Initialize Camera " << camera_index << std::endl;
	error = flycaptureInitialize( context, camera_index );
	PG_HANDLE_ERROR( error, "flycaptureInitialize()" );

	//
	// Report camera info.
	//

	FlyCaptureInfoEx info;
	error = flycaptureGetCameraInfo( context, &info );
	PG_HANDLE_ERROR( error, "flycaptureGetCameraInfo()" );

	ReportCameraInfo( &info );

	//
	// set properties
	//

	brightness = new Property ( context, FLYCAPTURE_BRIGHTNESS );
	exposure = new Property ( context, FLYCAPTURE_AUTO_EXPOSURE );
	shutter = new Property ( context, FLYCAPTURE_SHUTTER );
	gain = new Property ( context, FLYCAPTURE_GAIN );
	white_balance = new Property ( context, FLYCAPTURE_WHITE_BALANCE );
		
#if 0	
	cout << "BRIGHTNESS:" << endl;
	GetPropertyRangeEx( FLYCAPTURE_BRIGHTNESS );
	cout << "AUTO EXPOSURE:" << endl;
    GetPropertyRangeEx( FLYCAPTURE_AUTO_EXPOSURE );
	cout << "WHITE BALANCE:" << endl;
    GetPropertyRangeEx( FLYCAPTURE_WHITE_BALANCE );
	cout << "SHUTTER:" << endl;
    GetPropertyRangeEx( FLYCAPTURE_SHUTTER );
	cout << "GAIN:" << endl;
    GetPropertyRangeEx( FLYCAPTURE_GAIN );

	// brightness: 0:255 / 0:511
	flycaptureSetCameraPropertyEx( context, FLYCAPTURE_BRIGHTNESS,
	//									false, true, false, 64, 0 );
										false, true, false, 50, 0 );
	// auto_exposure: 1:1023 / 0:498
	flycaptureSetCameraPropertyEx( context, FLYCAPTURE_AUTO_EXPOSURE, 
	//									false, true, false, 350, 0 );
										false, true, false, 250, 0 );
	// white_balance: 0: 63 / 0:255
	flycaptureSetCameraPropertyEx( context, FLYCAPTURE_WHITE_BALANCE,
	//									false, true, false, 30, 30 );
										false, true, false, 42, 54 );
	// shutter: 325: 1023 / 0:7
	flycaptureSetCameraPropertyEx( context, FLYCAPTURE_SHUTTER,
	//									false, true, false, 800, 0 );
										false, true, false, 100, 0 );
	// gain: 1: 1023 / 0:255
	flycaptureSetCameraPropertyEx( context, FLYCAPTURE_GAIN,
	//									false, true, false, 350, 0 );
										false, true, false, 400, 0 );
#endif

	//	SetColorProcessingMethod( DEFAULT_COLOR_PROCESSING_METHOD );
	SetColorProcessingMethod( FLYCAPTURE_RIGOROUS );
	PG_HANDLE_ERROR( error, "flycaptureSetColorProcessingMethod" );

#if 0	
	GetPropertyRange( FLYCAPTURE_BRIGHTNESS, brightness );
	GetPropertyRange( FLYCAPTURE_AUTO_EXPOSURE, exposure );
	GetPropertyRange( FLYCAPTURE_SHUTTER, shutter );
	GetPropertyRange( FLYCAPTURE_GAIN, gain );
	GetPropertyRange( FLYCAPTURE_WHITE_BALANCE, white_balance );

	GetProperty( FLYCAPTURE_BRIGHTNESS, brightness );
	GetProperty( FLYCAPTURE_AUTO_EXPOSURE, exposure );
	GetProperty( FLYCAPTURE_SHUTTER, shutter );
	GetProperty( FLYCAPTURE_GAIN, gain );
	GetProperty( FLYCAPTURE_WHITE_BALANCE, white_balance );
#endif	

	return 0;
}

int PointGreyCamera::InitSynchronized( void )
{	
	//
	// Enumerate the cameras on the bus.
	//

	const	unsigned int	MAX_CAMS = 32;
	FlyCaptureInfoEx  ar_info[ MAX_CAMS ];
	unsigned int	size = MAX_CAMS;

	error = flycaptureBusEnumerateCamerasEx( ar_info, &size );
	PG_HANDLE_ERROR( error, "flycaptureBusEnumerateCameras()" );

	//
	// Create the context.
	//

	error = flycaptureCreateContext( &g_arContext );
	PG_HANDLE_ERROR( error, "flycaptureCreateContext()" );

	//
	// Initialize the camera.
	//

	std::cout << "Initialize Multiple camera " << camera_index << std::endl;
	error = flycaptureInitialize( g_arContext, camera_index );
	PG_HANDLE_ERROR( error, "flycaptureInitialize()" );

	// 
	// Reset the camera to default factory settings by asserting bit 0
	// 
	error = flycaptureSetCameraRegister( g_arContext, INITIALIZE, 0x80000000 );
	PG_HANDLE_ERROR( error,"flycaptureSetCameraRegister()"  );

	// 
	// Power-up the camera (for cameras that support this feature)
	// 
	error = flycaptureSetCameraRegister( g_arContext, CAMERA_POWER, 0x80000000 );
	PG_HANDLE_ERROR( error,"flycaptureSetCameraRegister()"  );

	//
	// Report camera info.
	//

	FlyCaptureInfoEx info;
	error = flycaptureGetCameraInfo( g_arContext, &info );
	PG_HANDLE_ERROR( error, "flycaptureGetCameraInfo()" );

	ReportCameraInfo( &info );

	//
	// set properties
	//

	brightness		= new Property( g_arContext, FLYCAPTURE_BRIGHTNESS );
	exposure		= new Property( g_arContext, FLYCAPTURE_AUTO_EXPOSURE );
	shutter			= new Property( g_arContext, FLYCAPTURE_SHUTTER );
	gain			= new Property( g_arContext, FLYCAPTURE_GAIN );
	white_balance	= new Property( g_arContext, FLYCAPTURE_WHITE_BALANCE );

	//this->SetShutter(7);
	//this->SetShutterRatio( 7.0/982.0);
	flycaptureSetCameraAbsPropertyEx(g_arContext, FLYCAPTURE_SHUTTER, false, true, false, 6.02f);
	//flycaptureSetCameraAbsPropertyEx(g_arContext, FLYCAPTURE_SHUTTER, false, true, false, 7.2f);

	//	SetColorProcessingMethod( DEFAULT_COLOR_PROCESSING_METHOD );
	SetColorProcessingMethod( FLYCAPTURE_RIGOROUS );

	PG_HANDLE_ERROR( error, "flycaptureSetColorProcessingMethod" );

	//
	// Allocate a series of image buffers for every camera.
	//
	//allocateBuffers();
	for ( int uibuffer = 0; uibuffer < _BUFFERS; uibuffer ++ ){
		g_arpBuffers[uibuffer] = new unsigned char[width*height];
	}

	//
	// Create a context for and initialize every camera on the bus.
	//

	/*
	printf( "Initializing multiple capture camera %u.\n", 0 );
	error = ::flycaptureInitializePlus( 
		g_arContext, 
		camera_index,
		0,
		g_arpBuffers );
	_HANDLE_ERROR( error, "flycaptureInitializePlus()" );
	*/

	return 0;
}
int PointGreyCamera::StartSynchronized( int mode )
{
	if ( width == 1600 && height == 1200 ) { v = FLYCAPTURE_VIDEOMODE_1600x1200Y8; } 
	else if ( width == 1280 && height == 960 ) { v = FLYCAPTURE_VIDEOMODE_1280x960Y8; } 
	else if ( width == 1024 && height == 768 ) 
	{
#ifdef DRAGONFLY2
		v = FLYCAPTURE_VIDEOMODE_1024x768RGB;
#else
		v = FLYCAPTURE_VIDEOMODE_1024x768Y8;
#endif
	} 
	else if ( width == 800 && height == 600 ) 
	{
#ifdef DRAGONFLY2
		v = FLYCAPTURE_VIDEOMODE_800x600RGB;
#else
		v = FLYCAPTURE_VIDEOMODE_800x600Y8;
#endif
	} 
	else if ( width == 640 && height == 480 ) 
	{
#ifdef DRAGONFLY2
		v = FLYCAPTURE_VIDEOMODE_640x480RGB;
#else
		v = FLYCAPTURE_VIDEOMODE_640x480Y8;
#endif
	} 
	else if ( width == 320 && height == 240 ) { v = FLYCAPTURE_VIDEOMODE_320x240YUV422; } 
	else if ( width == 160 && height == 120 ) { v = FLYCAPTURE_VIDEOMODE_160x120YUV444; } 
	else {
		std::cerr << "PointGreyCamera::Start(): illegal video mode, set any" << std::endl;
		v = FLYCAPTURE_VIDEOMODE_ANY;
	}

	// 
	// Set frame rate
	// 
	if ( frame_rate == 60.0f ) { f = FLYCAPTURE_FRAMERATE_60; }
	else if ( frame_rate == 120.0f ) { f = FLYCAPTURE_FRAMERATE_120; }
	else if ( frame_rate == 30.0f ) { f = FLYCAPTURE_FRAMERATE_30; } 
	else if ( frame_rate == 15.0f ) { f = FLYCAPTURE_FRAMERATE_15; } 
	else if ( frame_rate == 7.5f ) { f = FLYCAPTURE_FRAMERATE_7_5; } 
	else {
		std::cerr << "PointGreyCamera::Start(): illegal frame rate, set any" << std::endl;
		f = FLYCAPTURE_FRAMERATE_ANY;
	}

	bool bTriggerPresent;
	error = flycaptureQueryTrigger ( g_arContext, &bTriggerPresent, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
	_HANDLE_ERROR( error ,"flycaptureQueryTrigger() ERROR \n" );

	if ( !bTriggerPresent ) {
		printf("This camera does not support external trigger... exiting\n");
		return 1;
	}

	int iPolarity, iSource, iRawValue, iMode;
	error = flycaptureGetTrigger( g_arContext, NULL, &iPolarity, &iSource, &iRawValue, &iMode, NULL );
	_HANDLE_ERROR( error  ,"flycaptureGetTrigger()");

	if( mode == 0 ) {
		error = flycaptureSetTrigger( g_arContext, true, iPolarity, iSource, 14, 0 );
		_HANDLE_ERROR( error , "flycaptureSetCameraTrigger()\n"  );
		printf( "Going into Hardware triggered asynchronous Trigger_Mode_0.\n" );
	}
	else if ( mode == 1 ){
		error = flycaptureSetTrigger( g_arContext, true, iPolarity, 7, 14, 0 );
		_HANDLE_ERROR( error,"flycaptureSetCameraTrigger()" );
		printf( "Going into Software triggered asynchronous Trigger_Mode_0.\n" );
	}
	
	printf( "Starting Synchronized Camera.\n\n" );

	is_capturing = true;
	error = flycaptureStart( g_arContext, v, f );
	_HANDLE_ERROR( error,"flycaptureStart()" );

	// 
	// Poll the camera to make sure the camera is actually in trigger mode
	// before we start it (avoids timeouts due to the trigger not being armed)
	// 

	checkTriggerReady( g_arContext );
	EmptyImageBuffer();
	// this->SendSoftwareTrigger();

	return 0;
}
int PointGreyCamera::EmptyImageBuffer()
{
	/*
	error = flycaptureSetGrabTimeoutEx(g_arContext,0);
	_HANDLE_ERROR( error, "flycaptureSetGrabTimeoutEx()" );
	
	FlyCaptureImage    image;
	while(error!=FLYCAPTURE_TIMEOUT) {
		_HANDLE_ERROR( error, "flycaptureSetGrabTimeoutEx()" );
		error = flycaptureGrabImage2( g_arContext, &image );
	}

	_HANDLE_ERROR( error, "flycaptureSetGrabTimeoutEx()" );
	error = flycaptureSetGrabTimeoutEx(g_arContext,5);
	_HANDLE_ERROR( error, "flycaptureSetGrabTimeoutEx()" );
	return -1;
	*/

	FlyCaptureError error = flycaptureSetGrabTimeoutEx(g_arContext, 0);
	int i = 0;
	while ( true ) 
	{
		FlyCaptureImage image;
		error = flycaptureGrabImage2(g_arContext, &image);
		if ( error == FLYCAPTURE_TIMEOUT ) {
			if ( i == 0 ) { printf("No images to be cleared found.\n"); }
			break;
		}
		else {
			printf("Image %i cleared\n", i);
		}
		i++;
	}

	printf("Images cleared\n");
	flycaptureSetGrabTimeoutEx(g_arContext, 30);
	
	return 0;
}

int PointGreyCamera::Start( void )
{
	// 
	// Set Video Mode
	// 根据 宽高 和 帧率设置 摄像头的捕捉模式开始捕捉
	// 

	if ( width == 1600 && height == 1200 ) { v = FLYCAPTURE_VIDEOMODE_1600x1200Y8; } 
	else if ( width == 1280 && height == 960 ) { v = FLYCAPTURE_VIDEOMODE_1280x960Y8; } 
	else if ( width == 1024 && height == 768 ) 
	{
#ifdef DRAGONFLY2
		v = FLYCAPTURE_VIDEOMODE_1024x768RGB;
#else
		v = FLYCAPTURE_VIDEOMODE_1024x768Y8;
#endif
	} 
	else if ( width == 800 && height == 600 ) 
	{
#ifdef DRAGONFLY2
		v = FLYCAPTURE_VIDEOMODE_800x600RGB;
#else
		v = FLYCAPTURE_VIDEOMODE_800x600Y8;
#endif
	} 
	else if ( width == 640 && height == 480 ) 
	{
#ifdef DRAGONFLY2
		v = FLYCAPTURE_VIDEOMODE_640x480RGB;
#else
		v = FLYCAPTURE_VIDEOMODE_640x480Y8;
#endif
	} 
	else if ( width == 320 && height == 240 ) { v = FLYCAPTURE_VIDEOMODE_320x240YUV422; } 
	else if ( width == 160 && height == 120 ) { v = FLYCAPTURE_VIDEOMODE_160x120YUV444; } 
	else {
		std::cerr << "PointGreyCamera::Start(): illegal video mode, set any" << std::endl;
		v = FLYCAPTURE_VIDEOMODE_ANY;
	}

	// 
	// Set Frame Rate
	// 
	
	if ( frame_rate == 60.0f ) { f = FLYCAPTURE_FRAMERATE_60; } 
	else if ( frame_rate == 30.0f ) { f = FLYCAPTURE_FRAMERATE_30; } 
	else if ( frame_rate == 15.0f ) { f = FLYCAPTURE_FRAMERATE_15; } 
	else if ( frame_rate == 7.5f ) { f = FLYCAPTURE_FRAMERATE_7_5; } 
	else {
		std::cerr << "PointGreyCamera::Start(): Illegal Frame Rate, Set any." << std::endl;
		f = FLYCAPTURE_FRAMERATE_ANY;
	}
	
	//
	// Start grabbing images in the current videomode and framerate.
	//
	
	std::cout << "Start Camera...\n" << std::endl;
	error = flycaptureStart ( context, v, f );
	PG_HANDLE_ERROR ( error, "flycaptureStart()" );

	is_capturing = true;

	return 0;
}

// 
// Capture Image
// 

int PointGreyCamera::Capture ( unsigned char * buf )
{
	FlyCaptureImage capturedImg;
	FlyCaptureImage convertedImg;

	error = flycaptureGrabImage2 ( context, &capturedImg );
	PG_HANDLE_ERROR( error, "flycaptureGrabImage2()" );

	unsigned char * tmpBuf = new unsigned char[(capturedImg.iRows) * (capturedImg.iCols) * 4];
	memset(tmpBuf, 0, (capturedImg.iRows) * (capturedImg.iCols) * 4);
	convertedImg.pData = tmpBuf;
	convertedImg.pixelFormat = FLYCAPTURE_BGRU;

	error = flycaptureConvertImage( context, &capturedImg, &convertedImg );
	PG_HANDLE_ERROR( error, "flycaptureConvertImage()" );

	unsigned char * srcRow = convertedImg.pData;
	// unsigned char * srcRow = tmpBuf;
	unsigned char * srcData = NULL;
	unsigned char * destData = buf;

	for (int i = 0; i < convertedImg.iRows; ++i, srcRow += convertedImg.iRowInc) 
	{
		srcData = srcRow;
		
		for (int j = 0; j < convertedImg.iCols; ++j, srcData += 4, destData += 4) 
		{
			destData[0] = srcData[2];
			destData[1] = srcData[1];
			destData[2] = srcData[0];

			destData[3] = srcData[3];
		}
	}

	// memcpy ( buf, tmpBuf, capturedImg.iRows * capturedImg.iRowInc );
	convertedImg.pData = NULL;
	delete [] tmpBuf;

	return 0;
}

int PointGreyCamera::SynchronizedCapture( unsigned char * buf)
{
	FlyCaptureImage image, convert;

	//
	// Start the camera and grab any excess images that are already in the pipe.
	// Although it is extremely rare for spurious images to occur, it is
	// possible for the grab call to return an image that is not a result of a
	// user-generated trigger. To grab excess images, set a zero-length timeout.
	// 
	// A value of zero makes the grab call non-blocking.
	//
	// printf( "Checking for any buffered images..." );
	// error = flycaptureSetGrabTimeoutEx( context, 0 );
	// _HANDLE_ERROR( error, "flycaptureSetGrabTimeoutEx()" );
	//

	error = flycaptureGrabImage2( g_arContext, &image );
	if ( error == FLYCAPTURE_OK ) {
		printf( "buffered image found. Flush successful.\n" );
	}
	else if ( error == FLYCAPTURE_TIMEOUT ) {
		printf( "FLYCAPTURE_TIMEOUT no flush required! (normal behaviour)\n" );
	}
	else {
		_HANDLE_ERROR( error, "flycaptureGrabImage2()" );
	}

	convert.pData = buf;
	convert.pixelFormat = FLYCAPTURE_BGRU;

	error = flycaptureConvertImage( g_arContext, &image, &convert );
	PG_HANDLE_ERROR( error, "flycaptureConvertImage() <-- SynchornizedCapture" );

	return 0;
}

int PointGreyCamera::SendSoftwareTrigger()
{
	checkTriggerReady( g_arContext );

	//
	// Camera is now ready to be triggered, so generate software trigger
	// by writing a '0' to bit 31
	//
	// printf( "Press the Enter key to initiate a software trigger.\n" );

	error = flycaptureSetCameraRegister ( g_arContext, SOFT_ASYNC_TRIGGER, 0x80000000 );
	PG_HANDLE_ERROR( error,"flycaptureSetCameraRegister()" );
	printf( "Software Trigger Send Already.\n" );
}

int PointGreyCamera::CaptureSingle( unsigned char * buf )
{
	FlyCaptureImage convert;

	// Unlock Image Buffers
	error = ::flycaptureLockNext( g_arContext, &g_arImageplus );
	PG_HANDLE_ERROR( error, "flycaptureLockNext() ERROR" );

	convert.pData = buf;
	convert.pixelFormat = FLYCAPTURE_BGRU;

	// error = flycaptureConvertImage( g_arContext, &g_arImageplus.image, &convert );
	// PG_HANDLE_ERROR( error, "flycaptureConvertImage()" );

	// error = ::flycaptureUnlock( g_arContext, g_arImageplus.uiBufferIndex );
	// _HANDLE_ERROR( error, "flycaptureUnlock ERROR" );

	// error = ::flycaptureStop( g_arContext );
	// _HANDLE_ERROR( error, "flycaptureStop()" );

	return 0;
}

int PointGreyCamera::CaptureMultiple( unsigned char **buf ,int count)
{
	error = ::flycaptureStartLockNext( g_arContext, v, f );
	_HANDLE_ERROR( error, "flycaptureStart()" );

	FlyCaptureImage convert;
	for ( unsigned int uiImage = 0; uiImage < count ; uiImage++ )
	{
		// Unlock Image Buffers
		error = ::flycaptureLockNext( g_arContext, &g_arImageplus );
		PG_HANDLE_ERROR( error, "flycaptureLockNext() ERROR" );

		error = ::flycaptureUnlock( g_arContext, g_arImageplus.uiBufferIndex );
		_HANDLE_ERROR( error, "flycaptureUnlock ERROR" );

#ifdef  DEBUG_CAMERA
		cout << "Captured Sequence "<<g_arImageplus.uiSeqNum << endl;
#endif

		convert.pData = buf[uiImage];
		convert.pixelFormat = FLYCAPTURE_BGRU;

		error = flycaptureConvertImage( g_arContext, &g_arImageplus.image, &convert );
		PG_HANDLE_ERROR( error, "flycaptureConvertImage()" );
	}

	error = ::flycaptureStop( g_arContext );
	_HANDLE_ERROR( error, "flycaptureStop()" );

	// for ( unsigned int uiImage = 0; uiImage < count ; uiImage++ )
	// {
	//		g_arImageplus.image.pData = g_arpBuffers[uiImage];
	//		convert.pData = buf[uiImage];
	//		convert.pixelFormat = FLYCAPTURE_BGRU;

	//		error = flycaptureConvertImage( g_arContext, &g_arImageplus.image, &convert );
	//		PG_HANDLE_ERROR( error, "flycaptureConvertImage()" );
	// }	

	return 0;
}

int PointGreyCamera::checkTriggerReady ( FlyCaptureContext context )
{
	// FlyCaptureError   error;
	if ( context == NULL )  { context = g_arContext; }

	// 
	// Do our check to make sure the camera is ready to be triggered
	// by looking at bits 30-31. Any value other than 1 indicates
	// the camera is not ready to be triggered.
	// 

	unsigned long ulValue;
	error = flycaptureGetCameraRegister( context, SOFT_ASYNC_TRIGGER, &ulValue );
	PG_HANDLE_ERROR( error, "flycaptureGetCameraRegister()"  );

	while ( ulValue != 0x80000001 )
	{
		error = flycaptureGetCameraRegister( context, SOFT_ASYNC_TRIGGER, &ulValue );
		PG_HANDLE_ERROR( error,"flycaptureGetCameraRegister()"  );
	}

	return FLYCAPTURE_OK;
}

int PointGreyCamera::Stop( void )
{
	//
	// Stop the camera
	//

	std::cout << "Stop camera" << std::endl;
	error = flycaptureStop( context );
	PG_HANDLE_ERROR( error, "flycaptureStop()" );

	return 0;
}

void PointGreyCamera::Brighter( void )
{
	float	s = GetShutterRatio();
	float	g = GetGainRatio();

	if ( s < 1.0f ) {
		s += 0.05f;
		if ( s > 1.0f ) {
			s = 1.0f;
		}
		SetShutterRatio( s );
	} else {													// s = 1.0f
		g += 0.05f;
		if ( g > 1.0f ) {
			g = 1.0f;
		}
		SetGainRatio( g );
	}
}

void PointGreyCamera::Darker( void )
{
	float	s = GetShutterRatio();
	float	g = GetGainRatio();

	if ( g > 0.0f ) {
		g -= 0.05f;
		if ( g < 0.0f ) {
			g = 0.0f;
		}
		SetGainRatio( g );
	} else {													// g = 0.0f
		s -= 0.05f;
		if ( s < 0.0f ) {
			s = 0.0f;
		}
		SetShutterRatio( s );
	}
}
