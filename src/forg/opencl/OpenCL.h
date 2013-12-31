#ifndef _FORG_OPENCL_H_
#define _FORG_OPENCL_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <config.h>

#if defined(FORG_ENABLE_OPENCL)

#include <base.h>
#include <CL/opencl.h>

namespace OpenCL
{

    class CLPlatform;
    class CLDevice;
    class CLMemObject;

    /////////////////////////////////////////////////////////////////////////////////////

    class FORG_API CLLibrary
    {
        CLPlatform* m_platforms;
        cl_uint m_num_platforms;

    public:
        CLLibrary();
        ~CLLibrary();

        CLPlatform* GetPlatform(cl_uint index) const;

        bool Initialize();
    };

    /////////////////////////////////////////////////////////////////////////////////////

    class FORG_API CLPlatform
    {
        cl_platform_id m_platform_id;
        CLDevice* m_devices;
        cl_uint m_num_devices;

    public:
        CLPlatform();
        ~CLPlatform();

        cl_platform_id GetPlatformId() const { return m_platform_id; }
        CLDevice* GetDevice(cl_uint index) const;

        bool Initialize(cl_platform_id platform_id);
    };

    /////////////////////////////////////////////////////////////////////////////////////

    class FORG_API CLDevice
    {
        cl_device_id m_device_id;

    public:
        CLDevice();
        ~CLDevice();

        cl_device_id GetDeviceId() const { return m_device_id; }

        bool Initialize(cl_device_id device_id);
    };

    /////////////////////////////////////////////////////////////////////////////////////

    class FORG_API CLProgram
    {
        cl_program m_program;
        cl_int m_error;

    public:
        CLProgram();
        ~CLProgram();

        cl_program GetProgram() const { return m_program; }

        bool CreateProgramWithSource(cl_context context, const char* src, const size_t length);
        bool CreateProgramWithSource(cl_context context, cl_uint count, const char ** strings, const size_t * lengths);

        bool BuildProgram(cl_device_id device_id);
        bool BuildProgram(cl_device_id* device_ids, cl_uint dev_count, const char* options);
    };

    /////////////////////////////////////////////////////////////////////////////////////

    class FORG_API CLContext
    {
        cl_context m_context;
        cl_int m_error;

    public:
        CLContext();
        ~CLContext();

        cl_context GetContext() const { return m_context; }

        bool Create(cl_platform_id platform_id, cl_device_id device_id);
        bool Create(cl_platform_id platform_id, cl_device_id* device_ids, cl_uint count);

    };

    /////////////////////////////////////////////////////////////////////////////////////

    struct CLKernelWorkGroupInfo
    {
        size_t WorkGroupSize;
        size_t PreferredWorkGroupSizeMultiple;
        cl_ulong LocalMemSize;
        cl_ulong PrivateMemSize;
        size_t CompileWorkGroupSize[3];
    };

    class FORG_API CLKernel
    {
        cl_int m_error;
        cl_kernel m_kernel;

    public:
        CLKernel();
        ~CLKernel();

        cl_kernel GetKernel() const { return m_kernel; }

        bool Create(cl_program program, const char* kernel_name);

        bool SetKernelArg(cl_uint arg_index, size_t arg_size, const void* arg_value);

        bool SetKernelArg(cl_uint arg_index, OpenCL::CLMemObject& buffer);

        bool GetKernelWorkGroupInfo(cl_device_id device, CLKernelWorkGroupInfo& info);
    };

    /////////////////////////////////////////////////////////////////////////////////////

    class FORG_API CLMemObject
    {
        cl_int m_error;
        cl_mem m_mem_object;

    public:
        CLMemObject();
        ~CLMemObject();

        void Release();

        cl_mem GetMemObject() const { return m_mem_object; }

        bool CreateBuffer(cl_context context, cl_mem_flags flags, size_t size, void* host_ptr);

        bool CreateImage2D(cl_context context, cl_mem_flags opts,
            const cl_image_format *format, size_t width, size_t height,
            size_t row_pitch, void *data);
    };

    /////////////////////////////////////////////////////////////////////////////////////

    class FORG_API CLCommandQueue
    {
        cl_int m_error;
        cl_command_queue m_queue;

    public:
        CLCommandQueue();
        ~CLCommandQueue();

        cl_command_queue GetQueue() const { return m_queue; }

        bool Create(cl_context context, cl_device_id device, cl_command_queue_properties properties);

        bool Flush();

        bool Finish();

        bool EnqueueNDRangeKernel(cl_kernel kernel, cl_uint work_dim, 
            const size_t* global_work_offset, const size_t* global_work_size, const size_t* local_work_size);

        bool EnqueueReadBuffer(cl_mem buffer, cl_bool blocking_read,
            size_t offset, size_t size, void *ptr);

        bool EnqueueReadImage(cl_mem image, cl_bool blocking,
            const size_t origin[3], const size_t region[3],
            size_t row_pitch, size_t slice_pitch,
            void *ptr);

        bool EnqueueWriteImage(cl_mem image,
            cl_bool blocking_write,
            const size_t *origin,
            const size_t *region,
            size_t input_row_pitch,
            size_t input_slice_pitch,
            const void * ptr);
    };

    /////////////////////////////////////////////////////////////////////////////////////
};

#endif

#endif  // _FORG_OPENCL_H_
