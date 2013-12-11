//#include "stdafx.h"

#include "OpenCL.h"
#include <forg/forg.h>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

namespace OpenCL {

    enum { MAX_NUM_PLATFORMS = 64 };
    enum { MAX_NUM_DEVICES = 64 };

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

    /* Command Queue APIs */
    typedef cl_command_queue(*PFNCLCREATECOMMANDQUEUE)(cl_context                     /* context */,
        cl_device_id                   /* device */,
        cl_command_queue_properties    /* properties */,
        cl_int *                       /* errcode_ret */);

    typedef cl_int(*PFNCLRELEASECOMMANDQUEUE)(cl_command_queue /* command_queue */);

    /* Memory Object APIs */
    typedef cl_mem(*PFNCLCREATEBUFFER)(cl_context   /* context */,
        cl_mem_flags /* flags */,
        size_t       /* size */,
        void *       /* host_ptr */,
        cl_int *     /* errcode_ret */);

    typedef cl_int(*PFNCLRELEASEMEMOBJECT)(cl_mem /* memobj */);

    /* Program Object APIs  */
    typedef cl_program(*PFNCLCREATEPROGRAMWITHSOURCE)(cl_context        /* context */,
        cl_uint           /* count */,
        const char **     /* strings */,
        const size_t *    /* lengths */,
        cl_int *          /* errcode_ret */);

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

    PFNCLCREATEPROGRAMWITHSOURCE clCreateProgramWithSource;
    PFNCLBUILDPROGRAM clBuildProgram;
    PFNCLRELEASEPROGRAM clReleaseProgram;

    PFNCLCREATEKERNEL clCreateKernel;
    PFNCLRELEASEKERNEL clReleaseKernel;
    PFNCLSETKERNELARG clSetKernelArg;

    PFNCLCREATEBUFFER clCreateBuffer;
    PFNCLRELEASEMEMOBJECT clReleaseMemObject;

    PFNCLCREATECOMMANDQUEUE clCreateCommandQueue;
    PFNCLRELEASECOMMANDQUEUE clReleaseCommandQueue;

    PFNCLENQUEUENDRANGEKERNEL clEnqueueNDRangeKernel;
    PFNCLENQUEUEREADBUFFER clEnqueueReadBuffer;

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

    cl_int CLErrorCheck(cl_int error_code, int line, LPCSTR file, LPCSTR func)
    {
        if (error_code != CL_SUCCESS)
        {
            char dbuf[512];

            sprintf(
                dbuf,
                _T("*** Unexpected error encountered! ***\n%s(%d): Error Code: %s (0x%x) Calling: %s\n\n"),
                file, line, CLGetErrorString(error_code), error_code, func
                );

            forg::debug::DbgOutputString(dbuf);
        }

        return error_code;
    }

#define CLV(x) OpenCL::CLErrorCheck((x), __LINE__, __FILE__, "")

    bool LoadOpenCL()
    {
        HMODULE libocl = LoadLibrary("OpenCL64.dll");
        OpenCL::clGetPlatformIDs = (OpenCL::PFNCLGETPLATFORMIDS)GetProcAddress(libocl, "clGetPlatformIDs");
        OpenCL::clGetPlatformInfo = (OpenCL::PFNCLGETPLATFORMINFO)GetProcAddress(libocl, "clGetPlatformInfo");
        OpenCL::clGetDeviceIDs = (OpenCL::PFNCLGETDEVICEIDS)GetProcAddress(libocl, "clGetDeviceIDs");
        OpenCL::clGetDeviceInfo = (OpenCL::PFNCLGETDEVICEINFO)GetProcAddress(libocl, "clGetDeviceInfo");
        OpenCL::clCreateContext = (OpenCL::PFNCLCREATECONTEXT)GetProcAddress(libocl, "clCreateContext");
        OpenCL::clReleaseContext = (OpenCL::PFNCLRELEASECONTEXT)GetProcAddress(libocl, "clReleaseContext");
        OpenCL::clCreateProgramWithSource = (OpenCL::PFNCLCREATEPROGRAMWITHSOURCE)GetProcAddress(libocl, "clCreateProgramWithSource");
        OpenCL::clBuildProgram = (OpenCL::PFNCLBUILDPROGRAM)GetProcAddress(libocl, "clBuildProgram");
        OpenCL::clReleaseProgram = (OpenCL::PFNCLRELEASEPROGRAM)GetProcAddress(libocl, "clReleaseProgram");
        OpenCL::clCreateKernel = (OpenCL::PFNCLCREATEKERNEL)GetProcAddress(libocl, "clCreateKernel");
        OpenCL::clReleaseKernel = (OpenCL::PFNCLRELEASEKERNEL)GetProcAddress(libocl, "clReleaseKernel");
        OpenCL::clSetKernelArg = (OpenCL::PFNCLSETKERNELARG)GetProcAddress(libocl, "clSetKernelArg");
        OpenCL::clCreateBuffer = (OpenCL::PFNCLCREATEBUFFER)GetProcAddress(libocl, "clCreateBuffer");
        OpenCL::clReleaseMemObject = (OpenCL::PFNCLRELEASEMEMOBJECT)GetProcAddress(libocl, "clReleaseMemObject");
        OpenCL::clCreateCommandQueue = (OpenCL::PFNCLCREATECOMMANDQUEUE)GetProcAddress(libocl, "clCreateCommandQueue");
        OpenCL::clReleaseCommandQueue = (OpenCL::PFNCLRELEASECOMMANDQUEUE)GetProcAddress(libocl, "clReleaseCommandQueue");
        OpenCL::clEnqueueNDRangeKernel = (OpenCL::PFNCLENQUEUENDRANGEKERNEL)GetProcAddress(libocl, "clEnqueueNDRangeKernel");
        OpenCL::clEnqueueReadBuffer = (OpenCL::PFNCLENQUEUEREADBUFFER)GetProcAddress(libocl, "clEnqueueReadBuffer");

        return true;
    }

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

    /////////////////////////////////////////////////////////////////////////////////////
    // CLDevice
    /////////////////////////////////////////////////////////////////////////////////////
    CLDevice::CLDevice()
    {

    }

    CLDevice::~CLDevice()
    {

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
            DBG_MSG("[OpenCL] Device: %s\n", device_info.data());

            // Device vendor
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_VENDOR, 0, nullptr, &string_size);
            device_info.resize(string_size);
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_VENDOR, string_size, device_info.data(), &string_size);
            DBG_MSG("[OpenCL] Vendor: %s\n", device_info.data());

            // Device version
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_VERSION, 0, nullptr, &string_size);
            device_info.resize(string_size);
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_VERSION, string_size, device_info.data(), &string_size);
            DBG_MSG("[OpenCL] Version: %s\n", device_info.data());

            // Driver version
            OpenCL::clGetDeviceInfo(m_device_id, CL_DRIVER_VERSION, 0, nullptr, &string_size);
            device_info.resize(string_size);
            OpenCL::clGetDeviceInfo(m_device_id, CL_DRIVER_VERSION, string_size, device_info.data(), &string_size);
            DBG_MSG("[OpenCL] Driver: %s\n", device_info.data());

            // Device type
            char* dev_type_name[] = { "CL_DEVICE_TYPE_DEFAULT", "CL_DEVICE_TYPE_CPU", "CL_DEVICE_TYPE_GPU", "CL_DEVICE_TYPE_ACCELERATOR" };
            cl_device_type dev_type;
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_TYPE, sizeof(dev_type), &dev_type, nullptr);
            DBG_MSG("[OpenCL] Device type: %s\n", dev_type_name[forg::first_bit((forg::uint)dev_type)]);

            // CL_DEVICE_MAX_COMPUTE_UNITS
            cl_uint max_cu;
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(max_cu), &max_cu, nullptr);
            DBG_MSG("[OpenCL] Max compute units: %d\n", max_cu);

            // CL_DEVICE_MAX_CLOCK_FREQUENCY
            cl_uint max_cf;
            OpenCL::clGetDeviceInfo(m_device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(max_cf), &max_cf, nullptr);
            DBG_MSG("[OpenCL] Max clock frequency: %dMhz\n", max_cf);

        }

        return true;
    }

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
        if (m_context != nullptr)
        {
            OpenCL::clReleaseContext(m_context);
            m_context = nullptr;
        }
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
        if (m_queue != nullptr)
        {
            OpenCL::clReleaseCommandQueue(m_queue);
            m_queue = nullptr;
        }
    }

    bool CLCommandQueue::Create(cl_context context, cl_device_id device, cl_command_queue_properties properties)
    {
        m_queue = OpenCL::clCreateCommandQueue(context, device, properties, &m_error);

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

    /////////////////////////////////////////////////////////////////////////////////////

} // namespace OpenCL