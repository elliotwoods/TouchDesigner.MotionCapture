//-------------------------------------------------------------------------------------------------------------------
// xiAPIplus implementation file
// XIMEA Application Programming Interface Object Oriented Approach
//-------------------------------------------------------------------------------------------------------------------

#include <os_common_header.h>
#include <stdio.h>
#include <xiAPIplus/xiapiplus.h>

#ifdef WIN32
// disable warning C4996: 'sprintf': This function or variable may be unsafe - use _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#endif

// forward declarations

void ReportException(XI_RETURN res, const char* location);

// -----------------------------------------------
// parameters handlers implementation
// -----------------------------------------------

#include "xiAPIplus_parameters.cpp"

// -----------------------------------------------
// xiAPIplus
// -----------------------------------------------

unsigned long xiAPIplus::GetNumberOfConnectedCameras()
{
	DWORD count=0;
	XI_RETURN res=xiGetNumberDevices(&count);
	if (res) ReportException(res,"GetNumberOfConnectedCameras");
	return count;
}

#if 0
// -----------------------------------------------
// general parametes interface
// working with set of parameters
// -----------------------------------------------

void xiAPIplus_Camera_Parameters::Save_to_File(char* filename);
void xiAPIplus_Camera_Parameters::Load_from_File(char* filename);
void xiAPIplus_Camera_Parameters::Save_to_Preset(int id);
void xiAPIplus_Camera_Parameters::Load_from_Preset(int id);
int xiAPIplus_Camera_Parameters::Get_Count();
void xiAPIplus_Camera_Parameters::Get_Name(int id, char* name, int name_max_size);
void xiAPIplus_Camera_Parameters::Get_Value(int id, char* value, int value_max_size);
void xiAPIplus_Camera_Parameters::Set_Value(int id, char* value);

#endif // 0

// exception handling
void ReportException(XI_RETURN res, const char* location)
{
	// generate exception
	char descr[200]="(missing_error_description)";
	int total_errors=sizeof(xiAPIplus_errorcodes_list)/sizeof(xiapi_errorcode_t);
	// find description
	for(int i=0;i<total_errors;i++)
	{
		if (xiAPIplus_errorcodes_list[i].code == res)
		{
			// found
			const char* d=xiAPIplus_errorcodes_list[i].descr;
			if (sizeof(descr) > strlen(d) + sizeof("%s (xiAPIplus_Camera::%s)") + strlen(location))
			{
				sprintf(descr,"%s (xiAPIplus_Camera::%s)",xiAPIplus_errorcodes_list[i].descr,location);
			}
			else
			{
				strcpy(descr,"(descr_too_long)");
			}
			break;
		}
	}
	throw xiAPIplus_Exception(res, descr);
}

void xiAPIplus_Camera::CheckResult(XI_RETURN res, const char* location)
{
	xiAPIPlusDP((DBFOUT "%d\n",res));
	if (res != XI_OK)
	{
		ReportException(res, location);
	}
}

void xiAPIplus_Camera::CheckResultParam(XI_RETURN res, const char* location, const char* param)
{
	xiAPIPlusDP((DBFOUT "%d\n",res));
	if (res != XI_OK)
	{
		char buff[MAX_PATH] = "";
		sprintf(buff, "%s - %s", location, param);
		ReportException(res, buff);
	}
}

void xiAPIplus_Exception::GetDescription(char* name, size_t name_max_size)
{
	if (!name) return;
	if (!description) return;
	if (name_max_size<sizeof(description)) return;
	strncpy(name,description,name_max_size);
}

void xiAPIplus_Exception::PrintError()
{
	char descr[200]="";
	GetDescription(descr,sizeof(descr)-1);
	printf("xiAPIplus-Error %d:%s\n",GetErrorNumber(),descr);
}

unsigned long xiAPIplus_Camera::GetNumberOfConnectedCameras()
{
	xiAPIplus api;
	return api.GetNumberOfConnectedCameras();
}

// -----------------------------------------------
// class xiAPIplus - Image
// -----------------------------------------------

xiAPIplus_Image::xiAPIplus_Image()
{
	memset(&image,0,sizeof(image));
	image.size = sizeof(image);
	is_copy = false;
}

// -----------------------------------------------
// class xiAPIplus - Camera
// -----------------------------------------------

xiAPIplus_Camera::xiAPIplus_Camera()
{
	camera_handle=NULL;
	image_timeout_ms=1000;
	acquisition_active=false;
	is_debug_enabled=false;
}

xiAPIplus_Camera::~xiAPIplus_Camera()
{
	Close();
}

void xiAPIplus_Camera::OpenFirst()
{
	OpenByID(0);
}

void xiAPIplus_Camera::OpenByID(unsigned long id)
{
	xiAPIPlusDP((DBFIN "%s\n","OpenByID"));
	XI_RETURN res=xiOpenDevice(id, &camera_handle);
	CheckResult(res,"OpenByID");

	if(!camera_handle) res = XI_INVALID_HANDLE;
	CheckResult(res,"OpenByID");
}
void xiAPIplus_Camera::OpenBySN(char* serial_number)
{
	if (!serial_number)  ReportException(XI_WRONG_PARAM_VALUE, "xiAPIplus_Camera::OpenBySN()");

	xiAPIPlusDP((DBFIN "%s\n", "OpenBySN"));
	XI_RETURN res = xiOpenDeviceBy(XI_OPEN_BY_SN, serial_number, &camera_handle);
	CheckResult(res, "OpenBySN");

	if (!camera_handle) res = XI_INVALID_HANDLE;
	CheckResult(res, "OpenBySN");
}

void xiAPIplus_Camera::OpenByPath(char* device_path)
{
	if(!device_path)  ReportException(XI_WRONG_PARAM_VALUE, "xiAPIplus_Camera::OpenByPath()");

	xiAPIPlusDP((DBFIN "%s\n","OpenByPath"));
	XI_RETURN res=xiOpenDeviceBy(XI_OPEN_BY_INST_PATH, device_path, &camera_handle);
	CheckResult(res,"OpenByPath");

	if(!camera_handle) res = XI_INVALID_HANDLE;
	CheckResult(res,"OpenByPath");
}

void xiAPIplus_Camera::OpenByUserID(char* user_id)
{
	if(!user_id)  ReportException(XI_WRONG_PARAM_VALUE, "xiAPIplus_Camera::OpenByUserID()");

	xiAPIPlusDP((DBFIN "%s\n","OpenByUserID"));
	XI_RETURN res=xiOpenDeviceBy(XI_OPEN_BY_USER_ID, user_id, &camera_handle);
	CheckResult(res,"OpenByUserID");

	if(!camera_handle) res = XI_INVALID_HANDLE;
	CheckResult(res,"OpenByUserID");
}

void xiAPIplus_Camera::OpenByLocation(char* location)
{
	if(!location)  ReportException(XI_WRONG_PARAM_VALUE, "xiAPIplus_Camera::OpenByLocation()");

	xiAPIPlusDP((DBFIN "%s\n","OpenByUserID"));
	XI_RETURN res=xiOpenDeviceBy(XI_OPEN_BY_LOC_PATH, location, &camera_handle);
	CheckResult(res,"OpenByLocation");

	if(!camera_handle) res = XI_INVALID_HANDLE;
	CheckResult(res,"OpenByLocation");
}

// ---------------------------------------------------------------------
// camera close

void xiAPIplus_Camera::Close()
{
	xiAPIPlusDP((DBFIN "%s\n","Close"));
	if (camera_handle)
	{
		XI_RETURN res=xiCloseDevice(camera_handle);
		CheckResult(res,"Close");
	}
	camera_handle=NULL;
}

// ---------------------------------------------------------------------
// acquisition
void xiAPIplus_Camera::StartAcquisition()
{
	CheckCamHandle("StartAcquisition");
	XI_RETURN res=xiStartAcquisition(camera_handle);
	CheckResult(res,"StartAcquisition");
	acquisition_active=true;
}

void xiAPIplus_Camera::StopAcquisition()
{
	CheckCamHandle("StopAcquisition");
	XI_RETURN res=xiStopAcquisition(camera_handle);
	CheckResult(res,"StopAcquisition");
	acquisition_active=false;
}

bool xiAPIplus_Camera::IsAcquisitionActive()
{
	return acquisition_active;
}

// ---------------------------------------------------------------------
// get next image
// receiving next image from xiAPI
// if (app_image is defined) storing result to app_image
// else storing result to last_image, also returing pointer to it

xiAPIplus_Image* xiAPIplus_Camera::GetNextImage(xiAPIplus_Image* app_image)
{
	CheckCamHandle("GetNextImage");
	xiAPIplus_Image* image=&last_image;
	if (app_image)
		image = app_image;

	XI_RETURN res = XI_OK;
	res = xiGetImage(camera_handle, image_timeout_ms, image->GetXI_IMG());
	CheckResult(res,"GetNextImage");
	return image;
}

// ---------------------------------------------------------------------
// get last received image
// returns last received image (by function GetNextImage)

xiAPIplus_Image* xiAPIplus_Camera::GetLastImage()
{
	return &last_image;
}

// ---------------------------------------------------------------------
// set next image timeout in milliseconds

void xiAPIplus_Camera::SetNextImageTimeout_ms(int timeout_ms)
{
	image_timeout_ms=timeout_ms;
}

// ---------------------------------------------------------------------
// get next image timeout in milliseconds

int xiAPIplus_Camera::GetNextImageTimeout_ms()
{
	return image_timeout_ms;
}

// ---------------------------------------------------------------------
// return xiAPI parameter by name

void xiAPIplus_Camera::GetXIAPIParamString(char* xiapi_param_name, char* value, int value_max_size)
{
	CheckCamHandle("GetXIAPIParamStr");
	XI_RETURN res=xiGetParamString(camera_handle, xiapi_param_name, value, value_max_size);
	CheckResultParam(res,"GetXIAPIParamStr", xiapi_param_name);
}

void xiAPIplus_Camera::SetXIAPIParamString(char* xiapi_param_name, char* value, unsigned int value_max_size)
{
	CheckCamHandle("SetXIAPIParamStr");
	XI_RETURN res=xiSetParamString(camera_handle, xiapi_param_name, value, value_max_size);
	CheckResultParam(res,"SetXIAPIParamStr", xiapi_param_name);
}

void xiAPIplus_Camera::GetXIAPIParamInt(char* xiapi_param_name, int* value)
{
	CheckCamHandle("GetXIAPIParamInt");
	XI_RETURN res=xiGetParamInt(camera_handle, xiapi_param_name, value);
	CheckResultParam(res,"GetXIAPIParamInt", xiapi_param_name);
}
void xiAPIplus_Camera::SetXIAPIParamInt(char* xiapi_param_name, int value)
{
	CheckCamHandle("SetXIAPIParamInt");
	XI_RETURN res=xiSetParamInt(camera_handle, xiapi_param_name, value);
	CheckResultParam(res,"SetXIAPIParamInt", xiapi_param_name);
}

void xiAPIplus_Camera::GetXIAPIParamFloat(char* xiapi_param_name, float* value)
{
	CheckCamHandle("GetXIAPIParamFloat");
	XI_RETURN res=xiGetParamFloat(camera_handle, xiapi_param_name, value);
	CheckResultParam(res,"GetXIAPIParamFloat", xiapi_param_name);
}

void xiAPIplus_Camera::SetXIAPIParamFloat(char* xiapi_param_name, float value)
{
	CheckCamHandle("SetXIAPIParamFloat");
	XI_RETURN res=xiSetParamFloat(camera_handle, xiapi_param_name, value);
	CheckResultParam(res,"SetXIAPIParamFloat", xiapi_param_name);
}

void xiAPIplus_Camera::SetXIAPIParam(char* xiapi_param_name, void* value, size_t value_size, XI_PRM_TYPE type)
{
	CheckCamHandle("SetXIAPIParam");
	XI_RETURN res=xiSetParam(camera_handle, xiapi_param_name, value, (DWORD)value_size,type);
	CheckResultParam(res,"SetXIAPIParam", xiapi_param_name);
}

void xiAPIplus_Camera::GetXIAPIParam(char* xiapi_param_name, void* value, size_t * value_size, XI_PRM_TYPE * type)
{
	CheckCamHandle("GetXIAPIParam");
	XI_RETURN res=xiGetParam(camera_handle, xiapi_param_name, value, (DWORD*)value_size,type);
	CheckResultParam(res,"GetXIAPIParam", xiapi_param_name);
}

// ---------------------------------------------------------------------
// Image Destructor

xiAPIplus_Image::~xiAPIplus_Image()
{
	if (IsCopy())
	{
		// desctruct copy
		XI_IMG* image=GetXI_IMG();
		if (image->bp && GetPixelsArraySize())
		{
			if(image->bp)
				free(image->bp);
		}
	}
}

// ---------------------------------------------------------------------
// get bytes per pixel

int xiAPIplus_Image::GetBytesPerPixel()
{
	int bpp=1;
	switch (image.frm)
	{
	case XI_MONO8:
	case XI_RAW8:
	case XI_RAW8X2:
	case XI_RAW8X4:
	case XI_RGB_PLANAR:
		bpp=1;
		break;
	case XI_MONO16:
	case XI_RAW16:
	case XI_RAW16X2:
	case XI_RAW16X4:
	case XI_RGB16_PLANAR:
		bpp=2;
		break;
	case XI_RGB24:
		bpp=3;
		break;
	case XI_RAW32:
	case XI_RGB32:
	case XI_RAW32FLOAT:
		bpp=4;
		break;
	case XI_RGB48:
		bpp = 6;
		break;
	case XI_RGB64:
		bpp = 8;
		break;
	default:
		ReportException(XI_NOT_SUPPORTED_DATA_FORMAT, "GetBytesPerPixel");
		break;
	}
	return bpp;
}

// ---------------------------------------------------------------------

int xiAPIplus_Image::GetBitCount()
{
	int vals=1;
	switch (image.frm)
	{
	case XI_MONO8:
	case XI_RAW8:
	case XI_RAW8X2:
	case XI_RAW8X4:
	case XI_RGB_PLANAR:	vals = 8; break;
	case XI_MONO16:
	case XI_RAW16:
	case XI_RAW16X2:
	case XI_RAW16X4:
	case XI_RGB16_PLANAR: vals = 16; break;
	case XI_RGB24:		vals = 24; break;
	case XI_RAW32:
	case XI_RGB32:
	case XI_RAW32FLOAT:	vals = 32; break;
	case XI_RGB48:		vals = 48; break;
	case XI_RGB64:		vals = 64; break;
	default:
		ReportException(XI_NOT_SUPPORTED_DATA_FORMAT, "GetBitCount");
		break;
	}
	return vals;
}

// ---------------------------------------------------------------------
// Image Clone
// make new copy of existing image
// this copy should be deleted by caller after finishing of use

void xiAPIplus_Image::CopyTo(xiAPIplus_Image* image_copy)
{
	XI_IMG* orig=GetXI_IMG();
	XI_IMG* clone=image_copy->GetXI_IMG();
	// copy structure
	memcpy(clone,orig,sizeof(XI_IMG));
	// copy image data
	clone->bp = malloc(GetPixelsArraySize());
	if (!clone->bp)
	{
		ReportException(XI_MEMORY_ALLOCATION, "CopyTo");
	}else memcpy(clone->bp, orig->bp, GetPixelsArraySize());
}

// ---------------------------------------------------------------------
// return core xiAPI image

XI_IMG* xiAPIplus_Image::GetXI_IMG()
{
	return &image;
}

// ---------------------------------------------------------------------

int xiAPIplus_Image::GetPadding_X()
{
	switch(image.frm)
	{
	case XI_RAW16:
	case XI_MONO16:
		return image.padding_x/2;
	default:
		return image.padding_x;
	}
}

// ---------------------------------------------------------------------

xiAPIplus_ImageProcessing::xiAPIplus_ImageProcessing()
{
	processing_handle_ = NULL;
	XI_RETURN res=xiProcOpen(&processing_handle_);
	CheckResult(res,"xiProcOpen");
}

// ---------------------------------------------------------------------

xiAPIplus_ImageProcessing::~xiAPIplus_ImageProcessing()
{
	XI_RETURN res=xiProcClose(processing_handle_);
	CheckResult(res,"xiProcClose");
}

// ---------------------------------------------------------------------

void xiAPIplus_ImageProcessing::CheckResult(XI_RETURN res, const char* location)
{
	if (res != XI_OK)
	{
		ReportException(res, location);
	}
}

// -------------------------------------------------------------------

void xiAPIplus_ImageProcessing::SetParam(const char* prm, void* val, DWORD size, XI_PRM_TYPE type)
{
	XI_RETURN res=xiProcSetParam(processing_handle_,prm,val,size,type);
	CheckResult(res,"xiProcSetParam");
}

// ---------------------------------------------------------------------

void xiAPIplus_ImageProcessing::PushImage(unsigned char* first_pixel)
{
	XI_RETURN res=xiProcPushImage(processing_handle_,first_pixel);
	CheckResult(res,"xiProcPushImage");
}

// ---------------------------------------------------------------------

void xiAPIplus_ImageProcessing::PullImage()
{
	XI_RETURN res=xiProcPullImage(processing_handle_,0, last_image_.GetXI_IMG());
	CheckResult(res,"xiPullImage");
}

// ---------------------------------------------------------------------

void xiAPIplus_ImageProcessing::GetImage(xiAPIplus_Image* ret_image)
{
	last_image_.CopyTo(ret_image);
}

// ---------------------------------------------------------------------