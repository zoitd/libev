/**
 * epoll事件监听示例程序
 *
 * 本程序演示了如何使用Linux系统的epoll机制来监听多种类型的事件：
 * 1. 标准输入的可读事件（EPOLLIN）
 * 2. 文件的修改事件（使用inotify）
 *
 * epoll是Linux特有的高效事件通知机制，类似于BSD的kqueue，
 * 可以监听大量文件描述符的I/O事件。对于文件系统变化监听，
 * 我们使用inotify机制。
 *
 * 编译命令: g++ -std=c++17 -o epoll_test epoll_test.cpp
 * 运行方式: ./epoll_test
 *
 * 适用于Linux系统
 */

// 系统头文件包含
#include <algorithm>     // C++算法库（std::fill等）
#include <cerrno>        // 定义errno全局变量和各种错误码常量（如EINTR, EAGAIN等）
#include <cstdlib>       // C标准库函数（exit, EXIT_FAILURE等）
#include <cstring>       // C字符串操作函数（strlen, strcpy, memset, strerror等）
#include <fcntl.h>       // 文件控制选项和标志（O_CREAT, O_RDWR, O_NONBLOCK等）
#include <iostream>      // C++标准输入输出流（std::cout, std::cerr等）
#include <string>        // C++字符串类std::string及其相关操作
#include <sys/epoll.h>   // epoll相关的结构体和函数声明（epoll_create1, epoll_ctl, epoll_wait等）
#include <sys/inotify.h> // inotify相关的结构体和函数声明（inotify_init1, inotify_add_watch等）
#include <sys/stat.h>    // 文件状态和权限相关定义（S_IRUSR, S_IWUSR等）
#include <sys/types.h>   // 基本数据类型定义（pid_t, size_t, ssize_t等）
#include <unistd.h>      // UNIX标准函数定义（read, write, close, lseek等）

/**
 * 错误处理辅助函数
 *
 * 当系统调用失败时，打印错误信息并退出程序。
 * 使用strerror(errno)将错误码转换为可读的错误描述。
 *
 * @param message 自定义的错误描述信息
 */
void error_exit(const std::string &message)
{
    // 输出错误信息到标准错误流，格式为："错误描述: 系统错误信息"
    std::cerr << message << ": " << strerror(errno) << std::endl;
    // 以失败状态码退出程序
    exit(EXIT_FAILURE);
}

int main()
{
    // ========== 第一步：创建epoll实例 ==========

    /**
     * 创建一个新的epoll实例
     *
     * epoll_create1()系统调用创建一个新的epoll实例，返回一个文件描述符。
     * 这个文件描述符可以用于后续的epoll_ctl()和epoll_wait()调用。
     *
     * EPOLL_CLOEXEC标志确保在exec()调用时自动关闭该文件描述符。
     */
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1)
    {
        error_exit("创建epoll失败");
    }

    // ========== 第二步：创建inotify实例用于文件监听 ==========

    /**
     * 创建inotify实例用于监听文件系统事件
     *
     * inotify_init1()创建一个inotify实例，返回文件描述符。
     * IN_CLOEXEC标志确保在exec()调用时自动关闭该文件描述符。
     */
    int inotify_fd = inotify_init1(IN_CLOEXEC);
    if (inotify_fd == -1)
    {
        error_exit("创建inotify失败");
    }

    // 输出程序启动信息
    std::cout << "epoll示例启动..." << std::endl;
    std::cout << "监听标准输入和测试文件..." << std::endl;

    // ========== 第三步：准备测试文件 ==========

    /**
     * 创建并打开一个测试文件用于监视文件系统事件
     *
     * open()参数说明：
     * - O_CREAT: 如果文件不存在则创建
     * - O_RDWR:  以读写模式打开
     * - S_IRUSR: 文件所有者具有读权限
     * - S_IWUSR: 文件所有者具有写权限
     */
    const char *test_file_path = "epoll_test_file.txt";
    const int test_file = open(test_file_path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (test_file == -1)
    {
        error_exit("打开测试文件失败");
    }

    // 向测试文件写入初始内容，以便后续观察文件变化
    constexpr char *initial_content = "这是初始内容\n";
    write(test_file, initial_content, strlen(initial_content));

    /**
     * 使用inotify监听测试文件的变化
     *
     * inotify_add_watch()参数说明：
     * - inotify_fd: inotify实例的文件描述符
     * - test_file_path: 要监听的文件路径
     * - IN_MODIFY: 监听文件内容修改事件
     * - IN_ATTRIB: 监听文件属性变化事件
     */
    int watch_fd = inotify_add_watch(inotify_fd, test_file_path, IN_MODIFY | IN_ATTRIB);
    if (watch_fd == -1)
    {
        error_exit("添加inotify监听失败");
    }

    // 输出用户操作提示信息
    std::cout << "创建了测试文件: " << test_file_path << std::endl;
    std::cout << "你可以在另一个终端中修改该文件来触发事件" << std::endl;
    std::cout << "例如: echo '新内容' >> " << test_file_path << std::endl;
    std::cout << "或者在此终端输入内容来触发标准输入事件" << std::endl;

    // ========== 第四步：配置要监听的事件 ==========

    /**
     * 创建epoll_event结构体来定义要监听的事件
     *
     * struct epoll_event结构体字段说明：
     * - events: 事件类型（如EPOLLIN, EPOLLOUT等）
     * - data:   用户数据，可以是文件描述符或指针
     */
    struct epoll_event stdin_event = {
        .events = EPOLLIN,           // EPOLLIN：监听可读事件
        .data = {.fd = STDIN_FILENO} // 标准输入文件描述符
    };

    struct epoll_event inotify_event = {
        .events = EPOLLIN,         // EPOLLIN：监听inotify的可读事件
        .data = {.fd = inotify_fd} // inotify文件描述符
    };

    /**
     * 将事件注册到epoll中
     *
     * epoll_ctl()函数参数说明：
     * - epoll_fd: epoll实例的文件描述符
     * - EPOLL_CTL_ADD: 添加新的监听事件
     * - fd: 要监听的文件描述符
     * - event: 事件配置结构体
     */
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &stdin_event) == -1)
    {
        error_exit("添加标准输入监听失败");
    }

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inotify_fd, &inotify_event) == -1)
    {
        error_exit("添加inotify监听失败");
    }

    // ========== 第五步：主事件循环 ==========

    /**
     * 准备事件接收缓冲区和数据读取缓冲区
     */
    struct epoll_event events[10]; // 用于接收触发的事件，最多同时处理10个事件
    char *buffer = new char[256];  // 用于读取标准输入数据的缓冲区

    std::cout << "\n开始事件循环，等待事件发生...\n"
              << std::endl;

    // 无限循环等待和处理事件
    while (true)
    {
        /**
         * 等待事件发生
         *
         * epoll_wait()函数参数说明：
         * - epoll_fd: epoll实例的文件描述符
         * - events: 用于接收触发事件的数组
         * - 10: 最多接收10个事件
         * - -1: 无限等待直到有事件发生
         *
         * 返回值：触发的事件数量，失败时返回-1
         */
        int num_events = epoll_wait(epoll_fd, events, 10, -1);

        // 检查epoll_wait调用是否失败
        if (num_events == -1)
        {
            // EINTR表示系统调用被信号中断，这是正常情况，继续等待即可
            if (errno == EINTR)
            {
                continue; // 被信号中断，继续等待
            }
            // 其他错误则退出程序
            close(test_file);
            close(inotify_fd);
            close(epoll_fd);
            error_exit("epoll_wait等待失败");
        }

        // ========== 第六步：处理触发的事件 ==========

        /**
         * 遍历所有触发的事件并进行相应处理
         */
        for (int i = 0; i < num_events; i++)
        {
            // 获取触发事件的文件描述符
            int event_fd = events[i].data.fd;

            // ===== 处理标准输入事件 =====
            if (event_fd == STDIN_FILENO)
            {
                // 确认这是一个可读事件
                if (!(events[i].events & EPOLLIN))
                    continue;

                // 清空缓冲区，确保没有残留数据
                std::fill(buffer, buffer + 256, 0);

                /**
                 * 从标准输入读取数据
                 *
                 * read()函数参数：
                 * - STDIN_FILENO: 标准输入文件描述符
                 * - buffer: 数据缓冲区
                 * - 255: 最多读取255字节（留一个字节给'\0'）
                 *
                 * 返回值：实际读取的字节数，0表示EOF，-1表示错误
                 */
                ssize_t bytes_read = read(STDIN_FILENO, buffer, 255);
                if (bytes_read <= 0)
                    continue; // 读取失败或没有数据，继续处理下一个事件

                buffer[bytes_read] = '\0'; // 确保字符串以null结尾
                std::cout << "收到标准输入: " << buffer;

                /**
                 * 检查退出命令
                 * 如果用户输入包含"quit"或"exit"，则优雅地退出程序
                 */
                std::string input(buffer);
                if (input.find("quit") != std::string::npos || input.find("exit") != std::string::npos)
                {
                    std::cout << "退出程序..." << std::endl;
                    // 清理资源
                    delete[] buffer; // 释放动态分配的内存
                    close(test_file);
                    close(inotify_fd);
                    close(epoll_fd);
                    return 0;
                }
            }
            // ===== 处理inotify文件变化事件 =====
            else if (event_fd == inotify_fd)
            {
                // 确认这是一个可读事件
                if (!(events[i].events & EPOLLIN))
                    continue;

                /**
                 * 读取inotify事件数据
                 *
                 * inotify事件以struct inotify_event结构体的形式返回，
                 * 可能包含多个事件，需要逐个解析。
                 */
                char inotify_buffer[4096];
                ssize_t length = read(inotify_fd, inotify_buffer, sizeof(inotify_buffer));

                if (length == -1)
                {
                    if (errno != EAGAIN)
                        std::cerr << "读取inotify事件失败: " << strerror(errno) << std::endl;
                    continue;
                }

                /**
                 * 解析inotify事件
                 *
                 * inotify可能一次返回多个事件，需要逐个处理
                 */
                int offset = 0;
                while (offset < length)
                {
                    struct inotify_event *event = (struct inotify_event *)(inotify_buffer + offset);

                    // 检查事件类型并输出相应信息
                    if (event->mask & IN_MODIFY)
                    {
                        std::cout << "检测到文件修改事件！" << std::endl;

                        /**
                         * 读取文件的最新内容
                         *
                         * lseek()将文件指针重置到文件开头：
                         * - test_file: 文件描述符
                         * - 0: 偏移量（从开头开始）
                         * - SEEK_SET: 从文件开头计算偏移量
                         */
                        lseek(test_file, 0, SEEK_SET);

                        char file_buffer[1024]; // 文件内容缓冲区
                        ssize_t file_bytes_read = read(test_file, file_buffer, sizeof(file_buffer) - 1);

                        if (file_bytes_read > 0)
                        {
                            file_buffer[file_bytes_read] = '\0'; // 确保字符串结尾
                            std::cout << "文件当前内容:\n"
                                      << file_buffer << std::endl;
                        }
                        else
                        {
                            std::cout << "无法读取文件内容或文件为空" << std::endl;
                        }
                    }

                    if (event->mask & IN_ATTRIB)
                    {
                        std::cout << "检测到文件属性变化事件！" << std::endl;
                    }

                    if (event->mask & IN_DELETE_SELF)
                    {
                        std::cout << "文件被删除！" << std::endl;
                    }

                    if (event->mask & IN_MOVE_SELF)
                    {
                        std::cout << "文件被移动！" << std::endl;
                    }

                    // 移动到下一个事件
                    offset += sizeof(struct inotify_event) + event->len;
                }
            }
        }
    }

    // ========== 程序清理阶段 ==========

    /**
     * 清理分配的资源
     *
     * 注意：在正常情况下，由于上面是无限循环，这段代码不会被执行。
     * 但为了代码的完整性和良好的编程习惯，我们还是包含了资源清理代码。
     * 实际的清理工作在退出条件中已经完成。
     */
    delete[] buffer;   // 释放动态分配的内存
    close(test_file);  // 关闭测试文件描述符
    close(inotify_fd); // 关闭inotify文件描述符
    close(epoll_fd);   // 关闭epoll文件描述符

    return 0; // 程序正常退出
}