#define EVENT_STRINGIFY(s) #s
#define EVENT_VERSION(a, b) EVENT_STRINGIFY(a) "." EVENT_STRINGIFY(b)

/*
 * 获取libev的版本号字符串
 * @return 返回版本号字符串(格式为"主版本号.次版本号")
 */
const char *event_get_version(void)
{
    /* returns ABI, not API or library, version */
    return EVENT_VERSION(11, 3);
}

#include <iostream>

int main()
{
    std::cout << event_get_version() << std::endl;
    std::string ss = "123";
}