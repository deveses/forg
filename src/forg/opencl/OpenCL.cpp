#include "forg_pch.h"

#include <config.h>

#if defined(FORG_ENABLE_OPENCL)

#include "OpenCL.h"
#include <forg.h>

#include <CL/opencl.h>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

namespace OpenCL {

    enum { MAX_NUM_PLATFORMS = 64 };
    enum { MAX_NUM_DEVICES = 64 };


    /////////////////////////////////////////////////////////////////////////////////////

#define CL_CASE_ERROR_MSG(x) case x: return #x;
    LPCSTR CLGetErrorString(cl_int err)
    {
        switch (err) {
            CL_CASE_ERROR_MSG(CL_SUCCESS);
            CL_CASE_ERROR_MSG(CL_DEVICE_NOT_FOUND);
            CL_CASE_ERROR_MSG(CL_DEVICE_NOT_AVAILABLE);
            CL_CASE_ERROR_MSG(CL_COMPILER_NOT_AVAILABLE);
            CL_CASE_ERROR_MSG(CL_MEM_OBJECT_ALLOCATION_FAILURE);
            CL_CASE_ERROR_MSG(CL_OUT_OF_RESOURCES);
            CL_CASE_ERROR_MSG(CL_OUT_OF_HOST_MEMORY);
            CL_CASE_ERROR_MSG(CL_PROFILING_INFO_NOT_AVAILABLE);
            CL_CASE_ERROR_MSG(CL_MEM_COPY_OVERLAP);
            CL_CASE_ERROR_MSG(CL_IMAGE_FORMAT_MISMATCH);
            CL_CASE_ERROR_MSG(CL_IMAGE_FORMAT_NOT_SUPPORTED);
            CL_CASE_ERROR_MSG(CL_BUILD_PROGRAM_FAILURE);
            CL_CASE_ERROR_MSG(CL_MAP_FAILURE);
            CL_CASE_ERROR_MSG(CL_MISALIGNED_SUB_BUFFER_OFFSET);
            CL_CASE_ERROR_MSG(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);

            CL_CASE_ERROR_MSG(CL_INVALID_VALUE);
            CL_CASE_ERROR_MSG(CL_INVALID_DEVICE_TYPE);
            CL_CASE_ERROR_MSG(CL_INVALID_PLATFORM);
            CL_CASE_ERROR_MSG(CL_INVALID_DEVICE);
            CL_CASE_ERROR_MSG(CL_INVALID_CONTEXT);
            CL_CASE_ERROR_MSG(CL_INVALID_QUEUE_PROPERTIES);
            CL_CASE_ERROR_MSG(CL_INVALID_COMMAND_QUEUE);
            CL_CASE_ERROR_MSG(CL_INVALID_HOST_PTR);
            CL_CASE_ERROR_MSG(CL_INVALID_MEM_OBJECT);
            CL_CASE_ERROR_MSG(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
            CL_CASE_ERROR_MSG(CL_INVALID_IMAGE_SIZE);
            CL_CASE_ERROR_MSG(CL_INVALID_SAMPLER);
            CL_CASE_ERROR_MSG(CL_INVALID_BINARY);
            CL_CASE_ERROR_MSG(CL_INVALID_BUILD_OPTIONS);
            CL_CASE_ERROR_MSG(CL_INVALID_PROGRAM);
            CL_CASE_ERROR_MSG(CL_INVALID_PROGRAM_EXECUTABLE);
            CL_CASE_ERROR_MSG(CL_INVALID_KERNEL_NAME);
            CL_CASE_ERROR_MSG(CL_INVALID_KERNEL_DEFINITION);
            CL_CASE_ERROR_MSG(CL_INVALID_KERNEL);
            CL_CASE_ERROR_MSG(CL_INVALID_ARG_INDEX);
            CL_CASE_ERROR_MSG(CL_INVALID_ARG_VALUE);
            CL_CASE_ERROR_MSG(CL_INVALID_ARG_SIZE);
            CL_CASE_ERROR_MSG(CL_INVALID_KERNEL_ARGS);
            CL_CASE_ERROR_MSG(CL_INVALID_WORK_DIMENSION);
            CL_CASE_ERROR_MSG(CL_INVALID_WORK_GROUP_SIZE);
            CL_CASE_ERROR_MSG(CL_INVALID_WORK_ITEM_SIZE);
            CL_CASE_ERROR_MSG(CL_INVALID_GLOBAL_OFFSET);
            CL_CASE_ERROR_MSG(CL_INVALID_EVENT_WAIT_LIST);
            CL_CASE_ERROR_MSG(CL_INVALID_EVENT);
            CL_CASE_ERROR_MSG(CL_INVALID_OPERATION);
            CL_CASE_ERROR_MSG(CL_INVALID_GL_OBJECT);
            CL_CASE_ERROR_MSG(CL_INVALID_BUFFER_SIZE);
            CL_CASE_ERROR_MSG(CL_INVALID_MIP_LEVEL);
            CL_CASE_ERROR_MSG(CL_INVALID_GLOBAL_WORK_SIZE);
            CL_CASE_ERROR_MSG(CL_INVALID_PROPERTY);
        default:
            return "Unknown Error";
        }

        return 0;
    }
#undef CL_CASE_ERROR_MSG

    cl_int CLErrorCheck(cl_int error_code, int line, LPCSTR file, LPCSTR func)
    {
        if (error_code != CL_SUCCESS)
        {
            char dbuf[512];

            if (func && func[0] != 0)
            {
                sprintf(
                    dbuf,
                    _T("*** Unexpected error encountered! ***\n%s(%d): Error Code: %s (0x%x) Calling: %s\n\n"),
                    file, line, CLGetErrorString(error_code), error_code, func
                    );
            }
            else
            {
                sprintf(
                    dbuf,
                    _T("*** Unexpected error encountered! ***\n%s(%d): Error Code: %s (0x%x)\n\n"),
                    file, line, CLGetErrorString(error_code), error_code);
            }

            forg::debug::DbgOutputString(dbuf);
        }

        return error_code;
    }

#define CLV(x) OpenCL::CLErrorCheck((x), __LINE__, __FILE__, "")

    /////////////////////////////////////////////////////////////////////////////////////

    /* Platform API */
    typedef cl_int(*PFNCLGETPLATFORMIDS)(cl_uint          /* num_entries */,
        cl_platform_id * /* platforms */,
        cl_uint *        /* num_platforms */);

    typedef cl_int(*PFNCLGETPLATFORMINFO)(cl_platform_id   /* platform */,
        cl_platform_info /* param_name */,
        size_t           /* param_value_size */,
        void *           /* param_value */,
        size_t *         /* param_value_size_ret */);

    /* Device APIs */
    typedef cl_int(*PFNCLGETDEVICEIDS)(cl_platform_id   /* platform */,
        cl_device_type   /* device_type */,
        cl_uint          /* num_entries */,
        cl_device_id *   /* devices */,
        cl_uint *        /* num_devices */);

    typedef cl_int(*PFNCLGETDEVICEINFO)(cl_device_id    /* device */,
        cl_device_info  /* param_name */,
        size_t          /* param_value_size */,
        void *          /* param_value */,
        size_t *        /* param_value_size_ret */);

    typedef cl_context(*PFNCLCREATECONTEXT)(const cl_context_properties * /* properties */,
        cl_uint                       /* num_devices */,
        const cl_device_id *          /* devices */,
        void (CL_CALLBACK * /* pfn_notify */)(const char *, const void *, size_t, void *),
        void *                        /* user_data */,
        cl_int *                      /* errcode_ret */);

    typedef cl_int(*PFNCLRELEASECONTEXT)(cl_context /* context */);

    typedef cl_int(*PFNCLRETAINCONTEXT)(cl_context /* context */);

    /* Command Queue APIs */
    typedef cl_command_queue(*PFNCLCREATECOMMANDQUEUE)(cl_context                     /* context */,
        cl_device_id                   /* device */,
        cl_command_queue_properties    /* properties */,
        cl_int *                       /* errcode_ret */);

    typedef cl_int(*PFNCLRELEASECOMMANDQUEUE)(cl_command_queue /* command_queue */);

    typedef cl_int (*PFNCLRETAINCOMMANDQUEUE)(cl_command_queue /* command_queue */);

    /* Memory Object APIs */
    typedef cl_mem(*PFNCLCREATEBUFFER)(cl_context   /* context */,
        cl_mem_flags /* flags */,
        size_t       /* size */,
        void *       /* host_ptr */,
        cl_int *     /* errcode_ret */);

    typedef cl_mem (*PFNCLCREATESUBBUFFER)(cl_mem                   /* buffer */,
        cl_mem_flags             /* flags */,
        cl_buffer_create_type    /* buffer_create_type */,
        const void *             /* buffer_create_info */,
        cl_int *                 /* errcode_ret */);

    typedef cl_mem (*PFNCLCREATEIMAGE2D)(cl_context              /* context */,
        cl_mem_flags            /* flags */,
        const cl_image_format * /* image_format */,
        size_t                  /* image_width */,
        size_t                  /* image_height */,
        size_t                  /* image_row_pitch */,
        void *                  /* host_ptr */,
        cl_int *                /* errcode_ret */);

    typedef cl_mem (*PFNCLCREATEIMAGE3D)(cl_context              /* context */,
        cl_mem_flags            /* flags */,
        const cl_image_format * /* image_format */,
        size_t                  /* image_width */,
        size_t                  /* image_height */,
        size_t                  /* image_depth */,
        size_t                  /* image_row_pitch */,
        size_t                  /* image_slice_pitch */,
        void *                  /* host_ptr */,
        cl_int *                /* errcode_ret */);

    typedef cl_int(*PFNCLRELEASEMEMOBJECT)(cl_mem /* memobj */);

    /* Program Object APIs  */
    typedef cl_program(*PFNCLCREATEPROGRAMWITHSOURCE)(cl_context        /* context */,
        cl_uint           /* count */,
        const char **     /* strings */,
        const size_t *    /* lengths */,
        cl_int *          /* errcode_ret */);

    typedef cl_int(*PFNCLGETPROGRAMBUILDINFO)(cl_program            /* program */,
        cl_device_id          /* device */,
        cl_program_build_info /* param_name */,
        size_t                /* param_value_size */,
        void *                /* param_value */,
        size_t *              /* param_value_size_ret */);

    typedef cl_int(*PFNCLRELEASEPROGRAM)(cl_program /* program */);

    typedef cl_int(*PFNCLBUILDPROGRAM)(cl_program           /* program */,
        cl_uint              /* num_devices */,
        const cl_device_id * /* device_list */,
        const char *         /* options */,
        void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
        void *               /* user_data */);

    /* Kernel Object APIs */
    typedef cl_kernel(*PFNCLCREATEKERNEL)(cl_program      /* program */,
        const char *    /* kernel_name */,
        cl_int *        /* errcode_ret */);

    typedef cl_int(*PFNCLRELEASEKERNEL)(cl_kernel   /* kernel */);

    typedef cl_int(*PFNCLSETKERNELARG)(cl_kernel    /* kernel */,
        cl_uint      /* arg_index */,
        size_t       /* arg_size */,
        const void * /* arg_value */);

    typedef cl_int(*PFNCLGETKERNELWORKGROUPINFO)(cl_kernel /* kernel */,
        cl_device_id               /* device */,
        cl_kernel_work_group_info  /* param_name */,
        size_t                     /* param_value_size */,
        void *                     /* param_value */,
        size_t *                   /* param_value_size_ret */);

    /* Flush and Finish APIs */
    typedef cl_int(*PFNCLFLUSH)(cl_command_queue /* command_queue */);

    typedef cl_int(*PFNCLFINISH)(cl_command_queue /* command_queue */);

    /* Enqueued Commands APIs */
    typedef cl_int(*PFNCLENQUEUEREADBUFFER)(cl_command_queue    /* command_queue */,
        cl_mem              /* buffer */,
        cl_bool             /* blocking_read */,
        size_t              /* offset */,
        size_t              /* cb */,
        void *              /* ptr */,
        cl_uint             /* num_events_in_wait_list */,
        const cl_event *    /* event_wait_list */,
        cl_event *          /* event */);

    typedef cl_int (*PFNCLENQUEUEWRITEBUFFER)(cl_command_queue   /* command_queue */,
        cl_mem             /* buffer */,
        cl_bool            /* blocking_write */,
        size_t             /* offset */,
        size_t             /* cb */,
        const void *       /* ptr */,
        cl_uint            /* num_events_in_wait_list */,
        const cl_event *   /* event_wait_list */,
        cl_event *         /* event */);

    typedef cl_int(*PFNCLENQUEUEREADIMAGE)(cl_command_queue     /* command_queue */,
        cl_mem               /* image */,
        cl_bool              /* blocking_read */,
        const size_t *       /* origin[3] */,
        const size_t *       /* region[3] */,
        size_t               /* row_pitch */,
        size_t               /* slice_pitch */,
        void *               /* ptr */,
        cl_uint              /* num_events_in_wait_list */,
        const cl_event *     /* event_wait_list */,
        cl_event *           /* event */);

    typedef cl_int(*PFNCLENQUEUEWRITEIMAGE)(cl_command_queue    /* command_queue */,
        cl_mem              /* image */,
        cl_bool             /* blocking_write */,
        const size_t *      /* origin[3] */,
        const size_t *      /* region[3] */,
        size_t              /* input_row_pitch */,
        size_t              /* input_slice_pitch */,
        const void *        /* ptr */,
        cl_uint             /* num_events_in_wait_list */,
        const cl_event *    /* event_wait_list */,
        cl_event *          /* event */);

    typedef cl_int(*PFNCLENQUEUENDRANGEKERNEL)(cl_command_queue /* command_queue */,
        cl_kernel        /* kernel */,
        cl_uint          /* work_dim */,
        const size_t *   /* global_work_offset */,
        const size_t *   /* global_work_size */,
        const size_t *   /* local_work_size */,
        cl_uint          /* num_events_in_wait_list */,
        const cl_event * /* event_wait_list */,
        cl_event *       /* event */);

    PFNCLGETPLATFORMIDS clGetPlatformIDs;
    PFNCLGETPLATFORMINFO clGetPlatformInfo;
    PFNCLGETDEVICEIDS clGetDeviceIDs;
    PFNCLGETDEVICEINFO clGetDeviceInfo;

    PFNCLCREATECONTEXT clCreateContext;
    PFNCLRELEASECONTEXT clReleaseContext;
    PFNCLRETAINCONTEXT clRetainContext;

    PFNCLCREATEPROGRAMWITHSOURCE clCreateProgramWithSource;
    PFNCLBUILDPROGRAM clBuildProgram;
    PFNCLGETPROGRAMBUILDINFO clGetProgramBuildInfo;
    PFNCLRELEASEPROGRAM clReleaseProgram;

    PFNCLCREATEKERNEL clCreateKernel;
    PFNCLRELEASEKERNEL clReleaseKernel;
    PFNCLSETKERNELARG clSetKernelArg;
    PFNCLGETKERNELWORKGROUPINFO clGetKernelWorkGroupInfo;

    PFNCLCREATEBUFFER clCreateBuffer;
    PFNCLCREATESUBBUFFER clCreateSubBuffer;
    PFNCLCREATEIMAGE2D clCreateImage2D;
    PFNCLCREATEIMAGE3D clCreateImage3D;
    PFNCLRELEASEMEMOBJECT clReleaseMemObject;

    PFNCLCREATECOMMANDQUEUE clCreateCommandQueue;
    PFNCLRELEASECOMMANDQUEUE clReleaseCommandQueue;
    PFNCLRETAINCOMMANDQUEUE clRetainCommandQueue;

    PFNCLFLUSH clFlush;
    PFNCLFINISH clFinish;

    PFNCLENQUEUENDRANGEKERNEL clEnqueueNDRangeKernel;
    PFNCLENQUEUEREADBUFFER clEnqueueReadBuffer;
    PFNCLENQUEUEWRITEBUFFER clEnqueueWriteBuffer;
    PFNCLENQUEUEREADIMAGE clEnqueueReadImage;
    PFNCLENQUEUEWRITEIMAGE clEnqueueWriteImage;


#define OPENCL_GETPROCADDRESS(Type, Name) OpenCL::Name = (OpenCL::Type)GetProcAddress(libocl, #Name);
    bool LoadOpenCL()
    {
        //HMODULE libocl = LoadLibrary("OpenCL64.dll");
        HMODULE libocl = LoadLibrary("OpenCL.dll");
        OPENCL_GETPROCADDRESS(PFNCLGETPLATFORMIDS, clGetPlatformIDs);
        OPENCL_GETPROCADDRESS(PFNCLGETPLATFORMINFO, clGetPlatformInfo);
        OPENCL_GETPROCADDRESS(PFNCLGETDEVICEIDS, clGetDeviceIDs);
        OPENCL_GETPROCADDRESS(PFNCLGETDEVICEINFO, clGetDeviceInfo);

        OPENCL_GETPROCADDRESS(PFNCLCREATECONTEXT, clCreateContext);
        OPENCL_GETPROCADDRESS(PFNCLRELEASECONTEXT, clReleaseContext);
        OPENCL_GETPROCADDRESS(PFNCLRETAINCONTEXT, clRetainContext);

        OPENCL_GETPROCADDRESS(PFNCLCREATEPROGRAMWITHSOURCE, clCreateProgramWithSource);
        OPENCL_GETPROCADDRESS(PFNCLBUILDPROGRAM, clBuildProgram);
        OPENCL_GETPROCADDRESS(PFNCLGETPROGRAMBUILDINFO, clGetProgramBuildInfo);
        OPENCL_GETPROCADDRESS(PFNCLRELEASEPROGRAM, clReleaseProgram);

        OPENCL_GETPROCADDRESS(PFNCLCREATEKERNEL, clCreateKernel);
        OPENCL_GETPROCADDRESS(PFNCLRELEASEKERNEL, clReleaseKernel);
        OPENCL_GETPROCADDRESS(PFNCLSETKERNELARG, clSetKernelArg);
        OPENCL_GETPROCADDRESS(PFNCLGETKERNELWORKGROUPINFO, clGetKernelWorkGroupInfo);

        OPENCL_GETPROCADDRESS(PFNCLCREATEBUFFER, clCreateBuffer);
        OPENCL_GETPROCADDRESS(PFNCLCREATESUBBUFFER, clCreateSubBuffer);
        OPENCL_GETPROCADDRESS(PFNCLCREATEIMAGE2D, clCreateImage2D);
        OPENCL_GETPROCADDRESS(PFNCLCREATEIMAGE3D, clCreateImage3D);
        OPENCL_GETPROCADDRESS(PFNCLRELEASEMEMOBJECT, clReleaseMemObject);
        
        OPENCL_GETPROCADDRESS(PFNCLCREATECOMMANDQUEUE, clCreateCommandQueue);
        OPENCL_GETPROCADDRESS(PFNCLRELEASECOMMANDQUEUE, clReleaseCommandQueue);
        OPENCL_GETPROCADDRESS(PFNCLRETAINCOMMANDQUEUE, clRetainCommandQueue);

        OPENCL_GETPROCADDRESS(PFNCLFLUSH, clFlush);
        OPENCL_GETPROCADDRESS(PFNCLFINISH, clFinish);

        OPENCL_GETPROCADDRESS(PFNCLENQUEUENDRANGEKERNEL, clEnqueueNDRangeKernel);
        OPENCL_GETPROCADDRESS(PFNCLENQUEUEREADBUFFER, clEnqueueReadBuffer);
        OPENCL_GETPROCADDRESS(PFNCLENQUEUEWRITEBUFFER, clEnqueueWriteBuffer);
        OPENCL_GETPROCADDRESS(PFNCLENQUEUEREADIMAGE, clEnqueueReadImage);
        OPENCL_GETPROCADDRESS(PFNCLENQUEUEWRITEIMAGE, clEnqueueWriteImage);

        return true;
    }
#undef OPENCL_GETPROCADDRESS

    /////////////////////////////////////////////////////////////////////////////////////
    // CLLibrary
    /////////////////////////////////////////////////////////////////////////////////////
    CLLibrary::CLLibrary()
    {
        m_num_platforms = 0;
        m_platforms = nullptr;
    }

    CLLibrary::~CLLibrary()
    {
        if (m_platforms != nullptr)
        {
            delete[] m_platforms;
            m_platforms = nullptr;
        }
    }

    CLPlatform* CLLibrary::GetPlatform(cl_uint index) const 
    { 
        return &m_platforms[index]; 
    }

    bool CLLibrary::Initialize()
    {
        if (!LoadOpenCL() || m_platforms != nullptr)
        {
            return false;
        }

        cl_uint platformIdCount = 0;

        CLV(OpenCL::clGetPlatformIDs(0, nullptr, &platformIdCount));

        if (platformIdCount == 0)
        {
            DBG_MSG("[OpenCL] No platform found.");
            return false;
        }

        if (platformIdCount > MAX_NUM_PLATFORMS)
        {
            platformIdCount = MAX_NUM_PLATFORMS;
        }

        cl_platform_id platformIds[MAX_NUM_PLATFORMS];
        CLV(OpenCL::clGetPlatformIDs(platformIdCount, platformIds, nullptr));

        m_platforms = new CLPlatform[platformIdCount];
        m_num_platforms = platformIdCount;

        for (cl_uint i = 0; i < platformIdCount; i++)
        {
            m_platforms[i].Initialize(platformIds[i]);
        }

        return true;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // CLPlatform
    /////////////////////////////////////////////////////////////////////////////////////
    CLPlatform::CLPlatform()
    {
        m_platform_id = nullptr;
        m_devices = nullptr;
        m_num_devices = 0;
    }

    CLPlatform::~CLPlatform()
    {
        if (m_devices != nullptr)
        {
            delete[] m_devices;
            m_devices = nullptr;
            m_num_devices = 0;
        }
    }

    CLDevice* CLPlatform::GetDevice(cl_uint index) const 
    { 
        return &m_devices[index]; 
    }

    bool CLPlatform::Initialize(cl_platform_id platform_id)
    {
        cl_uint deviceIdCount = 0;

        m_platform_id = platform_id;

        // Get platform info
        PrintInfo();

        CLV(OpenCL::clGetDeviceIDs(m_platform_id, CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceIdCount));


        cl_device_id deviceIds[MAX_NUM_DEVICES];

        if (deviceIdCount > MAX_NUM_DEVICES)
        {
            deviceIdCount = MAX_NUM_DEVICES;
        }

        OpenCL::clGetDeviceIDs(m_platform_id, CL_DEVICE_TYPE_ALL, deviceIdCount, deviceIds, nullptr);

        m_num_devices = deviceIdCount;
        m_devices = new CLDevice[deviceIdCount];

        for (cl_uint i = 0; i < deviceIdCount; i++)
        {
            m_devices[i].Initialize(deviceIds[i]);
        }

        return true;
    }

    void CLPlatform::PrintInfo()
    {
        size_t string_size = 0;
        forg::core::vector<char> platform_info;

        // platform name
        OpenCL::clGetPlatformInfo(m_platform_id, CL_PLATFORM_NAME, 0, nullptr, &string_size);
        platform_info.resize(string_size);
        OpenCL::clGetPlatformInfo(m_platform_id, CL_PLATFORM_NAME, string_size, platform_info.data(), &string_size);
        DBG_MSG("[OpenCL] Platform: %s\n", platform_info.data());

        // version
        OpenCL::clGetPlatformInfo(m_platform_id, CL_PLATFORM_VERSION, 0, nullptr, &string_size);
        platform_info.resize(string_size);
        OpenCL::clGetPlatformInfo(m_platform_id, CL_PLATFORM_VERSION, string_size, platform_info.data(), &string_size);
        DBG_MSG("[OpenCL] Version: %s\n", platform_info.data());

        // vendor
        OpenCL::clGetPlatformInfo(m_platform_id, CL_PLATFORM_VENDOR, 0, nullptr, &string_size);
        platform_info.resize(string_size);
        OpenCL::clGetPlatformInfo(m_platform_id, CL_PLATFORM_VENDOR, string_size, platform_info.data(), &string_size);
        DBG_MSG("[OpenCL] Vendor: %s\n", platform_info.data());

        // profile
        OpenCL::clGetPlatformInfo(m_platform_id, CL_PLATFORM_PROFILE, 0, nullptr, &string_size);
        platform_info.resize(string_size);
        OpenCL::clGetPlatformInfo(m_platform_id, CL_PLATFORM_PROFILE, string_size, platform_info.data(), &string_size);
        DBG_MSG("[OpenCL] Profile: %s\n", platform_info.data());

        // extensions
        OpenCL::clGetPlatformInfo(m_platform_id, CL_PLATFORM_EXTENSIONS, 0, nullptr, &string_size);
        platform_info.resize(string_size);
        OpenCL::clGetPlatformInfo(m_platform_id, CL_PLATFORM_EXTENSIONS, string_size, platform_info.data(), &string_size);
        DBG_MSG("[OpenCL] Extensions: %s\n", platform_info.data());
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // CLDevice
    /////////////////////////////////////////////////////////////////////////////////////
    CLDevice::CLDevice()
    {

    }

    CLDevice::~CLDevice()
    {

    }

#define CL_GETDEVICEINFO_PRIMITIVE(param, type) { \
    type value; \
    OpenCL::clGetDeviceInfo(m_device_id, param, sizeof(value), &value, nullptr); \
    DBG_MSG("[OpenCL] [Device] "#param" %d\n", value); \
    }
#define CL_GETDEVICEINFO_PRIMITIVEL(param, type) { \
    type value; \
    OpenCL::clGetDeviceInfo(m_device_id, param, sizeof(value), &value, nullptr); \
    DBG_MSG("[OpenCL] [Device] "#param" %llu\n", value); \
    }
    
    bool CLDevice::Initialize(cl_device_id device_id)
    {
        m_device_id = device_id;

        // Get device info
        {
            size_t string_size = 0;
            forg::core::vector<char> device_info;

            // Device name
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_NAME, 0, nullptr, &string_size);
            device_info.resize(string_size);
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_NAME, string_size, device_info.data(), &string_size);
            DBG_MSG("[OpenCL] [Device] Nane: %s\n", device_info.data());

            // Device vendor
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_VENDOR, 0, nullptr, &string_size);
            device_info.resize(string_size);
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_VENDOR, string_size, device_info.data(), &string_size);
            DBG_MSG("[OpenCL] [Device] Vendor: %s\n", device_info.data());

            // Device version
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_VERSION, 0, nullptr, &string_size);
            device_info.resize(string_size);
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_VERSION, string_size, device_info.data(), &string_size);
            DBG_MSG("[OpenCL] [Device] Version: %s\n", device_info.data());

            // Driver version
            OpenCL::clGetDeviceInfo(m_device_id, CL_DRIVER_VERSION, 0, nullptr, &string_size);
            device_info.resize(string_size);
            OpenCL::clGetDeviceInfo(m_device_id, CL_DRIVER_VERSION, string_size, device_info.data(), &string_size);
            DBG_MSG("[OpenCL] [Device] Driver version: %s\n", device_info.data());

            // Device type
            char* dev_type_name[] = { "CL_DEVICE_TYPE_DEFAULT", "CL_DEVICE_TYPE_CPU", "CL_DEVICE_TYPE_GPU", "CL_DEVICE_TYPE_ACCELERATOR" };
            cl_device_type dev_type;
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_TYPE, sizeof(dev_type), &dev_type, nullptr);
            DBG_MSG("[OpenCL] [Device] Type: %s\n", dev_type_name[forg::first_bit((forg::uint)dev_type)]);

            CL_GETDEVICEINFO_PRIMITIVE(CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint);
            CL_GETDEVICEINFO_PRIMITIVE(CL_DEVICE_MAX_WORK_GROUP_SIZE, size_t);
            CL_GETDEVICEINFO_PRIMITIVE(CL_DEVICE_MAX_CLOCK_FREQUENCY, cl_uint);
            CL_GETDEVICEINFO_PRIMITIVE(CL_DEVICE_ADDRESS_BITS, cl_uint);
            CL_GETDEVICEINFO_PRIMITIVE(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, cl_uint);
            CL_GETDEVICEINFO_PRIMITIVE(CL_DEVICE_IMAGE_SUPPORT, cl_bool);
            CL_GETDEVICEINFO_PRIMITIVE(CL_DEVICE_IMAGE2D_MAX_WIDTH, size_t);
            CL_GETDEVICEINFO_PRIMITIVE(CL_DEVICE_IMAGE2D_MAX_HEIGHT, size_t);
            CL_GETDEVICEINFO_PRIMITIVE(CL_DEVICE_MAX_SAMPLERS, cl_uint);
            CL_GETDEVICEINFO_PRIMITIVE(CL_DEVICE_MAX_PARAMETER_SIZE, size_t);
            CL_GETDEVICEINFO_PRIMITIVEL(CL_DEVICE_GLOBAL_MEM_SIZE, cl_ulong);
            CL_GETDEVICEINFO_PRIMITIVEL(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, cl_ulong);
            CL_GETDEVICEINFO_PRIMITIVE(CL_DEVICE_MAX_CONSTANT_ARGS, cl_uint);
            CL_GETDEVICEINFO_PRIMITIVEL(CL_DEVICE_LOCAL_MEM_SIZE, cl_ulong);

        }

        return true;
    }
#undef CL_GETDEVICEINFO_PRIMITIVE
#undef CL_GETDEVICEINFO_PRIMITIVEL

    /////////////////////////////////////////////////////////////////////////////////////
    // CLContext
    /////////////////////////////////////////////////////////////////////////////////////
    CLContext::CLContext()
    {
        m_context = nullptr;
        m_error = CL_SUCCESS;
    }

    CLContext::~CLContext()
    {
        Release();
    }

    void CLContext::Retain()
    {
        if (m_context != nullptr)
        {
            m_error = OpenCL::clRetainContext(m_context);

            CLV(m_error);
        }
    }

    void CLContext::Release()
    {
        if (m_context != nullptr)
        {
            OpenCL::clReleaseContext(m_context);
            m_context = nullptr;
        }
    }

    bool CLContext::Create(const CLContext& context)
    {
        m_context = context.m_context;
        m_error = CL_SUCCESS;

        Retain();

        return (m_error == CL_SUCCESS);
    }

    bool CLContext::Create(cl_platform_id platform_id, cl_device_id device_id)
    {
        // create context
        const cl_context_properties contextProperties[] =
        {
            CL_CONTEXT_PLATFORM,
            reinterpret_cast<cl_context_properties> (platform_id),
            0, 0
        };

        m_context = OpenCL::clCreateContext(contextProperties, 1,
            &device_id, nullptr, nullptr, &m_error);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    bool CLContext::Create(cl_platform_id platform_id, cl_device_id* device_ids, cl_uint count)
    {
        // create context
        const cl_context_properties contextProperties[] =
        {
            CL_CONTEXT_PLATFORM,
            reinterpret_cast<cl_context_properties> (platform_id),
            0, 0
        };

        m_context = OpenCL::clCreateContext(contextProperties, count,
            device_ids, nullptr, nullptr, &m_error);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // CLProgram
    /////////////////////////////////////////////////////////////////////////////////////

    CLProgram::CLProgram()
    {
        m_program = nullptr;
        m_error = CL_SUCCESS;
    }

    CLProgram::~CLProgram()
    {
        if (m_program != nullptr)
        {
            OpenCL::clReleaseProgram(m_program);
            m_program = nullptr;
        }
    }

    bool CLProgram::CreateProgramWithSource(cl_context context, cl_uint count, const char ** strings, const size_t * lengths)
    {
        m_program = OpenCL::clCreateProgramWithSource(context, count, strings, lengths, &m_error);
        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    bool CLProgram::CreateProgramWithSource(cl_context context, const char* src, const size_t length)
    {
        size_t lengths[1] = { length };
        const char* sources[1] = { src };

        return CreateProgramWithSource(context, 1, sources, lengths);
    }

    bool CLProgram::BuildProgram(cl_device_id device_id)
    {
        m_error = OpenCL::clBuildProgram(m_program, 1, &device_id, nullptr, nullptr, nullptr);
        CLV(m_error);

        if (m_error != CL_SUCCESS)
        {
            cl_program_build_info binfo;
            forg::core::vector<char> blog;
            size_t log_size = 0;

            OpenCL::clGetProgramBuildInfo(m_program, device_id, CL_PROGRAM_BUILD_LOG, 0, 0, &log_size);            
            blog.resize(log_size);
            OpenCL::clGetProgramBuildInfo(m_program, device_id, CL_PROGRAM_BUILD_LOG, log_size, blog.data(), 0);
            DBG_MSG("[OpenCL] [Program] clBuildProgram failed. Build log: %s\n", blog.data());
        }

        return (m_error == CL_SUCCESS);
    }

    bool CLProgram::BuildProgram(cl_device_id* device_ids, cl_uint dev_count, const char* options)
    {
        m_error = OpenCL::clBuildProgram(m_program, dev_count, device_ids, nullptr, nullptr, nullptr);
        CLV(m_error);
        return (m_error == CL_SUCCESS);
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // CLKernel
    /////////////////////////////////////////////////////////////////////////////////////

    CLKernel::CLKernel()
    {
        m_kernel = nullptr;
        m_error = CL_SUCCESS;
    }

    CLKernel::~CLKernel()
    {
        if (m_kernel != nullptr)
        {
            OpenCL::clReleaseKernel(m_kernel);
            m_kernel = nullptr;
        }
    }

    bool CLKernel::Create(cl_program program, const char* kernel_name)
    {
        m_kernel = OpenCL::clCreateKernel(program, kernel_name, &m_error);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    bool CLKernel::SetKernelArg(cl_uint arg_index, size_t arg_size, const void* arg_value)
    {
        m_error = OpenCL::clSetKernelArg(m_kernel, arg_index, arg_size, arg_value);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    bool CLKernel::SetKernelArg(cl_uint arg_index, OpenCL::CLMemObject& buffer)
    {
        cl_mem mobj = buffer.GetMemObject();
        return SetKernelArg(arg_index, sizeof(cl_mem), &mobj);
    }

    bool CLKernel::GetKernelWorkGroupInfo(cl_device_id device, CLKernelWorkGroupInfo& info)
    {
        OpenCL::clGetKernelWorkGroupInfo(m_kernel, device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(info.WorkGroupSize), &info.WorkGroupSize, nullptr);
        OpenCL::clGetKernelWorkGroupInfo(m_kernel, device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(info.PreferredWorkGroupSizeMultiple), &info.PreferredWorkGroupSizeMultiple, nullptr);
        OpenCL::clGetKernelWorkGroupInfo(m_kernel, device, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(info.LocalMemSize), &info.LocalMemSize, nullptr);
        OpenCL::clGetKernelWorkGroupInfo(m_kernel, device, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(info.PrivateMemSize), &info.PrivateMemSize, nullptr);
        m_error = OpenCL::clGetKernelWorkGroupInfo(m_kernel, device, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(info.CompileWorkGroupSize), &info.CompileWorkGroupSize, nullptr);

        return (m_error == CL_SUCCESS);
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // CLMemObject
    /////////////////////////////////////////////////////////////////////////////////////

    CLMemObject::CLMemObject()
    {
        m_mem_object = nullptr;
        m_error = CL_SUCCESS;
    }

    CLMemObject::~CLMemObject()
    {
        Release();

        m_error = CL_SUCCESS;
    }

    void CLMemObject::Release()
    {
        if (m_mem_object != nullptr)
        {
            OpenCL::clReleaseMemObject(m_mem_object);
            m_mem_object = nullptr;
        }
    }

    bool CLMemObject::CreateBuffer(cl_context context, cl_mem_flags flags, size_t size, void* host_ptr)
    {
        m_mem_object = OpenCL::clCreateBuffer(context, flags, size, host_ptr, &m_error);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    bool CLMemObject::CreateImage2D(cl_context context, cl_mem_flags opts,
        const cl_image_format *format, size_t width, size_t height,
        size_t row_pitch, void *data)
    {
        m_mem_object = OpenCL::clCreateImage2D(context, opts, format, width, height, row_pitch, data, &m_error);

        CLV(m_error);
        
        return (m_error == CL_SUCCESS);
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // CLCommandQueue
    /////////////////////////////////////////////////////////////////////////////////////

    CLCommandQueue::CLCommandQueue()
    {
        m_queue = nullptr;
        m_error = CL_SUCCESS;
    }

    CLCommandQueue::~CLCommandQueue()
    {
        Release();
    }

    void CLCommandQueue::Retain()
    {
        if (m_queue != nullptr)
        {
            m_error = OpenCL::clRetainCommandQueue(m_queue);

            CLV(m_error);
        }
    }

    void CLCommandQueue::Release()
    {
        if (m_queue != nullptr)
        {
            OpenCL::clReleaseCommandQueue(m_queue);
            m_queue = nullptr;
        }
    }

    bool CLCommandQueue::Create(const CLCommandQueue& queue)
    {
        m_queue = queue.m_queue;
        m_error = CL_SUCCESS;

        Retain();

        return (m_error == CL_SUCCESS);
    }

    bool CLCommandQueue::Create(cl_context context, cl_device_id device, cl_command_queue_properties properties)
    {
        m_queue = OpenCL::clCreateCommandQueue(context, device, properties, &m_error);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    bool CLCommandQueue::Flush()
    {
        m_error = OpenCL::clFlush(m_queue);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    bool CLCommandQueue::Finish()
    {
        m_error = OpenCL::clFinish(m_queue);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    bool CLCommandQueue::EnqueueNDRangeKernel(cl_kernel kernel, cl_uint work_dim,
        const size_t* global_work_offset, const size_t* global_work_size, const size_t* local_work_size)
    {
        m_error = OpenCL::clEnqueueNDRangeKernel(m_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, 0, nullptr, nullptr);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    bool CLCommandQueue::EnqueueReadBuffer(cl_mem buffer, cl_bool blocking_read,
        size_t offset, size_t size, void *ptr)
    {
        m_error = OpenCL::clEnqueueReadBuffer(m_queue, buffer, blocking_read, offset, size, ptr, 0, nullptr, nullptr);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    bool CLCommandQueue::EnqueueWriteBuffer(cl_mem buffer, cl_bool blocking_write,
        size_t offset, size_t size, const void *ptr)
    {
        m_error = OpenCL::clEnqueueWriteBuffer(m_queue, buffer, blocking_write, offset, size, ptr, 0, nullptr, nullptr);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    bool CLCommandQueue::EnqueueReadImage(cl_mem image, cl_bool blocking,
        const size_t origin[3], const size_t region[3],
        size_t row_pitch, size_t slice_pitch,
        void *ptr)
    {
        m_error = OpenCL::clEnqueueReadImage(m_queue, image, blocking, origin, region, row_pitch, slice_pitch, ptr, 0, nullptr, nullptr);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    bool CLCommandQueue::EnqueueWriteImage(cl_mem image,
        cl_bool blocking_write,
        const size_t *origin,
        const size_t *region,
        size_t input_row_pitch,
        size_t input_slice_pitch,
        const void * ptr)
    {
        m_error = OpenCL::clEnqueueWriteImage(m_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr, 0, nullptr, nullptr);

        CLV(m_error);

        return (m_error == CL_SUCCESS);
    }

    /////////////////////////////////////////////////////////////////////////////////////

} // namespace OpenCL

#endif //defined(FORG_ENABLE_OPENCL)