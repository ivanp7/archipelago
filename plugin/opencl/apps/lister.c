#include <stdlib.h>
#include <stdio.h>

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>

#define COLOR_RESET         "\033[0m"

#define COLOR_FG_BLACK      "\033[30m"
#define COLOR_FG_RED        "\033[31m"
#define COLOR_FG_GREEN      "\033[32m"
#define COLOR_FG_YELLOW     "\033[33m"
#define COLOR_FG_BLUE       "\033[34m"
#define COLOR_FG_MAGENTA    "\033[35m"
#define COLOR_FG_CYAN       "\033[36m"
#define COLOR_FG_WHITE      "\033[37m"

#define COLOR_BG_BLACK      "\033[40m"
#define COLOR_BG_RED        "\033[41m"
#define COLOR_BG_GREEN      "\033[42m"
#define COLOR_BG_YELLOW     "\033[43m"
#define COLOR_BG_BLUE       "\033[44m"
#define COLOR_BG_MAGENTA    "\033[45m"
#define COLOR_BG_CYAN       "\033[46m"
#define COLOR_BG_WHITE      "\033[47m"

#define COLOR_FG_BRI_BLACK      "\033[90m"
#define COLOR_FG_BRI_RED        "\033[91m"
#define COLOR_FG_BRI_GREEN      "\033[92m"
#define COLOR_FG_BRI_YELLOW     "\033[93m"
#define COLOR_FG_BRI_BLUE       "\033[94m"
#define COLOR_FG_BRI_MAGENTA    "\033[95m"
#define COLOR_FG_BRI_CYAN       "\033[96m"
#define COLOR_FG_BRI_WHITE      "\033[97m"

#define COLOR_BG_BRI_BLACK      "\033[100m"
#define COLOR_BG_BRI_RED        "\033[101m"
#define COLOR_BG_BRI_GREEN      "\033[102m"
#define COLOR_BG_BRI_YELLOW     "\033[103m"
#define COLOR_BG_BRI_BLUE       "\033[104m"
#define COLOR_BG_BRI_MAGENTA    "\033[105m"
#define COLOR_BG_BRI_CYAN       "\033[106m"
#define COLOR_BG_BRI_WHITE      "\033[107m"

#define COLOR_STRING COLOR_FG_BRI_WHITE
#define COLOR_NUMBER COLOR_FG_BRI_YELLOW
#define COLOR_ERROR COLOR_FG_BRI_RED

#define PRINT(msg) fprintf(stderr, msg)
#define PRINT_(msg, ...) fprintf(stderr, msg, __VA_ARGS__)

#define ERROR(msg) fprintf(stderr, "\n" COLOR_ERROR "Error" COLOR_RESET \
        ": " msg ".\n")
#define ERROR_(msg, ...) fprintf(stderr, "\n" COLOR_ERROR "Error" COLOR_RESET \
        ": " msg ".\n", __VA_ARGS__)

int main(void)
{
    cl_int ret;

    cl_uint num_platforms;

    ret = clGetPlatformIDs(0, (cl_platform_id*)NULL, &num_platforms);
    if (ret != CL_SUCCESS)
    {
        ERROR("couldn't obtain number of OpenCL platforms");
        exit(EXIT_FAILURE);
    }

    PRINT_("Number of OpenCL platforms: " COLOR_NUMBER "%u" COLOR_RESET "\n", num_platforms);

    cl_platform_id *platform_list = malloc(sizeof(cl_platform_id) * num_platforms);
    if (platform_list == NULL)
    {
        ERROR("couldn't allocate OpenCL platform list");
        perror("malloc()");
        exit(EXIT_FAILURE);
    }

    ret = clGetPlatformIDs(num_platforms, platform_list, (cl_uint*)NULL);
    if (ret != CL_SUCCESS)
    {
        ERROR("couldn't obtain OpenCL platform list");
        free(platform_list);
        exit(EXIT_FAILURE);
    }

    cl_uint device_list_size = 0;
    cl_device_id *device_list = NULL;

    for (cl_uint platform_idx = 0; platform_idx < num_platforms; platform_idx++)
    {
        {
            char name[256] = {0};

            ret = clGetPlatformInfo(platform_list[platform_idx],
                    CL_PLATFORM_NAME, sizeof(name)-1, name, (size_t*)NULL);
            if (ret != CL_SUCCESS)
            {
                ERROR_("couldn't obtain name of OpenCL platform #"
                        COLOR_NUMBER "%x" COLOR_RESET, platform_idx);
                free(platform_list);
                free(device_list);
                exit(EXIT_FAILURE);
            }

            PRINT_("#" COLOR_NUMBER "%x" COLOR_RESET ": "
                    COLOR_STRING "%s" COLOR_RESET "\n", platform_idx, name);
        }

        {
            cl_uint num_devices;

            ret = clGetDeviceIDs(platform_list[platform_idx],
                    CL_DEVICE_TYPE_DEFAULT, 0, (cl_device_id*)NULL, &num_devices);
            if (ret != CL_SUCCESS)
            {
                ERROR_("couldn't obtain number of OpenCL devices for platform #"
                        COLOR_NUMBER "%x" COLOR_RESET, platform_idx);
                free(platform_list);
                free(device_list);
                exit(EXIT_FAILURE);
            }

            PRINT_("  number of devices: " COLOR_NUMBER "%u" COLOR_RESET "\n", num_devices);

            if (num_devices > device_list_size)
            {
                cl_device_id *new_device_list = realloc(device_list,
                        sizeof(cl_device_id) * num_devices);
                if (new_device_list == NULL)
                {
                    ERROR("couldn't allocate OpenCL device list");
                    perror("realloc()");
                    free(platform_list);
                    free(device_list);
                    exit(EXIT_FAILURE);
                }

                device_list_size = num_devices;
                device_list = new_device_list;
            }

            ret = clGetDeviceIDs(platform_list[platform_idx],
                    CL_DEVICE_TYPE_DEFAULT, num_devices, device_list, (cl_uint*)NULL);
            if (ret != CL_SUCCESS)
            {
                ERROR_("couldn't obtain OpenCL device list for platform #"
                        COLOR_NUMBER "%x" COLOR_RESET, platform_idx);
                free(platform_list);
                free(device_list);
                exit(EXIT_FAILURE);
            }

            for (cl_uint device_idx = 0; device_idx < num_devices; device_idx++)
            {
                char name[256] = {0};

                clGetDeviceInfo(device_list[device_idx],
                        CL_DEVICE_NAME, sizeof(name)-1, name, (size_t*)NULL);
                if (ret != CL_SUCCESS)
                {
                    ERROR_("couldn't obtain name of OpenCL device #"
                            COLOR_NUMBER "%x" COLOR_RESET " for platform #"
                            COLOR_NUMBER "%x" COLOR_RESET, device_idx, platform_idx);
                    free(platform_list);
                    free(device_list);
                    exit(EXIT_FAILURE);
                }

                PRINT_("    #" COLOR_NUMBER "%x" COLOR_RESET ": "
                        COLOR_STRING "%s" COLOR_RESET "\n", device_idx, name);
            }
        }
    }

    free(platform_list);
    free(device_list);
}

